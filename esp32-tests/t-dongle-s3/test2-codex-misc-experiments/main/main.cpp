#include <cassert>

#include "app.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_st7735.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace app {

void init_backlight()
{
    gpio_config_t config = {};
    config.pin_bit_mask = 1ULL << kPinBacklight;
    config.mode = GPIO_MODE_OUTPUT;
    ESP_ERROR_CHECK(gpio_config(&config));

    // The T-Dongle S3 backlight control is active-low.
    ESP_ERROR_CHECK(gpio_set_level(kPinBacklight, 0));
}

esp_lcd_panel_io_handle_t init_lcd()
{
    spi_bus_config_t bus_config = {};
    bus_config.sclk_io_num = kPinLcdSclk;
    bus_config.mosi_io_num = kPinLcdMosi;
    bus_config.miso_io_num = GPIO_NUM_NC;
    bus_config.quadwp_io_num = GPIO_NUM_NC;
    bus_config.quadhd_io_num = GPIO_NUM_NC;
    bus_config.max_transfer_sz = kScreenWidth * kDrawBufferLines * sizeof(lv_color16_t);
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.cs_gpio_num = kPinLcdCs;
    io_config.dc_gpio_num = kPinLcdDc;
    io_config.spi_mode = 0;
    io_config.pclk_hz = kPixelClockHz;
    io_config.trans_queue_depth = 10;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;

    esp_lcd_panel_io_handle_t io_handle = nullptr;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(static_cast<esp_lcd_spi_bus_handle_t>(SPI2_HOST),
                                             &io_config,
                                             &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = kPinLcdReset;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
    panel_config.bits_per_pixel = 16;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7735(io_handle, &panel_config, &g_app.panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(g_app.panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(g_app.panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(g_app.panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(g_app.panel, kPanelGapX, kPanelGapY));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(g_app.panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(g_app.panel, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(g_app.panel, true));

    return io_handle;
}

void init_button()
{
    gpio_config_t config = {};
    config.pin_bit_mask = 1ULL << kPinButton;
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&config));
}

void init_lvgl(esp_lcd_panel_io_handle_t io_handle)
{
    lv_init();

    g_app.display = lv_display_create(kScreenWidth, kScreenHeight);
    lv_display_set_user_data(g_app.display, g_app.panel);
    lv_display_set_color_format(g_app.display, LV_COLOR_FORMAT_RGB565);

    const size_t buffer_pixels = kScreenWidth * kDrawBufferLines;
    void *buf1 = heap_caps_malloc(buffer_pixels * sizeof(lv_color16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    void *buf2 = heap_caps_malloc(buffer_pixels * sizeof(lv_color16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf1 != nullptr);
    assert(buf2 != nullptr);

    lv_display_set_buffers(g_app.display,
                           buf1,
                           buf2,
                           buffer_pixels * sizeof(lv_color16_t),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(g_app.display, lvgl_flush_callback);

    esp_lcd_panel_io_callbacks_t callbacks = {};
    callbacks.on_color_trans_done = lcd_flush_ready_callback;
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &callbacks, g_app.display));

    esp_timer_create_args_t tick_args = {};
    tick_args.callback = lvgl_tick_callback;
    tick_args.name = "lvgl_tick";
    esp_timer_handle_t tick_timer = nullptr;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, kLvglTickPeriodMs * 1000ULL));
}

void lvgl_task(void *arg)
{
    (void) arg;

    while (true) {
        _lock_acquire(&g_app.lvgl_lock);
        update_ui_locked(false);
        uint32_t delay_ms = lv_timer_handler();
        _lock_release(&g_app.lvgl_lock);

        if (delay_ms < kLvglTaskMinDelayMs) {
            delay_ms = kLvglTaskMinDelayMs;
        } else if (delay_ms > kLvglTaskMaxDelayMs) {
            delay_ms = kLvglTaskMaxDelayMs;
        }

        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void button_task(void *arg)
{
    (void) arg;

    bool last_sample = true;
    bool stable_level = true;
    int64_t changed_at_us = esp_timer_get_time();

    while (true) {
        const bool level = gpio_get_level(kPinButton) != 0;
        const int64_t now_us = esp_timer_get_time();

        if (level != last_sample) {
            last_sample = level;
            changed_at_us = now_us;
        }

        if (level != stable_level && (now_us - changed_at_us) >= (kButtonDebounceMs * 1000LL)) {
            stable_level = level;

            if (!stable_level) {
                const DisplayMode mode = g_app.mode.load(std::memory_order_relaxed);
                const DisplayMode next = next_mode(mode);
                g_app.mode.store(next, std::memory_order_relaxed);
                ESP_LOGI(kTag, "Switched to %s view", mode_name(next));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(kButtonPollMs));
    }
}

}  // namespace app

extern "C" void app_main(void)
{
    ESP_LOGI(app::kTag, "Initialize NVS");
    app::init_nvs();

    ESP_LOGI(app::kTag, "Initialize button");
    app::init_button();

    ESP_LOGI(app::kTag, "Initialize display backlight");
    app::init_backlight();

    ESP_LOGI(app::kTag, "Initialize LCD");
    esp_lcd_panel_io_handle_t io_handle = app::init_lcd();

    ESP_LOGI(app::kTag, "Initialize LVGL");
    app::init_lvgl(io_handle);

    ESP_LOGI(app::kTag, "Initialize Wi-Fi scanner");
    app::init_wifi_scanner();

    _lock_acquire(&app::g_app.lvgl_lock);
    app::build_ui();
    _lock_release(&app::g_app.lvgl_lock);

    ESP_LOGI(app::kTag, "Start UI tasks");
    xTaskCreate(app::lvgl_task, "lvgl", 8192, nullptr, 4, nullptr);
    xTaskCreate(app::button_task, "button", 3072, nullptr, 3, nullptr);
    xTaskCreate(app::wifi_scan_task, "wifi_scan", app::kWifiScanTaskStackSize, nullptr, app::kWifiScanTaskPriority, nullptr);
}

#include <stdint.h>
#include <stddef.h>
#include <sys/lock.h>
#include <sys/param.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_lcd_io_i2c.h"
#include "esp_lcd_io_i80.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_st7796.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "class/hid/hid_device.h"

namespace {

constexpr char kTag[] = "wt32_ui";
constexpr int kHorRes = 480;
constexpr int kVerRes = 320;
constexpr int kTouchRawXMax = 320;
constexpr int kTouchRawYMax = 480;
constexpr int kDrawBufferLines = 40;
constexpr int kLvglTickPeriodMs = 2;
constexpr int kLvglTaskStackSize = 8192;
constexpr UBaseType_t kLvglTaskPriority = 4;
constexpr int kUsbHidTaskStackSize = 4096;
constexpr UBaseType_t kUsbHidTaskPriority = 3;
constexpr int kPixelClockHz = 20 * 1000 * 1000;
constexpr int kUsbHidQueueLength = 16;
constexpr int kUsbHidReleaseDelayMs = 15;
constexpr int kUsbHidRetryDelayMs = 2;
constexpr bool kDisplaySwapXY = true;
constexpr bool kDisplayMirrorX = true;
constexpr bool kDisplayMirrorY = true;
constexpr bool kTouchSwapXY = true;
constexpr bool kTouchMirrorX = false;
constexpr bool kTouchMirrorY = true;
constexpr int kButtonCols = 6;
constexpr int kButtonRows = 4;
constexpr int kButtonCount = kButtonCols * kButtonRows;
constexpr size_t kUsbStringDescriptorCount = 5;
constexpr size_t kHidKeycodeArraySize = 6;
constexpr uint8_t kHidKeyboardModifierNone = 0;
constexpr uint8_t kHidInterfaceStringIndex = 4;
constexpr uint8_t kHidInEndpointAddress = 0x81;
constexpr uint8_t kHidEndpointSize = 16;
constexpr uint8_t kHidPollIntervalMs = 10;

constexpr gpio_num_t kPinBacklight = GPIO_NUM_45;
constexpr gpio_num_t kPinReset = GPIO_NUM_4;
constexpr gpio_num_t kPinDc = GPIO_NUM_0;
constexpr gpio_num_t kPinWr = GPIO_NUM_47;
constexpr gpio_num_t kPinTouchInt = GPIO_NUM_7;
constexpr gpio_num_t kPinTouchSda = GPIO_NUM_6;
constexpr gpio_num_t kPinTouchScl = GPIO_NUM_5;

constexpr int kDataPins[] = {
    9, 46, 3, 8, 18, 17, 16, 15,
};

enum : uint8_t {
    kUsbHidInterfaceCount = 1,
    kUsbHidConfigurationValue = 1,
    kUsbHidConfigurationStringIndex = 0,
    kUsbHidInterfaceNumber = 0,
};

constexpr int kUsbDescriptorTotalLength = TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN;

const uint8_t kHidReportDescriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD)),
};

const char kUsbLanguageIdDescriptor[] = {0x09, 0x04};

const char *kUsbStringDescriptor[kUsbStringDescriptorCount] = {
    kUsbLanguageIdDescriptor,
    "Matsi",
    "WT32-SC01-PLUS HID",
    "WT32SC01PLUS",
    "Touch Keyboard",
};

const uint8_t kHidConfigurationDescriptor[] = {
    TUD_CONFIG_DESCRIPTOR(kUsbHidConfigurationValue,
                          kUsbHidInterfaceCount,
                          kUsbHidConfigurationStringIndex,
                          kUsbDescriptorTotalLength,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
                          100),
    TUD_HID_DESCRIPTOR(kUsbHidInterfaceNumber,
                       kHidInterfaceStringIndex,
                       false,
                       sizeof(kHidReportDescriptor),
                       kHidInEndpointAddress,
                       kHidEndpointSize,
                       kHidPollIntervalMs),
};

constexpr lv_style_selector_t style_selector(lv_part_t part, lv_state_t state)
{
    return static_cast<lv_style_selector_t>(part) | static_cast<lv_style_selector_t>(state);
}

_lock_t s_lvgl_lock;
lv_display_t *s_display = nullptr;
esp_lcd_panel_handle_t s_panel = nullptr;
esp_lcd_touch_handle_t s_touch = nullptr;
QueueHandle_t s_usb_key_queue = nullptr;

bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t *, void *user_ctx)
{
    lv_display_flush_ready(static_cast<lv_display_t *>(user_ctx));
    return false;
}

void lvgl_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *color_map)
{
    const int offset_x1 = area->x1;
    const int offset_x2 = area->x2;
    const int offset_y1 = area->y1;
    const int offset_y2 = area->y2;

    lv_draw_sw_rgb565_swap(color_map, (offset_x2 + 1 - offset_x1) * (offset_y2 + 1 - offset_y1));
    esp_lcd_panel_handle_t panel = static_cast<esp_lcd_panel_handle_t>(lv_display_get_user_data(display));
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, offset_x1, offset_y1, offset_x2 + 1, offset_y2 + 1, color_map));
}

void lvgl_touch_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    esp_lcd_touch_point_data_t touch_points[1] = {};
    uint8_t touch_cnt = 0;

    esp_lcd_touch_handle_t touch = static_cast<esp_lcd_touch_handle_t>(lv_indev_get_user_data(indev));
    ESP_ERROR_CHECK(esp_lcd_touch_read_data(touch));
    ESP_ERROR_CHECK(esp_lcd_touch_get_data(touch, touch_points, &touch_cnt, 1));

    if (touch_cnt > 0) {
        data->point.x = touch_points[0].x;
        data->point.y = touch_points[0].y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void lvgl_tick_cb(void *)
{
    lv_tick_inc(kLvglTickPeriodMs);
}

uint8_t map_label_to_hid_keycode(const char *label)
{
    if (label == nullptr || label[0] == '\0' || label[1] != '\0') {
        return 0;
    }

    const char ch = label[0];
    if (ch >= 'A' && ch <= 'Z') {
        return static_cast<uint8_t>(HID_KEY_A + (ch - 'A'));
    }
    if (ch >= 'a' && ch <= 'z') {
        return static_cast<uint8_t>(HID_KEY_A + (ch - 'a'));
    }
    if (ch >= '1' && ch <= '9') {
        return static_cast<uint8_t>(HID_KEY_1 + (ch - '1'));
    }
    if (ch == '0') {
        return HID_KEY_0;
    }

    return 0;
}

void queue_usb_keypress(const char *label)
{
    const uint8_t keycode = map_label_to_hid_keycode(label);
    if (keycode == 0) {
        ESP_LOGW(kTag, "No HID mapping for label '%s'", label);
        return;
    }

    if (s_usb_key_queue == nullptr) {
        ESP_LOGW(kTag, "USB HID queue is not initialized yet");
        return;
    }

    if (xQueueSend(s_usb_key_queue, &keycode, 0) != pdPASS) {
        ESP_LOGW(kTag, "USB HID queue is full, dropped key '%s'", label);
    }
}

void button_event_cb(lv_event_t *event)
{
    const char *label = static_cast<const char *>(lv_event_get_user_data(event));
    ESP_LOGI(kTag, "%s", label);
    queue_usb_keypress(label);
}

lv_obj_t *create_action_button(lv_obj_t *parent, const char *text)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_add_event_cb(button, button_event_cb, LV_EVENT_PRESSED, const_cast<char *>(text));
    const lv_style_selector_t default_sel = style_selector(LV_PART_MAIN, LV_STATE_DEFAULT);
    const lv_style_selector_t pressed_sel = style_selector(LV_PART_MAIN, LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 10, default_sel);
    lv_obj_set_style_radius(button, 10, pressed_sel);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, default_sel);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, pressed_sel);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x174F86), default_sel);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xC62828), pressed_sel);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(0x174F86), default_sel);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(0xC62828), pressed_sel);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_NONE, default_sel);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_NONE, pressed_sel);
    lv_obj_set_style_shadow_width(button, 0, default_sel);
    lv_obj_set_style_shadow_width(button, 0, pressed_sel);
    lv_obj_set_style_border_width(button, 0, default_sel);
    lv_obj_set_style_border_width(button, 0, pressed_sel);
    lv_obj_set_style_outline_width(button, 0, default_sel);
    lv_obj_set_style_outline_width(button, 0, pressed_sel);
    lv_obj_set_style_pad_all(button, 0, default_sel);
    lv_obj_set_style_pad_all(button, 0, pressed_sel);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

void build_ui()
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0D1B2A), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "WT32-SC01-PLUS");
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xE0E1DD), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *grid = lv_obj_create(screen);
    lv_obj_set_size(grid, LV_PCT(100), 262);
    lv_obj_align(grid, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_left(grid, 12, 0);
    lv_obj_set_style_pad_right(grid, 12, 0);
    lv_obj_set_style_pad_top(grid, 0, 0);
    lv_obj_set_style_pad_bottom(grid, 0, 0);
    lv_obj_set_style_pad_row(grid, 8, 0);
    lv_obj_set_style_pad_column(grid, 8, 0);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);

    static int32_t col_desc[kButtonCols + 1] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };
    static int32_t row_desc[kButtonRows + 1] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(grid, col_desc, row_desc);

    static const char *kButtonNames[kButtonCount] = {
        "A", "B", "C", "D", "E", "F",
        "G", "H", "I", "J", "K", "L",
        "M", "N", "O", "P", "Q", "R",
        "S", "T", "U", "V", "W", "X",
    };

    for (int index = 0; index < kButtonCount; ++index) {
        lv_obj_t *button = create_action_button(grid, kButtonNames[index]);
        lv_obj_set_grid_cell(button,
                             LV_GRID_ALIGN_STRETCH,
                             index % kButtonCols,
                             1,
                             LV_GRID_ALIGN_STRETCH,
                             index / kButtonCols,
                             1);
    }
}

void lvgl_task(void *)
{
    while (true) {
        _lock_acquire(&s_lvgl_lock);
        uint32_t delay_ms = lv_timer_handler();
        _lock_release(&s_lvgl_lock);

        delay_ms = MAX(delay_ms, 5U);
        delay_ms = MIN(delay_ms, 50U);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void usb_hid_task(void *)
{
    while (true) {
        uint8_t keycode = 0;
        if (xQueueReceive(s_usb_key_queue, &keycode, portMAX_DELAY) != pdPASS) {
            continue;
        }

        auto send_keyboard_report = [](const uint8_t *keycodes) -> bool {
            while (tud_mounted()) {
                if (tud_hid_ready() &&
                    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, kHidKeyboardModifierNone, keycodes)) {
                    return true;
                }
                vTaskDelay(pdMS_TO_TICKS(kUsbHidRetryDelayMs));
            }
            return false;
        };

        if (!tud_mounted()) {
            ESP_LOGW(kTag, "USB host is not mounted, dropped keycode 0x%02X", keycode);
            continue;
        }

        // Clear any prior state before issuing a new press. This prevents same-key
        // repeats from inheriting a previously missed release.
        if (!send_keyboard_report(nullptr)) {
            ESP_LOGW(kTag, "USB host disconnected before idle report for keycode 0x%02X", keycode);
            continue;
        }

        uint8_t pressed_keys[kHidKeycodeArraySize] = {};
        pressed_keys[0] = keycode;
        if (!send_keyboard_report(pressed_keys)) {
            ESP_LOGW(kTag, "USB host disconnected before press report for keycode 0x%02X", keycode);
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(kUsbHidReleaseDelayMs));

        if (!send_keyboard_report(nullptr)) {
            ESP_LOGW(kTag, "USB host disconnected before release report for keycode 0x%02X", keycode);
        }
    }
}

void init_backlight()
{
    const gpio_config_t config = {
        .pin_bit_mask = 1ULL << kPinBacklight,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(gpio_set_level(kPinBacklight, 1));
}

esp_lcd_panel_io_handle_t init_lcd_io()
{
    esp_lcd_i80_bus_handle_t i80_bus = nullptr;
    esp_lcd_i80_bus_config_t bus_config = {};
    bus_config.dc_gpio_num = kPinDc;
    bus_config.wr_gpio_num = kPinWr;
    bus_config.clk_src = LCD_CLK_SRC_DEFAULT;
    for (size_t i = 0; i < sizeof(kDataPins) / sizeof(kDataPins[0]); ++i) {
        bus_config.data_gpio_nums[i] = kDataPins[i];
    }
    bus_config.bus_width = 8;
    bus_config.max_transfer_bytes = kHorRes * kDrawBufferLines * sizeof(uint16_t);
    bus_config.dma_burst_size = 64;
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_handle_t io_handle = nullptr;
    esp_lcd_panel_io_i80_config_t io_config = {};
    io_config.cs_gpio_num = -1;
    io_config.pclk_hz = kPixelClockHz;
    io_config.trans_queue_depth = 10;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    io_config.dc_levels.dc_idle_level = 0;
    io_config.dc_levels.dc_cmd_level = 0;
    io_config.dc_levels.dc_dummy_level = 0;
    io_config.dc_levels.dc_data_level = 1;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
    return io_handle;
}

void init_panel(esp_lcd_panel_io_handle_t io_handle)
{
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = kPinReset;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
    panel_config.bits_per_pixel = 16;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(s_panel, kDisplaySwapXY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel, kDisplayMirrorX, kDisplayMirrorY));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel, true));
}

void init_touch()
{
    i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_NUM_1;
    bus_config.sda_io_num = kPinTouchSda;
    bus_config.scl_io_num = kPinTouchScl;
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;
    i2c_master_bus_handle_t i2c_bus = nullptr;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    esp_lcd_panel_io_i2c_config_t io_config = {};
    io_config.dev_addr = ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS;
    io_config.control_phase_bytes = 1;
    io_config.dc_bit_offset = 0;
    io_config.lcd_cmd_bits = 8;
    io_config.flags.disable_control_phase = 1;
    io_config.scl_speed_hz = 400000;

    esp_lcd_panel_io_handle_t tp_io = nullptr;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &tp_io));

    esp_lcd_touch_config_t touch_config = {};
    // The FT5x06 reports native portrait coordinates; transform flags are applied after this range.
    touch_config.x_max = kTouchRawXMax;
    touch_config.y_max = kTouchRawYMax;
    touch_config.rst_gpio_num = GPIO_NUM_NC;
    touch_config.int_gpio_num = kPinTouchInt;
    touch_config.levels.reset = 0;
    touch_config.levels.interrupt = 0;
    // Keep the LCD orientation unchanged and map FT5x06 coordinates to that view.
    touch_config.flags.swap_xy = kTouchSwapXY;
    touch_config.flags.mirror_x = kTouchMirrorX;
    touch_config.flags.mirror_y = kTouchMirrorY;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io, &touch_config, &s_touch));
}

void init_lvgl(esp_lcd_panel_io_handle_t io_handle)
{
    lv_init();

    s_display = lv_display_create(kHorRes, kVerRes);
    lv_display_set_user_data(s_display, s_panel);
    lv_display_set_color_format(s_display, LV_COLOR_FORMAT_RGB565);

    const size_t draw_buffer_size = kHorRes * kDrawBufferLines * sizeof(lv_color16_t);
    void *buf1 = heap_caps_malloc(draw_buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    void *buf2 = heap_caps_malloc(draw_buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf1 != nullptr);
    assert(buf2 != nullptr);

    lv_display_set_buffers(s_display, buf1, buf2, draw_buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_display, lvgl_flush_cb);

    const esp_lcd_panel_io_callbacks_t io_callbacks = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &io_callbacks, s_display));

    lv_indev_t *touch_input = lv_indev_create();
    lv_indev_set_type(touch_input, LV_INDEV_TYPE_POINTER);
    lv_indev_set_user_data(touch_input, s_touch);
    lv_indev_set_read_cb(touch_input, lvgl_touch_cb);

    const esp_timer_create_args_t timer_args = {
        .callback = lvgl_tick_cb,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick",
        .skip_unhandled_events = false,
    };
    esp_timer_handle_t tick_timer = nullptr;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, kLvglTickPeriodMs * 1000));
}

void init_usb_hid()
{
    s_usb_key_queue = xQueueCreate(kUsbHidQueueLength, sizeof(uint8_t));
    assert(s_usb_key_queue != nullptr);

    tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();
    tusb_cfg.descriptor.device = nullptr;
    tusb_cfg.descriptor.full_speed_config = kHidConfigurationDescriptor;
    tusb_cfg.descriptor.string = kUsbStringDescriptor;
    tusb_cfg.descriptor.string_count = kUsbStringDescriptorCount;
#if (TUD_OPT_HIGH_SPEED)
    tusb_cfg.descriptor.high_speed_config = kHidConfigurationDescriptor;
#endif

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    xTaskCreate(usb_hid_task, "usb_hid", kUsbHidTaskStackSize, nullptr, kUsbHidTaskPriority, nullptr);
}

} // namespace

extern "C" uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return kHidReportDescriptor;
}

extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance,
                                           uint8_t report_id,
                                           hid_report_type_t report_type,
                                           uint8_t *buffer,
                                           uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

extern "C" void tud_hid_set_report_cb(uint8_t instance,
                                       uint8_t report_id,
                                       hid_report_type_t report_type,
                                       uint8_t const *buffer,
                                       uint16_t bufsize)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

extern "C" void app_main(void)
{
    ESP_LOGI(kTag, "Initialize backlight");
    init_backlight();

    ESP_LOGI(kTag, "Initialize LCD");
    esp_lcd_panel_io_handle_t io_handle = init_lcd_io();
    init_panel(io_handle);

    ESP_LOGI(kTag, "Initialize touch");
    init_touch();

    ESP_LOGI(kTag, "Initialize LVGL");
    init_lvgl(io_handle);

    ESP_LOGI(kTag, "Initialize USB HID");
    init_usb_hid();

    _lock_acquire(&s_lvgl_lock);
    build_ui();
    _lock_release(&s_lvgl_lock);

    ESP_LOGI(kTag, "Start LVGL task");
    xTaskCreate(lvgl_task, "lvgl", kLvglTaskStackSize, nullptr, kLvglTaskPriority, nullptr);
}

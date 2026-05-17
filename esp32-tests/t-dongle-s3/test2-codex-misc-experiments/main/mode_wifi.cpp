#include "app.h"

#include <cassert>
#include <cstdio>
#include <cstring>

#include "esp_check.h"
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

namespace app {

namespace {

lv_color16_t color16_from_color(lv_color_t color)
{
    lv_color16_t out = {};
    const uint16_t raw = lv_color_to_u16(color);
    std::memcpy(&out, &raw, sizeof(raw));
    return out;
}

lv_color_t wifi_activity_color(uint8_t intensity)
{
    if (intensity < 32) {
        return lv_color_hex(0x020610);
    }
    if (intensity < 96) {
        const uint8_t blue = static_cast<uint8_t>(40 + ((intensity - 32) * 2));
        const uint8_t green = static_cast<uint8_t>((intensity - 32) / 2);
        return lv_color_make(0, green, blue);
    }
    if (intensity < 176) {
        const uint8_t green = static_cast<uint8_t>(80 + ((intensity - 96) * 2));
        const uint8_t blue = static_cast<uint8_t>(168 - ((intensity - 96) * 2));
        return lv_color_make(0, green, blue);
    }

    const uint8_t red = static_cast<uint8_t>((intensity - 176) * 3);
    const uint8_t green = 255;
    const uint8_t blue = static_cast<uint8_t>(40 > (intensity - 176) ? (40 - (intensity - 176)) : 0);
    return lv_color_make(red, green, blue);
}

}  // namespace

void init_wifi_ui(lv_obj_t *screen)
{
    g_app.wifi_canvas = lv_canvas_create(screen);
    lv_obj_remove_style_all(g_app.wifi_canvas);
    lv_obj_set_size(g_app.wifi_canvas, kScreenWidth, kScreenHeight);
    lv_obj_set_pos(g_app.wifi_canvas, 0, 0);
    lv_obj_add_flag(g_app.wifi_canvas, LV_OBJ_FLAG_HIDDEN);
    g_app.wifi_canvas_buf = heap_caps_malloc(kScreenWidth * kScreenHeight * sizeof(lv_color16_t),
                                             MALLOC_CAP_INTERNAL);
    assert(g_app.wifi_canvas_buf != nullptr);
    lv_canvas_set_buffer(g_app.wifi_canvas,
                         g_app.wifi_canvas_buf,
                         kScreenWidth,
                         kScreenHeight,
                         LV_COLOR_FORMAT_RGB565);
}

uint8_t normalize_wifi_rssi(int rssi)
{
    if (rssi <= -95) {
        return 0;
    }
    if (rssi >= -35) {
        return 255;
    }
    return static_cast<uint8_t>(((rssi + 95) * 255) / 60);
}

void reset_wifi_waterfall_locked()
{
    lv_canvas_fill_bg(g_app.wifi_canvas, lv_color_black(), LV_OPA_COVER);
}

void push_wifi_waterfall_row_locked(const uint8_t channel_levels[kWifiChannels], uint16_t ap_count)
{
    auto *buffer = static_cast<lv_color16_t *>(g_app.wifi_canvas_buf);
    std::memmove(buffer + kScreenWidth,
                 buffer,
                 static_cast<size_t>(kScreenWidth) * (kScreenHeight - 1) * sizeof(lv_color16_t));

    const lv_color16_t separator = color16_from_color(lv_color_hex(0x081018));
    for (int ch = 0; ch < kWifiChannels; ++ch) {
        const int x0 = (ch * kScreenWidth) / kWifiChannels;
        const int x1 = ((ch + 1) * kScreenWidth) / kWifiChannels;
        const lv_color16_t color = color16_from_color(wifi_activity_color(channel_levels[ch]));
        for (int x = x0; x < x1; ++x) {
            buffer[x] = color;
        }
        if (ch > 0 && x0 < kScreenWidth) {
            buffer[x0] = separator;
        }
    }

    lv_obj_invalidate(g_app.wifi_canvas);

    char status[24] = {};
    std::snprintf(status, sizeof(status), "%u APs seen", static_cast<unsigned>(ap_count));
    lv_label_set_text(g_app.hint_label, status);
}

void init_nvs()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void init_wifi_scanner()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif != nullptr);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_start());

    g_app.wifi_ready = true;
}

void wifi_scan_task(void *arg)
{
    (void) arg;

    wifi_ap_record_t records[kWifiScanMaxRecords] = {};

    while (true) {
        if (!g_app.wifi_ready || g_app.mode.load(std::memory_order_relaxed) != DisplayMode::WifiWaterfall) {
            vTaskDelay(pdMS_TO_TICKS(kWifiScanIdleMs));
            continue;
        }

        wifi_scan_config_t scan_config = {};
        scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
        scan_config.show_hidden = false;
        scan_config.scan_time.active.min = 20;
        scan_config.scan_time.active.max = 30;

        const esp_err_t scan_err = esp_wifi_scan_start(&scan_config, true);
        if (scan_err != ESP_OK) {
            ESP_LOGW(kTag, "Wi-Fi scan failed: %s", esp_err_to_name(scan_err));
            vTaskDelay(pdMS_TO_TICKS(250));
            continue;
        }

        uint16_t ap_total = 0;
        uint16_t ap_count = kWifiScanMaxRecords;
        std::memset(records, 0, sizeof(records));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_total));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, records));

        uint8_t levels[kWifiChannels] = {};
        for (uint16_t i = 0; i < ap_count; ++i) {
            const uint8_t channel = records[i].primary;
            if (channel < 1 || channel > kWifiChannels) {
                continue;
            }

            const uint8_t strength = normalize_wifi_rssi(records[i].rssi);
            uint8_t &slot = levels[channel - 1];
            int boosted = static_cast<int>(slot) + (strength / 3) + 12;
            if (strength > slot) {
                slot = strength;
            }
            if (boosted > slot) {
                slot = static_cast<uint8_t>(boosted > 255 ? 255 : boosted);
            }
        }

        if (g_app.mode.load(std::memory_order_relaxed) == DisplayMode::WifiWaterfall) {
            _lock_acquire(&g_app.lvgl_lock);
            if (g_app.mode.load(std::memory_order_relaxed) == DisplayMode::WifiWaterfall) {
                push_wifi_waterfall_row_locked(levels, ap_total);
            }
            _lock_release(&g_app.lvgl_lock);
        }

        vTaskDelay(pdMS_TO_TICKS(60));
    }
}

}  // namespace app

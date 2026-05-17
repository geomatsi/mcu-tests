#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <ctime>

#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "lvgl.h"
#include "sys/lock.h"

namespace app {

inline constexpr char kTag[] = "t_dongle_clock";

inline constexpr int kScreenWidth = 160;
inline constexpr int kScreenHeight = 80;
inline constexpr int kDrawBufferLines = 20;
inline constexpr int kPixelClockHz = 27 * 1000 * 1000;
inline constexpr int kLvglTickPeriodMs = 2;
inline constexpr int kButtonPollMs = 10;
inline constexpr int kButtonDebounceMs = 30;
inline constexpr int64_t kMatrixStepUs = 70 * 1000;
inline constexpr int kLvglTaskMinDelayMs = 1;
inline constexpr int kLvglTaskMaxDelayMs = 50;

inline constexpr gpio_num_t kPinButton = GPIO_NUM_0;
inline constexpr gpio_num_t kPinBacklight = GPIO_NUM_38;
inline constexpr gpio_num_t kPinLcdMosi = GPIO_NUM_3;
inline constexpr gpio_num_t kPinLcdSclk = GPIO_NUM_5;
inline constexpr gpio_num_t kPinLcdCs = GPIO_NUM_4;
inline constexpr gpio_num_t kPinLcdDc = GPIO_NUM_2;
inline constexpr gpio_num_t kPinLcdReset = GPIO_NUM_1;

inline constexpr int kPanelGapX = 1;
inline constexpr int kPanelGapY = 26;

inline constexpr int kMatrixCellSize = 4;
inline constexpr int kMatrixCols = kScreenWidth / kMatrixCellSize;
inline constexpr int kMatrixRows = kScreenHeight / kMatrixCellSize;
inline constexpr int kMatrixMaxTrail = 6;

inline constexpr int kMandelbrotMaxIter = 48;
inline constexpr int kMandelbrotRowsPerStep = 4;

inline constexpr int64_t kScopeStepUs = 2 * 1000;
inline constexpr int kScopeSamplesPerStep = 12;

inline constexpr int kWifiChannels = 13;
inline constexpr int kWifiScanTaskStackSize = 6144;
inline constexpr int kWifiScanTaskPriority = 2;
inline constexpr int kWifiScanIdleMs = 120;
inline constexpr uint16_t kWifiScanMaxRecords = 32;

enum class DisplayMode : uint8_t {
    Time,
    Date,
    Matrix,
    Mandelbrot,
    Oscilloscope,
    WifiWaterfall,
};

struct MatrixColumnState {
    int head_row = 0;
    int speed_divider = 1;
    int speed_phase = 0;
    int trail_length = 3;
};

struct AppState {
    _lock_t lvgl_lock = {};
    lv_display_t *display = nullptr;
    esp_lcd_panel_handle_t panel = nullptr;

    lv_obj_t *mode_label = nullptr;
    lv_obj_t *clock_label = nullptr;
    lv_obj_t *hint_label = nullptr;

    lv_obj_t *matrix_layer = nullptr;
    lv_obj_t *matrix_pixels[kMatrixCols][kMatrixMaxTrail] = {};

    lv_obj_t *mandelbrot_canvas = nullptr;
    void *mandelbrot_canvas_buf = nullptr;

    lv_obj_t *scope_canvas = nullptr;
    void *scope_canvas_buf = nullptr;

    lv_obj_t *wifi_canvas = nullptr;
    void *wifi_canvas_buf = nullptr;

    MatrixColumnState matrix_columns[kMatrixCols] = {};
    int16_t scope_main_trace[kScreenWidth] = {};
    int16_t scope_echo_trace[kScreenWidth] = {};

    std::atomic<DisplayMode> mode{DisplayMode::Time};
    int64_t last_rendered_second = -1;
    DisplayMode last_text_mode = DisplayMode::Time;
    DisplayMode last_visible_mode = DisplayMode::Time;

    int64_t last_matrix_step_us = 0;

    bool mandelbrot_rendered = false;
    int mandelbrot_next_row = 0;

    int64_t last_scope_step_us = 0;
    float scope_phase = 0.0f;
    float scope_mod_phase = 0.0f;
    float scope_echo_value = 0.0f;

    bool wifi_ready = false;
};

extern AppState g_app;

bool lcd_flush_ready_callback(esp_lcd_panel_io_handle_t panel_io,
                              esp_lcd_panel_io_event_data_t *event_data,
                              void *user_ctx);
void lvgl_flush_callback(lv_display_t *display, const lv_area_t *area, uint8_t *color_map);
void lvgl_tick_callback(void *arg);

DisplayMode next_mode(DisplayMode mode);
const char *mode_name(DisplayMode mode);
void format_clock_text(time_t epoch_seconds, bool show_date, char *buffer, size_t buffer_size);
void set_widget_hidden(lv_obj_t *widget, bool hidden);
void set_clock_widgets_hidden(bool hidden);
void update_text_ui_locked(DisplayMode mode, bool force);
void update_ui_locked(bool force);
void build_ui();

void init_matrix_ui(lv_obj_t *screen);
void reset_matrix_scene_locked();
void update_matrix_ui_locked(bool force);

void init_mandelbrot_ui(lv_obj_t *screen);
void render_mandelbrot_locked(bool force);

void init_scope_ui(lv_obj_t *screen);
void reset_scope_locked();
void render_scope_locked(bool force);

void init_wifi_ui(lv_obj_t *screen);
void reset_wifi_waterfall_locked();
void push_wifi_waterfall_row_locked(const uint8_t channel_levels[kWifiChannels], uint16_t ap_count);
uint8_t normalize_wifi_rssi(int rssi);
void init_nvs();
void init_wifi_scanner();
void wifi_scan_task(void *arg);

}  // namespace app

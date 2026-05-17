#include "app.h"

#include <cassert>

#include "esp_heap_caps.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_timer.h"

namespace app {

AppState g_app = {};

bool lcd_flush_ready_callback(esp_lcd_panel_io_handle_t panel_io,
                              esp_lcd_panel_io_event_data_t *event_data,
                              void *user_ctx)
{
    (void) panel_io;
    (void) event_data;
    auto *display = static_cast<lv_display_t *>(user_ctx);
    lv_display_flush_ready(display);
    return false;
}

void lvgl_flush_callback(lv_display_t *display, const lv_area_t *area, uint8_t *color_map)
{
    auto *panel = static_cast<esp_lcd_panel_handle_t>(lv_display_get_user_data(display));
    lv_draw_sw_rgb565_swap(color_map, lv_area_get_width(area) * lv_area_get_height(area));
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel,
                                              area->x1,
                                              area->y1,
                                              area->x2 + 1,
                                              area->y2 + 1,
                                              color_map));
}

void lvgl_tick_callback(void *arg)
{
    (void) arg;
    lv_tick_inc(kLvglTickPeriodMs);
}

DisplayMode next_mode(DisplayMode mode)
{
    switch (mode) {
    case DisplayMode::Time:
        return DisplayMode::Date;
    case DisplayMode::Date:
        return DisplayMode::Matrix;
    case DisplayMode::Matrix:
        return DisplayMode::Mandelbrot;
    case DisplayMode::Mandelbrot:
        return DisplayMode::Oscilloscope;
    case DisplayMode::Oscilloscope:
        return DisplayMode::WifiWaterfall;
    case DisplayMode::WifiWaterfall:
        return DisplayMode::Time;
    }

    return DisplayMode::Time;
}

const char *mode_name(DisplayMode mode)
{
    switch (mode) {
    case DisplayMode::Time:
        return "time";
    case DisplayMode::Date:
        return "date";
    case DisplayMode::Matrix:
        return "matrix";
    case DisplayMode::Mandelbrot:
        return "mandelbrot";
    case DisplayMode::Oscilloscope:
        return "oscilloscope";
    case DisplayMode::WifiWaterfall:
        return "wifi";
    }

    return "time";
}

void format_clock_text(time_t epoch_seconds, bool show_date, char *buffer, size_t buffer_size)
{
    std::tm timeinfo = {};
    gmtime_r(&epoch_seconds, &timeinfo);

    if (show_date) {
        std::strftime(buffer, buffer_size, "%Y-%m-%d", &timeinfo);
    } else {
        std::strftime(buffer, buffer_size, "%H:%M:%S", &timeinfo);
    }
}

void set_widget_hidden(lv_obj_t *widget, bool hidden)
{
    if (hidden) {
        lv_obj_add_flag(widget, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(widget, LV_OBJ_FLAG_HIDDEN);
    }
}

void update_text_ui_locked(DisplayMode mode, bool force)
{
    const int64_t epoch_seconds = esp_timer_get_time() / 1000000LL;
    const bool show_date = (mode == DisplayMode::Date);

    if (!force && epoch_seconds == g_app.last_rendered_second && mode == g_app.last_text_mode) {
        return;
    }

    char buffer[32] = {};
    format_clock_text(static_cast<time_t>(epoch_seconds), show_date, buffer, sizeof(buffer));

    lv_label_set_text(g_app.mode_label, show_date ? "DATE" : "TIME");
    lv_label_set_text(g_app.clock_label, buffer);
    lv_label_set_text(g_app.hint_label, show_date ? "BOOT: MATRIX" : "BOOT: DATE");

    lv_obj_set_style_text_font(g_app.clock_label,
                               show_date ? &lv_font_montserrat_18 : &lv_font_montserrat_28,
                               0);
    lv_obj_set_style_text_color(g_app.clock_label,
                                show_date ? lv_palette_lighten(LV_PALETTE_AMBER, 1)
                                          : lv_palette_lighten(LV_PALETTE_CYAN, 1),
                                0);
    lv_obj_align(g_app.clock_label, LV_ALIGN_CENTER, 0, -2);

    g_app.last_rendered_second = epoch_seconds;
    g_app.last_text_mode = mode;
}

void set_clock_widgets_hidden(bool hidden)
{
    lv_obj_t *widgets[] = {g_app.mode_label, g_app.clock_label, g_app.hint_label};
    for (lv_obj_t *widget : widgets) {
        set_widget_hidden(widget, hidden);
    }
}

void update_ui_locked(bool force)
{
    const DisplayMode mode = g_app.mode.load(std::memory_order_relaxed);

    if (force || mode != g_app.last_visible_mode) {
        const bool text_mode = (mode == DisplayMode::Time || mode == DisplayMode::Date);
        const bool matrix_mode = (mode == DisplayMode::Matrix);
        const bool mandelbrot_mode = (mode == DisplayMode::Mandelbrot);
        const bool scope_mode = (mode == DisplayMode::Oscilloscope);
        const bool wifi_mode = (mode == DisplayMode::WifiWaterfall);

        if (text_mode) {
            set_widget_hidden(g_app.mode_label, false);
            set_widget_hidden(g_app.clock_label, false);
            set_widget_hidden(g_app.hint_label, false);
        } else if (mandelbrot_mode) {
            set_widget_hidden(g_app.mode_label, false);
            set_widget_hidden(g_app.clock_label, true);
            set_widget_hidden(g_app.hint_label, false);
            lv_label_set_text(g_app.mode_label, "MANDEL");
            lv_label_set_text(g_app.hint_label, "Rendering 0%");
        } else if (scope_mode) {
            set_widget_hidden(g_app.mode_label, false);
            set_widget_hidden(g_app.clock_label, true);
            set_widget_hidden(g_app.hint_label, false);
            lv_label_set_text(g_app.mode_label, "SCOPE");
            lv_label_set_text(g_app.hint_label, "BOOT: WIFI");
        } else if (wifi_mode) {
            set_widget_hidden(g_app.mode_label, false);
            set_widget_hidden(g_app.clock_label, true);
            set_widget_hidden(g_app.hint_label, false);
            lv_label_set_text(g_app.mode_label, "WIFI 2.4");
            lv_label_set_text(g_app.hint_label, "Scanning...");
        } else {
            set_clock_widgets_hidden(true);
        }

        if (matrix_mode) {
            lv_obj_clear_flag(g_app.matrix_layer, LV_OBJ_FLAG_HIDDEN);
            reset_matrix_scene_locked();
        } else {
            lv_obj_add_flag(g_app.matrix_layer, LV_OBJ_FLAG_HIDDEN);
        }

        if (mandelbrot_mode) {
            lv_obj_clear_flag(g_app.mandelbrot_canvas, LV_OBJ_FLAG_HIDDEN);
            render_mandelbrot_locked(true);
        } else {
            lv_obj_add_flag(g_app.mandelbrot_canvas, LV_OBJ_FLAG_HIDDEN);
            g_app.mandelbrot_next_row = 0;
            g_app.mandelbrot_rendered = false;
        }

        if (scope_mode) {
            lv_obj_clear_flag(g_app.scope_canvas, LV_OBJ_FLAG_HIDDEN);
            reset_scope_locked();
            render_scope_locked(true);
        } else {
            lv_obj_add_flag(g_app.scope_canvas, LV_OBJ_FLAG_HIDDEN);
            g_app.last_scope_step_us = 0;
        }

        if (wifi_mode) {
            lv_obj_clear_flag(g_app.wifi_canvas, LV_OBJ_FLAG_HIDDEN);
            reset_wifi_waterfall_locked();
        } else {
            lv_obj_add_flag(g_app.wifi_canvas, LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_set_style_bg_color(lv_screen_active(),
                                  text_mode ? lv_color_hex(0x050814) : lv_color_hex(0x000000),
                                  0);
        g_app.last_rendered_second = -1;
        g_app.last_visible_mode = mode;
    }

    if (mode == DisplayMode::Matrix) {
        update_matrix_ui_locked(force);
    } else if (mode == DisplayMode::Mandelbrot) {
        render_mandelbrot_locked(false);
    } else if (mode == DisplayMode::Oscilloscope) {
        render_scope_locked(false);
    } else if (mode == DisplayMode::WifiWaterfall) {
        // Updated by the Wi-Fi scan task.
    } else {
        update_text_ui_locked(mode, force);
    }
}

void build_ui()
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x050814), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);

    g_app.mode_label = lv_label_create(screen);
    lv_label_set_text(g_app.mode_label, "TIME");
    lv_obj_set_width(g_app.mode_label, kScreenWidth);
    lv_obj_set_style_text_align(g_app.mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(g_app.mode_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align(g_app.mode_label, LV_ALIGN_TOP_MID, 0, 6);

    g_app.clock_label = lv_label_create(screen);
    lv_label_set_text(g_app.clock_label, "00:00:00");
    lv_obj_set_width(g_app.clock_label, kScreenWidth);
    lv_obj_set_style_text_align(g_app.clock_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(g_app.clock_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_letter_space(g_app.clock_label, 1, 0);
    lv_obj_set_style_text_color(g_app.clock_label, lv_palette_lighten(LV_PALETTE_CYAN, 1), 0);
    lv_obj_align(g_app.clock_label, LV_ALIGN_CENTER, 0, -2);

    g_app.hint_label = lv_label_create(screen);
    lv_label_set_text(g_app.hint_label, "BOOT to show date");
    lv_obj_set_width(g_app.hint_label, kScreenWidth);
    lv_obj_set_style_text_align(g_app.hint_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(g_app.hint_label, lv_palette_darken(LV_PALETTE_BLUE_GREY, 1), 0);
    lv_obj_align(g_app.hint_label, LV_ALIGN_BOTTOM_MID, 0, -6);

    init_matrix_ui(screen);
    init_mandelbrot_ui(screen);
    init_scope_ui(screen);
    init_wifi_ui(screen);

    lv_obj_move_foreground(g_app.mode_label);
    lv_obj_move_foreground(g_app.clock_label);
    lv_obj_move_foreground(g_app.hint_label);

    update_ui_locked(true);
}

}  // namespace app

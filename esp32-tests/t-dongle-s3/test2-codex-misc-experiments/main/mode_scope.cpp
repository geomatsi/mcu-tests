#include "app.h"

#include <cassert>
#include <cmath>

#include "esp_heap_caps.h"
#include "esp_timer.h"

namespace app {

namespace {

void canvas_draw_line(lv_obj_t *canvas, int x0, int y0, int x1, int y1, lv_color_t color)
{
    int dx = std::abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        if (x0 >= 0 && x0 < kScreenWidth && y0 >= 0 && y0 < kScreenHeight) {
            lv_canvas_set_px(canvas, x0, y0, color, LV_OPA_COVER);
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int err2 = 2 * err;
        if (err2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (err2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

int clamp_y(int y)
{
    if (y < 0) {
        return 0;
    }
    if (y >= kScreenHeight) {
        return kScreenHeight - 1;
    }
    return y;
}

float clamp_scope_value(float value)
{
    if (value < 2.0f) {
        return 2.0f;
    }
    const float max_y = static_cast<float>(kScreenHeight - 3);
    if (value > max_y) {
        return max_y;
    }
    return value;
}

int next_scope_sample()
{
    const float center_y = static_cast<float>(kScreenHeight) * 0.5f;
    const float amplitude = static_cast<float>(kScreenHeight) * 0.24f;
    const float carrier = std::sin(g_app.scope_phase)
                        + 0.42f * std::sin((g_app.scope_phase * 2.31f) + 0.80f)
                        + 0.16f * std::sin((g_app.scope_phase * 4.91f) + 2.40f);
    const float envelope = 0.78f
                         + 0.18f * std::sin(g_app.scope_mod_phase)
                         + 0.08f * std::sin((g_app.scope_mod_phase * 0.37f) + 1.10f);
    const float main_value = center_y - (carrier * amplitude * envelope);

    g_app.scope_phase += 0.16f;
    g_app.scope_mod_phase += 0.011f;
    g_app.scope_echo_value = (g_app.scope_echo_value * 0.80f) + (main_value * 0.20f);
    g_app.scope_echo_value = clamp_scope_value(g_app.scope_echo_value);
    return static_cast<int>(clamp_scope_value(main_value));
}

}  // namespace

void init_scope_ui(lv_obj_t *screen)
{
    g_app.scope_canvas = lv_canvas_create(screen);
    lv_obj_remove_style_all(g_app.scope_canvas);
    lv_obj_set_size(g_app.scope_canvas, kScreenWidth, kScreenHeight);
    lv_obj_set_pos(g_app.scope_canvas, 0, 0);
    lv_obj_add_flag(g_app.scope_canvas, LV_OBJ_FLAG_HIDDEN);
    g_app.scope_canvas_buf = heap_caps_malloc(kScreenWidth * kScreenHeight * sizeof(lv_color16_t),
                                              MALLOC_CAP_INTERNAL);
    assert(g_app.scope_canvas_buf != nullptr);
    lv_canvas_set_buffer(g_app.scope_canvas,
                         g_app.scope_canvas_buf,
                         kScreenWidth,
                         kScreenHeight,
                         LV_COLOR_FORMAT_RGB565);
}

void reset_scope_locked()
{
    lv_canvas_fill_bg(g_app.scope_canvas, lv_color_black(), LV_OPA_COVER);
    g_app.last_scope_step_us = 0;
    const int center = kScreenHeight / 2;
    for (int i = 0; i < kScreenWidth; ++i) {
        g_app.scope_main_trace[i] = center;
        g_app.scope_echo_trace[i] = center;
    }
    g_app.scope_phase = 0.0f;
    g_app.scope_mod_phase = 0.0f;
    g_app.scope_echo_value = static_cast<float>(center);
}

void render_scope_locked(bool force)
{
    const int64_t now_us = esp_timer_get_time();
    if (!force && g_app.last_scope_step_us != 0 && (now_us - g_app.last_scope_step_us) < kScopeStepUs) {
        return;
    }
    g_app.last_scope_step_us = now_us;

    if (force) {
        reset_scope_locked();
    }

    for (int i = 0; i < kScreenWidth - kScopeSamplesPerStep; ++i) {
        g_app.scope_main_trace[i] = g_app.scope_main_trace[i + kScopeSamplesPerStep];
        g_app.scope_echo_trace[i] = g_app.scope_echo_trace[i + kScopeSamplesPerStep];
    }
    for (int i = kScreenWidth - kScopeSamplesPerStep; i < kScreenWidth; ++i) {
        g_app.scope_main_trace[i] = static_cast<int16_t>(next_scope_sample());
        g_app.scope_echo_trace[i] = static_cast<int16_t>(g_app.scope_echo_value);
    }

    lv_canvas_fill_bg(g_app.scope_canvas, lv_color_black(), LV_OPA_COVER);

    const lv_color_t grid_color = lv_color_hex(0x103810);
    const lv_color_t center_color = lv_color_hex(0x1F5F1F);
    const lv_color_t echo_color = lv_color_hex(0x2FAF4F);
    const lv_color_t trace_color = lv_color_hex(0x98FF98);

    for (int x = 0; x < kScreenWidth; x += 20) {
        for (int y = 0; y < kScreenHeight; ++y) {
            lv_canvas_set_px(g_app.scope_canvas, x, y, grid_color, LV_OPA_COVER);
        }
    }
    for (int y = 0; y < kScreenHeight; y += 16) {
        const lv_color_t color = (y == (kScreenHeight / 2)) ? center_color : grid_color;
        for (int x = 0; x < kScreenWidth; ++x) {
            lv_canvas_set_px(g_app.scope_canvas, x, y, color, LV_OPA_COVER);
        }
    }

    for (int x = 1; x < kScreenWidth; ++x) {
        canvas_draw_line(g_app.scope_canvas,
                         x - 1,
                         clamp_y(g_app.scope_echo_trace[x - 1]),
                         x,
                         clamp_y(g_app.scope_echo_trace[x]),
                         echo_color);
        canvas_draw_line(g_app.scope_canvas,
                         x - 1,
                         clamp_y(g_app.scope_main_trace[x - 1]),
                         x,
                         clamp_y(g_app.scope_main_trace[x]),
                         trace_color);
    }
}

}  // namespace app

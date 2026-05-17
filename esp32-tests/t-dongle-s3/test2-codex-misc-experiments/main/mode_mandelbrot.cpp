#include "app.h"

#include <cassert>
#include <cstdio>

#include "esp_heap_caps.h"

namespace app {

namespace {

lv_color_t mandelbrot_color(int iter)
{
    if (iter >= kMandelbrotMaxIter) {
        return lv_color_hex(0x000000);
    }

    const uint8_t shade = static_cast<uint8_t>((iter * 255) / kMandelbrotMaxIter);
    const uint8_t red = static_cast<uint8_t>(shade / 5);
    const uint8_t green = static_cast<uint8_t>(40 + (shade * 3) / 5);
    const uint8_t blue = static_cast<uint8_t>(80 + shade / 2);
    return lv_color_make(red, green, blue);
}

void reset_mandelbrot_locked()
{
    lv_canvas_fill_bg(g_app.mandelbrot_canvas, lv_color_black(), LV_OPA_COVER);
    g_app.mandelbrot_next_row = 0;
    g_app.mandelbrot_rendered = false;
}

}  // namespace

void init_mandelbrot_ui(lv_obj_t *screen)
{
    g_app.mandelbrot_canvas = lv_canvas_create(screen);
    lv_obj_remove_style_all(g_app.mandelbrot_canvas);
    lv_obj_set_size(g_app.mandelbrot_canvas, kScreenWidth, kScreenHeight);
    lv_obj_set_pos(g_app.mandelbrot_canvas, 0, 0);
    lv_obj_add_flag(g_app.mandelbrot_canvas, LV_OBJ_FLAG_HIDDEN);
    g_app.mandelbrot_canvas_buf = heap_caps_malloc(kScreenWidth * kScreenHeight * sizeof(lv_color16_t),
                                                   MALLOC_CAP_INTERNAL);
    assert(g_app.mandelbrot_canvas_buf != nullptr);
    lv_canvas_set_buffer(g_app.mandelbrot_canvas,
                         g_app.mandelbrot_canvas_buf,
                         kScreenWidth,
                         kScreenHeight,
                         LV_COLOR_FORMAT_RGB565);
}

void render_mandelbrot_locked(bool force)
{
    if (force) {
        reset_mandelbrot_locked();
    }

    if (g_app.mandelbrot_rendered) {
        lv_label_set_text(g_app.hint_label, "BOOT: SCOPE");
        return;
    }

    if (g_app.mandelbrot_next_row == 0) {
        lv_label_set_text(g_app.mode_label, "MANDEL");
    }

    constexpr double x_min = -2.2;
    constexpr double x_max = 0.8;
    constexpr double y_min = -1.05;
    constexpr double y_max = 1.05;

    const int row_end = (g_app.mandelbrot_next_row + kMandelbrotRowsPerStep < kScreenHeight)
                            ? (g_app.mandelbrot_next_row + kMandelbrotRowsPerStep)
                            : kScreenHeight;

    for (int py = g_app.mandelbrot_next_row; py < row_end; ++py) {
        const double cy = y_min + (static_cast<double>(py) * (y_max - y_min) / (kScreenHeight - 1));
        for (int px = 0; px < kScreenWidth; ++px) {
            const double cx = x_min + (static_cast<double>(px) * (x_max - x_min) / (kScreenWidth - 1));

            double zx = 0.0;
            double zy = 0.0;
            int iter = 0;
            while ((zx * zx + zy * zy) <= 4.0 && iter < kMandelbrotMaxIter) {
                const double zx_next = zx * zx - zy * zy + cx;
                zy = (2.0 * zx * zy) + cy;
                zx = zx_next;
                ++iter;
            }

            lv_canvas_set_px(g_app.mandelbrot_canvas, px, py, mandelbrot_color(iter), LV_OPA_COVER);
        }
    }

    g_app.mandelbrot_next_row = row_end;
    if (g_app.mandelbrot_next_row >= kScreenHeight) {
        g_app.mandelbrot_rendered = true;
        lv_label_set_text(g_app.hint_label, "BOOT: SCOPE");
    } else {
        char progress[24] = {};
        const int percent = (g_app.mandelbrot_next_row * 100) / kScreenHeight;
        std::snprintf(progress, sizeof(progress), "Rendering %d%%", percent);
        lv_label_set_text(g_app.hint_label, progress);
    }
}

}  // namespace app

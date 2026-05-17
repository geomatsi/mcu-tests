#include "app.h"

#include "esp_random.h"
#include "esp_timer.h"

namespace app {

namespace {

void hide_matrix_pixel(lv_obj_t *pixel)
{
    lv_obj_add_flag(pixel, LV_OBJ_FLAG_HIDDEN);
}

void show_matrix_pixel(lv_obj_t *pixel, int col, int row, lv_color_t color)
{
    lv_obj_set_pos(pixel, col * kMatrixCellSize, row * kMatrixCellSize);
    lv_obj_set_style_bg_color(pixel, color, 0);
    lv_obj_clear_flag(pixel, LV_OBJ_FLAG_HIDDEN);
}

void reset_matrix_column(MatrixColumnState &column, bool initial)
{
    column.trail_length = 3 + static_cast<int>(esp_random() % (kMatrixMaxTrail - 2));
    column.speed_divider = 1 + static_cast<int>(esp_random() % 3);
    column.speed_phase = static_cast<int>(esp_random() % column.speed_divider);

    const int spawn_span = initial ? kMatrixRows + column.trail_length : (kMatrixRows / 2) + column.trail_length;
    column.head_row = -static_cast<int>(esp_random() % spawn_span);
}

}  // namespace

void init_matrix_ui(lv_obj_t *screen)
{
    g_app.matrix_layer = lv_obj_create(screen);
    lv_obj_remove_style_all(g_app.matrix_layer);
    lv_obj_set_size(g_app.matrix_layer, kScreenWidth, kScreenHeight);
    lv_obj_set_pos(g_app.matrix_layer, 0, 0);
    lv_obj_add_flag(g_app.matrix_layer, LV_OBJ_FLAG_HIDDEN);

    for (int col = 0; col < kMatrixCols; ++col) {
        for (int seg = 0; seg < kMatrixMaxTrail; ++seg) {
            lv_obj_t *pixel = lv_obj_create(g_app.matrix_layer);
            lv_obj_remove_style_all(pixel);
            lv_obj_set_size(pixel, kMatrixCellSize - 1, kMatrixCellSize - 1);
            lv_obj_set_style_radius(pixel, 0, 0);
            lv_obj_set_style_bg_opa(pixel, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(pixel, 0, 0);
            hide_matrix_pixel(pixel);
            g_app.matrix_pixels[col][seg] = pixel;
        }
        reset_matrix_column(g_app.matrix_columns[col], true);
    }
}

void reset_matrix_scene_locked()
{
    for (int col = 0; col < kMatrixCols; ++col) {
        reset_matrix_column(g_app.matrix_columns[col], true);
        for (int seg = 0; seg < kMatrixMaxTrail; ++seg) {
            hide_matrix_pixel(g_app.matrix_pixels[col][seg]);
        }
    }
    g_app.last_matrix_step_us = 0;
}

void update_matrix_ui_locked(bool force)
{
    const int64_t now_us = esp_timer_get_time();
    if (!force && g_app.last_matrix_step_us != 0 && (now_us - g_app.last_matrix_step_us) < kMatrixStepUs) {
        return;
    }
    g_app.last_matrix_step_us = now_us;

    static const lv_color_t kTrailColors[kMatrixMaxTrail] = {
        lv_color_hex(0xD8FFD8),
        lv_color_hex(0x7CFF7C),
        lv_color_hex(0x3CEB3C),
        lv_color_hex(0x18B818),
        lv_color_hex(0x0D7A0D),
        lv_color_hex(0x084E08),
    };

    for (int col = 0; col < kMatrixCols; ++col) {
        MatrixColumnState &column = g_app.matrix_columns[col];
        column.speed_phase++;
        if (column.speed_phase >= column.speed_divider) {
            column.speed_phase = 0;
            column.head_row++;
        }

        if (column.head_row - column.trail_length > kMatrixRows + 1) {
            reset_matrix_column(column, false);
        }

        for (int seg = 0; seg < kMatrixMaxTrail; ++seg) {
            lv_obj_t *pixel = g_app.matrix_pixels[col][seg];
            if (seg >= column.trail_length) {
                hide_matrix_pixel(pixel);
                continue;
            }

            const int row = column.head_row - seg;
            if (row < 0 || row >= kMatrixRows) {
                hide_matrix_pixel(pixel);
                continue;
            }

            show_matrix_pixel(pixel, col, row, kTrailColors[seg]);
        }
    }
}

}  // namespace app

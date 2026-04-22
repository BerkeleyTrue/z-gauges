#include "lvgl_ui.h"
#include "lvgl.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "lvgl_ui";

static lv_display_t *s_display = NULL;

// ============================================================
// Provided by Rust: hardware operations
// ============================================================
extern void rust_lcd_draw(int x1, int y1, int x2, int y2, const uint8_t *px_map);
extern bool rust_touch_read(int16_t *x, int16_t *y);

// ============================================================
// LVGL internals
// ============================================================
static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    rust_lcd_draw(area->x1, area->y1, area->x2, area->y2, px_map);
    lv_display_flush_ready(disp);
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    int16_t x, y;
    if (rust_touch_read(&x, &y)) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(1);
}

// ============================================================
// UI -- add screens and widgets here
// ============================================================
static void ui_build(void)
{
    lv_obj_t *scr = lv_screen_active();

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Z-Gauges");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

// ============================================================
// Public API
// ============================================================
void lvgl_ui_init(int h_res, int v_res)
{
    ESP_LOGI(TAG, "LVGL init...");
    lv_init();

    size_t buf_bytes = (size_t)h_res * 20 * sizeof(lv_color_t);
    lv_color_t *draw_buf = heap_caps_malloc(buf_bytes, MALLOC_CAP_DMA);
    assert(draw_buf != NULL && "LVGL draw buffer alloc failed");

    s_display = lv_display_create((int32_t)h_res, (int32_t)v_res);
    lv_display_set_flush_cb(s_display, flush_cb);
    lv_display_set_buffers(s_display, draw_buf, NULL, buf_bytes,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    const esp_timer_create_args_t tick_args = {
        .callback = tick_cb,
        .name     = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 1000)); // 1 ms

    ESP_LOGI(TAG, "Building UI...");
    ui_build();
}

void lvgl_ui_run(void)
{
    ESP_LOGI(TAG, "Entering LVGL loop");
    while (1) {
        uint32_t delay_ms = lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(delay_ms > 0 ? delay_ms : 5));
    }
}

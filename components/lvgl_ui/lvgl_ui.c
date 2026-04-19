#include "lvgl_ui.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "lvgl_ui";

static lv_display_t         *s_display = NULL;
static esp_lcd_touch_handle_t s_touch  = NULL;

// ============================================================
// Exposed to Rust: DMA flush-done callback
// Must live in C because it calls lv_display_flush_ready() (LVGL type).
// ============================================================
bool lvgl_on_color_trans_done(esp_lcd_panel_io_handle_t io,
                               esp_lcd_panel_io_event_data_t *edata,
                               void *user_ctx)
{
    (void)io; (void)edata; (void)user_ctx;
    lv_display_flush_ready(s_display);
    return false;
}

// ============================================================
// Exposed to Rust: CST816S init shim
// esp_lcd_touch_config_t is from the component registry and is not
// available in esp-idf-sys bindings, so this lives in C.
// ============================================================
void *lvgl_touch_new_cst816s(int sda_pin, int scl_pin,
                              int rst_pin, int int_pin,
                              int x_max, int y_max)
{
    i2c_master_bus_config_t i2c_cfg = {
        .clk_source             = I2C_CLK_SRC_DEFAULT,
        .i2c_port               = I2C_NUM_0,
        .scl_io_num             = scl_pin,
        .sda_io_num             = sda_pin,
        .glitch_ignore_cnt      = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus;
    ESP_ERROR_CHECK(i2c_master_bus_create(I2C_NUM_0, &i2c_cfg, &i2c_bus));

    esp_lcd_panel_io_handle_t tp_io;
    esp_lcd_panel_io_i2c_config_t tp_io_cfg = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_cfg, &tp_io));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max        = (uint16_t)x_max,
        .y_max        = (uint16_t)y_max,
        .rst_gpio_num = rst_pin,
        .int_gpio_num = int_pin,
        .levels       = { .reset = 0, .interrupt = 0 },
        .flags        = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
    };
    esp_lcd_touch_handle_t touch;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(tp_io, &tp_cfg, &touch));
    return (void *)touch;
}

// ============================================================
// LVGL internals
// ============================================================
static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel =
        (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
    esp_lcd_panel_draw_bitmap(panel,
        area->x1, area->y1, area->x2 + 1, area->y2 + 1,
        (uint16_t *)px_map);
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    uint16_t x[1], y[1];
    uint8_t cnt = 0;
    esp_lcd_touch_read_data(s_touch);
    bool touched = esp_lcd_touch_get_coordinates(s_touch, x, y, NULL, &cnt, 1);
    if (touched && cnt > 0) {
        data->point.x = x[0];
        data->point.y = y[0];
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
void lvgl_ui_init(void *panel, void *touch, int h_res, int v_res)
{
    s_touch = (esp_lcd_touch_handle_t)touch;

    ESP_LOGI(TAG, "LVGL init...");
    lv_init();

    size_t buf_bytes = (size_t)h_res * 20 * sizeof(lv_color_t);
    lv_color_t *draw_buf = heap_caps_malloc(buf_bytes, MALLOC_CAP_DMA);
    assert(draw_buf != NULL && "LVGL draw buffer alloc failed");

    s_display = lv_display_create((int32_t)h_res, (int32_t)v_res);
    lv_display_set_flush_cb(s_display, flush_cb);
    lv_display_set_buffers(s_display, draw_buf, NULL, buf_bytes,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(s_display, panel); // flush_cb retrieves panel here

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

#pragma once

#include "esp_lcd_panel_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DMA transfer-done callback for the LCD panel IO.
 * Assign to esp_lcd_panel_io_spi_config_t.on_color_trans_done in Rust.
 * Signals LVGL that the draw buffer is free for the next frame.
 */
bool lvgl_on_color_trans_done(esp_lcd_panel_io_handle_t io,
                               esp_lcd_panel_io_event_data_t *edata,
                               void *user_ctx);

/**
 * CST816S touch init shim.
 * Kept in C because esp_lcd_touch_config_t is a component-registry type
 * not present in esp-idf-sys bindings.
 * Returns an opaque esp_lcd_touch_handle_t cast to void*.
 */
void *lvgl_touch_new_cst816s(int sda_pin, int scl_pin,
                              int rst_pin, int int_pin,
                              int x_max, int y_max);

/**
 * Initialize LVGL with pre-created hardware handles from Rust.
 *   panel -- esp_lcd_panel_handle_t cast to void*
 *   touch -- esp_lcd_touch_handle_t cast to void* (from lvgl_touch_new_cst816s)
 */
void lvgl_ui_init(void *panel, void *touch, int h_res, int v_res);

/**
 * Enter the LVGL event loop. Does not return.
 */
void lvgl_ui_run(void);

#ifdef __cplusplus
}
#endif

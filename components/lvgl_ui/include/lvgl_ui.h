#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize LVGL: display buffer, flush/touch/tick callbacks, and UI.
 * Hardware is driven by Rust via rust_lcd_draw() and rust_touch_read().
 */
void lvgl_ui_init(int h_res, int v_res);

/**
 * Enter the LVGL event loop. Does not return.
 */
void lvgl_ui_run(void);

#ifdef __cplusplus
}
#endif

idf_component_register(
    SRCS "lvgl_ui.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_lcd esp_timer freertos driver
    PRIV_REQUIRES lvgl__lvgl espressif__esp_lcd_touch_cst816s
)

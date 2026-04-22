idf_component_register(
    SRCS "lvgl_ui.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_timer freertos
    PRIV_REQUIRES lvgl__lvgl
)

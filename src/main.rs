use esp_idf_sys::{self as _, esp, esp_lcd_panel_handle_t, esp_lcd_panel_io_handle_t};
use std::ffi::c_void;

// ============================================================
// Board pin definitions (Waveshare ESP32-S3-Touch-LCD-1.28)
// ============================================================
const PIN_LCD_SCLK: i32 = 10;
const PIN_LCD_MOSI: i32 = 11;
const PIN_LCD_CS: i32 = 9;
const PIN_LCD_DC: i32 = 8;
const PIN_LCD_RST: i32 = 12;
const PIN_LCD_BL: u32 = 40;

const PIN_TP_SDA: i32 = 6;
const PIN_TP_SCL: i32 = 7;
const PIN_TP_INT: i32 = 5;
const PIN_TP_RST: i32 = 13;

const LCD_H_RES: i32 = 240;
const LCD_V_RES: i32 = 240;
const LCD_PIXEL_CLOCK_HZ: u32 = 40_000_000;

// ============================================================
// C component interface
// ============================================================
extern "C" {
    // DMA flush-done callback -- assign to on_color_trans_done in io_cfg below.
    // Lives in C because it calls lv_display_flush_ready() (LVGL type).
    fn lvgl_on_color_trans_done(
        io: esp_lcd_panel_io_handle_t,
        edata: *mut esp_idf_sys::esp_lcd_panel_io_event_data_t,
        user_ctx: *mut c_void,
    ) -> bool;

    // CST816S touch init shim (see lvgl_ui.c for why this is in C).
    fn lvgl_touch_new_cst816s(
        sda_pin: i32,
        scl_pin: i32,
        rst_pin: i32,
        int_pin: i32,
        x_max: i32,
        y_max: i32,
    ) -> *mut c_void;

    fn lvgl_ui_init(panel: *mut c_void, touch: *mut c_void, h_res: i32, v_res: i32);
    fn lvgl_ui_run();
}

// ============================================================
// GC9A01 display init
// Returns esp_lcd_panel_handle_t as *mut c_void
// ============================================================
unsafe fn init_display() -> *mut c_void {
    use esp_idf_sys::*;

    // Backlight off during init to avoid flicker
    let bk_cfg = gpio_config_t {
        pin_bit_mask: 1u64 << PIN_LCD_BL,
        mode: gpio_mode_t_GPIO_MODE_OUTPUT,
        pull_up_en: gpio_pullup_t_GPIO_PULLUP_DISABLE,
        pull_down_en: gpio_pulldown_t_GPIO_PULLDOWN_DISABLE,
        intr_type: gpio_int_type_t_GPIO_INTR_DISABLE,
    };
    esp!(gpio_config(&bk_cfg)).unwrap();
    esp!(gpio_set_level(PIN_LCD_BL, 0)).unwrap();

    // SPI bus
    // Note: spi_bus_config_t has anonymous unions in ESP-IDF 5.x.
    // If this fails to compile, check the bindgen output for the actual field names.
    let mut bus_cfg: spi_bus_config_t = std::mem::zeroed();
    bus_cfg.__bindgen_anon_1.mosi_io_num = PIN_LCD_MOSI;
    bus_cfg.__bindgen_anon_2.miso_io_num = -1;
    bus_cfg.sclk_io_num = PIN_LCD_SCLK;
    bus_cfg.__bindgen_anon_3.quadwp_io_num = -1;
    bus_cfg.__bindgen_anon_4.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = LCD_H_RES * 20 * 2; // 20 draw-buffer lines * 2 bytes/px

    esp!(spi_bus_initialize(
        spi_host_device_t_SPI2_HOST,
        &bus_cfg,
        spi_dma_chan_t_SPI_DMA_CH_AUTO,
    ))
    .unwrap();

    // LCD panel IO over SPI
    let mut io_handle: esp_lcd_panel_io_handle_t = std::ptr::null_mut();
    let io_cfg = esp_lcd_panel_io_spi_config_t {
        dc_gpio_num: PIN_LCD_DC,
        cs_gpio_num: PIN_LCD_CS,
        pclk_hz: LCD_PIXEL_CLOCK_HZ,
        lcd_cmd_bits: 8,
        lcd_param_bits: 8,
        spi_mode: 0,
        trans_queue_depth: 10,
        on_color_trans_done: Some(lvgl_on_color_trans_done),
        user_ctx: std::ptr::null_mut(),
        flags: std::mem::zeroed(),
    };
    esp!(esp_lcd_new_panel_io_spi(
        spi_host_device_t_SPI2_HOST as esp_lcd_spi_bus_handle_t,
        &io_cfg,
        &mut io_handle,
    ))
    .unwrap();

    // GC9A01 panel
    let mut panel_handle: esp_lcd_panel_handle_t = std::ptr::null_mut();
    let panel_cfg = esp_lcd_panel_dev_config_t {
        reset_gpio_num: PIN_LCD_RST,
        rgb_ele_order: lcd_rgb_element_order_t_LCD_RGB_ENDIAN_BGR,
        bits_per_pixel: 16,
        ..std::mem::zeroed()
    };
    esp!(esp_lcd_new_panel_gc9a01(io_handle, &panel_cfg, &mut panel_handle)).unwrap();
    esp!(esp_lcd_panel_reset(panel_handle)).unwrap();
    esp!(esp_lcd_panel_init(panel_handle)).unwrap();
    esp!(esp_lcd_panel_invert_color(panel_handle, true)).unwrap(); // GC9A01 requires inversion
    esp!(esp_lcd_panel_mirror(panel_handle, true, false)).unwrap();
    esp!(esp_lcd_panel_disp_on_off(panel_handle, true)).unwrap();

    // Backlight on
    esp!(gpio_set_level(PIN_LCD_BL, 1)).unwrap();

    panel_handle as *mut c_void
}

// ============================================================
// CST816S touch init (via C shim -- see lvgl_ui.c)
// Returns esp_lcd_touch_handle_t as *mut c_void
// ============================================================
unsafe fn init_touch() -> *mut c_void {
    lvgl_touch_new_cst816s(PIN_TP_SDA, PIN_TP_SCL, PIN_TP_RST, PIN_TP_INT, LCD_H_RES, LCD_V_RES)
}

// ============================================================
// Entry point
// ============================================================
fn main() {
    esp_idf_svc::log::EspLogger::initialize_default();
    log::info!("z-gauges starting...");

    unsafe {
        let panel = init_display();
        let touch = init_touch();
        lvgl_ui_init(panel, touch, LCD_H_RES, LCD_V_RES);
        lvgl_ui_run(); // does not return
    }
}

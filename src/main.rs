use esp_idf_sys::{self as _, esp, esp_lcd_panel_handle_t, esp_lcd_panel_io_handle_t};

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

fn main() {
    // Temporary. Will disappear once ESP-IDF 4.4 is released, but for now it is necessary to call this function once,
    // or else some patches to the runtime implemented by esp-idf-sys might not link properly.
    esp_idf_sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    log::info!("z-gauges starting...");
}

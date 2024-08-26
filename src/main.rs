#![no_std] // don't use standard lib, use core instead
#![no_main] 
// #![cfg_attr(not(feature = "simulator"), no_main)]

// slint::include_modules!();
//
// // #![cfg(feature = "simulator")]
// fn main() -> Result<(), slint::PlatformError> {
//     let ui = AppWindow::new()?;
//     ui.on_request_increase_value({
//         let ui_handle = ui.as_weak();
//         move || {
//             let ui = ui_handle.unwrap();
//             ui.set_counter(ui.get_counter() + 1);
//         }
//     });
//     ui.run()

use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl, delay::Delay, peripherals::Peripherals, prelude::*, system::SystemControl,
};

#[entry]
fn main() -> ! {
    let peripherals = Peripherals::take();
    let system = SystemControl::new(peripherals.SYSTEM);

    let clocks = ClockControl::max(system.clock_control).freeze();
    let delay = Delay::new(&clocks);

    esp_println::logger::init_logger_from_env();

    loop {
        log::info!("Hello world!");
        delay.delay(500.millis());
    }
}

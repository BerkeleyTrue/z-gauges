#![no_std]
// #![cfg_attr(not(feature = "simulator"), no_main)]

slint::include_modules!();

// #![cfg(feature = "simulator")]
fn main() -> Result<(), slint::PlatformError> {
    let ui = AppWindow::new()?;
    ui.on_request_increase_value({
        let ui_handle = ui.as_weak();
        move || {
            let ui = ui_handle.unwrap();
            ui.set_counter(ui.get_counter() + 1);
        }
    });
    ui.run()
}

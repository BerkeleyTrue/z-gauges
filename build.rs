fn main() {
    slint_build::compile_with_config(
        "ui/appwindow.slint",
        slint_build::CompilerConfiguration::new()
            .embed_resources(slint_build::EmbedResourcesKind::EmbedForSoftwareRenderer),
    )
    .unwrap();
    slint_build::print_rustc_flags().unwrap();
    println!("cargo:rustc-link-arg-bins=-Tlinkall.x");
}

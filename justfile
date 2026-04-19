# Build debug (first build downloads ESP-IDF ~500MB -- takes a while)
build:
    cargo build

# Build release
release:
    cargo build --release

# Flash debug build and open serial monitor
flash:
    cargo run

# Flash release build and open serial monitor
flash-release:
    cargo run --release

# Open serial monitor without flashing
monitor:
    espflash monitor

# Remove build artifacts (keeps downloaded ESP-IDF)
clean:
    cargo clean

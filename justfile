default:
    @just --list

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

# Build the LSP docker image (rust-analyzer for ESP32 cross-compilation)
lsp:
    docker build -t z-gauges-rust-analyzer -f lsp/Dockerfile .

# Remove build artifacts (keeps downloaded ESP-IDF)
clean:
    cargo clean

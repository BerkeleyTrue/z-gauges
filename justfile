default:
    @just --list

image := "z-gauges"
idf_version := `grep 'ESP_IDF_VERSION' .cargo/config.toml | sed 's/.*= "\(.*\)"/\1/'`
binary := "target/z-gauges.elf"

# Build using Docker (handles ESP-IDF + Xtensa Rust toolchain)
build:
    docker build -t {{image}} -f Dockerfile --build-arg IDF_VERSION={{idf_version}} .
    @mkdir -p target
    @docker rm -f z-gauges-tmp 2>/dev/null || true
    docker create --name z-gauges-tmp {{image}}
    docker cp z-gauges-tmp:/z-gauges {{binary}}
    @docker rm z-gauges-tmp

# Flash the built binary and open serial monitor
flash:
    espflash flash --monitor {{binary}}

# Flash without opening monitor
flash-only:
    espflash flash {{binary}}

# Open serial monitor without flashing
monitor:
    espflash monitor

# Follow logs of the running LSP container
lsp-logs:
    docker logs -f $(docker ps -q --filter ancestor=z-gauges-rust-analyzer)

# Build the LSP docker image (rust-analyzer for ESP32 cross-compilation)
lsp:
    docker build -t z-gauges-rust-analyzer -f lsp/Dockerfile .

# Remove build artifacts
clean:
    docker rmi {{image}} 2>/dev/null || true
    rm -f {{binary}}

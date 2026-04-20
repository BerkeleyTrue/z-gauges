set -o pipefail

# this script will build the image to run rust-analyzer
# within a container with esp-rs idf toolchain set up
docker build -t z-gauges-rust-analyzer -f ./lsp/Dockerfile .

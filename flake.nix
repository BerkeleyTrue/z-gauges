{
  description = "Gauges for my datsun -- ESP32-S3 + LVGL via ESP-IDF";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs @ {
    flake-parts,
    nixpkgs,
    ...
  }:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux"];
      perSystem = {
        config,
        system,
        ...
      }: let
        pkgs = import nixpkgs {inherit system;};
      in {
        formatter.default = pkgs.alejandra;
        devShells.default = let
          buildInputs = with pkgs; [
            just

            # Rust -- rustup manages the actual toolchain (esp channel)
            rustup
            rustc
            rust-analyzer

            # ESP toolchain
            espup
            espflash

            # Required by embuild / ESP-IDF build system
            cmake
            ninja
            python3
            pkg-config
            git
          ];
        in
          pkgs.mkShell {
            name = "z-gauges";
            buildInputs = buildInputs;

            shellHook = ''
              echo -e "\e[1mSetting up ESP32-S3 toolchain\e[0m"

              # Only install if the export file is missing (avoids re-downloading on every shell entry)
              if [ ! -f ./exports-esp.sh ]; then
                espup install --targets esp32s3 --export-file ./exports-esp.sh
              fi
              source ./exports-esp.sh

              # ldproxy is the linker wrapper required by esp-idf-sys
              if ! command -v ldproxy &>/dev/null; then
                echo "Installing ldproxy..."
                cargo install ldproxy
              fi

              export PATH=$PATH:$HOME/.cargo/bin
            '';
          };
      };
      flake = {};
    };
}

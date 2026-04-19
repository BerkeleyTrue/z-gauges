{
  description = "Gauges for my datsun, built with slint";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";

    nixgl.url = "github:nix-community/nixGL";
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
        pkgs = import nixpkgs {
          inherit system;

          overlays = [
            inputs.nixgl.overlay
          ];
        };
      in {
        formatter.default = pkgs.alejandra;
        devShells.default = let
          buildInputs = with pkgs; [
            just

            cargo
            cargo-generate
             # rustc is provided espup tooling
            rustup
            rustfmt
            rust-analyzer

            # slint tools
            libGL
            qt5.full
            ffmpeg

            # esp dev
            espup
            espflash # flash binary to esp
          ];
        in
          pkgs.mkShell {
            name = "rust";
            buildInputs = buildInputs;

            shellHook = ''
              echo -e "\e[1mInstalling toolchains for esp"
              echo -e "-----------------------------\e[0m"
              espup install --targets esp32s3 --export-file ./exports-esp.sh
              source ./exports-esp.sh
              export PATH=$PATH:$HOME/.cargo/bin
            '';
          };
      };
      flake = {};
    };
}

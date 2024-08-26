{
  description = "Gauges for my datsun, built with slint";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";

    nixgl.url = "github:nix-community/nixGL";
    flake-parts.url = "github:hercules-ci/flake-parts";
    boulder.url = "github:berkeleytrue/nix-boulder-banner";
  };

  outputs = inputs @ {
    flake-parts,
    nixpkgs,
    ...
  }:
    flake-parts.lib.mkFlake {inherit inputs;} {
      imports = [
        inputs.boulder.flakeModule
      ];
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
        run = pkgs.writeShellScriptBin "run" ''
          echo "nix cargo run"
          ${pkgs.nixgl.auto.nixGLDefault}/bin/nixGL cargo run
        '';
        build = pkgs.writeShellScriptBin "build" ''
          echo "nix cargo build"
          ${pkgs.nixgl.auto.nixGLDefault}/bin/nixGL cargo build
        '';
      in {
        formatter.default = pkgs.alejandra;
        boulder.commands = [
          {
            exec = run;
            description = "cargo run";
          }
          {
            exec = build;
            description = "cargo build";
          }
        ];
        devShells.default = let
          buildInputs = with pkgs; [
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
            inputsFrom = [
              config.boulder.devShell
            ];

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

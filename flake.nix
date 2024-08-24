{
  description = "Gauges for my datsun, built with slint";

  inputs = {
    nixgl.url = "github:nix-community/nixGL";
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
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
      in {
        formatter.default = pkgs.alejandra;
        boulder.commands = [
          {
            exec = run;
            description = "cargo run";
          }
        ];
        devShells.default = let
          buildInputs = with pkgs; [
            openssl
            cargo
            rustc
            rustfmt
            libGL
            qt5.full
            ffmpeg
          ];
        in
          pkgs.mkShell {
            name = "rust";
            inputsFrom = [
              config.boulder.devShell
            ];

            buildInputs = buildInputs;

            # LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath buildInputs;
          };
      };
      flake = {};
    };
}

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
        devShells.default = pkgs.mkShell {
          name = "z-gauges";

          buildInputs = with pkgs; [
            just
            espflash
          ];
        };
      };
      flake = {};
    };
}

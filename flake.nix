{
  description = "AxionPlot - Graphing Calculator";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            cmake
            gcc14
            pkg-config
            git

            libx11
            libxrandr
            libxi
            libxcursor
            libxinerama
            libglvnd
            mesa

            fontconfig
            dejavu_fonts
            liberation_ttf

            python3
          ];

          shellHook = ''
            export AXION_FONT="$(fc-match --format='%{file}' DejaVuSans 2>/dev/null || true)"
            echo "AxionPlot dev shell - font: $AXION_FONT"
            echo "Run: make native"
          '';
        };
      }
    );
}

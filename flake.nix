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

            python3
          ];

          shellHook = ''
            echo "AxionPlot dev shell (raylib built via FetchContent)"
            echo "Run: cmake -B build && cmake --build build"
          '';
        };
      }
    );
}

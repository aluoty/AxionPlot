#!/usr/bin/env bash
# Cloudflare Pages build — installs cmake if missing, then builds WASM output to build-web/.
set -euo pipefail

cd "$(dirname "$0")/.."

CMAKE_VERSION=3.29.6
CMAKE_DIR=".ci/cmake-${CMAKE_VERSION}-linux-x86_64"

ensure_cmake() {
    if command -v cmake >/dev/null 2>&1; then
        echo "cmake: $(cmake --version | head -1)"
        return 0
    fi

    echo "cmake not found; attempting install..."

    if command -v apt-get >/dev/null 2>&1; then
        if sudo apt-get update -qq && sudo apt-get install -y -qq cmake make; then
            echo "cmake: installed via apt"
            return 0
        fi
    fi

    if command -v dnf >/dev/null 2>&1; then
        if sudo dnf install -y cmake make; then
            echo "cmake: installed via dnf"
            return 0
        fi
    fi

    echo "cmake: downloading portable binary"
    mkdir -p .ci
    if [[ ! -x "${CMAKE_DIR}/bin/cmake" ]]; then
        curl -fsSL \
            "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz" \
            | tar xz -C .ci
    fi
    export PATH="${PWD}/${CMAKE_DIR}/bin:${PATH}"
    echo "cmake: $(cmake --version | head -1)"
}

ensure_cmake

if ! command -v make >/dev/null 2>&1; then
    echo "Error: make is required but not installed."
    exit 1
fi

make setup-emsdk
make wasm SCALE=1

echo "Cloudflare build complete: build-web/"

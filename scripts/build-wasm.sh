#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build-web"

if ! command -v emcc >/dev/null 2>&1; then
    echo "Emscripten (emcc) not found."
    echo "Install the Emscripten SDK: https://emscripten.org/docs/getting_started/downloads.html"
    echo "Then run: source <emsdk>/emsdk_env.sh"
    exit 1
fi

emcmake cmake -B "${BUILD_DIR}" -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --config Release

cp "${ROOT}/favicon.svg" "${BUILD_DIR}/favicon.svg" 2>/dev/null || true
cp "${BUILD_DIR}/AxionPlot.html" "${BUILD_DIR}/index.html"

echo "Built: ${BUILD_DIR}/AxionPlot.html"
echo "Serve locally with: emrun ${BUILD_DIR}/AxionPlot.html"
echo "Or: python -m http.server --directory ${BUILD_DIR}"

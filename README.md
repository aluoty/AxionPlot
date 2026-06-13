# AxionPlot

A full graphing calculator built with **C** and **Raylib**, compiled to **WebAssembly** for the browser.

Plot functions interactively with pan/zoom, multiple graph modes, an on-screen keypad, scientific calculator, value tables, trace mode, and adjustable display quality.

## Features

### Graphing
- **Cartesian** plots: `y = sin(x)`, `x^2 + a*x + b`
- **Parametric** plots: `x(t) = cos(t)`, `y(t) = sin(t)`
- **Polar** plots: `r(t) = 1 + cos(t)`
- Up to 32 simultaneous graphs with show/hide toggles
- Adaptive grid with axis labels
- Trace mode: vertical cursor shows `(x, y)` on each visible graph
- Pan (drag) and zoom (scroll wheel) on the plot area

### Display quality
- Adjustable window sizes: 1280×720, 1600×900, 1920×1080, 2560×1440
- Supersampling quality: 1× (standard), 2× (high), 3× (ultra)
- MSAA and HiDPI support on desktop
- **Display** tab in-app (window size on desktop; canvas follows browser on web)

### Calculator
- Scientific expression evaluation (`sin`, `cos`, `tan`, `sqrt`, `ln`, `log`, `abs`, etc.)
- Variables `a`, `b`, `c` assignable via `a=2` or `a=sin(pi/4)`
- On-screen keypad for numbers, operators, and functions
- Value table for any plotted function

### Platforms
- Native desktop (Linux, macOS, Windows)
- Web browser via WebAssembly

## Requirements

- **CMake** 3.15+
- **make** and a C compiler (GCC or Clang)
- OpenGL development libraries (desktop only)
- **git** (for fetching Raylib and Emscripten)

## Build

All builds go through the **Makefile** (which wraps CMake):

```bash
make              # native desktop build (default 1920×1080 @ 2× quality)
make native       # same as above
make wasm         # WebAssembly build
make run          # build and run native app
make run-web      # build WASM and serve at http://localhost:8000
make clean        # remove build/ and build-web/
```

Override default resolution at build time:

```bash
make native WIDTH=1280 HEIGHT=720 SCALE=1
make wasm WIDTH=1920 HEIGHT=1080 SCALE=1   # web defaults to SCALE=1
```

## Emscripten setup

One-time install (clones SDK to `tools/emsdk/`):

```bash
make setup-emsdk
make wasm
```

The Makefile activates Emscripten automatically for `make wasm`. To use `emcc` directly in your shell:

```bash
source tools/emsdk/emsdk_env.sh
```

### Manual Emscripten install

If you prefer a system-wide install:

```bash
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
source ~/emsdk/emsdk_env.sh
make wasm
```

Fedora users may also need:

```bash
sudo dnf install cmake gcc git python3
```

## Run

**Desktop:**

```bash
make run
# or
./build/AxionPlot
```

**Web:**

```bash
make run-web
# Open http://localhost:8000/
```

Use the **Display** tab to change render quality. On desktop you can also change window size; on web the canvas follows the browser window.

## Deploy to Cloudflare Pages

Cloudflare's build image does not include **cmake** by default. Use the provided build script:

| Setting | Value |
|---------|-------|
| **Build command** | `bash scripts/cloudflare-build.sh` |
| **Build output directory** | `build-web` |

The script installs cmake (via apt or a portable download), sets up Emscripten, and builds the WASM bundle. It also copies `_headers` so `.wasm` files are served with the correct MIME type.

Alternative (if cmake is already available):

```bash
make setup-emsdk && make wasm SCALE=1
```

## Usage

| Input / Action | Result |
|----------------|--------|
| `sin(x)` | Plot sine wave (Cartesian mode) |
| `cos(t), sin(t)` | Plot circle (Parametric mode) |
| `1+cos(t)` | Plot cardioid (Polar mode) |
| `a=3` | Set variable `a` to 3 |
| **Display** tab | Change window size and render quality |
| **F11** | Toggle fullscreen (desktop) |
| Mouse drag (plot area) | Pan |
| Scroll wheel (plot area) | Zoom toward cursor |

## Project layout

```
src/           Application source
lib/           tinyexpr and raygui
web/           Emscripten HTML shell
CMakeLists.txt Build definition (used by Makefile)
Makefile       Build entry point
```

## License

See [LICENSE](LICENSE).

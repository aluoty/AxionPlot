# AxionPlot

A full graphing calculator built with **C** and **Raylib**, compiled to **WebAssembly** for the browser.

Plot functions interactively with pan/zoom, multiple graph modes, an on-screen keypad, scientific calculator, value tables, and trace mode.

## Features

### Graphing
- **Cartesian** plots: `y = sin(x)`, `x^2 + a*x + b`
- **Parametric** plots: `x(t) = cos(t)`, `y(t) = sin(t)`
- **Polar** plots: `r(t) = 1 + cos(t)`
- Up to 32 simultaneous graphs with show/hide toggles
- Adaptive grid with axis labels
- Trace mode: vertical cursor shows `(x, y)` on each visible graph
- Pan (drag) and zoom (scroll wheel) on the plot area

### Calculator
- Scientific expression evaluation (`sin`, `cos`, `tan`, `sqrt`, `ln`, `log`, `abs`, etc.)
- Variables `a`, `b`, `c` assignable via `a=2` or `a=sin(pi/4)`
- On-screen keypad for numbers, operators, and functions
- Value table for any plotted function

### Math support
- Operators: `+`, `-`, `*`, `/`, `^`
- Functions: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `sinh`, `cosh`, `tanh`, `sqrt`, `abs`, `ln`, `log`, `log10`, `exp`, `floor`, `ceil`, `fac`
- Constants: `pi`, `e`
- Implicit multiplication: `2x`, `3sin(x)`

### Platforms
- Native desktop (Linux, macOS, Windows)
- Web browser via WebAssembly

## Requirements

### Desktop build

- CMake 3.15+
- C compiler (GCC or Clang)
- OpenGL development libraries

### Web (WASM) build

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- CMake 3.15+

## Build

### Native (Linux / macOS / Windows)

```bash
./scripts/build-native.sh
./build/AxionPlot
```

### WebAssembly

```bash
source /path/to/emsdk/emsdk_env.sh
./scripts/build-wasm.sh
```

Output files are written to `build-web/`:

- `AxionPlot.html`
- `AxionPlot.js`
- `AxionPlot.wasm`
- `index.html` (copy of AxionPlot.html)

### Run in the browser

```bash
# Option 1: emrun (recommended)
emrun build-web/AxionPlot.html

# Option 2: any static file server
python -m http.server --directory build-web
# Open http://localhost:8000/
```

## Usage

| Input / Action | Result |
|----------------|--------|
| `sin(x)` | Plot sine wave (Cartesian mode) |
| `cos(t), sin(t)` | Plot circle (Parametric mode) |
| `1+cos(t)` | Plot cardioid (Polar mode) |
| `a=3` | Set variable `a` to 3 |
| **Graph** tab → **Plot** | Add current expression as a graph |
| **Calc** tab → **=** | Evaluate expression numerically |
| **Table** tab | View sampled values for a graph |
| **Trace** | Show y-values at cursor x on all graphs |
| **Home** | Reset pan/zoom to default view |
| Mouse drag (plot area) | Pan |
| Scroll wheel (plot area) | Zoom toward cursor |
| Enter | Plot or evaluate (depending on tab) |

## Project layout

```
src/          Application source (main, camera, graph, expr, ui)
lib/          tinyexpr (math parser) and raygui (UI)
web/          Emscripten HTML shell
scripts/      Build helpers (build-native.sh, build-wasm.sh)
```

## License

See [LICENSE](LICENSE).

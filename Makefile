# AxionPlot build — run `make help` for targets.
# CMakeLists.txt is the build definition; this file is the entry point.

PROJECT   := AxionPlot
BUILD     := build
WEB       := build-web
CMAKE     := cmake
JOBS      := $(shell nproc 2>/dev/null || echo 4)

WIDTH     ?= 1920
HEIGHT    ?= 1080
SCALE     ?= 2

EMSROOT   ?= $(CURDIR)/tools/emsdk

EMSENV    := $(EMSROOT)/emsdk_env.sh

# Run commands with emsdk activated when available
EMS_RUN = bash -lc 'source "$(EMSENV)" 2>/dev/null || true; $(1)'

CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release \
               -DAXION_WINDOW_WIDTH=$(WIDTH) \
               -DAXION_WINDOW_HEIGHT=$(HEIGHT) \
               -DAXION_RENDER_SCALE=$(SCALE)

.PHONY: all native wasm clean run run-web setup-emsdk emsdk-check help

all: native

help:
	@echo "Targets:"
	@echo "  make native              Build desktop app ($(WIDTH)x$(HEIGHT) @ $(SCALE)x)"
	@echo "  make wasm                Build WebAssembly (uses tools/emsdk if present)"
	@echo "  make setup-emsdk         Install Emscripten SDK to $(EMSROOT)"
	@echo "  make run                 Run native build"
	@echo "  make run-web             Serve WASM build on http://localhost:8000"
	@echo "  make clean               Remove build directories"
	@echo ""
	@echo "Options (override defaults):"
	@echo "  make native WIDTH=1280 HEIGHT=720 SCALE=1"
	@echo ""
	@echo "First-time web build:"
	@echo "  make setup-emsdk && make wasm"

native:
	$(CMAKE) -B $(BUILD) $(CMAKE_FLAGS)
	$(CMAKE) --build $(BUILD) --parallel $(JOBS)
	@echo "Built: $(BUILD)/$(PROJECT)"

wasm: emsdk-check
	$(call EMS_RUN,emcmake $(CMAKE) -B $(WEB) -DAXION_RENDER_SCALE=1 $(CMAKE_FLAGS))
	$(call EMS_RUN,$(CMAKE) --build $(WEB) --parallel $(JOBS))
	@cp favicon.svg $(WEB)/ 2>/dev/null || true
	@cp $(WEB)/$(PROJECT).html $(WEB)/index.html
	@echo "Built: $(WEB)/index.html"

setup-emsdk:
	@if [ -d "$(EMSROOT)" ]; then \
		echo "emsdk already exists at $(EMSROOT)"; \
	else \
		git clone https://github.com/emscripten-core/emsdk.git "$(EMSROOT)"; \
	fi
	cd "$(EMSROOT)" && ./emsdk install latest && ./emsdk activate latest
	@echo ""
	@echo "Emscripten installed to $(EMSROOT)"
	@echo "Run: make wasm"

emsdk-check:
	@bash -lc 'source "$(EMSENV)" 2>/dev/null; command -v emcc >/dev/null' || { \
		echo "Error: emcc not found."; \
		echo "Run: make setup-emsdk"; \
		exit 1; \
	}

run: native
	./$(BUILD)/$(PROJECT)

run-web: wasm
	python3 -m http.server --directory $(WEB)

clean:
	rm -rf $(BUILD) $(WEB)

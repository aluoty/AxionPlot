#include "web_api.h"

#include "display.h"

#include <stddef.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

static DisplaySettings *g_display = 0;

static int FindWindowPreset(int w, int h) {
    static const int presets[][2] = {
        {1280, 720},
        {1600, 900},
        {1920, 1080},
        {2560, 1440}
    };

    for (int i = 0; i < 4; i++) {
        if (presets[i][0] == w && presets[i][1] == h) return i;
    }
    return 2;
}

void WebApiBindDisplay(DisplaySettings *display) {
    g_display = display;
}

void WebApiSyncCanvas(int width, int height, int quality_preset) {
    if (width > 0 && height > 0) {
#if defined(__EMSCRIPTEN__)
        emscripten_set_canvas_element_size("#canvas", width, height);
#endif
        SetWindowSize(width, height);
        if (g_display != 0) {
            g_display->window_width = width;
            g_display->window_height = height;
            g_display->panel_width = DisplayPanelWidth(width);
            g_display->window_preset = FindWindowPreset(width, height);
        }
    }

    if (g_display != 0 && quality_preset >= 0 && quality_preset < 3) {
        DisplayApplyQualityPreset(g_display, quality_preset);
    }

    DisplaySyncInput();
}

#if defined(__EMSCRIPTEN__)
EMSCRIPTEN_KEEPALIVE
void AxionWebConfigure(int width, int height, int quality_preset) {
    WebApiSyncCanvas(width, height, quality_preset);
}
#endif

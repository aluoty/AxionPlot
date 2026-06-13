#include "display.h"

#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

static const int g_window_presets[][2] = {
    {1280, 720},
    {1600, 900},
    {1920, 1080},
    {2560, 1440}
};

static const int g_quality_scales[] = {1, 2, 3};

int DisplayPanelWidth(int screen_w) {
#if defined(PLATFORM_WEB)
    if (screen_w <= 0) return PANEL_WIDTH;

    int panel = (int)(screen_w * 0.28f);
    if (panel < 240) panel = 240;
    if (panel > 300) panel = 300;

    int min_plot = 240;
    if (panel > screen_w - min_plot) panel = screen_w - min_plot;
    if (panel < 200) panel = screen_w / 2;
    return panel;
#else
    (void)screen_w;
    return PANEL_WIDTH;
#endif
}

#if defined(__EMSCRIPTEN__)
static void DisplaySyncCanvasSize(void) {
    int canvas_w = 0;
    int canvas_h = 0;
    if (emscripten_get_canvas_element_size("#canvas", &canvas_w, &canvas_h) != EMSCRIPTEN_RESULT_SUCCESS) return;
    if (canvas_w < 1 || canvas_h < 1) return;
    if (canvas_w != GetScreenWidth() || canvas_h != GetScreenHeight()) {
        emscripten_set_canvas_element_size("#canvas", canvas_w, canvas_h);
        SetWindowSize(canvas_w, canvas_h);
    }
}
#endif

void DisplayInit(DisplaySettings *display, int width, int height, int render_scale) {
    *display = (DisplaySettings){0};
    display->window_width = width;
    display->window_height = height;
    display->panel_width = DisplayPanelWidth(width);
    display->render_scale = render_scale < 1 ? 1 : render_scale;
    display->window_preset = 2;
#if defined(PLATFORM_WEB)
    display->quality_preset = 0;
    display->render_scale = 1;
#else
    display->quality_preset = 1;
#endif
}

void DisplayShutdown(DisplaySettings *display) {
    (void)display;
}

void DisplayEnsureTarget(DisplaySettings *display) {
    (void)display;
}

static int FindWindowPreset(int w, int h) {
    for (int i = 0; i < 4; i++) {
        if (g_window_presets[i][0] == w && g_window_presets[i][1] == h) return i;
    }
    return 2;
}

void DisplayApplyWindowPreset(DisplaySettings *display, int preset) {
    if (preset < 0 || preset >= 4) return;

    display->window_preset = preset;
    display->window_width = g_window_presets[preset][0];
    display->window_height = g_window_presets[preset][1];

    if (display->fullscreen) {
        ToggleFullscreen();
        display->fullscreen = false;
    }

#if defined(__EMSCRIPTEN__)
    emscripten_set_canvas_element_size("#canvas", display->window_width, display->window_height);
#endif
    SetWindowSize(display->window_width, display->window_height);
}

void DisplayApplyQualityPreset(DisplaySettings *display, int preset) {
    if (preset < 0 || preset >= 3) return;

    display->quality_preset = preset;
    display->render_scale = g_quality_scales[preset];
}

void DisplayToggleFullscreen(DisplaySettings *display) {
    ToggleFullscreen();
    display->fullscreen = !display->fullscreen;

    if (!display->fullscreen) {
        SetWindowSize(display->window_width, display->window_height);
    }
}

void DisplaySyncInput(void) {
#if defined(__EMSCRIPTEN__)
    double css_w = 0.0;
    double css_h = 0.0;
    if (emscripten_get_element_css_size("#canvas", &css_w, &css_h) == EMSCRIPTEN_RESULT_SUCCESS &&
        css_w > 0.0 && css_h > 0.0) {
        SetMouseScale((float)GetScreenWidth() / (float)css_w, (float)GetScreenHeight() / (float)css_h);
    }
#endif
}

void DisplayBeginFrame(DisplaySettings *display) {
#if defined(__EMSCRIPTEN__)
    DisplaySyncCanvasSize();
#endif

    int width = GetScreenWidth();
    int height = GetScreenHeight();

    display->panel_width = DisplayPanelWidth(width);

    if (width != display->window_width || height != display->window_height) {
        if (!display->fullscreen) {
            display->window_width = width;
            display->window_height = height;
            display->window_preset = FindWindowPreset(width, height);
        }
    }

    BeginDrawing();
    ClearBackground((Color){6, 8, 18, 255});
}

void DisplayEndFrame(DisplaySettings *display) {
    (void)display;
    EndDrawing();
}

int DisplayRenderScale(const DisplaySettings *display) {
    return display->render_scale;
}

int DisplaySampleCount(const DisplaySettings *display, int plot_width) {
    int base = plot_width > 0 ? plot_width : 800;
    int samples = base * display->render_scale;

#if defined(PLATFORM_WEB)
    int min_samples = base > 0 ? base : 400;
    if (samples < min_samples) samples = min_samples;
    if (samples > 5000) samples = 5000;
#else
    if (samples < 800) samples = 800;
    if (samples > 12000) samples = 12000;
#endif
    return samples;
}

int DisplayFontSize(const DisplaySettings *display, int base) {
    return base + (display->render_scale - 1) * 2;
}

float DisplayLineWidth(const DisplaySettings *display, float base) {
    return base * (0.75f + 0.25f * (float)display->render_scale);
}

const char *DisplayResolutionLabel(const DisplaySettings *display) {
    static char label[64];
    snprintf(label, sizeof(label), "%dx%d @ %dx", display->window_width, display->window_height, display->render_scale);
    return label;
}

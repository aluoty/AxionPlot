#include "display.h"

#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten/html5.h>
#endif

static const int g_window_presets[][2] = {
    {1280, 720},
    {1600, 900},
    {1920, 1080},
    {2560, 1440}
};

static const int g_quality_scales[] = {1, 2, 3};

void DisplayInit(DisplaySettings *display, int width, int height, int render_scale) {
    *display = (DisplaySettings){0};
    display->window_width = width;
    display->window_height = height;
    display->render_scale = render_scale < 1 ? 1 : render_scale;
    display->window_preset = 2;
    display->quality_preset = 1;
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
        return;
    }
#endif
    SetMouseScale(1.0f, 1.0f);
}

void DisplayBeginFrame(DisplaySettings *display) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    if (width != display->window_width || height != display->window_height) {
        if (!display->fullscreen) {
            display->window_width = width;
            display->window_height = height;
            display->window_preset = FindWindowPreset(width, height);
        }
    }

    BeginDrawing();
    ClearBackground((Color){8, 8, 12, 255});
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
    if (samples < 800) samples = 800;
    if (samples > 12000) samples = 12000;
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

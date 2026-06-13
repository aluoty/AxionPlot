#include "display.h"

#include <stdio.h>

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
    display->target = (RenderTexture2D){0};
}

void DisplayShutdown(DisplaySettings *display) {
    if (display->target.id != 0) {
        UnloadRenderTexture(display->target);
        display->target = (RenderTexture2D){0};
    }
}

void DisplayEnsureTarget(DisplaySettings *display) {
    int rw = display->window_width * display->render_scale;
    int rh = display->window_height * display->render_scale;

    if (display->target.id != 0 &&
        display->target.texture.width == rw &&
        display->target.texture.height == rh) {
        return;
    }

    if (display->target.id != 0) {
        UnloadRenderTexture(display->target);
    }

    display->target = LoadRenderTexture(rw, rh);
    SetTextureFilter(display->target.texture, TEXTURE_FILTER_BILINEAR);
}

static int FindWindowPreset(int w, int h) {
    for (int i = 0; i < 4; i++) {
        if (g_window_presets[i][0] == w && g_window_presets[i][1] == h) return i;
    }
    return 2;
}

static int FindQualityPreset(int scale) {
    for (int i = 0; i < 3; i++) {
        if (g_quality_scales[i] == scale) return i;
    }
    return 1;
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
    DisplayEnsureTarget(display);
}

void DisplayApplyQualityPreset(DisplaySettings *display, int preset) {
    if (preset < 0 || preset >= 3) return;

    display->quality_preset = preset;
    display->render_scale = g_quality_scales[preset];
    DisplayEnsureTarget(display);
}

void DisplayToggleFullscreen(DisplaySettings *display) {
    ToggleFullscreen();
    display->fullscreen = !display->fullscreen;

    if (!display->fullscreen) {
        SetWindowSize(display->window_width, display->window_height);
    }

    DisplayEnsureTarget(display);
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

    DisplayEnsureTarget(display);

    BeginTextureMode(display->target);
    ClearBackground((Color){8, 8, 12, 255});

    Camera2D cam = {0};
    cam.zoom = (float)display->render_scale;
    cam.offset = (Vector2){0.0f, 0.0f};
    cam.target = (Vector2){0.0f, 0.0f};
    BeginMode2D(cam);
}

void DisplayEndFrame(DisplaySettings *display) {
    EndMode2D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    int width = GetScreenWidth();
    int height = GetScreenHeight();
    DrawTexturePro(
        display->target.texture,
        (Rectangle){0.0f, 0.0f, (float)display->target.texture.width, -(float)display->target.texture.height},
        (Rectangle){0.0f, 0.0f, (float)width, (float)height},
        (Vector2){0.0f, 0.0f},
        0.0f,
        WHITE
    );
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
    return base * display->render_scale;
}

float DisplayLineWidth(const DisplaySettings *display, float base) {
    return base * (float)display->render_scale;
}

const char *DisplayResolutionLabel(const DisplaySettings *display) {
    static char label[64];
    snprintf(label, sizeof(label), "%dx%d @ %dx", display->window_width, display->window_height, display->render_scale);
    return label;
}

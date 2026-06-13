#ifndef DISPLAY_H
#define DISPLAY_H

#include "axion_plot.h"

typedef struct {
    int window_width;
    int window_height;
    int render_scale;
    int window_preset;
    int quality_preset;
    bool fullscreen;
    RenderTexture2D target;
} DisplaySettings;

void DisplayInit(DisplaySettings *display, int width, int height, int render_scale);
void DisplayShutdown(DisplaySettings *display);
void DisplayApplyWindowPreset(DisplaySettings *display, int preset);
void DisplayApplyQualityPreset(DisplaySettings *display, int preset);
void DisplayToggleFullscreen(DisplaySettings *display);
void DisplayEnsureTarget(DisplaySettings *display);

void DisplayBeginFrame(DisplaySettings *display);
void DisplayEndFrame(DisplaySettings *display);

int DisplayRenderScale(const DisplaySettings *display);
int DisplaySampleCount(const DisplaySettings *display, int plot_width);
int DisplayFontSize(const DisplaySettings *display, int base);
float DisplayLineWidth(const DisplaySettings *display, float base);

const char *DisplayResolutionLabel(const DisplaySettings *display);

#endif

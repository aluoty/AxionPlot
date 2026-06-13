#include "axion_plot.h"
#include "camera.h"
#include "display.h"
#include "graph.h"
#include "ui.h"
#include "web_api.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/html5.h>
#endif

#ifndef AXION_WINDOW_WIDTH
#define AXION_WINDOW_WIDTH 1280
#endif
#ifndef AXION_WINDOW_HEIGHT
#define AXION_WINDOW_HEIGHT 720
#endif
#ifndef AXION_RENDER_SCALE
#define AXION_RENDER_SCALE 2
#endif

int main(void) {
    unsigned int config = FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT;
#if defined(PLATFORM_WEB)
    config &= ~(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
#endif
    SetConfigFlags(config);

    int win_w = AXION_WINDOW_WIDTH;
    int win_h = AXION_WINDOW_HEIGHT;
#if defined(__EMSCRIPTEN__)
    emscripten_get_canvas_element_size("#canvas", &win_w, &win_h);
    if (win_w < 320) win_w = AXION_WINDOW_WIDTH;
    if (win_h < 240) win_h = AXION_WINDOW_HEIGHT;
#endif
    InitWindow(win_w, win_h, "AxionPlot - Graphing Calculator");
    SetTargetFPS(60);

    if (GetScreenWidth() < 1 || GetScreenHeight() < 1) {
        TraceLog(LOG_ERROR, "Invalid window size after InitWindow");
        return 1;
    }

    DisplaySettings display;
    DisplayInit(&display, GetScreenWidth(), GetScreenHeight(), AXION_RENDER_SCALE);
    WebApiBindDisplay(&display);
#if defined(__EMSCRIPTEN__)
    WebApiSyncCanvas(GetScreenWidth(), GetScreenHeight(), display.quality_preset);
#endif

    PlotCamera camera = {0.0f, 0.0f, 60.0f};
    PlotVars vars = {1.0, 1.0, 1.0};
    GraphList graphs;
    GraphListInit(&graphs);
    UiInit();
    CameraResetForViewport(&camera, GetScreenWidth(), GetScreenHeight(), display.panel_width);

    while (!WindowShouldClose()) {
        DisplaySyncInput();

        int width = GetScreenWidth();
        int height = GetScreenHeight();
        int panel_w = display.panel_width;
        Vector2 mouse = GetMousePosition();
        Vector2 world = PlotScreenToWorld(&camera, mouse.x, mouse.y, width, height, panel_w);

        if (!UiWantsInput()) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && mouse.x > panel_w) {
                Vector2 delta = GetMouseDelta();
                CameraPan(&camera, -delta.x / camera.scale, delta.y / camera.scale);
            }

            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f && mouse.x > panel_w) {
                float factor = wheel > 0.0f ? 1.1f : 0.9f;
                CameraZoom(&camera, mouse.x, mouse.y, factor, width, height, panel_w);
            }
        }

        if (IsKeyPressed(KEY_F11)) {
            DisplayToggleFullscreen(&display);
        }

        DisplayBeginFrame(&display);
        DrawPlotGrid(&camera, &display, width, height);
        GraphListDraw(&graphs, &camera, &vars, &display, width, height);
        if (UiTraceEnabled()) {
            GraphListDrawTrace(&graphs, &camera, &vars, UiTraceX(), &display, width, height);
        }
        UiFrame(&graphs, &vars, &camera, &display, world);
        DisplayEndFrame(&display);
    }

    DisplayShutdown(&display);
    GraphListFree(&graphs);
    CloseWindow();
    return 0;
}

#include "axion_plot.h"
#include "camera.h"
#include "display.h"
#include "graph.h"
#include "ui.h"

#ifndef AXION_WINDOW_WIDTH
#define AXION_WINDOW_WIDTH 1920
#endif
#ifndef AXION_WINDOW_HEIGHT
#define AXION_WINDOW_HEIGHT 1080
#endif
#ifndef AXION_RENDER_SCALE
#define AXION_RENDER_SCALE 2
#endif

int main(void) {
    unsigned int config = FLAG_WINDOW_RESIZABLE;
#if !defined(PLATFORM_WEB)
    config |= FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI;
#endif
    SetConfigFlags(config);
    InitWindow(AXION_WINDOW_WIDTH, AXION_WINDOW_HEIGHT, "AxionPlot - Graphing Calculator");
    SetTargetFPS(60);

    DisplaySettings display;
    DisplayInit(&display, GetScreenWidth(), GetScreenHeight(), AXION_RENDER_SCALE);

    PlotCamera camera = {0.0f, 0.0f, 60.0f};
    PlotVars vars = {1.0, 1.0, 1.0};
    GraphList graphs;
    GraphListInit(&graphs);
    UiInit();

    while (!WindowShouldClose()) {
        DisplaySyncInput();

        int width = GetScreenWidth();
        int height = GetScreenHeight();
        Vector2 mouse = GetMousePosition();
        Vector2 world = PlotScreenToWorld(&camera, mouse.x, mouse.y, width, height);

        if (!UiWantsInput()) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && mouse.x > PANEL_WIDTH) {
                Vector2 delta = GetMouseDelta();
                CameraPan(&camera, -delta.x / camera.scale, delta.y / camera.scale);
            }

            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f && mouse.x > PANEL_WIDTH) {
                float factor = wheel > 0.0f ? 1.1f : 0.9f;
                CameraZoom(&camera, mouse.x, mouse.y, factor, width, height);
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

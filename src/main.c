#include "axion_plot.h"
#include "camera.h"
#include "graph.h"
#include "ui.h"

int main(void) {
    const int screen_w = 1280;
    const int screen_h = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screen_w, screen_h, "AxionPlot - Graphing Calculator");
    SetTargetFPS(60);

    PlotCamera camera = {0.0f, 0.0f, 60.0f};
    PlotVars vars = {1.0, 1.0, 1.0};
    GraphList graphs;
    GraphListInit(&graphs);
    UiInit();

    while (!WindowShouldClose()) {
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

        BeginDrawing();
        ClearBackground((Color){8, 8, 12, 255});
        DrawPlotGrid(&camera, width, height);
        GraphListDraw(&graphs, &camera, &vars, width, height);
        if (UiTraceEnabled()) {
            GraphListDrawTrace(&graphs, &camera, &vars, UiTraceX(), width, height);
        }
        UiFrame(&graphs, &vars, &camera, world);
        EndDrawing();
    }

    GraphListFree(&graphs);
    CloseWindow();
    return 0;
}

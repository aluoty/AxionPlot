#include "camera.h"

Vector2 PlotWorldToScreen(const PlotCamera *cam, float x, float y, int screen_w, int screen_h, int panel_w) {
    float plot_w = (float)(screen_w - panel_w);
    return (Vector2){
        (x - cam->cam_x) * cam->scale + panel_w + plot_w * 0.5f,
        screen_h * 0.5f - (y - cam->cam_y) * cam->scale
    };
}

Vector2 PlotScreenToWorld(const PlotCamera *cam, float sx, float sy, int screen_w, int screen_h, int panel_w) {
    float plot_w = (float)(screen_w - panel_w);
    return (Vector2){
        (sx - panel_w - plot_w * 0.5f) / cam->scale + cam->cam_x,
        (screen_h * 0.5f - sy) / cam->scale + cam->cam_y
    };
}

void CameraPan(PlotCamera *cam, float dx, float dy) {
    cam->cam_x += dx;
    cam->cam_y += dy;
}

void CameraZoom(PlotCamera *cam, float mouse_sx, float mouse_sy, float factor, int screen_w, int screen_h, int panel_w) {
    Vector2 before = PlotScreenToWorld(cam, mouse_sx, mouse_sy, screen_w, screen_h, panel_w);
    cam->scale *= factor;
    Vector2 after = PlotScreenToWorld(cam, mouse_sx, mouse_sy, screen_w, screen_h, panel_w);
    cam->cam_x += before.x - after.x;
    cam->cam_y += before.y - after.y;
}

void CameraReset(PlotCamera *cam) {
    cam->cam_x = 0.0f;
    cam->cam_y = 0.0f;
    cam->scale = 60.0f;
}

void CameraResetForViewport(PlotCamera *cam, int screen_w, int screen_h, int panel_w) {
    cam->cam_x = 0.0f;
    cam->cam_y = 0.0f;

    int plot_h = screen_h > 0 ? screen_h : 480;
    cam->scale = (float)plot_h / 10.0f;
    if (cam->scale < 20.0f) cam->scale = 20.0f;
    if (cam->scale > 120.0f) cam->scale = 120.0f;

    (void)screen_w;
    (void)panel_w;
}

void CameraGetBounds(const PlotCamera *cam, int screen_w, int screen_h, int panel_w, float *x_min, float *x_max, float *y_min, float *y_max) {
    Vector2 tl = PlotScreenToWorld(cam, (float)panel_w, 0.0f, screen_w, screen_h, panel_w);
    Vector2 br = PlotScreenToWorld(cam, (float)screen_w, (float)screen_h, screen_w, screen_h, panel_w);
    *x_min = tl.x;
    *x_max = br.x;
    *y_min = br.y;
    *y_max = tl.y;
}

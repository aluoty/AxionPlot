#ifndef CAMERA_H
#define CAMERA_H

#include "axion_plot.h"

Vector2 PlotWorldToScreen(const PlotCamera *cam, float x, float y, int screen_w, int screen_h, int panel_w);
Vector2 PlotScreenToWorld(const PlotCamera *cam, float sx, float sy, int screen_w, int screen_h, int panel_w);
void CameraPan(PlotCamera *cam, float dx, float dy);
void CameraZoom(PlotCamera *cam, float mouse_sx, float mouse_sy, float factor, int screen_w, int screen_h, int panel_w);
void CameraReset(PlotCamera *cam);
void CameraResetForViewport(PlotCamera *cam, int screen_w, int screen_h, int panel_w);
void CameraGetBounds(const PlotCamera *cam, int screen_w, int screen_h, int panel_w, float *x_min, float *x_max, float *y_min, float *y_max);

#endif

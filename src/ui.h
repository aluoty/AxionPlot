#ifndef UI_H
#define UI_H

#include "axion_plot.h"
#include "display.h"

void UiInit(void);
bool UiWantsInput(void);
bool UiTraceEnabled(void);
float UiTraceX(void);
void UiFrame(GraphList *graphs, PlotVars *vars, PlotCamera *camera, DisplaySettings *display, Vector2 cursor_world);

#endif

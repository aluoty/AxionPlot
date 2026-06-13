#ifndef GRAPH_H
#define GRAPH_H

#include "axion_plot.h"

void GraphListInit(GraphList *list);
void GraphListFree(GraphList *list);
bool GraphListAdd(GraphList *list, PlotMode mode, const char *expr_a, const char *expr_b, PlotVars *vars, char *error_msg, int error_size);
bool GraphListRemove(GraphList *list, int id);
bool GraphListToggle(GraphList *list, int id);
void GraphListClear(GraphList *list);
void GraphListDraw(const GraphList *list, const PlotCamera *cam, PlotVars *vars, int screen_w, int screen_h);
void GraphListDrawTrace(const GraphList *list, const PlotCamera *cam, PlotVars *vars, float trace_x, int screen_w, int screen_h);
void DrawPlotGrid(const PlotCamera *cam, int screen_w, int screen_h);

#endif

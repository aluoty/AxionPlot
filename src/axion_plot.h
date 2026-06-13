#ifndef AXION_PLOT_H
#define AXION_PLOT_H

#include "raylib.h"
#include "tinyexpr.h"

#define MAX_GRAPHS 32
#define EXPR_MAX_LEN 256
#define PANEL_WIDTH 360
#define PANEL_PADDING 10

typedef struct {
    float cam_x;
    float cam_y;
    float scale;
} PlotCamera;

typedef struct {
    double a;
    double b;
    double c;
} PlotVars;

typedef enum {
    PLOT_CARTESIAN = 0,
    PLOT_PARAMETRIC,
    PLOT_POLAR
} PlotMode;

typedef struct {
    int id;
    PlotMode mode;
    char label[EXPR_MAX_LEN];
    char expr_a[EXPR_MAX_LEN];
    char expr_b[EXPR_MAX_LEN];
    Color color;
    bool visible;
    te_expr *compiled_a;
    te_expr *compiled_b;
} PlotGraph;

typedef struct {
    PlotGraph items[MAX_GRAPHS];
    int count;
    int id_counter;
} GraphList;

typedef enum {
    PARSE_ERROR = 0,
    PARSE_VAR,
    PARSE_GRAPH
} ParseResult;

typedef enum {
    UI_TAB_GRAPH = 0,
    UI_TAB_CALC,
    UI_TAB_TABLE,
    UI_TAB_DISPLAY
} UiTab;

#endif

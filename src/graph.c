#include "graph.h"

#include "camera.h"
#include "expr.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static Color GraphColor(int id) {
    return ColorFromHSV((float)((id * 137) % 360), 0.85f, 0.75f);
}

static void MakeLabel(PlotMode mode, const char *a, const char *b, char *out, int out_size) {
    if (mode == PLOT_PARAMETRIC) {
        snprintf(out, out_size, "(%s, %s)", a, b);
    } else if (mode == PLOT_POLAR) {
        snprintf(out, out_size, "r=%s", a);
    } else {
        snprintf(out, out_size, "y=%s", a);
    }
}

static double NiceStep(double range) {
    if (range <= 0.0) return 1.0;
    double rough = range / 8.0;
    double mag = pow(10.0, floor(log10(rough)));
    double norm = rough / mag;
    if (norm < 1.5) return mag;
    if (norm < 3.5) return 2.0 * mag;
    if (norm < 7.5) return 5.0 * mag;
    return 10.0 * mag;
}

static void FormatTick(double value, char *buf, int size) {
    if (fabs(value) < 1e-9) {
        snprintf(buf, size, "0");
    } else if (fabs(value) >= 10000.0 || (fabs(value) < 0.001 && value != 0.0)) {
        snprintf(buf, size, "%.2e", value);
    } else {
        snprintf(buf, size, "%g", value);
    }
}

void GraphListInit(GraphList *list) {
    memset(list, 0, sizeof(*list));
}

void GraphListFree(GraphList *list) {
    for (int i = 0; i < list->count; i++) {
        ExprFree(list->items[i].compiled_a);
        ExprFree(list->items[i].compiled_b);
        list->items[i].compiled_a = NULL;
        list->items[i].compiled_b = NULL;
    }
    list->count = 0;
}

bool GraphListAdd(GraphList *list, PlotMode mode, const char *expr_a, const char *expr_b, PlotVars *vars, char *error_msg, int error_size) {
    if (list->count >= MAX_GRAPHS) {
        snprintf(error_msg, error_size, "Max graphs reached");
        return false;
    }

    int error = 0;
    te_expr *compiled_a = ExprCompile(expr_a, vars, &error);
    if (compiled_a == NULL) {
        snprintf(error_msg, error_size, "%s", ExprErrorString(error));
        return false;
    }

    te_expr *compiled_b = NULL;
    if (mode == PLOT_PARAMETRIC) {
        compiled_b = ExprCompile(expr_b, vars, &error);
        if (compiled_b == NULL) {
            ExprFree(compiled_a);
            snprintf(error_msg, error_size, "%s", ExprErrorString(error));
            return false;
        }
    }

    PlotGraph *graph = &list->items[list->count++];
    graph->id = list->id_counter++;
    graph->mode = mode;
    graph->visible = true;
    snprintf(graph->expr_a, sizeof(graph->expr_a), "%s", expr_a);
    snprintf(graph->expr_b, sizeof(graph->expr_b), "%s", expr_b ? expr_b : "");
    MakeLabel(mode, expr_a, expr_b, graph->label, sizeof(graph->label));
    graph->color = GraphColor(graph->id);
    graph->compiled_a = compiled_a;
    graph->compiled_b = compiled_b;
    error_msg[0] = '\0';
    return true;
}

bool GraphListRemove(GraphList *list, int id) {
    int index = -1;
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].id == id) {
            index = i;
            break;
        }
    }
    if (index < 0) return false;

    ExprFree(list->items[index].compiled_a);
    ExprFree(list->items[index].compiled_b);
    for (int i = index; i < list->count - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    list->count--;
    return true;
}

bool GraphListToggle(GraphList *list, int id) {
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].id == id) {
            list->items[i].visible = !list->items[i].visible;
            return true;
        }
    }
    return false;
}

void GraphListClear(GraphList *list) {
    GraphListFree(list);
}

static void DrawPolyline(const PlotCamera *cam, const DisplaySettings *display, int screen_w, int screen_h, Color color,
                         int count, double (*sample_x)(int i, void *ctx), double (*sample_y)(int i, void *ctx), void *ctx) {
    bool first = true;
    Vector2 prev = {0};
    float thickness = DisplayLineWidth(display, 1.8f);

    for (int i = 0; i < count; i++) {
        double wx = sample_x(i, ctx);
        double wy = sample_y(i, ctx);
        if (!isfinite(wx) || !isfinite(wy)) {
            first = true;
            continue;
        }

        Vector2 point = PlotWorldToScreen(cam, (float)wx, (float)wy, screen_w, screen_h);
        if (first) {
            prev = point;
            first = false;
        } else {
            DrawLineEx(prev, point, thickness, color);
            prev = point;
        }
    }
}

typedef struct {
    const PlotGraph *graph;
    PlotVars *vars;
    double t_min;
    double t_max;
    int samples;
} ParamCtx;

static double ParamSampleX(int i, void *ctx) {
    ParamCtx *p = (ParamCtx *)ctx;
    double denom = p->samples > 1 ? (double)(p->samples - 1) : 1.0;
    double t = p->t_min + (p->t_max - p->t_min) * i / denom;
    return ExprEval(p->graph->compiled_a, 0.0, t, p->vars);
}

static double ParamSampleY(int i, void *ctx) {
    ParamCtx *p = (ParamCtx *)ctx;
    double denom = p->samples > 1 ? (double)(p->samples - 1) : 1.0;
    double t = p->t_min + (p->t_max - p->t_min) * i / denom;
    return ExprEval(p->graph->compiled_b, 0.0, t, p->vars);
}

typedef struct {
    const PlotGraph *graph;
    PlotVars *vars;
    int samples;
} PolarCtx;

static double PolarSampleX(int i, void *ctx) {
    PolarCtx *p = (PolarCtx *)ctx;
    double denom = p->samples > 1 ? (double)(p->samples - 1) : 1.0;
    double t = 6.28318530718 * i / denom;
    double r = ExprEval(p->graph->compiled_a, 0.0, t, p->vars);
    return r * cos(t);
}

static double PolarSampleY(int i, void *ctx) {
    PolarCtx *p = (PolarCtx *)ctx;
    double denom = p->samples > 1 ? (double)(p->samples - 1) : 1.0;
    double t = 6.28318530718 * i / denom;
    double r = ExprEval(p->graph->compiled_a, 0.0, t, p->vars);
    return r * sin(t);
}

typedef struct {
    const PlotGraph *graph;
    PlotVars *vars;
    const PlotCamera *cam;
    int screen_w;
    int screen_h;
} CartCtx;

static double CartSampleX(int i, void *ctx) {
    CartCtx *p = (CartCtx *)ctx;
    return PlotScreenToWorld(p->cam, (float)i + (float)PANEL_WIDTH, 0.0f, p->screen_w, p->screen_h).x;
}

static double CartSampleY(int i, void *ctx) {
    CartCtx *p = (CartCtx *)ctx;
    double x = CartSampleX(i, ctx);
    return ExprEval(p->graph->compiled_a, x, 0.0, p->vars);
}

void DrawPlotGrid(const PlotCamera *cam, const DisplaySettings *display, int screen_w, int screen_h) {
    float x_min, x_max, y_min, y_max;
    CameraGetBounds(cam, screen_w, screen_h, &x_min, &x_max, &y_min, &y_max);

    float x_step = (float)NiceStep(x_max - x_min);
    float y_step = (float)NiceStep(y_max - y_min);

    float x_start = floorf(x_min / x_step) * x_step;
    float y_start = floorf(y_min / y_step) * y_step;
    int label_fs = DisplayFontSize(display, 12);
    float grid_w = DisplayLineWidth(display, 1.0f);
    float axis_w = DisplayLineWidth(display, 1.5f);

    for (float x = x_start; x <= x_max; x += x_step) {
        Vector2 top = PlotWorldToScreen(cam, x, y_max, screen_w, screen_h);
        Vector2 bot = PlotWorldToScreen(cam, x, y_min, screen_w, screen_h);
        bool axis = fabsf(x) < x_step * 0.01f;
        Color c = axis ? (Color){80, 80, 80, 255} : (Color){24, 24, 24, 255};
        DrawLineEx((Vector2){top.x, 0}, (Vector2){bot.x, (float)screen_h}, axis ? axis_w : grid_w, c);

        if (top.x >= PANEL_WIDTH && top.x <= screen_w) {
            char label[32];
            FormatTick(x, label, sizeof(label));
            DrawText(label, (int)top.x + 2, screen_h - label_fs - 4, label_fs, (Color){140, 140, 140, 255});
        }
    }

    for (float y = y_start; y <= y_max; y += y_step) {
        Vector2 left = PlotWorldToScreen(cam, x_min, y, screen_w, screen_h);
        Vector2 right = PlotWorldToScreen(cam, x_max, y, screen_w, screen_h);
        bool axis = fabsf(y) < y_step * 0.01f;
        Color c = axis ? (Color){80, 80, 80, 255} : (Color){24, 24, 24, 255};
        DrawLineEx((Vector2){(float)PANEL_WIDTH, left.y}, (Vector2){(float)screen_w, right.y}, axis ? axis_w : grid_w, c);

        if (left.y >= 0 && left.y <= screen_h - 20) {
            char label[32];
            FormatTick(y, label, sizeof(label));
            DrawText(label, PANEL_WIDTH + 4, (int)left.y - label_fs, label_fs, (Color){140, 140, 140, 255});
        }
    }

    Vector2 origin = PlotWorldToScreen(cam, 0.0f, 0.0f, screen_w, screen_h);
    if (origin.x >= PANEL_WIDTH && origin.x <= screen_w && origin.y >= 0 && origin.y <= screen_h) {
        DrawCircleV(origin, DisplayLineWidth(display, 3.0f), (Color){0, 255, 204, 180});
    }
}

void GraphListDraw(const GraphList *list, const PlotCamera *cam, PlotVars *vars, const DisplaySettings *display, int screen_w, int screen_h) {
    int plot_w = screen_w - PANEL_WIDTH;
    int curve_samples = DisplaySampleCount(display, plot_w);

    for (int g = 0; g < list->count; g++) {
        const PlotGraph *graph = &list->items[g];
        if (!graph->visible) continue;

        if (graph->mode == PLOT_PARAMETRIC) {
            ParamCtx ctx = {graph, vars, -6.28318530718, 6.28318530718, curve_samples};
            DrawPolyline(cam, display, screen_w, screen_h, graph->color, curve_samples, ParamSampleX, ParamSampleY, &ctx);
        } else if (graph->mode == PLOT_POLAR) {
            PolarCtx ctx = {graph, vars, curve_samples};
            DrawPolyline(cam, display, screen_w, screen_h, graph->color, curve_samples, PolarSampleX, PolarSampleY, &ctx);
        } else {
            CartCtx ctx = {graph, vars, cam, screen_w, screen_h};
            if (plot_w < 1) continue;
            DrawPolyline(cam, display, screen_w, screen_h, graph->color, curve_samples, CartSampleX, CartSampleY, &ctx);
        }
    }
}

void GraphListDrawTrace(const GraphList *list, const PlotCamera *cam, PlotVars *vars, float trace_x, const DisplaySettings *display, int screen_w, int screen_h) {
    Vector2 top = PlotWorldToScreen(cam, trace_x, 0.0f, screen_w, screen_h);
    DrawLineEx((Vector2){top.x, 0}, (Vector2){top.x, (float)screen_h}, DisplayLineWidth(display, 1.0f), (Color){60, 60, 60, 255});

    for (int g = 0; g < list->count; g++) {
        const PlotGraph *graph = &list->items[g];
        if (!graph->visible || graph->mode != PLOT_CARTESIAN) continue;

        double y = ExprEval(graph->compiled_a, trace_x, 0.0, vars);
        if (!isfinite(y)) continue;

        Vector2 pt = PlotWorldToScreen(cam, trace_x, (float)y, screen_w, screen_h);
        float r = DisplayLineWidth(display, 5.0f);
        DrawCircleV(pt, r, graph->color);

        char info[64];
        snprintf(info, sizeof(info), "(%.3g, %.3g)", trace_x, y);
        DrawText(info, (int)pt.x + 8, (int)pt.y - 8, DisplayFontSize(display, 12), graph->color);
    }
}

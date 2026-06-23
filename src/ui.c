#include "ui.h"

#include "camera.h"
#include "expr.h"
#include "graph.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

static char g_input_a[EXPR_MAX_LEN] = {0};
static char g_input_b[EXPR_MAX_LEN] = {0};
static char g_calc_input[EXPR_MAX_LEN] = {0};
static char g_status[128] = "Enter an expression or use the keypad";
static char g_calc_result[64] = {0};
static bool g_edit_a = false;
static bool g_edit_b = false;
static bool g_edit_calc = false;
static bool g_trace = false;
static float g_trace_x = 0.0f;
static UiTab g_tab = UI_TAB_GRAPH;
static PlotMode g_plot_mode = PLOT_CARTESIAN;
static int g_table_graph = 0;
static int g_table_scroll = 0;

static void AppendText(char *buf, int size, const char *text) {
    int len = (int)strlen(buf);
    int add = (int)strlen(text);
    if (len + add >= size - 1) return;
    strcat(buf, text);
}

static void Backspace(char *buf) {
    int len = (int)strlen(buf);
    if (len > 0) buf[len - 1] = '\0';
}

static char *ActiveBuffer(void) {
    if (g_tab == UI_TAB_CALC) return g_calc_input;
    if (g_plot_mode == PLOT_PARAMETRIC && g_edit_b) return g_input_b;
    return g_input_a;
}

static void InsertToken(const char *token) {
    AppendText(ActiveBuffer(), EXPR_MAX_LEN, token);
}

static void ApplyVarAssignment(const char *expr, PlotVars *vars) {
    char var_name[8];
    char value_expr[EXPR_MAX_LEN];
    ParseResult result = ExprParseInput(expr, value_expr, sizeof(value_expr), var_name, sizeof(var_name));
    if (result != PARSE_VAR) return;

    double value = ExprEvalValue(value_expr, vars);
    if (!isfinite(value)) {
        snprintf(g_status, sizeof(g_status), "Invalid value for %s", var_name);
        return;
    }

    if (strcmp(var_name, "a") == 0) vars->a = value;
    else if (strcmp(var_name, "b") == 0) vars->b = value;
    else if (strcmp(var_name, "c") == 0) vars->c = value;

    snprintf(g_status, sizeof(g_status), "%s = %g", var_name, value);
}

static void AddGraph(GraphList *graphs, PlotVars *vars) {
    char error[64] = {0};

    if (g_input_a[0] == '\0') {
        snprintf(g_status, sizeof(g_status), "Enter an expression");
        return;
    }

    char expr_a[EXPR_MAX_LEN];
    char var_name[8];
    ParseResult parsed = ExprParseInput(g_input_a, expr_a, sizeof(expr_a), var_name, sizeof(var_name));
    if (parsed == PARSE_VAR) {
        ApplyVarAssignment(g_input_a, vars);
        g_input_a[0] = '\0';
        return;
    }

    const char *a = g_input_a;
    const char *b = g_input_b;
    if (parsed == PARSE_GRAPH) a = expr_a;

    if (g_plot_mode == PLOT_PARAMETRIC) {
        if (g_input_b[0] == '\0') {
            snprintf(g_status, sizeof(g_status), "Enter both x(t) and y(t)");
            return;
        }
    }

    if (GraphListAdd(graphs, g_plot_mode, a, b, vars, error, sizeof(error))) {
        snprintf(g_status, sizeof(g_status), "Added graph");
        g_input_a[0] = '\0';
        g_input_b[0] = '\0';
    } else {
        snprintf(g_status, sizeof(g_status), "%s", error[0] ? error : "Could not add graph");
    }
}

static void EvaluateCalc(PlotVars *vars) {
    if (g_calc_input[0] == '\0') return;

    char expr[EXPR_MAX_LEN];
    char var_name[8];
    ParseResult result = ExprParseInput(g_calc_input, expr, sizeof(expr), var_name, sizeof(var_name));

    if (result == PARSE_VAR) {
        ApplyVarAssignment(g_calc_input, vars);
        g_calc_input[0] = '\0';
        g_calc_result[0] = '\0';
        return;
    }

    double value = ExprEvalValue(expr, vars);
    if (isfinite(value)) {
        snprintf(g_calc_result, sizeof(g_calc_result), "= %g", value);
        snprintf(g_status, sizeof(g_status), "Result computed");
    } else {
        g_calc_result[0] = '\0';
        snprintf(g_status, sizeof(g_status), "Invalid expression");
    }
}

static void DrawKeypad(int x, int y, int w) {
    const int cols = 4;
    const int btn_h = 26;
    const int gap = 4;
    int btn_w = (w - gap * (cols - 1)) / cols;

    const char *keys[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", ".", "(", ")",
        "sin(", "cos(", "tan(", "sqrt(",
        "ln(", "log(", "abs(", "^",
        "pi", "e", "x", "t",
        "a", "b", "c", "del"
    };

    int row = 0;
    int col = 0;
    for (int i = 0; i < (int)(sizeof(keys) / sizeof(keys[0])); i++) {
        Rectangle btn = {
            (float)(x + col * (btn_w + gap)),
            (float)(y + row * (btn_h + gap)),
            (float)btn_w,
            (float)btn_h
        };

        if (GuiButton(btn, keys[i])) {
            if (strcmp(keys[i], "del") == 0) {
                Backspace(ActiveBuffer());
            } else if (strcmp(keys[i], "pi") == 0) {
                InsertToken("pi");
            } else {
                InsertToken(keys[i]);
            }
        }

        col++;
        if (col >= cols) {
            col = 0;
            row++;
        }
    }
}

static void DrawGraphTab(GraphList *graphs, PlotVars *vars, PlotCamera *camera, int panel_w, int inner_w, int *y) {
    const char *mode_names = "Cartesian;Parametric;Polar";
    GuiComboBox((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 24}, mode_names, (int *)&g_plot_mode);
    *y += 30;

    Rectangle box_a = {PANEL_PADDING, (float)*y, (float)inner_w - 70, 28};
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_edit_a = CheckCollisionPointRec(GetMousePosition(), box_a);
        if (g_edit_a) g_edit_b = false;
    }
    GuiTextBox(box_a, g_input_a, EXPR_MAX_LEN, g_edit_a);
    if (GuiButton((Rectangle){(float)(panel_w - PANEL_PADDING - 60), (float)*y, 60, 28}, "Plot")) {
        AddGraph(graphs, vars);
    }
    *y += 34;

    if (g_plot_mode == PLOT_PARAMETRIC) {
        Rectangle box_b = {PANEL_PADDING, (float)*y, (float)inner_w, 28};
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            g_edit_b = CheckCollisionPointRec(GetMousePosition(), box_b);
            if (g_edit_b) g_edit_a = false;
        }
        GuiTextBox(box_b, g_input_b, EXPR_MAX_LEN, g_edit_b);
        *y += 34;
    }

    if (GuiButton((Rectangle){PANEL_PADDING, (float)*y, (float)(inner_w / 2 - 2), 24}, g_trace ? "Trace ON" : "Trace")) {
        g_trace = !g_trace;
    }
    if (GuiButton((Rectangle){PANEL_PADDING + inner_w / 2 + 2, (float)*y, (float)(inner_w / 2 - 2), 24}, "Home")) {
        CameraResetForViewport(camera, GetScreenWidth(), GetScreenHeight(), panel_w);
    }
    *y += 30;

    if (GuiButton((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 24}, "Clear all graphs")) {
        GraphListClear(graphs);
        snprintf(g_status, sizeof(g_status), "Cleared graphs");
    }
    *y += 30;

    char vars_text[96];
    snprintf(vars_text, sizeof(vars_text), "a=%g  b=%g  c=%g", vars->a, vars->b, vars->c);
    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20}, vars_text);
    *y += 24;

    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20}, "Graphs:");
    *y += 22;

    for (int i = 0; i < graphs->count; i++) {
        const PlotGraph *graph = &graphs->items[i];
        DrawRectangle(PANEL_PADDING, *y + 4, 3, 20, graph->color);

        char vis_id[32];
        snprintf(vis_id, sizeof(vis_id), graph->visible ? "#v%d" : "#h%d", graph->id);
        if (GuiButton((Rectangle){(float)(PANEL_PADDING + 8), (float)*y, 24, 28}, vis_id)) {
            GraphListToggle(graphs, graph->id);
        }

        char del_id[32];
        snprintf(del_id, sizeof(del_id), "x##%d", graph->id);
        if (GuiButton((Rectangle){(float)(panel_w - PANEL_PADDING - 28), (float)*y, 28, 28}, del_id)) {
            GraphListRemove(graphs, graph->id);
        }

        Color text_color = graph->visible ? (Color){190, 235, 220, 255} : (Color){100, 110, 125, 255};
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(text_color));
        GuiLabel((Rectangle){PANEL_PADDING + 36, (float)*y, (float)inner_w - 72, 28}, graph->label);
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt((Color){190, 235, 220, 255}));
        *y += 32;
    }
}

static void DrawCalcTab(PlotVars *vars, int panel_w, int inner_w, int *y) {
    Rectangle box = {PANEL_PADDING, (float)*y, (float)inner_w - 70, 28};
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_edit_calc = CheckCollisionPointRec(GetMousePosition(), box);
        if (g_edit_calc) {
            g_edit_a = false;
            g_edit_b = false;
        }
    }
    GuiTextBox(box, g_calc_input, EXPR_MAX_LEN, g_edit_calc);
    if (GuiButton((Rectangle){(float)(panel_w - PANEL_PADDING - 60), (float)*y, 60, 28}, "=")) {
        EvaluateCalc(vars);
    }
    *y += 34;

    if (g_calc_result[0] != '\0') {
        GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 24}, g_calc_result);
        *y += 28;
    }

    char vars_text[96];
    snprintf(vars_text, sizeof(vars_text), "a=%g  b=%g  c=%g", vars->a, vars->b, vars->c);
    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20}, vars_text);
    *y += 24;
}

static void DrawTableTab(const GraphList *graphs, PlotVars *vars, int inner_w, int *y) {
    if (graphs->count == 0) {
        GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20}, "No graphs to tabulate");
        *y += 24;
        return;
    }

    if (g_table_graph >= graphs->count) g_table_graph = 0;

    const char *names[32];
    char name_bufs[32][EXPR_MAX_LEN];
    for (int i = 0; i < graphs->count && i < 32; i++) {
        snprintf(name_bufs[i], sizeof(name_bufs[i]), "%s", graphs->items[i].label);
        names[i] = name_bufs[i];
    }

    char combo[512] = {0};
    for (int i = 0; i < graphs->count && i < 32; i++) {
        if (i > 0) strncat(combo, ";", sizeof(combo) - strlen(combo) - 1);
        strncat(combo, names[i], sizeof(combo) - strlen(combo) - 1);
    }

    GuiComboBox((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 24}, combo, &g_table_graph);
    *y += 30;

    const PlotGraph *graph = &graphs->items[g_table_graph];
    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20},
             graph->mode == PLOT_PARAMETRIC ? "t       x         y" :
             graph->mode == PLOT_POLAR ? "t       r" : "x       y");
    *y += 22;

    int rows = 12;
    for (int i = 0; i < rows; i++) {
        int idx = g_table_scroll + i;
        char row[96];

        if (graph->mode == PLOT_PARAMETRIC) {
            double t = -6.0 + idx * 12.0 / (rows - 1);
            double x = ExprEval(graph->compiled_a, 0.0, t, vars);
            double yv = ExprEval(graph->compiled_b, 0.0, t, vars);
            snprintf(row, sizeof(row), "%6.2f  %8.3g  %8.3g", t, x, yv);
        } else if (graph->mode == PLOT_POLAR) {
            double t = idx * 6.28318530718 / (rows - 1);
            double r = ExprEval(graph->compiled_a, 0.0, t, vars);
            snprintf(row, sizeof(row), "%6.2f  %8.3g", t, r);
        } else {
            double x = -6.0 + idx * 12.0 / (rows - 1);
            double yv = ExprEval(graph->compiled_a, x, 0.0, vars);
            snprintf(row, sizeof(row), "%8.3g  %8.3g", x, yv);
        }

        GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 18}, row);
        *y += 18;
    }

    if (GuiButton((Rectangle){PANEL_PADDING, (float)*y, (float)(inner_w / 2 - 2), 24}, "Up")) {
        if (g_table_scroll > 0) g_table_scroll--;
    }
    if (GuiButton((Rectangle){PANEL_PADDING + inner_w / 2 + 2, (float)*y, (float)(inner_w / 2 - 2), 24}, "Down")) {
        g_table_scroll++;
    }
    *y += 30;
}

void UiInit(void) {
    GuiLoadStyleDefault();
    GuiEnable();
    GuiSetStyle(DEFAULT, TEXT_SIZE, 15);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt((Color){190, 235, 220, 255}));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt((Color){210, 245, 235, 255}));
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt((Color){8, 12, 24, 200}));
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, ColorToInt((Color){12, 18, 32, 220}));
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt((Color){60, 120, 110, 60}));
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt((Color){100, 200, 180, 120}));
}

bool UiWantsInput(void) {
    int panel_w = DisplayPanelWidth(GetScreenWidth());
    return g_edit_a || g_edit_b || g_edit_calc ||
           CheckCollisionPointRec(GetMousePosition(), (Rectangle){0, 0, (float)panel_w, (float)GetScreenHeight()});
}

bool UiTraceEnabled(void) {
    return g_trace && g_tab == UI_TAB_GRAPH;
}

float UiTraceX(void) {
    return g_trace_x;
}

static void DrawTabBar(int inner_w, int *y) {
    if (GuiButton((Rectangle){PANEL_PADDING, (float)*y, (float)(inner_w / 4 - 2), 26}, "Graph")) g_tab = UI_TAB_GRAPH;
    if (GuiButton((Rectangle){PANEL_PADDING + inner_w / 4 + 2, (float)*y, (float)(inner_w / 4 - 2), 26}, "Calc")) g_tab = UI_TAB_CALC;
    if (GuiButton((Rectangle){PANEL_PADDING + inner_w / 2 + 4, (float)*y, (float)(inner_w / 4 - 2), 26}, "Table")) g_tab = UI_TAB_TABLE;
    if (GuiButton((Rectangle){PANEL_PADDING + 3 * inner_w / 4 + 6, (float)*y, (float)(inner_w / 4 - 2), 26}, "Display")) g_tab = UI_TAB_DISPLAY;
    *y += 34;
}

static void DrawDisplayTab(DisplaySettings *display, int inner_w, int *y) {
    const char *quality_names = "Standard (1x);High (2x);Ultra (3x)";
    int prev_quality = display->quality_preset;

#if defined(PLATFORM_WEB)
    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20}, "Canvas follows browser window");
    *y += 24;
#else
    const char *window_names = "1280x720;1600x900;1920x1080;2560x1440";
    int prev_window = display->window_preset;

    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20}, "Window size");
    *y += 22;
    if (GuiComboBox((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 24}, window_names, &display->window_preset)) {
        if (display->window_preset != prev_window) {
            DisplayApplyWindowPreset(display, display->window_preset);
            snprintf(g_status, sizeof(g_status), "Window set to %s", DisplayResolutionLabel(display));
        }
    }
    *y += 30;
#endif

    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20}, "Render quality");
    *y += 22;
    if (GuiComboBox((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 24}, quality_names, &display->quality_preset)) {
        if (display->quality_preset != prev_quality) {
            DisplayApplyQualityPreset(display, display->quality_preset);
            snprintf(g_status, sizeof(g_status), "Quality set to %dx supersampling", display->render_scale);
        }
    }
    *y += 30;

    char info[96];
    snprintf(info, sizeof(info), "Output: %s", DisplayResolutionLabel(display));
    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 20}, info);
    *y += 24;

#if !defined(PLATFORM_WEB)
    if (GuiButton((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 28}, display->fullscreen ? "Exit fullscreen (F11)" : "Fullscreen (F11)")) {
        DisplayToggleFullscreen(display);
    }
    *y += 34;
#endif

    GuiLabel((Rectangle){PANEL_PADDING, (float)*y, (float)inner_w, 36},
             "Higher quality increases curve\nsampling and line thickness.");
    *y += 48;
}

void UiFrame(GraphList *graphs, PlotVars *vars, PlotCamera *camera, DisplaySettings *display, Vector2 cursor_world) {
    int height = GetScreenHeight();
    int panel_w = display->panel_width;
    const int inner_w = panel_w - PANEL_PADDING * 2;
    const int keypad_h = 7 * 30;
    const int footer_h = 48;
    const int keypad_y = height - footer_h - keypad_h;

    DrawRectangle(0, 0, panel_w, height, (Color){4, 6, 14, 230});
    DrawLine(panel_w, 0, panel_w, height, (Color){100, 200, 180, 80});

    if (g_trace && g_tab == UI_TAB_GRAPH) {
        g_trace_x = cursor_world.x;
    }

    int y = PANEL_PADDING;
    DrawTabBar(inner_w, &y);

    if (g_tab == UI_TAB_GRAPH) DrawGraphTab(graphs, vars, camera, panel_w, inner_w, &y);
    else if (g_tab == UI_TAB_CALC) DrawCalcTab(vars, panel_w, inner_w, &y);
    else if (g_tab == UI_TAB_TABLE) DrawTableTab(graphs, vars, inner_w, &y);
    else DrawDisplayTab(display, inner_w, &y);

    DrawKeypad(PANEL_PADDING, keypad_y, inner_w);

    char cursor_text[80];
    snprintf(cursor_text, sizeof(cursor_text), "(%.3g, %.3g)", cursor_world.x, cursor_world.y);
    GuiLabel((Rectangle){PANEL_PADDING, (float)(height - 44), (float)inner_w, 18}, cursor_text);
    GuiLabel((Rectangle){PANEL_PADDING, (float)(height - 24), (float)inner_w, 18}, g_status);

    if (IsKeyPressed(KEY_ENTER)) {
        if (g_tab == UI_TAB_CALC) EvaluateCalc(vars);
        else if (g_tab == UI_TAB_GRAPH) AddGraph(graphs, vars);
    }
}

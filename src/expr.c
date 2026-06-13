#include "expr.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static double g_x;
static double g_t;
static double g_a;
static double g_b;
static double g_c;

static te_variable g_te_vars[] = {
    {"x", &g_x},
    {"t", &g_t},
    {"theta", &g_t},
    {"a", &g_a},
    {"b", &g_b},
    {"c", &g_c}
};

static int IsVarName(const char *name) {
    return strcmp(name, "a") == 0 || strcmp(name, "b") == 0 || strcmp(name, "c") == 0;
}

const char *ExprErrorString(int error) {
    switch (error) {
        case 0: return "OK";
        case -1: return "Unexpected token";
        case -2: return "Unbalanced parentheses";
        case -3: return "Division by zero";
        case -4: return "Unknown variable";
        case -5: return "Expected closing parenthesis";
        case -6: return "Unexpected end of expression";
        default: return "Parse error";
    }
}

void ExprPreprocess(const char *input, char *output, int output_size) {
    int out = 0;
    int len = (int)strlen(input);

    for (int i = 0; i < len && out < output_size - 1; i++) {
        char c = input[i];

        if (c == '*' && i + 1 < len && input[i + 1] == '*') {
            output[out++] = '^';
            i++;
            continue;
        }

        if (out < output_size - 2 && isdigit((unsigned char)c)) {
            int j = i + 1;
            while (j < len && (isalpha((unsigned char)input[j]) || input[j] == '_')) j++;
            if (j > i + 1) {
                output[out++] = c;
                output[out++] = '*';
                continue;
            }
        }

        if (out < output_size - 2 && isalpha((unsigned char)c)) {
            int j = i + 1;
            while (j < len && isdigit((unsigned char)input[j])) j++;
            if (j > i + 1) {
                output[out++] = c;
                output[out++] = '*';
                continue;
            }
        }

        output[out++] = c;
    }

    output[out] = '\0';
}

ParseResult ExprParseInput(const char *input, char *expr_out, int expr_size, char *var_name_out, int var_name_size) {
    char trimmed[EXPR_MAX_LEN];
    int start = 0;
    int end = (int)strlen(input);

    while (start < end && isspace((unsigned char)input[start])) start++;
    while (end > start && isspace((unsigned char)input[end - 1])) end--;

    int trimmed_len = end - start;
    if (trimmed_len <= 0 || trimmed_len >= EXPR_MAX_LEN) return PARSE_ERROR;

    memcpy(trimmed, input + start, trimmed_len);
    trimmed[trimmed_len] = '\0';

    char *eq = strstr(trimmed, "=");
    if (eq != NULL && strstr(trimmed, "==") == NULL) {
        int name_len = (int)(eq - trimmed);
        while (name_len > 0 && isspace((unsigned char)trimmed[name_len - 1])) name_len--;

        char name[8] = {0};
        if (name_len <= 0 || name_len >= (int)sizeof(name)) return PARSE_ERROR;
        memcpy(name, trimmed, name_len);
        name[name_len] = '\0';

        if (!IsVarName(name)) return PARSE_GRAPH;

        const char *value = eq + 1;
        while (*value && isspace((unsigned char)*value)) value++;

        snprintf(var_name_out, var_name_size, "%s", name);
        snprintf(expr_out, expr_size, "%s", value);
        return PARSE_VAR;
    }

    snprintf(expr_out, expr_size, "%s", trimmed);
    return PARSE_GRAPH;
}

static void BindVars(PlotVars *vars) {
    g_a = vars->a;
    g_b = vars->b;
    g_c = vars->c;
}

te_expr *ExprCompile(const char *expr, PlotVars *vars, int *error) {
    char processed[EXPR_MAX_LEN];
    ExprPreprocess(expr, processed, sizeof(processed));
    BindVars(vars);
    return te_compile(processed, g_te_vars, 6, error);
}

double ExprEvalValue(const char *expr, PlotVars *vars) {
    char processed[EXPR_MAX_LEN];
    ExprPreprocess(expr, processed, sizeof(processed));
    BindVars(vars);

    int error = 0;
    te_expr *compiled = te_compile(processed, g_te_vars + 2, 4, &error);
    if (compiled == NULL) return NAN;

    double result = te_eval(compiled);
    te_free(compiled);
    return result;
}

double ExprEval(te_expr *compiled, double x, double t, PlotVars *vars) {
    if (compiled == NULL) return NAN;
    BindVars(vars);
    g_x = x;
    g_t = t;
    return te_eval(compiled);
}

void ExprFree(te_expr *compiled) {
    te_free(compiled);
}

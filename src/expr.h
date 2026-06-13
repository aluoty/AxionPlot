#ifndef EXPR_H
#define EXPR_H

#include "axion_plot.h"

void ExprPreprocess(const char *input, char *output, int output_size);
ParseResult ExprParseInput(const char *input, char *expr_out, int expr_size, char *var_name_out, int var_name_size);
te_expr *ExprCompile(const char *expr, PlotVars *vars, int *error);
double ExprEvalValue(const char *expr, PlotVars *vars);
double ExprEval(te_expr *compiled, double x, double t, PlotVars *vars);
void ExprFree(te_expr *compiled);
const char *ExprErrorString(int error);

#endif

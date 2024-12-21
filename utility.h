// utility.h
#ifndef UTILITY_H
#define UTILITY_H
#include "scanner.h"
#include "gcode.h"

int do_math(int current_value, const char *operator, int operand);
int evaluate_condition(int left, const char *operator, int right);
int expect_token(int *i, State expected_type, const char *error_message);
Symbol *get_symbol(const char *identifier);
void initialize_variable(const char *var_name, const char *value);
int is_comparison_operator(State type);
int is_valid_operand(State type);
int map_initial_value(const char *value);
void optimize_ast(ASTNode *root);

#endif

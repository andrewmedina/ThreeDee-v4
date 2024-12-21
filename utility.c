#include "scanner.h"
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "gcode.h"
#include "utility.h"

Symbol symbol_table[100];
char used_variables[100][100];
int symbol_count = 0;
int used_var_count = 0;

// Helper function to do math based on the given operator
int do_math(int current_value, const char *operator, int operand)
{
    switch (operator[0])
    {
    case '+':
        return current_value + operand;
    case '-':
        return current_value - operand;
    case '*':
        return current_value * operand;
    case '/':
        if (operand == 0)
        {
            fprintf(stderr, "Error: Division by zero\n");
            exit(EXIT_FAILURE);
        }
        return current_value / operand;
    default:
        fprintf(stderr, "Error: Unsupported operator '%s'\n", operator);
        exit(EXIT_FAILURE);
    }
}

// Fold constant expressions in the AST
void fold_constants(ASTNode *node)
{
    if (!node)
        return;

    // Recursively process child nodes
    fold_constants(node->left);
    fold_constants(node->right);

    // Fold constants if the node is an expression
    if (node->type == AST_EXPRESSION && node->left && node->left->right)
    {
        ASTNode *left = node->left;
        ASTNode *operator_node = node->left->right;
        ASTNode *right = operator_node ? operator_node->right : NULL;

        // Check that both children are constants
        if (left && right && left->type == AST_INTEGER && right->type == AST_INTEGER)
        {
            int result = do_math(atoi(left->value), operator_node->value, atoi(right->value));
            printf("\n* Folded %d %s %d to %d\n", atoi(left->value), operator_node->value, atoi(right->value), result);

            // Replace the expression node with an integer
            node->type = AST_INTEGER;
            snprintf(node->value, sizeof(node->value), "%d", result);

            // Mark child nodes as NULL
            node->left = node->right = NULL;
        }
    }
}

// Check if a variable is already marked as used
int is_variable_used(const char *var_name)
{
    for (int i = 0; i < used_var_count; i++)
    {
        if (strcmp(used_variables[i], var_name) == 0)
            return 1;
    }
    return 0;
}

// Mark a variable as used
void variable_is_used(const char *var_name)
{
    if (!is_variable_used(var_name) && used_var_count < 100)
        strcpy(used_variables[used_var_count++], var_name);
}

// Determine all used variables in the AST
void determine_used_variables(ASTNode *node)
{
    if (!node)
        return;

    // If PRINT or EXPRESSION references an identifier, mark it as used
    if ((node->type == AST_PRINT || node->type == AST_EXPRESSION) && node->left && node->left->type == AST_IDENTIFIER)
        variable_is_used(node->left->value);

    // If IF or WHILE references an identifier, mark it as used
    if ((node->type == AST_IF_STATEMENT || node->type == AST_WHILE) && node->left && node->left->left && node->left->left->type == AST_IDENTIFIER)
        variable_is_used(node->left->left->value);

    // Recursively check child nodes
    determine_used_variables(node->left);
    determine_used_variables(node->right);
}

// Eliminate dead code from the AST
ASTNode *eliminate_dead_code(ASTNode *node)
{
    if (!node)
        return NULL;

    // Recursively eliminate dead code in child nodes
    node->left = eliminate_dead_code(node->left);
    node->right = eliminate_dead_code(node->right);

    // Remove unused variable initializations
    if (node->type == AST_COMMAND && strcmp(node->value, "CREATE") == 0 && node->left && !is_variable_used(node->left->value))
    {
        printf("\n* Removed unused variable %s\n", node->left->value);
        // Skip node
        return node->right;
    }

    // Remove unused variable assignments
    if (node->type == AST_ASSIGNMENT && node->left && !is_variable_used(node->left->value))
    {
        printf("\n* Removed unused assignment %s\n", node->left->value);
        // Skip node
        return node->right;
    }

    // Return the node if not dead code
    return node;
}

// Optimize the AST
void optimize_ast(ASTNode *root)
{
    printf("\nFolding constants...\n");
    fold_constants(root);

    printf("\nEliminating dead code...\n");
    determine_used_variables(root);
    root = eliminate_dead_code(root);
}

// Helper function to evaluate comparison operators
int evaluate_condition(int left, const char *operator, int right)
{
    switch (operator[0])
    {
    case '>':
        return operator[1] == '=' ? left >= right : left> right;
    case '<':
        return operator[1] == '=' ? left <= right : left<right;
    case '=': // This assumes that the following character is also '='
        return left == right;
    case '!': // This assumes that the following character is '='
        return left != right;
    default:
        fprintf(stderr, "Error: Unsupported operator '%s'\n", operator);
        return 0;
    }
}

// Helper function to expect and consume a token of a specific type
int expect_token(int *i, State expected_type, const char *error_message)
{
    if (tokens[*i].type != expected_type)
    {
        fprintf(stderr, "Syntax error: %s\n", error_message);
        return 0;
    }
    (*i)++; // Consume the token
    return 1;
}

// Helper function to get or create a symbol
Symbol *get_symbol(const char *identifier)
{
    for (int i = 0; i < symbol_count; i++)
    {
        if (strcmp(symbol_table[i].identifier, identifier) == 0)
        {
            return &symbol_table[i];
        }
    }

    // Add a new symbol if not found
    Symbol *new_symbol = &symbol_table[symbol_count++];
    strncpy(new_symbol->identifier, identifier, sizeof(new_symbol->identifier) - 1);
    new_symbol->value = 0; // Default value
    return new_symbol;
}

// Helper function to check if a token type is a comparison operator
int is_comparison_operator(State type)
{
    return type == LESS_THAN || type == GREATER_THAN ||
           type == LESS_EQUAL || type == GREATER_EQUAL ||
           type == EQUAL || type == NOT_EQUAL;
}

// Helper function to check if a token type is a valid operand
int is_valid_operand(State type)
{
    return type == IDENTIFIER || type == INTEGER || type == PARAMETER;
}

// Helper function to map parameters to numerical values
int map_initial_value(const char *value)
{
    if (strcmp(value, "HIGH") == 0)
        return 10;
    else if (strcmp(value, "MEDIUM") == 0)
        return 5;
    else if (strcmp(value, "LOW") == 0)
        return 1;
    fprintf(stderr, "Error: Unsupported initialization value '%s'\n", value);
    exit(EXIT_FAILURE);
}

// Helper function to initialize variables with set values and output relevant Gcode
void initialize_variable(const char *var_name, const char *value)
{
    Symbol *symbol = get_symbol(var_name);
    symbol->value = map_initial_value(value);
    printf("G92 %s%d ; Initialize %s to %s (%d)\n", var_name, symbol->value, var_name, value, symbol->value);
}

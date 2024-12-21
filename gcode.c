#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gcode.h"
#include "utility.h"

// Parse condition from an AST node and get the variable, operator, and comparison integer, then return the symbol for the variable
Symbol *parse_the_condition(ASTNode *condition, const char **operator, int * compare_value)
{
    ASTNode *var_name = condition->left;
    *operator= var_name->right->value;
    *compare_value = atoi(var_name->right->right->value);
    return get_symbol(var_name->value);
}

// Process an assignment statement from an AST node and update the assigned variable's value
void process_assignment(ASTNode *statement)
{
    if (!statement || !statement->left || !statement->left->right)
        return;

    ASTNode *identifier = statement->left;
    ASTNode *operator_node = identifier->right;

    ASTNode *operand = operator_node ? operator_node->right : NULL;
    if (!operand)
    {
        fprintf(stderr, "Error: Assignment missing operand\n");
        return;
    }

    Symbol *assigned_var = get_symbol(identifier->value);
    if (!assigned_var)
    {
        fprintf(stderr, "Error: Undefined variable '%s'\n", identifier->value);
        return;
    }

    // Update the assigned variable's value and print the Gcode comment indicating it
    assigned_var->value = strcmp(operator_node->value, "=") == 0 ? atoi(operand->value) : do_math(assigned_var->value, operator_node->value, atoi(operand->value));
    printf("; Updated %s to %d\n", identifier->value, assigned_var->value);
}

// Process a sequence of statement AST nodes
void process_statements(ASTNode *statement)
{
    while (statement)
    {
        // Either process AST assignments separately or generate Gcode, then move on to the next statement
        statement->type == AST_ASSIGNMENT ? process_assignment(statement) : generate_gcode(statement);
        statement = statement->right;
    }
}

// Generate Gcode based on a given AST node
void generate_gcode(ASTNode *node)
{
    if (!node)
        return; // Prevent null pointer access

    switch (node->type)
    {
    case AST_COMMAND:
        if (node->left && node->left->right)
            initialize_variable(node->left->value, node->left->right->value);
        break;
    case AST_ASSIGNMENT:
        if (node->left && node->left->right)
            process_assignment(node);
        break;
    case AST_PRINT:
        if (node->left)
        {
            Symbol *symbol = get_symbol(node->left->value);
            if (symbol)
                printf("M117 %s%d ; Printed value of %s\n", node->left->value, symbol->value, node->left->value);
        }
        break;
    case AST_IF_STATEMENT:
    {
        if (!node->left)
        {
            fprintf(stderr, "Error: IF statement missing condition\n");
            return;
        }

        // Parse the IF statement's condition
        const char *operator;
        int compare_value;
        Symbol *cond_symbol = parse_the_condition(node->left, &operator, & compare_value);
        if (!cond_symbol)
        {
            fprintf(stderr, "Error: Undefined variable in IF condition\n");
            return;
        }

        // If the condition evaluates to true, process the IF block's statements
        if (evaluate_condition(cond_symbol->value, operator, compare_value))
            process_statements(node->left->right->left);
        break;
    }
    case AST_WHILE:
    {
        if (!node->left)
        {
            fprintf(stderr, "Error: WHILE node missing condition\n");
            return;
        }

        // Parse the WHILE loop's condition
        const char *operator;
        int compare_value;
        Symbol *loop_var = parse_the_condition(node->left, &operator, & compare_value);
        if (!loop_var)
        {
            fprintf(stderr, "Error: Undefined variable in WHILE condition\n");
            return;
        }

        // While the condition evaluates to true, process the WHILE block's statements
        while (evaluate_condition(loop_var->value, operator, compare_value))
            process_statements(node->left->right->left);
        break;
    }
    default:
        break;
    }

    // Recursively process sibling AST nodes
    generate_gcode(node->right);
}

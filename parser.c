#include <stdio.h>
#include <string.h>
#include "utility.h"
#include "parser.h"

// Parse a condition from tokens
ASTNode *parse_condition(int *i)
{
    // Check if the tokens form a valid condition
    if (tokens[*i].type == IDENTIFIER &&
        is_comparison_operator(tokens[*i + 1].type) &&
        is_valid_operand(tokens[*i + 2].type))
    {
        // Create the condition node
        ASTNode *node = create_ast_node(AST_CONDITION, "CONDITION");

        // Create the identifier node
        node->left = create_ast_node(AST_IDENTIFIER, tokens[*i].value);

        // Create the operator node
        node->left->right = create_ast_node(AST_OPERATOR, tokens[*i + 1].value);

        // Determine the right operand's type and create the node
        node->left->right->right = create_ast_node(map_token_to_ast_type(tokens[*i + 2].type), tokens[*i + 2].value);

        // Advance the token index past the condition
        *i += 3;
        return node;
    }

    // Report a syntax error if the condition is invalid
    fprintf(stderr, "Syntax error: Invalid condition.\n");
    return NULL;
}

// Parse an IF statement or WHILE loop
ASTNode *parse_control_statement(int *i)
{
    // Determine expected AST node type and token type
    ASTNodeType node_type;
    const char *node_name;
    int if_statement_detected = tokens[*i].type == IF;
    if (if_statement_detected)
    {
        node_type = AST_IF_STATEMENT;
        node_name = "IF_STATEMENT";
    }
    else if (tokens[*i].type == WHILE)
    {
        node_type = AST_WHILE;
        node_name = "WHILE";
    }
    else
    {
        return NULL;
    }

    // Check for control keyword followed by an opening parenthesis
    if (tokens[*i + 1].type == OPEN_PAREN)
    {
        ASTNode *control_node = create_ast_node(node_type, node_name);
        *i += 2; // Advance token index past control keyword and the opening parenthesis

        // Parse the condition inside the parentheses
        ASTNode *condition = parse_condition(i);
        if (!condition)
            return NULL;
        control_node->left = condition;

        // Expect a closing parenthesis
        if (!expect_token(i, CLOSE_PAREN, "Missing ')' after condition."))
            return NULL;

        // Parse the statement block
        ASTNode *block_node = parse_statement_block(i);
        if (!block_node)
        {
            fprintf(stderr, "Syntax error: Missing statement block after %s.\n", node_name);
            return NULL;
        }
        condition->right = block_node;

        // If it's an IF statement, check for an optional ELSE
        if (if_statement_detected && tokens[*i].type == ELSE)
        {
            (*i)++; // Advance token index past ELSE
            ASTNode *else_node = create_ast_node(AST_ELSE_STATEMENT, "ELSE_STATEMENT");

            // Link ELSE_STATEMENT directly to IF_STATEMENT
            control_node->right = else_node;

            // Parse the ELSE statement block
            ASTNode *else_block = parse_statement_block(i);
            if (!else_block)
            {
                fprintf(stderr, "Syntax error: Missing statement block after ELSE.\n");
                return NULL;
            }
            else_node->left = else_block; // Attach the ELSE block to ELSE_STATEMENT
        }

        return control_node;
    }

    // Control statement not found
    fprintf(stderr, "Syntax error: Control statement not found.\n");
    return NULL;
}

// Parse PRINT, CREATE, and SET commands
ASTNode *parse_command(int *i)
{
    // Handle PRINT command
    if (tokens[*i].type == PRINT)
    {
        (*i)++; // Advance token index past PRINT
        if (!expect_token(i, IDENTIFIER, "Expected identifier after 'PRINT'."))
            return NULL;

        // Create PRINT node and attach IDENTIFIER node
        ASTNode *print_node = create_ast_node(AST_PRINT, "PRINT");
        print_node->left = create_ast_node(AST_IDENTIFIER, tokens[*i - 1].value);
        return print_node;
    }

    // Handle CREATE and SET commands
    if (tokens[*i].type == COMMAND)
    {
        const char *command_name = tokens[*i].value;
        (*i)++; // Advance token index past COMMAND

        ASTNode *first_node;

        // Handle CREATE command with IDENTIFIER and PARAMETER
        if (strcmp(command_name, "CREATE") == 0)
        {
            // Expect IDENTIFIER after CREATE
            if (tokens[*i].type != IDENTIFIER)
            {
                fprintf(stderr, "Syntax error: Expected identifier after 'CREATE'.\n");
                return NULL;
            }
            first_node = create_ast_node(AST_IDENTIFIER, tokens[*i].value);
            (*i)++; // Advance token index past IDENTIFIER

            // Expect PARAMETER after IDENTIFIER
            if (tokens[*i].type != PARAMETER)
            {
                fprintf(stderr, "Syntax error: Expected parameter after identifier in 'CREATE'.\n");
                return NULL;
            }
        }
        // Handle SET command with SETTING and PARAMETER
        else if (strcmp(command_name, "SET") == 0)
        {
            // Expect SETTING after SET
            if (tokens[*i].type != SETTING)
            {
                fprintf(stderr, "Syntax error: Expected setting after 'SET'.\n");
                return NULL;
            }
            first_node = create_ast_node(AST_SETTING, tokens[*i].value);
            (*i)++; // Advance token index past SETTING
        }
        else
        {
            // Unrecognized command
            return NULL;
        }

        // Create the main command node and link children
        ASTNode *command_node = create_ast_node(AST_COMMAND, command_name);
        command_node->left = first_node;
        first_node->right = create_ast_node(AST_PARAMETER, tokens[*i].value);
        ;
        (*i)++; // Advance token index past PARAMETER
        return command_node;
    }

    // Not a recognized command type
    fprintf(stderr, "Syntax error: Command not found.\n");
    return NULL;
}

// Parses a primary expression
ASTNode *parse_primary(int *i)
{
    ASTNodeType ast_type = map_token_to_ast_type(tokens[*i].type);
    if (ast_type != AST_UNKNOWN)
    {
        // Create AST node for the primary expression
        ASTNode *node = create_ast_node(ast_type, tokens[*i].value);
        (*i)++;
        return node;
    }
    fprintf(stderr, "Syntax error: Primary expression not found.\n");
    return NULL;
}

// Parses an expression with optional operator and right operand
ASTNode *parse_expression(int *i)
{
    // Parse left operand
    ASTNode *left = parse_primary(i);
    if (!left)
        return NULL;

    // Check if the next token is an operator
    if (tokens[*i].type == OPERATOR)
    {
        // Create operator node
        ASTNode *op = create_ast_node(AST_OPERATOR, tokens[*i].value);
        (*i)++; // Advance token index past OPERATOR

        // Parse right operand
        ASTNode *right = parse_primary(i);
        if (!right)
        {
            fprintf(stderr, "Syntax error: Expected identifier, integer, or parameter after operator.\n");
            return NULL;
        }

        // Create expression node and link children
        ASTNode *expr = create_ast_node(AST_EXPRESSION, "EXPRESSION");
        expr->left = left;
        left->right = op;
        op->right = right;

        return expr;
    }

    // Return left operand if no operator follows
    return left;
}

// Parse an assignment
ASTNode *parse_assignment(int *i)
{
    // Check for IDENTIFIER followed by ASSIGN token
    if (tokens[*i].type == IDENTIFIER && tokens[*i + 1].type == ASSIGN)
    {
        // Create AST node for the identifier
        ASTNode *id_node = create_ast_node(AST_IDENTIFIER, tokens[*i].value);
        (*i)++; // Advance token index past IDENTIFIER

        // Create AST node for the assignment operator
        ASTNode *assign_op_node = create_ast_node(AST_ASSIGN, "=");
        (*i)++; // Advance token index past ASSIGN

        // Parse the expression
        ASTNode *expr_node = parse_expression(i);
        if (!expr_node)
        {
            fprintf(stderr, "Syntax error: Invalid expression in assignment.\n");
            return NULL;
        }

        // Create the assignment node and link the children
        ASTNode *assign_node = create_ast_node(AST_ASSIGNMENT, "ASSIGNMENT");
        assign_node->left = id_node;
        id_node->right = assign_op_node;
        assign_op_node->right = expr_node;

        return assign_node;
    }
    fprintf(stderr, "Syntax error: Assignment statement not found.\n");
    return NULL; // Return NULL if not a valid assignment statement
}

// Parse a statement starting at the current token index
ASTNode *parse_statement(int *i)
{
    switch (tokens[*i].type)
    {
    case IF:
    case WHILE:
        // Parse IF or WHILE control statements
        return parse_control_statement(i);
    case PRINT:
    case COMMAND:
        // Parse PRINT, CREATE, or SET commands
        return parse_command(i);
    case IDENTIFIER:
        // Check for assignment following IDENTIFIER
        return tokens[*i + 1].type == ASSIGN ? parse_assignment(i) : NULL;
    default:
        // Unrecognized statement type
        fprintf(stderr, "Syntax error: Statement not found.\n");
        return NULL;
    }
}

// Parse a block of statements enclosed in curly braces or a single statement
ASTNode *parse_statement_block(int *i)
{
    // Check if the current token is an opening curly brace
    if (tokens[*i].type == OPEN_BRACE)
    {
        (*i)++; // Advance token index past opening curly brace
        // Create a node representing the statement block
        ASTNode *block_node = create_ast_node(AST_STATEMENT_BLOCK, "STATEMENT_BLOCK");
        ASTNode *current = NULL; // Pointer to link statements
        // Loop until a closing curly brace is encountered
        while (*i < token_count && tokens[*i].type != CLOSE_BRACE)
        {
            // Skip any newline tokens
            while (*i < token_count && tokens[*i].type == NEW_LINE)
                (*i)++;

            // Parse a single statement
            ASTNode *statement = parse_statement(i);
            if (!statement)
            {
                fprintf(stderr, "Syntax error: Invalid statement in block.\n");
                return NULL;
            }

            // Link the parsed statement to the block node
            if (!current)
                block_node->left = statement; // First statement in the block
            else
                current->right = statement; // Subsequent statements

            current = statement; // Update current to the latest statement

            // Skip any trailing newline tokens
            while (*i < token_count && tokens[*i].type == NEW_LINE)
                (*i)++;
        }
        // Verify that the block ends with a closing curly brace
        if (*i >= token_count || tokens[*i].type != CLOSE_BRACE)
        {
            fprintf(stderr, "Syntax error: Missing '}' to close statement block.\n");
            return NULL;
        }
        (*i)++;            // Advance token index past closing curly brace
        return block_node; // Return the parsed statement block
    }

    // If not a block, attempt to parse a single statement
    return parse_statement(i);
}

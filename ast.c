#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "scanner.h"

// Convert AST node types to strings
const char *ast_type_to_string(ASTNodeType type)
{
    switch (type)
    {
    case AST_COMMAND:
        return "COMMAND";
    case AST_PARAMETER:
        return "PARAMETER";
    case AST_SETTING:
        return "SETTING";
    case AST_OPERATOR:
        return "OPERATOR";
    case AST_INTEGER:
        return "INTEGER";
    case AST_IF_STATEMENT:
        return "IF_STATEMENT";
    case AST_WHILE:
        return "WHILE";
    case AST_CONDITION:
        return "CONDITION";
    case AST_ASSIGNMENT:
        return "ASSIGNMENT";
    case AST_ASSIGN:
        return "ASSIGN";
    case AST_STATEMENT_BLOCK:
        return "STATEMENT_BLOCK";
    case AST_IDENTIFIER:
        return "IDENTIFIER";
    case AST_PRINT:
        return "PRINT";
    case AST_EXPRESSION:
        return "EXPRESSION";
    case AST_ELSE_STATEMENT:
        return "ELSE_STATEMENT";
    default:
        return "UNKNOWN";
    }
}

// Build the AST
ASTNode *build_ast()
{
    int i = 0;
    ASTNode *root = NULL;
    ASTNode *current = NULL;

    while (i < token_count)
    {
        // Skip any newline tokens to find the next useful token
        while (i < token_count && tokens[i].type == NEW_LINE)
            i++;

        if (i >= token_count)
            break;

        // Parse a statement starting at the current token's index
        ASTNode *statement_node = parse_statement(&i);
        if (!statement_node)
        {
            fprintf(stderr, "Syntax error: Unexpected token '%s'\n", tokens[i].value);
            return NULL;
        }

        // Append the parsed statement to the AST
        if (!root)
            root = statement_node; // First statement becomes the root
        else
            current->right = statement_node; // Link subsequent statements

        current = statement_node; // Update current to the latest statement

        // Skip any trailing newline tokens before the next iteration
        while (i < token_count && tokens[i].type == NEW_LINE)
            i++;
    }

    return root; // Return the root of the constructed AST
}

// Create a new AST node
ASTNode *create_ast_node(ASTNodeType type, const char *value)
{
    ASTNode *node = calloc(1, sizeof(*node)); // Allocate and zero-initialize memory
    node->type = type;
    strncpy(node->value, value, sizeof(node->value) - 1); // Safely copy value
    return node;
}

ASTNodeType map_token_to_ast_type(State type)
{
    switch (type)
    {
    case IDENTIFIER:
        return AST_IDENTIFIER;
    case INTEGER:
        return AST_INTEGER;
    case PARAMETER:
        return AST_PARAMETER;
    default:
        return AST_UNKNOWN;
    }
}

// Recursively prints the AST with indents
void print_ast(ASTNode *root, int level)
{
    if (!root)
        return;

    // Print the current node
    printf("%*s%s: %s\n", level * 2, "", ast_type_to_string(root->type), root->value);

    // Print the left subtree
    print_ast(root->left, level + 1);

    // Print the right subtree
    print_ast(root->right, level);
}

// ast.h
#ifndef AST_H
#define AST_H

#include "scanner.h"

// Define AST Node types
typedef enum
{
    AST_ASSIGN,
    AST_ASSIGNMENT,
    AST_COMMAND,
    AST_CONDITION,
    AST_ELSE_STATEMENT,
    AST_EXPRESSION,
    AST_IDENTIFIER,
    AST_IF_STATEMENT,
    AST_INTEGER,
    AST_OPERATOR,
    AST_PARAMETER,
    AST_PRINT,
    AST_SETTING,
    AST_STATEMENT_BLOCK,
    AST_UNKNOWN,
    AST_WHILE,
} ASTNodeType;

// Define the AST Node structure
typedef struct ASTNode
{
    ASTNodeType type;
    char value[100];       // Value of the node
    struct ASTNode *left;  // Child nodes representing details of the command
    struct ASTNode *right; // Sibling nodes representing the next command in the sequence
} ASTNode;

const char *ast_type_to_string(ASTNodeType type);
ASTNode *build_ast();
ASTNode *create_ast_node(ASTNodeType type, const char *value);
ASTNodeType map_token_to_ast_type(State type);
void print_ast(ASTNode *root, int level);

#endif

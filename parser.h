// parser.h
#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

ASTNode *parse_statement(int *i);
ASTNode *parse_statement_block(int *i);

#endif

// scanner.h
#ifndef SCANNER_H
#define SCANNER_H
#define MAX_TOKENS 1000

typedef enum
{
    ASSIGN,
    CLOSE_BRACE,
    CLOSE_PAREN,
    COMMAND,
    ELSE,
    EQUAL,
    GREATER_EQUAL,
    GREATER_THAN,
    IDENTIFIER,
    IF,
    INTEGER,
    LESS_EQUAL,
    LESS_THAN,
    LEXICAL_ERROR,
    NEW_LINE,
    NOT_EQUAL,
    OPEN_BRACE,
    OPEN_PAREN,
    OPERATOR,
    PARAMETER,
    PRINT,
    SETTING,
    START,
    WHILE,
} State;

typedef struct
{
    State type;
    char value[100];
} Token;

extern Token tokens[MAX_TOKENS];
extern int token_count;

#endif

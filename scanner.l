%{
#include "scanner.h"

Token tokens[MAX_TOKENS];  // Define the token array
int token_count = 0;       // Define the token count

void add_token(State type, const char *value) {
    if (token_count >= MAX_TOKENS) {
        fprintf(stderr, "Error: Token list exceeded maximum capacity.\n");
        exit(1);
    }
    tokens[token_count].type = type;
    snprintf(tokens[token_count].value, sizeof(tokens[token_count].value), "%s", value);
    token_count++;
}

%}

/* Define the patterns for tokens and their corresponding transitions in our state machine */
%%

"CREATE"|"SET"                                { add_token(COMMAND, yytext); }
"PRINT"                                       { add_token(PRINT, yytext); }
"FAST"|"HIGH"|"LOW"|"MEDIUM"|"SLOW"|"STRONG"  { add_token(PARAMETER, yytext); }
"INFILL"|"LAYER_HEIGHT"|"SPEED"               { add_token(SETTING, yytext); }

"IF"                                          { add_token(IF, yytext); }
"ELSE"                                        { add_token(ELSE, yytext); }
"WHILE"                                       { add_token(WHILE, yytext); }

"("                                           { add_token(OPEN_PAREN, yytext); }
")"                                           { add_token(CLOSE_PAREN, yytext); }
"{"                                           { add_token(OPEN_BRACE, yytext); }
"}"                                           { add_token(CLOSE_BRACE, yytext); }

"="                                           { add_token(ASSIGN, yytext); }
"<"                                           { add_token(LESS_THAN, yytext); }
">"                                           { add_token(GREATER_THAN, yytext); }
"<="                                          { add_token(LESS_EQUAL, yytext); }
">="                                          { add_token(GREATER_EQUAL, yytext); }
"=="                                          { add_token(EQUAL, yytext); }
"!="                                          { add_token(NOT_EQUAL, yytext); }

[0-9]+                                        { add_token(INTEGER, yytext); }
"X"|"Y"|"Z"                                   { add_token(IDENTIFIER, yytext); }  // Identifiers: X, Y, or Z
[A-Za-z][a-zA-Z0-9_]*                         { add_token(LEXICAL_ERROR, yytext); } // Invalid identifiers: uppercase start
\n                                            { add_token(NEW_LINE, "\\n"); }
"+"|"-"|"*"|"/"                               { add_token(OPERATOR, yytext); }

[ \t\r]+                                      { /* Ignore whitespace */ }
.                                             { add_token(LEXICAL_ERROR, yytext); }  // Any other character is an error

%%

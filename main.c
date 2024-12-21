#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "gcode.h"
#include "utility.h"

int yylex();

int main(int argc, char **argv)
{
    extern FILE *yyin;
    yyin = fopen(argv[1], "r");
    yylex();

    ASTNode *ast = build_ast();
    printf("Original Abstract Syntax Tree:\n");
    print_ast(ast, 0);

    optimize_ast(ast);
    printf("\nOptimized Abstract Syntax Tree:\n");
    print_ast(ast, 0);

    printf("\nGenerated GCode:\n");
    generate_gcode(ast);
    return 0;
}

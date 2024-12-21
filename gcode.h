// gcode.h
#ifndef GCODE_H
#define GCODE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "scanner.h"

// Symbol table for tracking variable states
typedef struct
{
    char identifier[10];
    int value;
} Symbol;

void generate_gcode(ASTNode *node);

#endif
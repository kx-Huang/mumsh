#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"

// called in mumsh.c
int mumsh_parser();
void free_cmds();

// called in parser.c
int syntax_error(error_t error_type, char* content);

// for debugging
void debug_parser();

#endif  // PARSER_H

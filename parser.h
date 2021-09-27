#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"

int mumsh_parser();
void free_cmds();
int syntax_error(int error_type, char* content);

void debug_parser();

#endif  // PARSER_H

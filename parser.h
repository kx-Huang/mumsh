#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

token_t parser();
void syntax_error(int error_type, char* content);

void debug(token_t* token);

#endif  // PARSER_H

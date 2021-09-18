#ifndef PARSER_H
#define PARSER_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "io.h"

typedef struct token {
  int read_file;
  int write_file;
  int append_file;
  char src[BUFFER_SIZE];
  char dest[BUFFER_SIZE];
  char* argv[BUFFER_SIZE];
  size_t argc;
} token_t;

typedef struct parser {
  int is_src;
  int is_dest;
  int in_single_quote;
  int in_double_quote;
  char buffer[BUFFER_SIZE];
} parser_t;

token_t parser();
void exec_cmd(token_t*);

#endif  // PARSER_H

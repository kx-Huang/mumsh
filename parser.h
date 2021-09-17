#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

typedef struct token {
  int read_file;
  int write_file;
  int append;
  size_t argv_cnt;
  char dest[BUFFER_SIZE];
  char orig[BUFFER_SIZE];
  char* argv[BUFFER_SIZE];
} token_t;

typedef struct parser {
  int parse_dest;
  int parse_orig;
  int in_quote;
  int in_double_quote;
  int in_single_quote;
  size_t head;
  size_t tail;
} parser_t;

void parser();
void exec_cmd();

#endif  // PARSER_H

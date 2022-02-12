#ifndef DATA_H
#define DATA_H

#define BUFFER_SIZE 1024
#define TOKEN_SIZE 512
#define COMMAND_SIZE 512
#define PROCESS_SIZE 512

#include <stdio.h>

enum error_type {
  NORMAL = 0,
  NON_EXISTING_PROGRAM = 1,
  NON_EXISTING_FILE = 2,
  NO_PERMISSION = 3,
  DUP_INPUT_REDIRECTION = 4,
  DUP_OUTPUT_REDIRECTION = 5,
  ERROR_SYNTAX = 6,
  MISS_PROGRAM = 7,
  NON_EXISTING_DIR = 8,
  UNEXPECTED_ERROR = 9
};

typedef struct token {
  size_t argc;
  char *argv[TOKEN_SIZE];
} token_t;

typedef struct cmd {
  size_t cnt;
  int background;
  int read_file;
  int write_file;
  int append_file;
  char src[BUFFER_SIZE];
  char dest[BUFFER_SIZE];
  token_t cmds[COMMAND_SIZE];
} cmd_t;

typedef struct parser {
  size_t buffer_len;
  int is_src;
  int is_dest;
  int is_pipe;
  int in_single_quote;
  int in_double_quote;
  char buffer[BUFFER_SIZE];
} parser_t;

void debug_path(int from_home, size_t depth, char *token[TOKEN_SIZE]);

extern char cmd_buffer[BUFFER_SIZE];
extern cmd_t cmd;

#endif  // DATA_H

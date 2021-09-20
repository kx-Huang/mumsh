#ifndef DATA_H
#define DATA_H

#define BUFFER_SIZE 1024

enum error_type {
  NORMAL_EXIT = 0,
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

extern int ctrl_c;
extern char cmd_buffer[BUFFER_SIZE];

#endif  // DATA_H

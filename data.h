#ifndef DATA_H
#define DATA_H

#define BUFFER_SIZE 1024
#define TOKEN_SIZE 512
#define COMMAND_SIZE 512
#define PROCESS_SIZE 512

#define NO_HINT 0
#define FULL_HINT 1
#define MATCHED_HINT 2

#include <stdbool.h>
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

// for hinter
extern size_t len;
extern size_t pos;
extern size_t num_hint;
extern size_t num_history_all;
extern size_t num_history_match;
extern size_t width_clean;
extern size_t offset_prefix;
extern size_t index_history_all;
extern size_t index_history_match;
extern bool iterate_mode;
extern bool match_mode;
extern bool from_home;
extern char token[BUFFER_SIZE];
extern char puzzle[TOKEN_SIZE];
extern char path[TOKEN_SIZE];
extern char fit[BUFFER_SIZE];
extern char hint[BUFFER_SIZE][TOKEN_SIZE];
extern char history_all[BUFFER_SIZE][BUFFER_SIZE];
extern char history_match[BUFFER_SIZE][BUFFER_SIZE];

// for parser
extern char cmd_buffer[BUFFER_SIZE];
extern cmd_t cmd;

// for debug
void debug_path(int from_home, size_t depth, char *token[TOKEN_SIZE]);

#endif  // DATA_H

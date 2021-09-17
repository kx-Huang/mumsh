#ifndef IO_H
#define IO_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "data.h"

enum exit_status {
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

void prompt_mumsh(void);
void read_cmd(void);
void exit_mumsh(int error_code, char* content);

#endif  // IO_H

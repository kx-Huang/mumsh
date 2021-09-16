#ifndef IO_H
#define IO_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

enum exit_status {
  NORMAL_EXIT = 0,
  NON_EXISTING_PROGRAM = 1,
  NON_EXISTING_FILE = 2,
  NO_PERMISSION = 3,
  DUP_INPUT_REDIRECTION = 4,
  DUP_OUTPUT_REDIRECTION = 5,
  ERR_SYNTAX = 6,
  MISS_PROGRAM = 7,
  NON_EXISTING_DIR = 8
};

void prompt_mumsh(void);
void read_cmd(char* buffer);
void exit_mumsh(int error_code, char* content);

#endif  // IO_H

#ifndef IO_H
#define IO_H

#include <stdio.h>

#include "process.h"

#define RESET "\x1B[1;0m"
#define BLK "\x1B[1;30m"
#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"
#define ORGB "\x1B[43m"
#define BLUB "\x1B[44m"
#define PURB "\x1B[45m"

// called in mumsh.c
void mumsh_read_cmds();

// called in io.c
void prompt_prefix();
void prompt_path();
void prompt_mumsh();
void read_dangling_cmds(char *buffer);
void write_cmd_buffer(char *buffer);

#endif  // IO_H

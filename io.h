#ifndef IO_H
#define IO_H

#include <stdio.h>

#include "process.h"

// called in mumsh.c
void mumsh_read_cmds();

// called in io.c
void read_dangling_cmds(char *buffer);
void write_cmd_buffer(char *buffer);

#endif  // IO_H

#ifndef IO_H
#define IO_H

#include <stdio.h>

#include "process.h"

void mumsh_read_cmds();
void read_dangling_cmds(char *buffer);
void write_cmd_buffer(char *buffer);

#endif  // IO_H

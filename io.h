#ifndef IO_H
#define IO_H

#include <stdio.h>

#include "process.h"

void sigint_handler();
void sigint_handler_parent(int signal);
void sigint_handler_child(int signal);
void mumsh_read_cmds();
void read_dangling_cmds(char *buffer);
void write_cmd_buffer(char *buffer);

#endif  // IO_H

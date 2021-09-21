#ifndef IO_H
#define IO_H

#include <stdio.h>

#include "process.h"

void sigint_handler();
void sigint_handler_parent(int signal);
void sigint_handler_child(int signal);
void mumsh_prompt();
void mumsh_read_cmds();

#endif  // IO_H

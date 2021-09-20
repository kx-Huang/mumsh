#ifndef IO_H
#define IO_H

#include <stdio.h>

#include "process.h"

void sigint_handler();
void sigint_handler_parent(int signal);
void sigint_handler_child(int signal);
void prompt_mumsh();
void read_cmd();

#endif  // IO_H

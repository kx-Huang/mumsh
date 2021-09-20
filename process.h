#ifndef PROCESS_H
#define PROCESS_H

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "data.h"

void exec_cmd(token_t* token);
void exit_process(int exit_code, char* content);

#endif  // PROCESS_H

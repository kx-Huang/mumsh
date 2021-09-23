#ifndef PROCESS_H
#define PROCESS_H

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "data.h"

void io_redirect();
void mumsh_exec_cmds();
int mumsh_cmd_exit();
int mumsh_cmd_cd();
void mumsh_cmd_pwd(token_t* token);
void exec_cmd(token_t* token);
void exit_process(int exit_code, char* content);

#endif  // PROCESS_H

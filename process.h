#ifndef PROCESS_H
#define PROCESS_H

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "data.h"

#define MAX_PROCESS_SIZE 512
#define READ 0
#define WRITE 1

void input_redirect();
void output_redirect();
void mumsh_exec_cmds();
int mumsh_cmd_exit();
int mumsh_cmd_cd();
void mumsh_cmd_pwd(token_t* token);
void exec_cmd(token_t* token);
void exit_process(int exit_code, char* content);

char OLDPWD[BUFFER_SIZE];
pid_t pids[MAX_PROCESS_SIZE];
int pipe_fd[MAX_PROCESS_SIZE][2];

#endif  // PROCESS_H

#ifndef PROCESS_H
#define PROCESS_H

#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "data.h"

#define READ 0
#define WRITE 1

void sigint_handler();
void input_redirect();
void output_redirect();
void mumsh_exec_cmds();
int mumsh_cmd_exit();
int mumsh_cmd_cd();
void mumsh_cmd_pwd(token_t* token);
void exec_cmd(token_t* token);
void exit_process(int exit_code, char* content);

extern sigjmp_buf env;
extern volatile sig_atomic_t jump_active;

char OLDPWD[BUFFER_SIZE];
pid_t pids[PROCESS_SIZE];
int pipe_fd[PROCESS_SIZE][2];

void debug_child_exit_status(pid_t pid, int status);

#endif  // PROCESS_H

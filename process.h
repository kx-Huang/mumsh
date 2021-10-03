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
#define JOBS_CAPACITY 1024

typedef struct job {
  size_t bg_cnt;
  size_t job_cnt;
  size_t table_size;
  size_t* stat_table;
  pid_t** pid_table;
  char** cmd_table;
} job_t;

void sigint_handler();
void reap_background_jobs();
void mumsh_exec_cmds();
int mumsh_cmd_exit();
int mumsh_cmd_cd();
void init_jobs_table();
int add_bytes();
void print_formatted_cmds();
void input_redirect();
void output_redirect();
void mumsh_cmd_jobs(token_t* token);
void mumsh_cmd_pwd(token_t* token);
void exec_cmd(token_t* token);
void free_jobs();
void exit_process(int exit_code, char* content);

extern sigjmp_buf env;
extern volatile sig_atomic_t jump_active;

job_t job;
char OLDPWD[BUFFER_SIZE];
int pipe_fd[PROCESS_SIZE][2];

void debug_process(pid_t pid, int status);
void debug_jobs();

#endif  // PROCESS_H

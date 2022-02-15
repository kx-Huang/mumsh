#include "process.h"

job_t job;                     // store background jobs for cmd "jobs"
pid_t pgid;                    // store child process group pid for grouping
char OLDPWD[BUFFER_SIZE];      // store last working directory for cmd "cd -"
int pipe_fd[PROCESS_SIZE][2];  // store pipe file descriptor for piping

// ctrl-c interruption handler
void sigint_handler() {
  if (jump_active == 0) return;
  siglongjmp(env, 42);
}

// reap background jobs
void reap_background_jobs() {
  size_t max_loop = job.bg_cnt;
  for (size_t i = 0; i < max_loop; i++) {
    int status;
    pid_t pid;
    pid = waitpid(-1, &status, WNOHANG);
    // debug_process(pid, status);
    if (pid > 0) {
      job.bg_cnt--;
      for (size_t i = 0; i < job.job_cnt; i++)
        for (size_t j = 0; job.pid_table[i][j] != 0; j++) {
          if (job.stat_table[i] == 0) continue;
          if (job.pid_table[i][j] == pid) job.stat_table[i]--;
        }
    }
  }
}

// exit mumsh with cmd "exit"
int mumsh_cmd_exit() {
  if (cmd.cnt == 1 && cmd.cmds[0].argc >= 1 &&
      (strcmp(cmd.cmds[0].argv[0], "exit") == 0 ||
       strcmp(cmd.cmds[0].argv[0], "q") == 0))
    return NORMAL;
  return -1;
}

// change working directory with cmd "cd"
int mumsh_cmd_cd() {
  if (cmd.cnt == 1 && cmd.cmds[0].argc >= 1 &&
      strcmp(cmd.cmds[0].argv[0], "cd") == 0) {
    // get current path
    char path[BUFFER_SIZE] = {0};
    if (getcwd(path, BUFFER_SIZE) == NULL) return NORMAL;
    // cd
    if (cmd.cmds[0].argc == 1) {
      if (chdir(getenv("HOME")) == 0) strcpy(OLDPWD, path);
      // cd
    } else if (strcmp(cmd.cmds[0].argv[1], "~") == 0) {
      if (chdir(getenv("HOME")) == 0) strcpy(OLDPWD, path);
      // cd -
    } else if (strcmp(cmd.cmds[0].argv[1], "-") == 0) {
      if (strlen(OLDPWD) == 0) {  // no last path
        fputs("cd: OLDPWD not set\n", stderr);
      } else {  // cd to last path
        if (chdir(OLDPWD) == 0) {
          printf("%s\n", OLDPWD);
          fflush(stdout);
          strcpy(OLDPWD, path);
        }
      }
      // cd PATH
    } else {
      if (chdir(cmd.cmds[0].argv[1]) == 0) {
        strcpy(OLDPWD, path);
      } else {  // error: Non-existing file in input redirection
        fputs(cmd.cmds[0].argv[1], stderr);
        fputs(": No such file or directory\n", stderr);
      }
    }
    return NORMAL;
  }
  return -1;
}

// traverse status table to print jobs
void mumsh_cmd_jobs(token_t* token) {
  if (token->argc == 1 && strcmp(token->argv[0], "jobs") == 0) {
    for (size_t i = 0; i < job.job_cnt; i++) {
      printf("[%lu] ", i + 1);
      if (job.stat_table[i])
        printf("running ");
      else
        printf("done ");
      printf("%s", job.cmd_table[i]);
    }
    fflush(stdout);
    exit(NORMAL);
  }
}

// print working directory built-in cmd "pwd"
void mumsh_cmd_pwd(token_t* token) {
  if (token->argc == 1 && strcmp(token->argv[0], "pwd") == 0) {
    char path[BUFFER_SIZE] = {0};
    if (getcwd(path, BUFFER_SIZE) != NULL) {
      printf("%s\n", path);
      fflush(stdout);
      exit(NORMAL);
    }
    exit_process(UNEXPECTED_ERROR, "");
  }
}

// input redirection
void input_redirect() {
  if (cmd.read_file) {
    int file = open(cmd.src, O_RDONLY);
    // error: Non-existing file in input redirection
    if (file < 0) exit_process(NON_EXISTING_FILE, cmd.src);
    dup2(file, STDIN_FILENO);
  }
}

// output redirection
void output_redirect() {
  if (cmd.write_file) {
    if (cmd.append_file) {
      int file = open(cmd.dest, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
      // error: Failed to open file in output redirection
      if (file < 0) exit_process(NO_PERMISSION, cmd.dest);
      dup2(file, STDOUT_FILENO);
    } else {
      int file = open(cmd.dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
      // error: Failed to open file in output redirection
      if (file < 0) exit_process(NO_PERMISSION, cmd.dest);
      dup2(file, STDOUT_FILENO);
    }
  }
}

// execute cmd
void exec_cmd(token_t* token) {
  // error: Non-existing program
  if (execvp(token->argv[0], token->argv) < 0)
    exit_process(NON_EXISTING_PROGRAM, token->argv[0]);
}

// log background cmds for cmd "jobs"
void init_jobs_table() {
  // allocate memory for jobs table
  if (job.job_cnt == 0) {
    job.table_size = JOBS_CAPACITY;
    job.pid_table = malloc(job.table_size * sizeof(pid_t*));
    job.cmd_table = malloc(job.table_size * sizeof(char*));
    job.stat_table = malloc(job.table_size * sizeof(size_t));
  } else if (job.job_cnt == job.table_size) {
    job.table_size += JOBS_CAPACITY;
    job.pid_table = realloc(job.pid_table, job.table_size * sizeof(pid_t*));
    job.cmd_table = realloc(job.cmd_table, job.table_size * sizeof(char*));
    job.stat_table = realloc(job.stat_table, job.table_size * sizeof(size_t));
  }
  job.pid_table[job.job_cnt] = malloc(COMMAND_SIZE * sizeof(pid_t));
  memset(job.pid_table[job.job_cnt], 0, COMMAND_SIZE);
  job.stat_table[job.job_cnt] = cmd.cnt;
}

// safety append for sprintf
int add_bytes(int res_sprintf) { return (res_sprintf > 0) ? res_sprintf : 0; }

// prompt formatted background cmds
void print_formatted_cmds() {
  int len = 0;
  char* buffer = malloc(BUFFER_SIZE);
  for (size_t i = 0; i < cmd.cnt; i++) {
    for (size_t j = 0; j < cmd.cmds[i].argc; j++)
      len += add_bytes(sprintf(buffer + len, "%s ", cmd.cmds[i].argv[j]));
    if (i == 0 && cmd.read_file) {
      len += add_bytes(sprintf(buffer + len, "< %s", cmd.src));
      if (cmd.cnt > 1) len += add_bytes(sprintf(buffer + len, " "));
    }
    if ((i != cmd.cnt - 1)) len += add_bytes(sprintf(buffer + len, "| "));
  }
  if (cmd.write_file) {
    len += add_bytes(sprintf(buffer + len, ">"));
    if (cmd.append_file) len += add_bytes(sprintf(buffer + len, ">"));
    len += add_bytes(sprintf(buffer + len, " %s ", cmd.dest));
  }
  if (cmd.background) len += add_bytes(sprintf(buffer + len, "&\n"));
  printf("%s", buffer);
  job.cmd_table[job.job_cnt] = buffer;
}

// execute command sequence
void mumsh_exec_cmds() {
  // print background cmds
  if (cmd.background) {
    init_jobs_table();
    printf("[%lu] ", job.job_cnt + 1);
    print_formatted_cmds();
  }
  // execute command sequence
  for (size_t i = 0; i < cmd.cnt; i++) {
    // open new pipe
    if (cmd.cnt > 1 && i != cmd.cnt - 1)
      if (pipe(pipe_fd[i]) != 0) return;
    // fork a child process
    pid_t pid = fork();
    // set first child as children process group leader
    if (i == 0) pgid = pid;
    if (pid < 0) {
      exit_process(UNEXPECTED_ERROR, "");
    } else if (pid == 0) {  // child process
      // subscribe ctrl-c notification for child process
      struct sigaction sa_child_SIGINT;
      sa_child_SIGINT.sa_handler = sigint_handler;
      sigemptyset(&sa_child_SIGINT.sa_mask);
      sa_child_SIGINT.sa_flags = SA_RESTART;
      sigaction(SIGINT, &sa_child_SIGINT, NULL);
      // input redirection only in first cmd
      if (i == 0) input_redirect();
      // left pipe
      if (cmd.cnt > 1 && i != 0) {
        close(pipe_fd[i - 1][WRITE]);
        dup2(pipe_fd[i - 1][READ], STDIN_FILENO);
      }
      // right pipe
      if (cmd.cnt > 1 && i != cmd.cnt - 1) {
        close(pipe_fd[i][READ]);
        dup2(pipe_fd[i][WRITE], STDOUT_FILENO);
      }
      // output redirection only in last cmd
      if (i == cmd.cnt - 1) output_redirect();
      // exec cmd
      mumsh_cmd_jobs(&cmd.cmds[i]);
      mumsh_cmd_pwd(&cmd.cmds[i]);
      exec_cmd(&cmd.cmds[i]);
    }
    // parent process
    // put all child process into group of first child
    setpgid(pid, pgid);
    // set group of first child as foreground process group
    if (cmd.background == 0) {
      if (i == 0) tcsetpgrp(STDIN_FILENO, pgid);
    } else {  // save pid of background cmd into jobs table
      job.pid_table[job.job_cnt][i] = pid;
    }
    // close previous pipe
    if (cmd.cnt > 1 && i != 0) {
      close(pipe_fd[i - 1][WRITE]);
      close(pipe_fd[i - 1][READ]);
    }
  }
  // reap foreground cmds
  if (cmd.background == 0) {
    int status;
    pid_t res;
    // block parent process and wait for child process done
    for (size_t i = 0; i < cmd.cnt; i++) {
      res = waitpid(-pgid, &status, WUNTRACED);
      // debug_process(res, status);
    }
    // reset parent as terminal foreground process group leader
    tcsetpgrp(STDOUT_FILENO, getpgrp());
    // handle ctrl-c interruption
    if (WIFSIGNALED(status)) {
      printf("\n");
      fflush(stdout);
    }
  } else {  // add background job count for reaping in next loop
    job.job_cnt++;
    job.bg_cnt += cmd.cnt;
    // debug_jobs();
  }
}

// free after allocating memory for jobs table
void free_jobs() {
  for (size_t i = 0; i < job.job_cnt; i++) {
    free(job.pid_table[i]);
    free(job.cmd_table[i]);
  }
  free(job.pid_table);
  free(job.cmd_table);
  free(job.stat_table);
}

// print child process exit status
void debug_process(pid_t pid, int status) {
  if (WIFEXITED(status))
    printf("child %d exited normally with status %d\n", pid,
           WEXITSTATUS(status));
  else if (WIFSIGNALED(status))
    printf("child %d exited by signal %d\n", pid, WTERMSIG(status));
  else
    printf("child %d neither exited normally nor by a signal\n", pid);
  fflush(stdout);
}

// print jobs log
void debug_jobs() {
  printf("jobs table capacity: %lu\n", job.table_size);
  for (size_t i = 0; i < job.job_cnt; i++)
    for (size_t j = 0; job.pid_table[i][j] != 0; j++)
      printf("job[%lu][%lu]: %d\n", i, j, job.pid_table[i][j]);
}

// exit process with specified message and exit code
void exit_process(int exit_code, char* content) {
  switch (exit_code) {
    case NORMAL:
      printf("exit\n");
      fflush(stdout);
      break;
    case NON_EXISTING_PROGRAM:
      fputs(content, stderr);
      fputs(": command not found\n", stderr);
      break;
    case NON_EXISTING_FILE:
      fputs(content, stderr);
      fputs(": No such file or directory\n", stderr);
      break;
    case NO_PERMISSION:
      fputs(content, stderr);
      fputs(": Permission denied\n", stderr);
      break;
    case NON_EXISTING_DIR:
      fputs(content, stderr);
      fputs(": No such file or directory\n", stderr);
      break;
    case UNEXPECTED_ERROR:
      fputs("error: unexpected error\n", stderr);
      break;
    default:
      break;
  }
  exit(exit_code);
}

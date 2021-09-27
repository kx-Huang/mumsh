#include "process.h"

job_t job;                     // store background jobs for cmd "jobs"
char OLDPWD[BUFFER_SIZE];      // store last working directory for cmd "cd -"
pid_t pids[PROCESS_SIZE];      // store child process pid for process control
int pipe_fd[PROCESS_SIZE][2];  // store pipe file descriptor for piping

// ctrl-c interruption handler
void sigint_handler() {
  if (jump_active == 0) return;
  siglongjmp(env, 42);
}

// reap background jobs
void reap_background_jobs() {
  for (size_t i = 0; i < job.cnt; i++) {
    int status;
    pid_t res;
    res = waitpid(-1, &status, WNOHANG);
    //debug_process(res, status);
    if (res > 0) job.cnt--;
  }
}

// exit mumsh with cmd "exit"
int mumsh_cmd_exit() {
  if (cmd.cnt == 1 && cmd.cmds[0].argc >= 1 &&
      strcmp(cmd.cmds[0].argv[0], "exit") == 0)
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

int mumsh_cmd_jobs() {
  if (cmd.cnt == 1 && cmd.cmds[0].argc == 1 &&
      strcmp(cmd.cmds[0].argv[0], "jobs") == 0) {
    printf("print jobs here!\n");
    return 0;
  }
  return -1;
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

void exec_cmd(token_t* token) {
  // error: Non-existing program
  if (execvp(token->argv[0], token->argv) < 0)
    exit_process(NON_EXISTING_PROGRAM, token->argv[0]);
}

// execute cmd sequence
void mumsh_exec_cmds() {
  for (size_t i = 0; i < cmd.cnt; i++) {
    // open new pipe
    if (cmd.cnt > 1 && i != cmd.cnt - 1)
      if (pipe(pipe_fd[i]) != 0) return;
    // fork a child process
    pid_t pid = fork();
    pids[i] = pid;
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
      mumsh_cmd_pwd(&cmd.cmds[i]);
      exec_cmd(&cmd.cmds[i]);
    }
    // parent process
    // put all child process into group of first child
    setpgid(pids[i], pids[0]);
    // set group of first child as foreground process group
    if (cmd.background == 0)
      if (i == 0) tcsetpgrp(STDIN_FILENO, pids[0]);
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
      res = waitpid(-pids[0], &status, WUNTRACED);
      //debug_process(res, status);
    }
    // reset parent as terminal foreground process group leader
    tcsetpgrp(STDOUT_FILENO, getpgrp());
    // handle ctrl-c interruption
    if (WIFSIGNALED(status)) {
      printf("\n");
      fflush(stdout);
    }
  }
  // add background job count for reaping in next loop
  else
    job.cnt += cmd.cnt;
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

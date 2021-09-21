#include "process.h"

void mumsh_exec_cmds() {
  // todo: piping
  io_redirect();
  for (size_t i = 0; i < cmd.cnt; i++) exec_cmd(&cmd.cmds[i]);
}

void io_redirect() {
  // handle redirection
  if (cmd.read_file) {
    int file = open(cmd.src, O_RDONLY);
    // error: Non-existing file in input redirection
    if (file < 0) exit_process(NON_EXISTING_FILE, cmd.src);
    dup2(file, STDIN_FILENO);
  }
  if (cmd.append_file) {
    int file = open(cmd.dest, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
    // error: Failed to open file in output redirection
    if (file < 0) exit_process(NO_PERMISSION, cmd.dest);
    dup2(file, STDOUT_FILENO);
  } else if (cmd.write_file) {
    int file = open(cmd.dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    // error: Failed to open file in output redirection
    if (file < 0) exit_process(NO_PERMISSION, cmd.dest);
    dup2(file, STDOUT_FILENO);
  }
}

void exec_cmd(token_t* token) {
  // execute command if not empty
  // error: Non-existing program
  if (token->argc == 0) exit(0);
  if (execvp(token->argv[0], token->argv) < 0)
    exit_process(NON_EXISTING_PROGRAM, token->argv[0]);
}

// exit process with exit code
void exit_process(int exit_code, char* content) {
  switch (exit_code) {
    case NORMAL_EXIT:
      printf("exit\n");
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

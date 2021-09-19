#include "process.h"

void exec_cmd(token_t* token) {
  // handle redirection
  if (token->read_file) {
    int file = open(token->src, O_RDONLY);
    // error: Non-existing file in input redirection
    if (file < 0) exit_process(NON_EXISTING_FILE, token->src);
    dup2(file, STDIN_FILENO);
  }
  if (token->append_file) {
    int file = open(token->dest, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
    // error: Failed to open file in output redirection
    if (file < 0) exit_process(NO_PERMISSION, token->dest);
    dup2(file, STDOUT_FILENO);
  } else if (token->write_file) {
    int file = open(token->dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    // error: Failed to open file in output redirection
    if (file < 0) exit_process(NO_PERMISSION, token->dest);
    dup2(file, STDOUT_FILENO);
  }

  // execute command if not empty
  // error: Non-existing program
  if (token->argc == 0) exit(0);
  char* cmd = token->argv[0];
  if (execvp(cmd, token->argv) < 0) exit_process(NON_EXISTING_PROGRAM, cmd);
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

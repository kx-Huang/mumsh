#include "process.h"

// initialize OLDPWD for cmd "cd -"
char OLDPWD[BUFFER_SIZE] = {0};

int mumsh_cmd_exit() {
  if (cmd.cnt == 1 && cmd.cmds[0].argc == 1 &&
      strcmp(cmd.cmds[0].argv[0], "exit") == 0)
    return NORMAL_EXIT;
  return -1;
}

int mumsh_cmd_cd() {
  if (cmd.cnt == 1 && cmd.cmds[0].argc >= 1 &&
      strcmp(cmd.cmds[0].argv[0], "cd") == 0) {
    // get current path
    char path[BUFFER_SIZE] = {0};
    getcwd(path, BUFFER_SIZE);
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
    return NORMAL_EXIT;
  }
  return -1;
}

void mumsh_cmd_pwd(token_t* token) {
  if (token->argc == 1 && strcmp(token->argv[0], "pwd") == 0) {
    char path[BUFFER_SIZE] = {0};
    if (getcwd(path, BUFFER_SIZE) != NULL) {
      printf("%s\n", path);
      fflush(stdout);
      exit(NORMAL_EXIT);
    }
    exit_process(UNEXPECTED_ERROR, "");
  }
}

void mumsh_exec_cmds() {
  io_redirect();
  for (size_t i = 0; i < cmd.cnt; i++) {
    mumsh_cmd_pwd(&cmd.cmds[i]);
    exec_cmd(&cmd.cmds[i]);
  }
}

void io_redirect() {
  // handle redirection
  if (cmd.read_file) {
    int file = open(cmd.src, O_RDONLY);
    // error: Non-existing file in input redirection
    if (file < 0) exit_process(NON_EXISTING_FILE, cmd.src);
    dup2(file, STDIN_FILENO);
  }
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
  // execute command if not empty
  if (token->argc == 0) exit(NORMAL_EXIT);
  // error: Non-existing program
  if (execvp(token->argv[0], token->argv) < 0)
    exit_process(NON_EXISTING_PROGRAM, token->argv[0]);
}

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

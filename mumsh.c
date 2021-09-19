#include "mumsh.h"

int main() {
  // subscribe ctrl-c notification
  struct sigaction action;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = sigint_handler;
  sigaction(SIGINT, &action, NULL);

  // main loop
  while (1) {
    // prompt and read input
    read_cmd();

    // ctrl-c interrupt terminal input
    if (ctrl_c == SIGINT) {
      ctrl_c = 0;
      printf("\n");
      fflush(stdout);
      continue;
    }

    // parse input command
    token_t token = parser();
    //debug(&token);

    // cmd "exit"
    if (token.argc != 0 && strncmp(token.argv[0], "exit", 4) == 0)
      exit_process(NORMAL_EXIT, "");

    // create child process
    pid_t pid = fork();
    if (pid < 0)
      exit_process(UNEXPECTED_ERROR, "");
    else if (pid == 0) {
      // create new process group for child process
      if (setpgid(pid, 0) != 0) exit_process(UNEXPECTED_ERROR, "");
      exec_cmd(&token);
    }

    // todo: handle background jobs here

    // wait for child process done
    waitpid(pid, NULL, WUNTRACED);

    // ctrl-c interrupt child process
    if (ctrl_c == SIGINT) {
      printf("\n");
      fflush(stdout);
      ctrl_c = 0;
    }

    // free after malloc token
    for (size_t i = 0; i < token.argc; i++) free(token.argv[i]);
  }
}

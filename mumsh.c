#include "mumsh.h"

int main() {
  // subscribe ctrl-c and tty-io signal notification
  struct sigaction sa_SIGTT;
  struct sigaction sa_SIGINT;
  sa_SIGTT.sa_flags = 0;
  sa_SIGINT.sa_flags = 0;
  sigemptyset(&sa_SIGTT.sa_mask);
  sigemptyset(&sa_SIGINT.sa_mask);
  sa_SIGTT.sa_handler = SIG_IGN;
  sa_SIGINT.sa_handler = sigint_handler;
  sigaction(SIGINT, &sa_SIGINT, NULL);
  sigaction(SIGTTOU, &sa_SIGTT, NULL);
  sigaction(SIGTTIN, &sa_SIGTT, NULL);

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
    // debug(&token);

    // cmd "exit"
    if (token.argc != 0 && strncmp(token.argv[0], "exit", 4) == 0)
      exit_process(NORMAL_EXIT, "");

    // create child process
    pid_t pid = fork();
    if (pid < 0)
      exit_process(UNEXPECTED_ERROR, "");
    else if (pid == 0) {
      sigaction(SIGINT, &sa_SIGINT, NULL);
      exec_cmd(&token);
    }

    // set child as terminal foreground process group leader
    setpgid(pid, 0);
    tcsetpgrp(STDOUT_FILENO, pid);

    // todo: handle background jobs here

    // wait for child process done
    waitpid(pid, NULL, WUNTRACED);

    // reset parent as terminal foreground process group leader
    tcsetpgrp(STDOUT_FILENO, getpgrp());

    // free after malloc token
    for (size_t i = 0; i < token.argc; i++) free(token.argv[i]);
  }
}

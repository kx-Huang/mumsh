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
    mumsh_read_cmds();

    // ctrl-c interrupt terminal input
    if (ctrl_c == SIGINT) {
      ctrl_c = 0;
      printf("\n");
      fflush(stdout);
      continue;
    }

    // parse input command
    if (mumsh_parser() != NORMAL) {  // free allocated memory
      free_memory();
      continue;
    }
    // debug();

    // no command exist
    if (cmd.cnt == 0) continue;

    // cmd "exit"
    if (mumsh_cmd_exit() == NORMAL) {
      free_memory();
      exit_process(NORMAL, "");
    }

    // built-in cmd "cd"
    if (mumsh_cmd_cd() == NORMAL) {
      free_memory();
      continue;
    }

    // execute cmds which run in child process
    mumsh_exec_cmds();

    // free allocated memory
    free_memory();
  }
}

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
    if (mumsh_parser() != 0) continue;
    // debug();

    // no command exist
    if (cmd.cnt == 0) continue;

    // cmd "exit"
    if (cmd.cnt == 1 && cmd.cmds[0].argc == 1 &&
        strncmp(cmd.cmds[0].argv[0], "exit", 4) == 0)
      exit_process(NORMAL_EXIT, "");

    // create child process
    pid_t pid = fork();
    if (pid < 0)
      exit_process(UNEXPECTED_ERROR, "");
    else if (pid == 0)
      mumsh_exec_cmds();

    // set child as terminal foreground process group leader
    setpgid(pid, 0);
    tcsetpgrp(STDOUT_FILENO, pid);

    // todo: handle background jobs here

    // wait for child process done
    waitpid(pid, NULL, WUNTRACED);

    // reset parent as terminal foreground process group leader
    tcsetpgrp(STDOUT_FILENO, getpgrp());

    // free after malloc token
    for (size_t i = 0; i < cmd.cnt; i++)
      for (size_t j = 0; j < cmd.cmds[i].argc; j++) free(cmd.cmds[i].argv[j]);
    free(cmd.cmds);
  }
}

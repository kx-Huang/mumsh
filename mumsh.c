#include "mumsh.h"

int main() {
  // handle ctrl-c interruption
  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sigaction(SIGINT, &sa, NULL);

  // main loop
  while (1) {
    prompt_mumsh();
    read_cmd();
    if (strncmp(cmd_buffer, "exit", 4) == 0) exit_mumsh(NORMAL_EXIT, "");

    // create child process
    pid_t pid = fork();
    if (pid < 0)
      exit_mumsh(UNEXPECTED_ERROR, "");
    else if (pid == 0)  // run child process
      parser();

    // todo: handle background jobs here

    // wait for child process done
    if (waitpid(pid, NULL, 0) != pid) exit_mumsh(UNEXPECTED_ERROR, "");
  }
  exit(0);
}

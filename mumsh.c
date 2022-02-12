#include "mumsh.h"

// variables for handling ctrl-c interruption
sigjmp_buf env;
volatile sig_atomic_t jump_active;

// main entry
int main() {
  // subscribe ctrl-c signal and ignore tty-io signal
  struct sigaction sa_SIGTT;
  struct sigaction sa_SIGINT;
  // appoint signal handler funtion (or set ignore)
  sa_SIGTT.sa_handler = SIG_IGN;
  sa_SIGINT.sa_handler = sigint_handler;
  // making certain system calls restartable across signals
  sa_SIGTT.sa_flags = SA_RESTART;
  sa_SIGINT.sa_flags = SA_RESTART;
  // no permission to interrupt execution of signal handler
  sigemptyset(&sa_SIGTT.sa_mask);
  sigemptyset(&sa_SIGINT.sa_mask);
  // activate signal subscription
  sigaction(SIGINT, &sa_SIGINT, NULL);
  sigaction(SIGTTOU, &sa_SIGTT, NULL);
  sigaction(SIGTTIN, &sa_SIGTT, NULL);
  // main loop
  while (1) {
    // ctrl-c interruption jumps to here
    if (sigsetjmp(env, 1) == 42) {
      printf("\n");
      // reap background jobs triggered by ctrl-c
      reap_background_jobs();
      free_cmds();
      continue;
    }
    // safe lock in case ctrl-c come before jump point is set
    jump_active = 1;
    // prompt and read input
    mumsh_read_cmds();
    // reap background jobs triggered by input
    reap_background_jobs();
    // parse input command
    if (mumsh_parser() != NORMAL) {
      // free allocated memory
      free_cmds();
      continue;
    }
    //debug_parser();
    // no command exist
    if (cmd.cnt == 0) continue;
    // cmd "exit"
    if (mumsh_cmd_exit() == NORMAL) {
      free_cmds();
      free_jobs();
      exit_process(NORMAL, "");
    }
    // built-in cmd "cd"
    if (mumsh_cmd_cd() == NORMAL) {
      free_cmds();
      continue;
    }
    // execute cmds which run in child process
    mumsh_exec_cmds();
    // free allocated memory
    free_cmds();
  }
  free_jobs();
}

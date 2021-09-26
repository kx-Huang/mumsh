#include "mumsh.h"

// variables for handling ctrl-c interruption
sigjmp_buf env;
volatile sig_atomic_t jump_active;

// main entry
int main() {
  // subscribe ctrl-c and tty-io signal notification for parent process
  struct sigaction sa_SIGTT;
  struct sigaction sa_SIGINT;
  sa_SIGTT.sa_flags = 0;
  sa_SIGINT.sa_flags = 0;
  sa_SIGTT.sa_handler = SIG_IGN;
  sa_SIGINT.sa_handler = sigint_handler;
  sa_SIGTT.sa_flags = SA_RESTART;
  sa_SIGINT.sa_flags = SA_RESTART;
  sigemptyset(&sa_SIGTT.sa_mask);
  sigemptyset(&sa_SIGINT.sa_mask);
  sigaction(SIGINT, &sa_SIGINT, NULL);
  sigaction(SIGTTOU, &sa_SIGTT, NULL);
  sigaction(SIGTTIN, &sa_SIGTT, NULL);

  // main loop
  while (1) {
    // ctrl-c interruption jumps to here
    if (sigsetjmp(env, 1) == 42) {
      printf("\n");
      free_memory();
      continue;
    }
    jump_active = 1;
    // prompt and read input
    mumsh_read_cmds();
    // parse input command
    if (mumsh_parser() != NORMAL) {  // free allocated memory
      free_memory();
      continue;
    }
    //debug_parser();
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

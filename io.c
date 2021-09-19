#include "io.h"

int ctrl_c;
char cmd_buffer[BUFFER_SIZE];

// ctrl-c interruption handler
void sigint_handler() { ctrl_c = SIGINT; }

// // handle parent ctrl-c interruption
// void sigint_handler_parent(int signal) {
//   if (signal != SIGINT) return;
//   printf("\n");
//   prompt_mumsh();
// }

// // handle child ctrl-c interruption
// void sigint_handler_child(int signal) {
//   if (signal != SIGINT) return;
//   printf("\n");
//   fflush(stdout);
// }

// prompt "mumsh"
void prompt_mumsh() {
  printf("mumsh $ ");
  fflush(stdout);
}

// read cmd into buffer
void read_cmd() {
  prompt_mumsh();
  int ch = 0, i = 0;
  while ((ch = getchar())) {
    if (ctrl_c == SIGINT)
      break;
    else if (ch == EOF)
      exit_process(NORMAL_EXIT, "");
    cmd_buffer[i++] = ch;
    if (ch == '\n') {
      cmd_buffer[i] = '\0';
      break;
    }
  }
}

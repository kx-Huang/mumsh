#include "io.h"

int ctrl_c;
char cmd_buffer[BUFFER_SIZE];

// ctrl-c interruption handler
void sigint_handler() { ctrl_c = SIGINT; }

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

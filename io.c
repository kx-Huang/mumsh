#include "io.h"

char cmd_buffer[BUFFER_SIZE];

// read cmd into buffer
void mumsh_read_cmds() {
  printf("mumsh $ ");
  fflush(stdout);
  write_cmd_buffer(cmd_buffer);
}

void read_dangling_cmds(char *buffer) {
  printf("> ");
  fflush(stdout);
  write_cmd_buffer(buffer);
}

// write cmd into buffer
void write_cmd_buffer(char *buffer) {
  int ch = 0, i = 0;
  while ((ch = getchar())) {
    if (ch == EOF) exit_process(NORMAL, "");
    buffer[i++] = (char)ch;
    if (ch == '\n') {
      buffer[i] = '\0';
      break;
    }
  }
}

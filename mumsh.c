#include "mumsh.h"

char cmd_buffer[BUFFER_SIZE];

int main(void) {
  while (1) {
    prompt_mumsh();
    read_cmd(cmd_buffer);
    if (strncmp(cmd_buffer, "exit", 4) == 0) exit_mumsh(NORMAL_EXIT, "");
  }
  return 0;
}

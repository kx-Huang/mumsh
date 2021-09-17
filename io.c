#include "io.h"

int ctrl_c;
char cmd_buffer[BUFFER_SIZE];

// ctrl-c interruption handler
void sigint_handler() { ctrl_c = SIGINT; }

// prompt "mumsh"
void prompt_mumsh(void) {
  printf("mumsh $ ");
  fflush(stdout);
}

// read cmd into buffer
void read_cmd() {
  int ch = 0, i = 0;
  while ((ch = getchar())) {
    if (ctrl_c == SIGINT) {
      ctrl_c = 0;
      printf("\n");
      prompt_mumsh();
    } else if (ch == EOF)
      exit_mumsh(NORMAL_EXIT, "");
    cmd_buffer[i++] = ch;
    if (ch == '\n') {
      cmd_buffer[i] = '\0';
      return;
    }
  }
}

// exit with exit code
void exit_mumsh(int exit_code, char* content) {
  switch (exit_code) {
    case NORMAL_EXIT:
      printf("exit\n");
      break;
    case NON_EXISTING_PROGRAM:
      fputs(content, stderr);
      fputs(": command not found\n", stderr);
      break;
    case NON_EXISTING_FILE:
      fputs(content, stderr);
      fputs(": No such file or directory\n", stderr);
      break;
    case NO_PERMISSION:
      fputs(content, stderr);
      fputs(": Permission denied\n", stderr);
      break;
    case DUP_INPUT_REDIRECTION:
      fputs("error: duplicated input redirection\n", stderr);
      break;
    case DUP_OUTPUT_REDIRECTION:
      fputs("error: duplicated output redirection\n", stderr);
      break;
    case ERROR_SYNTAX:
      fputs("syntax error near unexpected token `", stderr);
      fputs(content, stderr);
      fputs("'\n", stderr);
      break;
    case MISS_PROGRAM:
      fputs("error: missing program\n", stderr);
      break;
    case NON_EXISTING_DIR:
      fputs(content, stderr);
      fputs(": No such file or directory\n", stderr);
      break;
    case UNEXPECTED_ERROR:
      fputs("error: unexpected error\n", stderr);
      break;
    default:
      break;
  }
  exit(exit_code);
}

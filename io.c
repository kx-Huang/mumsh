#include "io.h"

#include <stddef.h>
#include <stdlib.h>

#include "hinter.h"
#include "parser.h"

char cmd_buffer[BUFFER_SIZE];

// read commands into buffer
void mumsh_read_cmds() {
  prompt_prefix();
  hint_interface(cmd_buffer);
}

// read dangling commands
void read_dangling_cmds(char *buffer) {
  printf("> ");
  fflush(stdout);
  offset_prefix = 2;
  hint_interface(buffer);
}

// write raw commands into buffer (retired)
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

void prompt_prefix() {
  len = 0, pos = 0;
  offset_prefix = 0;
  memset(cmd_buffer, 0, BUFFER_SIZE);
  clean_hint();
  prompt_path();
  prompt_mumsh();
}

void prompt_mumsh() {
  printf(PURB WHT " mumsh " RESET " ");
  fflush(stdout);
  offset_prefix += 8;
}

// print path in mumsh prompts
void prompt_path() {
  char *token[TOKEN_SIZE] = {NULL};
  char *path = malloc(TOKEN_SIZE);
  if (getcwd(path, TOKEN_SIZE) != NULL) {
    size_t depth = 0;
    int from_home = 0;
    char *path_start = path;
    if ((strstr(path, getenv("HOME"))) != NULL) {
      from_home = 1;
      path_start = path + strlen(getenv("HOME"));
    }
    char *tmp;
    if ((tmp = strtok(path_start, "/")) != NULL) token[depth++] = tmp;
    while (tmp != NULL) {
      tmp = strtok(NULL, "/");
      token[depth++] = tmp;
    }
    if (depth > 0) depth--;
    // debug_path(from_home, depth, token);
    if (from_home)
      printf(BLUB WHT " ~" RESET);
    else
      printf(BLUB WHT " /" RESET);
    offset_prefix += 2;
    if (depth > 3) {
      printf(BLUB WHT "/..." RESET);
      offset_prefix += 4;
    }
    for (size_t i = 0; i < depth; i++) {
      if (i + 4 > depth) {
        if (i != 0 || (i == 0 && from_home)) {
          printf(BLUB WHT "/" RESET);
          offset_prefix++;
        }
        printf(BLUB WHT "%s" RESET, token[i]);
        offset_prefix += strlen(token[i]);
      }
    }
    printf(BLUB WHT " " RESET);
    offset_prefix++;
    fflush(stdout);
  }
  free(path);
}

// debug prompt path
void debug_path(int from_home, size_t depth, char *token[TOKEN_SIZE]) {
  printf("from home: %d\n", from_home);
  printf("depth: %lu\n", depth);
  for (size_t i = 0; token[i] != NULL; i++) printf("[%lu] %s\n", i, token[i]);
  fflush(stdout);
}

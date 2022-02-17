#include "hinter.h"

size_t len;
size_t pos;
size_t num_hint;
size_t width_clean;
size_t offset_prefix;
char token[BUFFER_SIZE];
char puzzle[BUFFER_SIZE];
char path[TOKEN_SIZE];
char fit[TOKEN_SIZE];
char hint[BUFFER_SIZE][TOKEN_SIZE] = {{0}};

// capture keyboard event
int keyboard_get(void) {
  int c = keyboard_char();
  return (c == KEY_ESCAPE) ? keyboard_arrow() : c;
}

// capture keyboard hit
int keyboard_hit(void) {
  int c = 0;
  tcgetattr(0, &oterm);
  memcpy(&term, &oterm, sizeof(term));
  term.c_lflag &= (size_t) ~(ICANON | ECHO);
  term.c_cc[VMIN] = 0;
  term.c_cc[VTIME] = 1;
  tcsetattr(0, TCSANOW, &term);
  c = getchar();
  clearerr(stdin);
  tcsetattr(0, TCSANOW, &oterm);
  if (c != -1) ungetc(c, stdin);
  return (c != -1 ? 1 : 0);
}

// capture keyboard character
int keyboard_char(void) {
  int c = 0;
  tcgetattr(0, &oterm);
  memcpy(&term, &oterm, sizeof(term));
  term.c_lflag &= (size_t) ~(ICANON | ECHO);
  term.c_cc[VMIN] = 1;
  term.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &term);
  c = getchar();
  clearerr(stdin);
  tcsetattr(0, TCSANOW, &oterm);
  return c;
}

// capture keyboardd arrow
int keyboard_arrow(void) {
  if (!keyboard_hit()) return KEY_ESCAPE;
  int c = keyboard_char();
  if (c == '[') {
    switch (keyboard_char()) {
      case 'A':
        c = KEY_UP;
        break;
      case 'B':
        c = KEY_DOWN;
        break;
      case 'C':
        c = KEY_RIGHT;
        break;
      case 'D':
        c = KEY_LEFT;
        break;
      default:
        c = 0;
        break;
    }
  } else {
    c = 0;
  }
  if (c == 0)
    while (keyboard_hit()) keyboard_char();
  return c;
}

// hinter main capture/calculate/print loop
void mumsh_hinter(char *buffer) {
  strcpy(path, "./");
  // main loop
  while (1) {
    // get keyboard input
    int c = keyboard_get();
    // enter key
    if (c == KEY_ENTER) {
      clean_hint();
      buffer[len] = '\n';
      printf("\n");
      break;
    }
    // escape key
    if (c == KEY_ESCAPE) continue;
    // tab key
    if (c == KEY_TAB) {
      clean_hint();
      int type = hint_type(buffer);
      // cursor after space or none: all hint
      if (type == 1) {
        strcpy(puzzle, "");
        strcpy(path, "./");
      }
      // cursor after token: matched hint
      if (type == 2) {
        find_token(buffer, " <>|");
        create_puzzle();
        // if token is a path, print exclude "." and ".."
        type = strlen(puzzle) ? 2 : 1;
      }
      find_match(type);
      // auto complete if only one match
      if (num_hint >= 1) {
        longest_fit();
        auto_complete(buffer);
      }
      if (num_hint > 1) print_hint();
      continue;
    }
    // up and down key
    if (c == KEY_UP | c == KEY_DOWN) {
      // TODO: show history
      continue;
    }
    // left arrow key
    if (c == KEY_LEFT) {
      if (pos > 0) {
        cursor_backward(1);
        pos--;
      }
      continue;
    }
    // right arrow key
    if (c == KEY_RIGHT) {
      if (pos < len) {
        cursor_forward(1);
        pos++;
      }
      continue;
    }
    // delete key
    if (c == KEY_DELETE) {
      delete_char(buffer);
      continue;
    }
    // ordinary buffer
    write_char(c, buffer);
  }
  // debug_hinter(buffer);
}

// clean printed matched fileneams
void clean_hint() {
  if (num_hint) {
    char whitespace[BUFFER_SIZE] = {0};
    memset(whitespace, ' ', width_clean);
    for (size_t i = 0; i < num_hint; i++) printf("\n%s", whitespace);
    _calibrate_cursor(width_clean);
  }
}

// determine hint type
// input: buffer
// hint type: 0 - no hint
//            1 - full hint (without "." and "..")
//            2 - matched hint (include "." and "..")
int hint_type(char buffer[BUFFER_SIZE]) {
  // cursor at leftmost
  if (pos == 0 && len == 0) return 1;
  if (pos == 0 && len > 0) return buffer[pos] == ' ' ? 1 : 0;
  // cursor at rightmost
  if (pos == len) return buffer[pos - 1] == ' ' ? 1 : 2;
  //  cursor in middle
  if (buffer[pos - 1] == ' ' && buffer[pos] == ' ') return 1;
  if (buffer[pos - 1] != ' ' && buffer[pos] == ' ') return 2;
  return 0;
}

// find current token
// input: buffer[], delimiter[]
// update: token[]
void find_token(char buffer[BUFFER_SIZE], char delimiter[8]) {
  char *tmp = NULL;
  char input_dup[BUFFER_SIZE];
  memset(token, 0, BUFFER_SIZE);
  memset(input_dup, 0, BUFFER_SIZE);
  strncpy(input_dup, buffer, pos);
  tmp = strtok(input_dup, delimiter);
  while (tmp != NULL) {
    strcpy(token, tmp);
    tmp = strtok(NULL, delimiter);
  }
}

// separate token into path and puzzle
// input: token[]
// output: path[], puzzle[]
void create_puzzle() {
  // find last "/"
  int pos_slash = -1;
  size_t len_token = strlen(token);
  for (size_t i = 0; i < len_token; i++)
    if (token[i] == '/') pos_slash = (int)i;
  // no slash
  if (pos_slash == -1) {
    strcpy(puzzle, token);
    strcpy(path, "./");
    return;
  }
  // slash at last
  if (pos_slash == (int)len_token - 1) {
    strcpy(path, token);
    strcpy(puzzle, "");
    return;
  }
  // slash in middle
  strncpy(path, token, (size_t)pos_slash + 1);
  path[pos_slash + 1] = 0;
  strncpy(puzzle, token + pos_slash + 1, len_token - (size_t)pos_slash - 1);
  puzzle[len_token - (size_t)pos_slash - 1] = 0;
}

// find all matched files
// input: hint type (0 - no hint, 1 - full hint, 2 - matched hint)
// update: hint[][], num_hint
void find_match(int type) {
  num_hint = 0;
  struct dirent *dir;
  DIR *d = opendir(path);
  if (d) {
    size_t len_puzzle = strlen(puzzle);
    while ((dir = readdir(d)) != NULL) {
      int match = 1;
      char filename[TOKEN_SIZE] = {0};
      strcpy(filename, dir->d_name);
      // append '/' to path
      if (dir->d_type == DT_DIR) strcat(filename, "/");
      size_t len_name = strlen(filename);
      // filter shorter filename
      if (len_puzzle > len_name) continue;
      // find matched filename start with same pattern
      for (size_t i = 0; i < len_puzzle; i++) {
        if (dir->d_name[i] != puzzle[i]) {
          match = 0;
          break;
        }
      }
      // store to hint table
      if (match) {
        if ((strcmp(filename, "./") && strcmp(filename, "../")) || type == 2) {
          strcpy(hint[num_hint], filename);
          width_clean = width_clean > len_name ? width_clean : len_name;
          num_hint++;
        }
      }
    }
    // sort hint in alphabetical order
    qsort(hint, num_hint, TOKEN_SIZE, strcmp);
    closedir(d);
  }
}

// find the longest fit in match
// input: hint[][]
// update: fit[]
void longest_fit() {
  memset(fit, 0, TOKEN_SIZE);
  if (num_hint <= 0) return;
  if (num_hint == 1) strcpy(fit, hint[0]);
  for (size_t j = 0; hint[0][j]; j++) {
    char c = hint[0][j];
    int same = 1;
    for (size_t i = 1; i < num_hint; i++)
      if (hint[i][j] != c) {
        same = 0;
        break;
      }
    if (!same) break;
    fit[j] = c;
  }
}

// auto complete the longest fit of match
// matched part
// input: fit[], puzzle[]
// update: buffer[], pos, len
void auto_complete(char buffer[BUFFER_SIZE]) {
  if (strlen(fit)) {
    size_t offset = len - pos;
    size_t len_puzzle = strlen(puzzle);
    size_t len_add = strlen(fit) - len_puzzle;
    len += len_add;
    for (size_t i = len - 1; i >= pos + len_add; i--)
      buffer[i] = buffer[i - len_add];
    for (size_t i = 0; i < len_add; i++) buffer[pos + i] = fit[len_puzzle + i];
    for (size_t i = pos; i < len; i++) putchar(buffer[i]);
    pos += len_add;
    if (offset > 0) cursor_backward((int)(offset));
  }
}

// print matched filenames
void print_hint() {
  if (num_hint) {
    for (size_t i = 0; i < num_hint; i++) printf("\n%s", hint[i]);
    _calibrate_cursor(strlen(hint[num_hint - 1]));
  }
}

// input one char
// update: buffer, pos, len
void write_char(int c, char buffer[BUFFER_SIZE]) {
  putchar(c);
  for (size_t i = ++len; i > pos; i--) buffer[i] = buffer[i - 1];
  buffer[pos++] = (char)c;
  for (size_t i = pos; i < len; i++) putchar(buffer[i]);
  for (size_t i = 0; i < len - pos; i++) cursor_backward(1);
}

// delete one char
// update: buffer, pos, len
void delete_char(char buffer[BUFFER_SIZE]) {
  if (pos) {
    cursor_backward(1);
    if (len) len--;
    for (size_t i = --pos; i < len; i++) buffer[i] = buffer[i + 1];
    buffer[len] = 0;
    for (size_t i = pos; i < len; i++) putchar(buffer[i]);
    printf(" ");
    cursor_backward((int)(len - pos + 1));
  }
}

// move cursor to user input
void _calibrate_cursor(size_t pos_now) {
  cursor_upward((int)num_hint);
  if (pos_now < pos + offset_prefix)
    cursor_forward((int)(pos + offset_prefix - pos_now));
  else if (pos_now > pos + offset_prefix)
    cursor_backward((int)(pos_now - pos - offset_prefix));
}

// debug hinter
void debug_hinter(char *buffer) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  printf("------ Debug ------\n");
  printf("input: %s", buffer);
  printf("path: %s\n", path);
  printf("puzzle: %s\n", puzzle);
  printf("cursor pos: %zu\n", pos);
  printf("buffer len: %zu\n", len);
  printf("ttl length: %d\n", w.ws_col);
  printf("ttl height: %d\n", w.ws_row);
  printf("-------------------\n");
}

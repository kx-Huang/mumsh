#include "hinter.h"

#define cursor_forward(x) printf("\033[%dC", (x))
#define cursor_backward(x) printf("\033[%DD", (x))
#define cursor_upward(x) printf("\033[%DA", (x))

#define KEY_ESCAPE 0x001b
#define KEY_DELETE 0x007f
#define KEY_TAB 0x0009
#define KEY_ENTER 0x000a
#define KEY_UP 0x0105
#define KEY_DOWN 0x0106
#define KEY_RIGHT 0x0107
#define KEY_LEFT 0x0108

static struct termios term, oterm;

static int getch(void);
static int kbhit(void);
static int kbesc(void);
static int kbget(void);

size_t len;
size_t pos;
size_t num_hint;
size_t width_clean;
size_t offset_prefix;
char token[BUFFER_SIZE];
char puzzle[TOKEN_SIZE];
char path[TOKEN_SIZE];
char hint[BUFFER_SIZE][TOKEN_SIZE] = {{0}};

static int getch(void) {
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

static int kbhit(void) {
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

static int kbesc(void) {
  if (!kbhit()) return KEY_ESCAPE;
  int c = getch();
  if (c == '[') {
    switch (getch()) {
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
    while (kbhit()) getch();
  return c;
}

static int kbget(void) {
  int c = getch();
  return (c == KEY_ESCAPE) ? kbesc() : c;
}

// move cursor to user buffer
void _calibrate_cursor(size_t pos_now) {
  cursor_upward((int)num_hint);
  if (pos_now < pos + offset_prefix)
    cursor_forward((int)(pos + offset_prefix - pos_now));
  else if (pos_now > pos + offset_prefix)
    cursor_backward((int)(pos_now - pos - offset_prefix));
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

// find current token
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

// return number of matched files
size_t solve_puzzle(int type) {
  size_t num_hint = 0;
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
  return num_hint;
}

// return position of the cursor after printing
void print_hint() {
  if (num_hint) {
    for (size_t i = 0; i < num_hint; i++) printf("\n%s", hint[i]);
    _calibrate_cursor(strlen(hint[num_hint - 1]));
  }
}

// auto complete the matched part of filename
void auto_complete(char buffer[BUFFER_SIZE]) {
  // find the maximum match
  char match[TOKEN_SIZE] = {0};
  if (num_hint > 1) {
    for (size_t j = 0; hint[0][j]; j++) {
      char c = hint[0][j];
      int same = 1;
      for (size_t i = 1; i < num_hint; i++)
        if (hint[i][j] != c) {
          same = 0;
          break;
        }
      if (!same) break;
      match[j] = c;
    }
  }
  if (num_hint == 1) strcpy(match, hint[0]);
  // plug in matched part
  if (strlen(match)) {
    size_t offset = len - pos;
    size_t len_puzzle = strlen(puzzle);
    size_t len_add = strlen(match) - len_puzzle;
    len += len_add;
    for (size_t i = len - 1; i >= pos + len_add; i--)
      buffer[i] = buffer[i - len_add];
    for (size_t i = 0; i < len_add; i++)
      buffer[pos + i] = match[len_puzzle + i];
    for (size_t i = pos; i < len; i++) putchar(buffer[i]);
    pos += len_add;
    if (offset > 0) cursor_backward((int)(offset));
  }
}

// buffer one char
void write_char(int c, char buffer[BUFFER_SIZE]) {
  putchar(c);
  for (size_t i = ++len; i > pos; i--) buffer[i] = buffer[i - 1];
  buffer[pos++] = (char)c;
  for (size_t i = pos; i < len; i++) putchar(buffer[i]);
  for (size_t i = 0; i < len - pos; i++) cursor_backward(1);
}

// delete one char
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

// 0: no hint
// 1: full hint (without "." and "..")
// 2: matched hint (include "." and "..")
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

void hint_interface(char *buffer) {
  int c;
  strcpy(path, "./");

  while (1) {
    c = kbget();
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
      // reset variable
      clean_hint();
      int type = hint_type(buffer);
      // all hint: after space or none
      if (type == 1) {
        strcpy(puzzle, "");
        strcpy(path, "./");
        num_hint = solve_puzzle(type);
        // only one match: auto complete
        if (num_hint >= 1) auto_complete(buffer);
        if (num_hint > 1) print_hint();
        continue;
      }
      // matched hint: after token
      if (type == 2) {
        find_token(buffer, " <>|");
        create_puzzle();
        // if token is a path, print exclude "." and ".."
        type = strlen(puzzle) ? 2 : 1;
        num_hint = solve_puzzle(type);
        // auto complete if only one found
        if (num_hint >= 1) auto_complete(buffer);
        if (num_hint > 1) print_hint();
      }
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
  // debug_hint(buffer);
}

void debug_hint(char *buffer) {
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

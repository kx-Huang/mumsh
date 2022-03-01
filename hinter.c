#include "hinter.h"

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
      clean_history_hint();
      save_history(buffer);
      buffer[len] = '\n';
      printf("\n");
      break;
    }
    // escape key
    if (c == KEY_ESCAPE) continue;
    // tab key
    if (c == KEY_TAB) {
      clean_history_hint();
      clean_hint();
      hint_t type = hint_type(buffer);
      // cursor after space or none: full hint
      if (type == FULL_HINT) {
        strcpy(puzzle, "");
        strcpy(path, "./");
      }
      // cursor after token: matched hint
      if (type == MATCHED_HINT) {
        find_token(buffer, " <>|");
        create_puzzle();
        // if token is a path, print exclude "." and ".."
        type = strlen(puzzle) ? MATCHED_HINT : FULL_HINT;
      }
      if (type != NO_HINT) {
        find_match_filename(type);
        // auto complete if only one match
        if (num_hint >= 1) {
          longest_fit();
          auto_complete(buffer);
        }
        if (num_hint > 1) print_hint();
      }
      continue;
    }
    // up and down key
    if (c == KEY_UP | c == KEY_DOWN) {
      // no response if not in history mode
      if (c == KEY_DOWN && !mode_hint && !mode_iterate) continue;
      clean_hint();
      // if last mode is iterate mode, up or down all history index by one
      if (mode_iterate) {
        if (c == KEY_UP) {
          if (num_history_all && index_history_all) index_history_all--;
        } else if (c == KEY_DOWN) {
          if (num_history_all && index_history_all < num_history_all - 1) {
            index_history_all++;
          } else {
            mode_hint = false;
            mode_iterate = false;
            clean_buffer(buffer);
            continue;
          }
        }
        // if last mode is match mode, up or down matched history index by one
      } else if (mode_hint) {
        if (c == KEY_UP) {
          // feature: click KEY UP, if not match then move cursor to right most
          if (num_history_hint && index_history_hint) index_history_hint--;
        } else if (c == KEY_DOWN) {
          if (num_history_hint && index_history_hint < num_history_hint - 1) {
            index_history_hint++;
          }
        }
        // if cursor leftmost, start iterate mode
      } else if (pos == 0) {
        if (c == KEY_DOWN) continue;
        mode_hint = false;
        mode_iterate = true;
        if (num_history_all) index_history_all = num_history_all - 1;
      } else {
        // find all matched history
        mode_hint = true;
        mode_iterate = false;
        find_match_history(buffer);
        if (num_history_hint <= 1) continue;
        index_history_hint = num_history_hint - 2;
      }
      // write history into buffer and display
      clean_buffer(buffer);
      strcpy(puzzle, "");
      if (mode_iterate && num_history_all)
        strcpy(fit, history_all[index_history_all]);
      else if (mode_hint && num_history_hint)
        strcpy(fit, history_hint[index_history_hint]);
      auto_complete(buffer);
      continue;
    }
    // left arrow key
    if (c == KEY_LEFT) {
      clean_history_hint();
      index_history_hint = 0;
      if (pos > 0) {
        cursor_backward((size_t)1);
        pos--;
      }
      continue;
    }
    // right arrow key
    if (c == KEY_RIGHT) {
      clean_history_hint();
      if (pos < len) {
        cursor_forward((size_t)1);
        pos++;
      }
      continue;
    }
    // delete key
    if (c == KEY_DELETE) {
      clean_history_hint();
      delete_char(buffer);
      continue;
    }
    // ordinary buffer
    mode_hint = false;
    mode_iterate = false;
    write_char(c, buffer);
  }
  // debug_hinter(buffer);
}

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

// compare method for qsort, used to sort hint[][]
int cmp(void const *a, void const *b) {
  return strcmp((char const *)a, (char const *)b);
}

// clean all input
// update: buffer[], len, pos
void clean_buffer(char buffer[BUFFER_SIZE]) {
  if (len > pos) {
    cursor_forward(len - pos);
    pos = len;
  }
  size_t len_delete = len;
  for (size_t i = 0; i < len_delete; i++) delete_char(buffer);
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
// input: buffer[]
// return: hint type
hint_t hint_type(char buffer[BUFFER_SIZE]) {
  // cursor at leftmost
  if (pos == 0 && len == 0) return FULL_HINT;
  if (pos == 0 && len > 0) return buffer[pos] == ' ' ? FULL_HINT : NO_HINT;
  // cursor at rightmost
  if (pos == len) return buffer[pos - 1] == ' ' ? FULL_HINT : MATCHED_HINT;
  //  cursor in middle
  if (buffer[pos - 1] == ' ' && buffer[pos] == ' ') return FULL_HINT;
  if (buffer[pos - 1] != ' ' && buffer[pos] == ' ') return MATCHED_HINT;
  return NO_HINT;
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
// input: hint type
// update: hint[][], num_hint
void find_match_filename(hint_t type) {
  num_hint = 0;
  from_home = false;
  // replace ~ with home path
  if (strncmp(path, "~/", 2) == 0) {
    from_home = true;
    struct passwd *pw = getpwuid(getuid());
    char tmp[BUFFER_SIZE] = {0};
    strcpy(tmp, path);
    strcpy(path, pw->pw_dir);
    strcat(path, "/");
    if (strlen(tmp) > 2) strcat(path, tmp + 2);
  }
  struct dirent *dir;
  DIR *d = opendir(path);
  if (d) {
    size_t len_puzzle = strlen(puzzle);
    while ((dir = readdir(d)) != NULL) {
      bool match = true;
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
          match = false;
          break;
        }
      }
      // store to hint table
      if (match) {
        if ((strcmp(filename, "./") && strcmp(filename, "../")) ||
            type == MATCHED_HINT) {
          strcpy(hint[num_hint], filename);
          width_clean = width_clean > len_name ? width_clean : len_name;
          num_hint++;
        }
      }
    }
    // sort hint in alphabetical order
    qsort(hint, num_hint, TOKEN_SIZE, cmp);
    closedir(d);
  }
}

// find the longest fit in match
// input: hint[][]
// update: fit[]
void longest_fit() {
  memset(fit, 0, BUFFER_SIZE);
  if (num_hint <= 0) return;
  if (num_hint == 1) strcpy(fit, hint[0]);
  for (size_t j = 0; hint[0][j]; j++) {
    char c = hint[0][j];
    bool same = true;
    for (size_t i = 1; i < num_hint; i++)
      if (hint[i][j] != c) {
        same = false;
        break;
      }
    if (!same) break;
    fit[j] = c;
  }
}

// find the matched command history except itself
// input: history[][], buffer[]
// update: history_hint[], num_history_hint
// feature: num_history_hint >= 1
void find_match_history(char buffer[BUFFER_SIZE]) {
  num_history_hint = 0;
  char last_match[BUFFER_SIZE] = {0};
  for (size_t i = 0; i < num_history_all; i++) {
    if (strlen(history_all[i]) > strlen(buffer) &&
        strcmp(last_match, history_all[i]) &&
        strncmp(history_all[i], buffer, strlen(buffer)) == 0) {
      strcpy(last_match, history_all[i]);
      history_hint = _alloc_2D_array(history_hint, &num_history_hint);
      history_hint[num_history_hint - 1] = malloc(BUFFER_SIZE);
      strcpy(history_hint[num_history_hint - 1], history_all[i]);
    }
  }
  // save current token into history
  history_hint = _alloc_2D_array(history_hint, &num_history_hint);
  history_hint[num_history_hint - 1] = malloc(BUFFER_SIZE);
  strcpy(history_hint[num_history_hint - 1], buffer);
}

// auto complete the longest fit of match
// input: fit[], puzzle[]
// update: buffer[], len, pos
void auto_complete(char buffer[BUFFER_SIZE]) {
  size_t len_fit = strlen(fit);
  for (size_t i = strlen(puzzle); i < len_fit; i++) write_char(fit[i], buffer);
}

// print matched filenames
void print_hint() {
  if (num_hint) {
    for (size_t i = 0; i < num_hint; i++) printf("\n%s", hint[i]);
    _calibrate_cursor(strlen(hint[num_hint - 1]));
  }
}

// input one char
// update: buffer[], pos, len
void write_char(int c, char buffer[BUFFER_SIZE]) {
  putchar(c);
  for (size_t i = ++len; i > pos; i--) buffer[i] = buffer[i - 1];
  buffer[pos++] = (char)c;
  for (size_t i = pos; i < len; i++) putchar(buffer[i]);
  for (size_t i = 0; i < len - pos; i++) cursor_backward((size_t)1);
}

// delete one char
// update: buffer[], pos, len
void delete_char(char buffer[BUFFER_SIZE]) {
  if (pos) {
    cursor_backward((size_t)1);
    if (len) len--;
    for (size_t i = --pos; i < len; i++) buffer[i] = buffer[i + 1];
    buffer[len] = 0;
    for (size_t i = pos; i < len; i++) putchar(buffer[i]);
    printf(" ");
    cursor_backward(len - pos + 1);
  }
}

// save command into history (except for all spaces)
void save_history(char buffer[BUFFER_SIZE]) {
  // if all space, do nothing
  int all_space = 1;
  size_t len = strlen(buffer);
  for (size_t i = 0; i < len; i++) {
    if (buffer[i] != ' ') {
      all_space = 0;
      break;
    }
  }
  if (all_space) return;
  // if command is same as last one, do nothing
  if (num_history_all && !strcmp(buffer, history_all[num_history_all - 1]))
    return;
  // save command to history
  history_all = _alloc_2D_array(history_all, &num_history_all);
  history_all[num_history_all - 1] = malloc(BUFFER_SIZE);
  strcpy(history_all[num_history_all - 1], buffer);
}

// move cursor to user input
void _calibrate_cursor(size_t pos_now) {
  cursor_upward(num_hint);
  if (pos_now < pos + offset_prefix)
    cursor_forward(pos + offset_prefix - pos_now);
  else if (pos_now > pos + offset_prefix)
    cursor_backward(pos_now - pos - offset_prefix);
}

// allocate memory for pointer
char **_alloc_2D_array(char **ptr, size_t *len) {
  (*len)++;
  if (*len == 1)
    return malloc(sizeof(char *));
  else
    return realloc(ptr, *len * sizeof(char *));
}

// clear history indicator
void clean_history_hint() {
  mode_hint = false;
  mode_iterate = false;
  index_history_all = 0;
  index_history_hint = 0;
  for (size_t i = 0; i < num_history_hint; i++) free(history_hint[i]);
  if (num_history_hint) free(history_hint);
  num_history_hint = 0;
}

// debug hinter
void debug_hinter(char *buffer) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  printf("------- Debug -------\n");
  printf("input: %s", buffer);
  printf("path: %s\n", path);
  printf("puzzle: %s\n", puzzle);
  printf("cursor pos: %zu\n", pos);
  printf("buffer len: %zu\n", len);
  printf("ttl length: %d\n", w.ws_col);
  printf("ttl height: %d\n", w.ws_row);
  printf("---- All History ----\n");
  for (size_t i = 0; i < num_history_all; i++)
    printf("[%zu]: %s\n", i, history_all[i]);
  printf("-- Matched History --\n");
  for (size_t i = 0; i < num_history_hint; i++)
    printf("[%zu]: %s\n", i, history_hint[i]);
  printf("------ Output -------\n");
}

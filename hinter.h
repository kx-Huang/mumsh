#ifndef HINT_H
#define HINT_H

#include <dirent.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "data.h"

#define cursor_forward(x) printf("\033[%zuC", x)
#define cursor_backward(x) printf("\033[%zuD", x)
#define cursor_upward(x) printf("\033[%zuA", x)

#define KEY_ESCAPE 0x001b
#define KEY_DELETE 0x007f
#define KEY_TAB 0x0009
#define KEY_ENTER 0x000a
#define KEY_UP 0x0105
#define KEY_DOWN 0x0106
#define KEY_RIGHT 0x0107
#define KEY_LEFT 0x0108

typedef enum hint_type {
  NO_HINT = 0,
  FULL_HINT = 1,
  MATCHED_HINT = 2
} hint_t;

size_t len;                 // length of input
size_t pos;                 // position of cursor
size_t num_hint;            // number of matched filenames
size_t num_history_all;     // number of all command history
size_t num_history_hint;    // number of matched command history
size_t width_clean;         // maximum width of hinted filenames
size_t offset_prefix;       // number of mumsh prefix character
size_t index_history_all;   // index of all command history
size_t index_history_hint;  // index of matched command history
bool mode_iterate;          // show all command history
bool mode_hint;             // show matched command history
bool from_home;             // path from home directory

char token[BUFFER_SIZE];             // path or word separate by space
char puzzle[TOKEN_SIZE];             // letters to be completed
char path[TOKEN_SIZE];               // path part of token
char fit[BUFFER_SIZE];               // longest string for auto-complete
char hint[BUFFER_SIZE][TOKEN_SIZE];  // all matched filenames in path of token
char history_all[BUFFER_SIZE][BUFFER_SIZE];    // all command history
char history_match[BUFFER_SIZE][BUFFER_SIZE];  // matched command history

// sample command: ls ~/Github/mum| > out (here '|' stands for cursor)
// token:   ~/Github/mum
// path:    ~/Github/
// puzzle:  mum
// fit:     mumsh
// hint:    mumsh
// len:     21
// pos:     15
// num_hint:    1
// width_clean: 0
// from_home:   true

// capture key-board input
struct termios term, oterm;
int keyboard_get(void);
int keyboard_hit(void);
int keyboard_char(void);
int keyboard_arrow(void);

// called in io.c
void mumsh_hinter(char *buffer);
void clean_hint();

// called in hinter.c
void clean_buffer(char buffer[BUFFER_SIZE]);
hint_t hint_type(char buffer[BUFFER_SIZE]);
void find_token(char buffer[BUFFER_SIZE], char delimiter[8]);
void create_puzzle();
void find_match_filename(hint_t type);
void find_match_history(char buffer[BUFFER_SIZE]);
void longest_fit();
void auto_complete(char buffer[BUFFER_SIZE]);
void print_hint();
void write_char(int c, char buffer[BUFFER_SIZE]);
void delete_char(char buffer[BUFFER_SIZE]);
void save_history(char buffer[BUFFER_SIZE]);
void _calibrate_cursor(size_t pos_now);

// for debug
void debug_hinter(char *buffer);

#endif  // HINT_H

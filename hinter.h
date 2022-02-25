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

size_t len;
size_t pos;
size_t num_hint;
size_t num_history_all;
size_t num_history_match;
size_t width_clean;
size_t offset_prefix;
size_t index_history_all;
size_t index_history_match;
bool iterate_mode;
bool match_mode;
bool from_home;
char token[BUFFER_SIZE];
char puzzle[TOKEN_SIZE];
char path[TOKEN_SIZE];
char fit[BUFFER_SIZE];
char hint[BUFFER_SIZE][TOKEN_SIZE];
char history_all[BUFFER_SIZE][BUFFER_SIZE];
char history_match[BUFFER_SIZE][BUFFER_SIZE];

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
int hint_type(char buffer[BUFFER_SIZE]);
void find_token(char buffer[BUFFER_SIZE], char delimiter[8]);
void create_puzzle();
void find_match_filename(int type);
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

#ifndef HINT_H
#define HINT_H

#include <dirent.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "data.h"

// called in io.c
void clean_hint();
void hint_interface(char *buffer);

// called in hinter.c
int hint_type(char buffer[BUFFER_SIZE]);
void find_token(char buffer[BUFFER_SIZE], char delimiter[8]);
void create_puzzle();
void solve_puzzle();
void print_hint();
void auto_complete(char buffer[BUFFER_SIZE]);
void write_char(int c, char buffer[BUFFER_SIZE]);
void delete_char(char buffer[BUFFER_SIZE]);

// for debug
void debug_hint(char *buffer);

#endif  // HINT_H

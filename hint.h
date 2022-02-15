#ifndef HINT_H
#define HINT_H

#include <dirent.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "data.h"

void clean_hint();
void hint_interface(char *buffer);

void debug_hint(char *buffer);

#endif  // HINT_H

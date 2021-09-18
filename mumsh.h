#ifndef MEMSH_H
#define MEMSH_H

#include <sys/wait.h>
#include <unistd.h>

#include "io.h"
#include "parser.h"

#define SIGINT 2

void sigint_handler();

#endif  // MEMSH_H

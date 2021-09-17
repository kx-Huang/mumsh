#include "parser.h"

void parser() {
  parse_pipe();
  build_pipe();
  parse_cmd();
  exec_cmd();
}

void parse_pipe() {}

void build_pipe() {}

void parse_cmd() {}

void exec_cmd() { exit(0); }

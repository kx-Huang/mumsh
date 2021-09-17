#include "parser.h"

void parser() {
  token_t token = {0, 0, 0, 0, {0}, {0}, {0}};
  parser_t parser = {0};
  // finite state machine
  for (size_t i = 0; cmd_buffer[i] != '\n'; i++) {
    // out of quotation region
    if (parser.in_quote == 0) {
      // "<" as operator
      if (cmd_buffer[i] == '<') {
        parser.parse_orig = 1;
        token.read_file = 1;

        // > as operator
      } else if (cmd_buffer[i] == '>') {
        parser.parse_dest = 1;
        token.write_file = 1;

        // >> as operator
        if (cmd_buffer[i + 1] == '>') {
          token.append = 1;
        }

        // ' as indicator
      } else if (cmd_buffer[i] == '\'') {
        parser.in_quote = 1;
        parser.in_single_quote = 1;

        // " as indicator
      } else if (cmd_buffer[i] == '\"') {
        parser.in_quote = 1;
        parser.in_double_quote = 1;

        // space as separator
      } else if (cmd_buffer[i] == ' ') {
        continue;

        // ordinary character
      } else {
        // where argv start
        if (i == 0 || cmd_buffer[i - 1] == ' ') parser.head = i;
        if (cmd_buffer[i + 1] == '\n' | cmd_buffer[i + 1] == ' ') {
          parser.tail = i;
          // where argv end
          if (parser.parse_dest) {
            // todo: put argv into corresponding place
          } else if (parser.parse_orig) {
            // todo: put argv into corresponding place
          }
        }
      }

      // in between quotation region
    } else if (parser.in_quote) {
      // todo: check imcomplete token
    }
  }
}

void exec_cmd() { exit(0); }

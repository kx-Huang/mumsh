#include "parser.h"

token_t parser() {
  token_t token = {0, 0, 0, {0}, {0}, {0}, 0};
  parser_t parser = {0, 0, 0, 0, {0}};

  // finite state machine
  for (size_t i = 0, j = 0; i < BUFFER_SIZE; i++) {
    // out of quotation region
    if (parser.in_single_quote == 0 && parser.in_double_quote == 0) {
      // space/tab/newline/redirector as separator
      if (cmd_buffer[i] == ' ' || cmd_buffer[i] == '\t' ||
          cmd_buffer[i] == '\n' || cmd_buffer[i] == '<' ||
          cmd_buffer[i] == '>') {
        // no buffer
        if (j == 0 && (cmd_buffer[i] == ' ' || cmd_buffer[i] == '\t')) continue;
        // have buffer
        parser.buffer[j] = '\0';
        if (j != 0) {
          if (parser.is_src) {
            memcpy(token.src, parser.buffer, j + 1);
            parser.is_src = 0;
          } else if (parser.is_dest) {
            memcpy(token.dest, parser.buffer, j + 1);
            parser.is_dest = 0;
          } else {
            token.argv[token.argc] = malloc(j + 1);
            memcpy(token.argv[token.argc++], parser.buffer, j + 1);
          }
        }
        // clear buffer
        j = 0;
        // < as operator
        if (cmd_buffer[i] == '<') {
          parser.is_dest = 0;
          parser.is_src = 1;
          token.read_file = 1;
        }
        // > as operator
        else if (cmd_buffer[i] == '>') {
          parser.is_src = 0;
          parser.is_dest = 1;
          token.write_file = 1;
          // >> as operator
          if (cmd_buffer[i + 1] == '>') {
            token.append_file = 1;
          }
        }
        // end parser
        else if (cmd_buffer[i] == '\n') {
          break;
        }
      }
      // ' as indicator
      else if (cmd_buffer[i] == '\'') {
        parser.in_single_quote = 1;
      }
      // " as indicator
      else if (cmd_buffer[i] == '\"') {
        parser.in_double_quote = 1;
      }
      // ordinary character, push into buffer
      else {
        parser.buffer[j++] = cmd_buffer[i];
      }
    }
    // in between quotation region
    else {
      // dangling quotation
      if (cmd_buffer[i] == '\n') {
        // todo: prompt for input
        exit_mumsh(UNEXPECTED_ERROR, "");
      }
      // single quotation ends
      if (parser.in_single_quote && cmd_buffer[i] == '\'') {
        parser.in_single_quote = 0;
        continue;
      }
      // double quotation ends
      if (parser.in_double_quote && cmd_buffer[i] == '\"') {
        parser.in_double_quote = 0;
        continue;
      }
      // ordinary character, push into buffer
      parser.buffer[j++] = cmd_buffer[i];
    }
  }

  return token;
}

void exec_cmd(token_t* token) {
  // handle redirection
  if (token->read_file) {
    int file = open(token->src, O_RDONLY);
    if (file < 0) exit_mumsh(NON_EXISTING_FILE, token->src);
    dup2(file, STDIN_FILENO);
  }
  if (token->append_file) {
    int file = open(token->dest, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (file < 0) exit_mumsh(NO_PERMISSION, token->dest);
    dup2(file, STDOUT_FILENO);
  } else if (token->write_file) {
    int file = open(token->dest, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file < 0) exit_mumsh(NO_PERMISSION, token->dest);
    dup2(file, STDOUT_FILENO);
  }

  // execute command
  int error = 0;
  char* cmd = token->argv[0];
  if (token->argc != 0) error = execvp(cmd, token->argv);
  if (error < 0) exit_mumsh(NON_EXISTING_PROGRAM, cmd);
  exit(0);
}

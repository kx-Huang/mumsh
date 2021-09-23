#include "parser.h"

cmd_t cmd = {0, 0, 0, 0, {0}, {0}, NULL};

void debug() {
  // printf("cmd cnt: %lu\n", cmd.cnt);
  for (size_t i = 0; i < cmd.cnt; i++)
    for (size_t j = 0; j < cmd.cmds[i].argc; j++)
      printf("argv[%lu][%lu]: \"%s\"\n", i, j, cmd.cmds[i].argv[j]);
  printf("src: \"%s\"\n", cmd.src);
  printf("dest: \"%s\"\n", cmd.dest);
  printf("read? %d\n", cmd.read_file);
  printf("write? %d\n", cmd.write_file);
  printf("append? %d\n", cmd.append_file);
}

void init_cmd() {
  cmd.cnt = 0;
  cmd.read_file = 0;
  cmd.write_file = 0;
  cmd.append_file = 0;
  memset(cmd.src, 0, BUFFER_SIZE);
  memset(cmd.dest, 0, BUFFER_SIZE);
  cmd.cmds = NULL;
}

int mumsh_parser() {
  // initialize data structure
  init_cmd();
  parser_t parser = {0, 0, 0, 0, 0, 0, {0}};
  token_t token = {0, {NULL}};

  // allocate memory for one token struct
  cmd.cmds = malloc(sizeof(token_t));

  // finite state machine
  for (size_t i = 0; i < BUFFER_SIZE; i++) {
    // out of quotation region
    if (parser.in_single_quote == 0 && parser.in_double_quote == 0) {
      // space/tab/newline/redirector as separator
      if (strchr(" \t\n<>|", cmd_buffer[i]) != NULL) {
        // no buffer
        if (parser.buffer_len == 0) {
          if (strchr(" \t", cmd_buffer[i]) != NULL) continue;
          if (strchr("<>|", cmd_buffer[i]) != NULL) {
            // error 6: Syntax Error
            if (parser.is_dest || parser.is_src) {
              if (cmd_buffer[i] == '<') return syntax_error(ERROR_SYNTAX, "<");
              if (cmd_buffer[i] == '>') return syntax_error(ERROR_SYNTAX, ">");
              if (cmd_buffer[i] == '|') return syntax_error(ERROR_SYNTAX, "|");
            }
          }

          // have buffer
        } else {
          parser.buffer[parser.buffer_len] = '\0';
          // save buffer as redirect source
          if (parser.is_src) {
            memcpy(cmd.src, parser.buffer, parser.buffer_len + 1);
            parser.is_src = 0;
            cmd.read_file = 1;
            // save buffer as redirect destination
          } else if (parser.is_dest) {
            memcpy(cmd.dest, parser.buffer, parser.buffer_len + 1);
            parser.is_dest = 0;
            cmd.write_file = 1;
            // save buffer as argument
          } else {
            token.argv[token.argc] = malloc(parser.buffer_len + 1);
            memcpy(token.argv[token.argc++], parser.buffer,
                   parser.buffer_len + 1);
          }
          // clear buffer
          parser.buffer_len = 0;
        }

        // change state of FSM
        // space/tab have no state to set
        if (cmd_buffer[i] == ' ' || cmd_buffer[i] == '\t') continue;

        // set buffer state for redirector
        if (cmd_buffer[i] == '<') {
          // error 4: Duplicated input redirection
          if (cmd.read_file || parser.is_pipe)
            return syntax_error(DUP_INPUT_REDIRECTION, "");
          parser.is_dest = 0;
          parser.is_src = 1;
        } else if (cmd_buffer[i] == '>') {
          // error 5: Duplicated output redirection
          if (cmd.write_file) return syntax_error(DUP_OUTPUT_REDIRECTION, "");
          parser.is_src = 0;
          parser.is_dest = 1;
          if (cmd_buffer[i + 1] == '>') {  // >>
            cmd.append_file = 1;
            i++;  // skip the second > for >> as a hole
          }

          // save argvs to cmd list and set buffer state for pipe
        } else if (cmd_buffer[i] == '|') {
          // error 7: Missing program
          if (token.argc == 0) return syntax_error(MISS_PROGRAM, "");
          // error 5: Duplicated output redirection
          if (cmd.write_file) return syntax_error(DUP_OUTPUT_REDIRECTION, "");
          cmd.cmds[cmd.cnt++] = token;
          token.argc = 0;
          memset(token.argv, 0, BUFFER_SIZE);
          cmd.cmds = realloc(cmd.cmds, 2 * sizeof(token_t));
          parser.is_pipe = 1;

          // incomplete input check and terminate parsing process
        } else if (cmd_buffer[i] == '\n') {
          // error 7: Missing program (only redirection)
          if (token.argc == 0 && (cmd.read_file || cmd.write_file)) {
            return syntax_error(MISS_PROGRAM, "");
          }
          if (parser.is_dest || parser.is_src ||
              (parser.is_pipe && token.argc == 0)) {
            read_dangling_cmds(&cmd_buffer[i + 1]);
            continue;
          }
          // done parsing cmd
          cmd.cmds[cmd.cnt++] = token;
          return NORMAL_EXIT;
        }
      } else if (cmd_buffer[i] == '\'') {
        parser.in_single_quote = 1;
      } else if (cmd_buffer[i] == '\"') {
        parser.in_double_quote = 1;
      } else {  // ordinary character, push into buffer
        parser.buffer[parser.buffer_len++] = cmd_buffer[i];
      }

      // in between quotation region
    } else {
      // dangling quotation
      if (cmd_buffer[i] == '\n') {
        read_dangling_cmds(&cmd_buffer[i + 1]);
        continue;
        // single quotation ends
      } else if (parser.in_single_quote && cmd_buffer[i] == '\'') {
        parser.in_single_quote = 0;
        continue;
        // double quotation ends
      } else if (parser.in_double_quote && cmd_buffer[i] == '\"') {
        parser.in_double_quote = 0;
        continue;
      }
      // ordinary character, push into buffer
      parser.buffer[parser.buffer_len++] = cmd_buffer[i];
    }
  }
  return NORMAL_EXIT;
}

// exit with exit code
int syntax_error(int error_type, char* content) {
  switch (error_type) {
    case DUP_INPUT_REDIRECTION:
      fputs("error: duplicated input redirection\n", stderr);
      break;
    case DUP_OUTPUT_REDIRECTION:
      fputs("error: duplicated output redirection\n", stderr);
      break;
    case ERROR_SYNTAX:
      fputs("syntax error near unexpected token `", stderr);
      fputs(content, stderr);
      fputs("'\n", stderr);
      break;
    case MISS_PROGRAM:
      fputs("error: missing program\n", stderr);
      break;
    case UNEXPECTED_ERROR:
      fputs("error: unexpected error\n", stderr);
      break;
    default:
      break;
  }
  return -1;
}

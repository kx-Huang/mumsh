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
        // no buffer when encounter separator
        if (parser.buffer_len == 0) {
          if (strchr(" \t", cmd_buffer[i]) != NULL) continue;
          if (strchr("<>", cmd_buffer[i]) != NULL) {
            // error 6: Syntax Error
            if (parser.is_dest || parser.is_src) {
              if (cmd_buffer[i] == '<') return syntax_error(ERROR_SYNTAX, "<");
              if (cmd_buffer[i] == '>') return syntax_error(ERROR_SYNTAX, ">");
            }
          }
        }
        // have buffer when encounter separator
        parser.buffer[parser.buffer_len] = '\0';
        if (parser.buffer_len != 0) {
          if (parser.is_src) {  // save buffer as redirect source
            memcpy(cmd.src, parser.buffer, parser.buffer_len + 1);
            parser.is_src = 0;
            cmd.read_file = 1;
          } else if (parser.is_dest) {  // save buffer as redirect destination
            memcpy(cmd.dest, parser.buffer, parser.buffer_len + 1);
            parser.is_dest = 0;
            cmd.write_file = 1;
          } else {  // save buffer as argument
            token.argv[token.argc] = malloc(parser.buffer_len + 1);
            memcpy(token.argv[token.argc++], parser.buffer,
                   parser.buffer_len + 1);
          }
          parser.buffer_len = 0;  // clear buffer
        }

        // space/tab have no meaning
        if (cmd_buffer[i] == ' ' || cmd_buffer[i] == '\t') continue;

        // redirector
        if (cmd_buffer[i] == '<') {  // < as operator
          // error 4: Duplicated input redirection
          if (strlen(cmd.src) != 0)
            return syntax_error(DUP_INPUT_REDIRECTION, "");
          parser.is_dest = 0;
          parser.is_src = 1;
        } else if (cmd_buffer[i] == '>') {  // > as operator
          // error 5: Duplicated output redirection
          if (strlen(cmd.dest) != 0)
            return syntax_error(DUP_OUTPUT_REDIRECTION, "");
          parser.is_src = 0;
          parser.is_dest = 1;
          if (cmd_buffer[i + 1] == '>') {  // >> as operator
            cmd.append_file = 1;
            i++;  // skip the second >
          }

          // pipe
        } else if (cmd_buffer[i] == '|') {  // | as command end flag
          // error: Missing program
          // set piping condition
          parser.is_pipe = 1;
        } else if (cmd_buffer[i] == '\n') {  // \n as end flag
          // todo: incomplete input check

          // done parsing cmd
          cmd.cmds[cmd.cnt++] = token;
          return 0;
        }
      } else if (cmd_buffer[i] == '\'') {  // ' as indicator
        parser.in_single_quote = 1;
      } else if (cmd_buffer[i] == '\"') {  // " as indicator
        parser.in_double_quote = 1;
      } else {  // ordinary character, push into buffer
        parser.buffer[parser.buffer_len++] = cmd_buffer[i];
      }
      // in between quotation region
    } else {
      // dangling quotation
      if (cmd_buffer[i] == '\n') {
        // todo: prompt for imcomplete input
        return syntax_error(UNEXPECTED_ERROR, "");
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

  return 0;
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

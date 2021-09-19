#include "parser.h"

void debug(token_t* token) {
  for (size_t i = 0; token->argv[i] != NULL; i++)
    printf("argv[%lu]: \"%s\"\n", i, token->argv[i]);
  printf("src: \"%s\"\n", token->src);
  printf("dest: \"%s\"\n", token->dest);
  printf("read? %d\n", token->read_file);
  printf("write? %d\n", token->write_file);
  printf("append? %d\n", token->append_file);
}

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
            // error: Duplicated input redirection
            if (strlen(token.src) != 0) {
              // to fix: no exit
              syntax_error(DUP_INPUT_REDIRECTION, "");
              memset(token.src, 0, BUFFER_SIZE);
            }
            memcpy(token.src, parser.buffer, j + 1);
            parser.is_src = 0;
          } else if (parser.is_dest) {
            // error: Duplicated output redirection
            if (strlen(token.dest) != 0) {
              syntax_error(DUP_OUTPUT_REDIRECTION, "");
              memset(token.dest, 0, BUFFER_SIZE);
            }
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
        syntax_error(UNEXPECTED_ERROR, "");
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

// exit with exit code
void syntax_error(int error_type, char* content) {
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
}

#include "mumsh.h"

int main() {
  // handle ctrl-c interruption
  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sigaction(SIGINT, &sa, NULL);

  // main loop
  while (1) {
    // prompt and read input
    read_cmd();
    token_t token = parser();

    // for (size_t i = 0; token.argv[i] != NULL; i++)
    //   printf("argv[%lu]: \"%s\"\n", i, token.argv[i]);
    // printf("src: \"%s\"\n", token.src);
    // printf("dest: \"%s\"\n", token.dest);
    // printf("read? %d\n", token.read_file);
    // printf("write? %d\n", token.write_file);
    // printf("append? %d\n", token.append_file);

    // cmd "exit"
    if (token.argc != 0 && strncmp(token.argv[0], "exit", 4) == 0)
      exit_mumsh(NORMAL_EXIT, "");

    // create child process
    pid_t pid = fork();
    if (pid < 0)  // error creating process
      exit_mumsh(UNEXPECTED_ERROR, "");
    else if (pid == 0) {  // run child process
      sigaction(SIGINT, &sa, NULL);
      exec_cmd(&token);
    }

    // todo: handle background jobs here

    // wait for child process done
    pid_t res = waitpid(pid, NULL, 0);
    if (res != pid) exit_mumsh(UNEXPECTED_ERROR, "");

    // free after malloc token
    for (size_t i = 0; i < token.argc; i++) free(token.argv[i]);
  }
  exit(0);
}

# Mumsh: VE482 Operating System Project 1
This project is a course project in VE482 Operating System at UM-SJTU Joint Institute. In this project, a mini shell `mumsh` is implemented with C. `mumsh` supports some basic shell functions including a read/parse/execute loop, I/O redirection under bash style syntax, pipes, internal cmd `pwd`/`cd`/`jobs`, `CTRL-C`/`CTRL-D` interruption, quotes, incomplete input waiting, syntax error handling and background jobs.

In this `README`, the following content will be included:

0.  files related to this project
1.  how to build and run `mumsh`
2.  how to play with `mumsh`
3.  how `mumsh` is constructed

## 0. Files related to project `mumsh`
We have 4 kinds of files in this project:
- README
  - It's strongly adviced to read `README.md` before running `mumsh` or reading source code, since it may give us more sense of what `mumsh` is doing in each stage.
- C Source files: (in executing order)
  - `mumsh.c`: where main read/parse/execute loop of `mumsh` locates
  - `io.c`: handle reading command line input of `mumsh`
  - `parser.c`: parse user input into formatted commands for coming execution
  - `process.c`: execute commands in child process according to specifications
- C header files: (hierarchy from top to bottom)
  - `mumsh.h`
  - `io.h`
  - `parser.h`
  - `process.h`: store global varbles for jobs only regarding process
  - `data.h`: store extern global varibles for read/parse/execute loop
- makefile
  - used for quick build and clean of executable file

## 1. Build and Run `mumsh`
- build: `$ make`
- run: `$ ./mumsh`

If everything is normal, we can see in the terminal `mumsh $ `, which indicates that `mumsh` is up and running, waiting for our input.

## 2. Play with `mumsh`

### 2.1 Overall `mumsh` Grammar in Backus-­Naur Form
``` sh
cmd [argv]* [| cmd [argv]* ]* [[> filename][< filename][>> filename]]* [&]
```

Seems abstract and mayebe get a little bit confused? Let me explain a little more. For more details, please check the following sections.

The input of `mumsh` can be made up of 4 components:
- command and argument: `cmd`, `argv`
- redirector and filename: `<`, `>`, `>>`, `filename`
- pipe indicator: `|`
- background job indicator: `&`

#### 2.1.1 Command and Argument
- `cmd` is a must, or `mumsh` will raise `error: missing program`
- `argv` is optional, we can choose to call a command with arguments or not.

#### 2.1.2 Redirector and Filename
- `<`, `>`, `>>` is optional, but we should input redirector along with filename
  - if any `<, >, |, &` instead of `filename` follows, `mumsh` will raise `error: syntax error near unexpected token ...`
  - if no character follows, `mumsh` will prompt us to keep input in newline
- `>` and `>>` can't exist in the same command, or `mumsh` will raise `error: duplicated output redirection`

#### 2.1.3 Pipe Indicator
- `|` is optional, but we should input `|` after one command and followed by another command
  - if no command before `|`, `mumsh` will raise `error: missing program`
  - if no character after `|`, `mumsh` will prompt us to keep input in newline
- `|` is incompatible with having `>` or `>>` before it, and having `<` after it
  - if `>` or `>>` comes before `|`, `mumsh` will raise `error: duplicated output redirection`
  - if `<` comes after `|`, `mumsh` will raise `error: duplicated input redirection`

#### 2.1.4 Background Job Indicator
- `&` is optional, but we should only input `&` at the end of input, or `mumsh` will ignore the character(s) after `&` is detected.

Now, we have our components of input to play with, and we can try it out in `mumsh` by assembling them into a whole input. As long as `mumsh` doesn't raise an error, our input syntax is valid, even though this input may give no output.

### 2.2 Simple Commands
- `mumsh` built-in commands
  - `exit`: exit `mumsh`
  - `pwd`: print working directory
  - `cd`: change working directory
  - `jobs`: print background jobs status
- executable commands (call other programs to do certain jobs)
  - `ls`: call program `/bin/ls`, which print files in current working directory
  - `bash`: call shell `/bin/bash`, which is also a shell like `mumsh` but with more powerful capabilities
  - we can input `ls /bin` to see more executable commands

### 2.3 I/0 Redirection (support bash style syntax)
- Input redirection
  - `< filename`: read from file named `filename`, or raise error if no `filename` exists

- Output redirection
  - `> filename`: overwrite if file named `filename` exists or create new file named `filename`
  - `>> filename`: append if file named `filename` exists or create new file named `filename`

- support bash style syntax
  - An arbitrary amount of space can exist between redirection symbols and arguments, starting from zero.
  - The redirection symbol can take place anywhere in the command.
  - for example: `<1.txt>3.txt cat 2.txt 4.txt`

### 2.4 Pipe
- takes output of one command as input of another command
  - basic pipe syntax: `echo 123 | grep 1`
- `mumsh` support parallel execution: all piped commands run in parallel
  - for example: `sleep 1 | sleep 1 | sleep 1` only takes 1 second to finish instead of 3 seconds
- `mumsh` support arbitrarily deep “cascade pipes”
  - for example: `echo hello world | grep h | grep h | ... | grep h`

### 2.5 CTRL-C
- interrupt all executing commands in foreground with `CTRL-C`
- cases:
  - clear user input and prompt new line

    ```sh
    mumsh $ echo ^C
    mumsh $
    ```

  - interrupt single executing command

    ```sh
    mumsh $ sleep 10
    ^C
    mumsh $
    ```

  - interrupt multiple executing commands

    ```sh
    mumsh $ sleep 10 | sleep 10 | sleep 10
    ^C
    mumsh $
    ```

  - CTRL-C don't interrupt background jobs

    ```sh
    mumsh $ sleep 10 &
    [1] sleep 10 &
    mumsh $ ^C
    mumsh $ jobs
    [1] running sleep 10 &
    mumsh $
    ```

### 2.6 CTRL-D
- if `mumsh` has no user input, `CTRL-D` will exit `mumsh`

  ```sh
  mumsh $ exit
  $
  ```
- if `mumsh` has user input, do nothing

  ```sh
  mumsh $ echo ^D
  ```

### 2.7 Quotes
- `mumsh` takes any character between `"` or `'` as ordinary character without special meaning.

  ```sh
  mumsh $ echo hello "| grep 'h' > 1.txt"
  hello | grep 'h' > 1.txt
  mumsh $
  ```

### 2.8 Background Jobs
- If `&` is added at the end of user input, `mumsh` will run jobs in background instead of waiting for execution to be done.
- Command `jobs` can keep track on every background jobs, no matter a job is `done` or `running`
- `mumsh` support pipe in background jobs

  ```sh
  mumsh $ sleep 10 &
  [1] sleep 10 &
  mumsh $ sleep 1 | sleep 1 | sleep 1 &
  [2] sleep 1 | sleep 1 | sleep 1 &
  mumsh $ jobs
  [1] running sleep 10 &
  [2] done sleep 1 | sleep 1 | sleep 1 &
  mumsh $
  ```

## 3. Construct `mumsh` Step by Step

In this section, we will go through the construction of `mumsh` step by step, giving us a general concept of how this shell work. This section is intended for helping **beginners** (just as me a week ago) grab some basic concept for implementing a shell.

However, some contents such as detailed data structure and marginal logic will be neglected. And the demo code is used for our better understanding instead of doing copy and paste. As a result, the grammar is not strictly follow the C standard. For more detail, we can read the source code directly. It's strongly recommended to understand the concept before doing any coding.

### 3.1 Main Read/Parse/Execute Loop
As we all know, a shell is a computer program which exposes an operating system's services to a human user or other program. It repeatedly takes commands from the keyboard, gives them to the operating system to perform and deliver corresponding output.

As a result, the first step for us is to have a main loop, repeatedly doing 3 things:
  1. read user input
  2. parse input into commands
  3. execute the commands

```C
 int main(){
   while(1){
     read_user_input();
     parser(); // let's call it parser, who parses user input into commands
     execute_cmds();
   }
 }
```

Now we have the basic structure of a shell, but you may notice that it runs forever. As a result, we need a exit-checking funtion in the main loop. If users want to exit the shell, they can just input the command `exit` and that's it, our shell just exit.

Assume we've already parsed user input into command `cmd` (we will talk about how to do parsing in later sections), we have


```C
void check_cmd_exit(){
   if (strcmp(cmd, "exit") == 0) exit(0);
}

int main(){
  while(1){
    read_user_input();
    parser();
    check_cmd_exit(); // if user input "exit", exit here
    execute_cmds();
  }
}
```

### 3.1.1 A Question Remains
You may argue that this exit-checking funtion can be in the part of `execute_cmds()`. Of course we can do that, but personally I perfer put it in the main loop. And the reason for this is exactly what makes **the most important** part: how is a command executed in the shell?

### 3.2 Execute a Command: `fork()` and `execvp()` System Calls

First, what is a system call? In [`Linux manual page`](https://man7.org/linux/man-pages/man2/syscalls.2.html), it writes

> "The system call is the fundamental interface between an application and the Linux kernel."

Basically, if we want our `computer program` requests a service from `the kernal of the operating system`, we use system call.

Just to recap, when we input `ls` in shell, the shell calls the program `/bin/ls`. More precisely, the shell asks `the kernal of the operating system` to execute the program `/bin/ls`.

To do so, we have to use `execvp()`, which is a system call under the library `<unistd.h>`, and it "replaces the current process image with a new process image". When `execvp()` is executed, the program file given by the first argument will be loaded into the caller's address space and over-write the program there.

For example, once the program we call (e.g. `/bin/ls`) starts its execution, the original program in the caller's address space (`mumsh`) is gone and is replaced by the new program (`/bin/ls`). Here "gone" means our `mumsh` just somehow vanished, the only program left is `/bin/ls`. Once `/bin/ls` finishes its jobs, nothing will be left.

Apparently, this should never happen for a shell to run normally. We still need our shell up and running! As a result, we use another system call: `fork()`.

But before getting into `fork()`, let's first talk about [`process`](https://www.redhat.com/sysadmin/linux-command-basics-7-commands-process-management#:~:text=In%20Linux%2C%20a%20process%20is,you%20have%20created%20a%20process.).

> In Linux, a process is any active (running) instance of a program. But what is a program? Well, technically, a program is any executable file held in storage on your machine.

Aforementioned `execvp()` system call just replace one program with another in a process, but it doesn't create a new process to make both programs execute at the same time. And here comes the `fork()` system call.

The `fork()` system call "creates a new process by duplicating the calling process. The new process is referred to as the `child process`. The calling process is referred to as the `parent process`." In this case, if we execute `/bin/ls` in child process, our `mumsh` process won't vanish.

Now, we have everything prepared:
1. use `fork()` system call to create a child process
2. use `execvp()` system call to execute the program in child process

For more detail of how to use them, please check [`Linux manual page`](https://man7.org/linux/man-pages/man1/man.1.html).

#### 3.2.1 `fork()` System Call
> [`fork()`](https://man7.org/linux/man-pages/man2/fork.2.html) - create a child process
> ```C
>   #include <unistd.h>
>   pid_t fork(void);
> ```
> **Return value**
> - On success
>   - the `PID` of the child process is returned in the parent
>   - `0` is returned in the child
> - On failure
>   - `-1` is returned in the parent
>   - no child process is created
>   - `errno` is set to indicate the error

#### 3.2.2 `execvp()` System Call
> [`execvp`](https://man7.org/linux/man-pages/man3/exec.3.html) - execute a file
> ```C
>   #include <unistd.h>
>   int execvp(const char *file, char *const argv[]);
> ```
> **Arguments**
> - `file`: pointer point to the filename associated with the file being executed
> - `argv`: an array of pointers to null-terminated strings that represent the argument list available to the new program
> **Return value**
> - On success: no return
> - On failure
>   - return `-1` if an error has occurred
>   - `errno` is set to indicate the error

```C
void execute_cmds(){
  pid_t pid = fork();
  // child and parent process reach here
  if (pid == 0) {
    // child process starts here
    execvp(cmd, argv);
    // child process won't reach here!
  } else {
  // parent process jumps here
  }
}
```

You may ask why the child process can't procceed after `execvp()`, and that's because `execvp()` system call have a unique feature: it never return if no error occurs. Basically,
- replace the program of caller (duplicated `mumsh`) on child process created by `fork()` with any program we call
- the program we call is up and running in that child process
- the child process terminated as the program finishes its execution

Now that the child process is gone, of course child process will never run the code after `execvp()`. What only left is the parent process.

### 3.3 Built-in Commands
Still remember there's [a question left in 3.1.1](#311-a-question-remains)? Why do I put `check_cmd_exit()` outside of `execute_cmds` in main loop? That's because `exit` is a `built-in command`.

```C
void check_cmd_exit(){
  if (strcmp(cmd, "exit") == 0) {
    exit(0); // exit in parent process
  }
}

int main(){
  while(1){
    read_user_input();
    parser();
    check_cmd_exit(); // if user input "exit", exit here
    execute_cmds();
  }
}
```

For normal commands like `ls` and `pwd`, our shell `execute commands` by `calling other programs`, which happens in the child process after we `fork()`. However, our `exit` is a `built-in command` which should happen in parent process, because we want to exit `mumsh` once and for all instead of shutting down a duplicated child of `mumsh`.

Similarly, we should implement command `cd` (change working directory) as `built-in command` in parent process, because if we run `cd` in child process, we change working directory for child `mumsh` instead of parent `mumsh`, and obviously, that's wrong, we want `cd` takes effect permanently.

```C
void check_cmd_cd(){
  if (strcmp(cmd, "cd") == 0) {
    chdir(argv); // system call
  }
}

int main(){
  while(1){
    read_user_input();
    parser();
    check_cmd_exit();
    check_cmd_cd(); // if user input "cd", change working directory here
    execute_cmds();
  }
}
```

To be clear, `built-in commands` are not equal or related to `commands run in parent process`, they are totally separate things. For example, We can still implement our `built-in` commands like `pwd` in child process, because
- firstly `pwd` doesn't tamper with parent process: it just print something and that's it. Putting it in child process doesn't change its output.
- and secondly, if `pwd` runs in the child process, we can do more fancy stuffs such as making it parts of a `pipe` or making it a `background job`.

```C
void check_cmd_pwd(){
  if (strcmp(cmd, "pwd") == 0) {
    getcwd(buffer, BUFFER_SIZE); // system call
    printf("%s\n", buffer);
    exit(0); // in child process now, don't forget to exit!
  }
}

void execute_cmds(){
  pid_t pid = fork();
  if (pid == 0) {
    check_cmd_pwd(); // our built-in pwd runs here
    execvp(cmd, argv);
  }
}

int main(){
  while(1){
    read_user_input();
    parser();
    check_cmd_exit();
    check_cmd_cd();
    execute_cmds();
  }
}
```

Until now, the basic structure of `mumsh` has been constructed.

```C
void check_cmd_cd(){
  if (strcmp(cmd, "cd") == 0) {
    chdir(argv); // system call
  }
}

void check_cmd_exit(){
  if (strcmp(cmd, "exit") == 0) {
    exit(0); // exit parent process
  }
}

void check_cmd_pwd(){
  if (strcmp(cmd, "pwd") == 0) {
    getcwd(buffer, BUFFER_SIZE); // system call
    printf("%s\n", buffer);
    exit(0); // exit child process
  }
}

void execute_cmds(){
  pid_t pid = fork();
  if (pid == 0) { // child process
    check_cmd_pwd();
    execvp(cmd, argv); // system call
  }
}

int main(){
  while(1){
    read_user_input();
    parser();
    check_cmd_exit();
    check_cmd_cd();
    execute_cmds();
  }
}
```

### 3.4 `waitpid()` System Call

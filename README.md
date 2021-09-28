# Mumsh: VE482 Operating System Project 1
This project is a course project in VE482 Operating System at UM-SJTU Joint Institute. In this project, a mini shell `mumsh` is implemented with C. `mumsh` supports some basic shell functions including a read/parse/execute loop, I/O redirection under bash style syntax, pipes, internal cmd `pwd`/`cd`/`jobs`, `CTRL-C`/`CTRL-D` interruption, quotes, incomplete input waiting, syntax error handling and background jobs.

In this `README`, the following content will be included:
1.  files related to this project
2.  how to build and run `mumsh`
3.  how to play with `mumsh`
4.  how `mumsh` is constructed

## Files related to project `mumsh`
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

## Build and Run `mumsh`
- build: `$ make`
- run: `$ ./mumsh`

If everything is normal, we can see in the terminal `mumsh $ `, which indicates that `mumsh` is up and running, waiting for your input.

## Play with `mumsh`

### Overall `mumsh` Grammar in Backus­Naur Form
``` sh
cmd [argv]* [ | cmd [argv]* ]* [[> filename] [< filename] [>> filename]]* [&]
```

Seems abstract and mayebe get a little bit confused? Let me explain a little more. For more details, please check the following sections.

The input of `mumsh` can be made up of 3 components:
1.  command and argument: `cmd`, `argv`
2.  redirector and filename: `<`, `>`, `>>`, `filename`
3.  pipe indicator: `|`
4.  background job indicator: `&`

#### 1. Command and Argument
- `cmd` is a must, or `mumsh` will raise `error: missing program`
- `argv` is optional, we can choose to call a command with arguments or not.

#### 2. Redirector and Filename
- `<`, `>`, `>>` is optional, but we should input redirector along with filename
  - if any `<, >, |, &` instead of `filename` follows, `mumsh` will raise `error: syntax error near unexpected token ...`
  - if no character follows, `mumsh` will prompt us to keep input in newline
- `>` and `>>` can't exist in the same command, or `mumsh` will raise `error: duplicated output redirection`

#### 3. Pipe Indicator
- `|` is optional, but we should input `|` after one command and followed by another command
  - if no command before `|`, `mumsh` will raise `error: missing program`
  - if no character after `|`, `mumsh` will prompt us to keep input in newline
- `|` is incompatible with having `>` or `>>` before it, and having `<` after it
  - if `>` or `>>` comes before `|`, `mumsh` will raise `error: duplicated output redirection`
  - if `<` comes after `|`, `mumsh` will raise `error: duplicated input redirection`

#### 4. Background Job Indicator
- `&` is optional, but we should only input `&` at the end of input, or `mumsh` will ignore the character(s) after `&` is detected.

Now, we have our components of input to play with, and we can try it out in `mumsh` by assembling them into a whole input. As long as `mumsh` doesn't raise an error, your input syntax is valid, even though your input may give no output.

### Simple Commands
- `mumsh` built-in commands
  - `exit`: exit `mumsh`
  - `pwd`: print working directory
  - `cd`: change working directory
  - `jobs`: print background jobs status
- executable commands (call other programs to do certain jobs)
  - `ls`: call program `/bin/ls`, which print files in current working directory
  - `bash`: call shell `bash`, which is also a shell like `mumsh` but with more powerful capabilities
  - we can input `ls /bin` to see more executable commands

### I/0 Redirection (support bash style syntax)
- Input redirection
  - `< filename`: read from file named `filename`, or raise error if no `filename` exists

- Output redirection
  - `> filename`: overwrite if file named `filename` exists or create new file named `filename`
  - `>> filename`: append if file named `filename` exists or create new file named `filename`

- support bash style syntax
  - An arbitrary amount of space can exist between redirection symbols and arguments, starting from zero.
  - The redirection symbol can take place anywhere in the command.
  - for example: `<1.txt>3.txt cat 2.txt 4.txt`

### Pipe
- takes output of one command as input of another command
  - basic pipe syntax: `echo 123 | grep 1`
- `mumsh` support parallel execution: all piped commands run in parallel
  - for example: `sleep 1 | sleep 1 | sleep 1` only takes 1 second to finish instead of 3 seconds
- `mumsh` support arbitrarily deep “cascade pipes”
  - for example: `echo hello world | grep h | grep h | ... | grep h`

### CTRL-C
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

### CTRL-D
- if `mumsh` has no user input, `CTRL-D` will exit `mumsh`
  ```sh
  mumsh $ exit
  $
  ```
- if `mumsh` has user input, do nothing
  ```sh
  mumsh $ echo ^D
  ```

### Quotes
- `mumsh` takes any character between `"` or `'` as ordinary character without special meaning.
  ```sh
  mumsh $ echo hello "| grep 'h' > 1.txt"
  hello | grep 'h' > 1.txt
  mumsh $
  ```

### Background Jobs
- If `&` is added at the end of user input, `mumsh` will run jobs in background instead of waiting for execution to be done.
- Command `jobs` can keep track on every background jobs, no matter a job is `done` or `running`
- `mumsh` support pipe in background jobs
```sh
mumsh $ sleep 10 &
[1] sleep 10 &
mumsh $ sleep 1 | sleep 1 | sleep 1 &
[2] sleep 1 | sleep 1 | sleep 1 &
mumsh $ jobs
[1] running sleep 100 &
[2] done sleep 1 | sleep 1 | sleep 1 &
mumsh $
```

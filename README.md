# Mumsh: VE482 Operating System Project 1
This project is a course project in `VE482 Operating System` at [`UM-SJTU Joint Institute`](https://www.ji.sjtu.edu.cn/).

In this project, a mini shell `mumsh` is implemented with `programming language C`. `mumsh` supports some basic shell functionalities including
- Read/parse/execute loop
- Incomplete input waiting
- Syntax error handling
- Quotes parsing
- Internal commands `exit`/`pwd`/`cd`/`jobs`,
- I/O redirection under bash style syntax
- Arbitrarily-deep pipes in parallel
- `CTRL-C`/`CTRL-D` interrupt
- Background jobs running.

In this `README`, the following content will be included:

1.  Files related to this project
2.  How to build and run `mumsh`
3.  How to play with `mumsh`
4.  How `mumsh` is constructed

----
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

---

## 1. Build and Run `mumsh`
- build: `$ make`
- run: `$ ./mumsh`

If everything is normal, we can see in the terminal `mumsh $ `, which indicates that `mumsh` is up and running, waiting for our input.

---

## 2. Play with `mumsh`

Once `mumsh` is up and running, we can start inputting some commands such as `ls` or `pwd` to test the basic functionalities if you have already been familiar to `shell`.

Of course, `mumsh` is only a product of a course project supporting basic functions, and yet to be improved. For its detailed ability, please check the following sections.

---

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

---

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

---

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

---

### 2.4 Pipe
- takes output of one command as input of another command
  - basic pipe syntax: `echo 123 | grep 1`
- `mumsh` support parallel execution: all piped commands run in parallel
  - for example: `sleep 1 | sleep 1 | sleep 1` only takes 1 second to finish instead of 3 seconds
- `mumsh` support arbitrarily deep “cascade pipes”
  - for example: `echo hello world | grep h | grep h | ... | grep h`

---

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

---

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

---

### 2.7 Quotes
- `mumsh` takes any character between `"` or `'` as ordinary character without special meaning.

  ```sh
  mumsh $ echo hello "| grep 'h' > 1.txt"
  hello | grep 'h' > 1.txt
  mumsh $
  ```

---

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

---

## 3. Construct `mumsh` Step by Step

In this section, we will go through the construction of `mumsh` step by step, giving us a general concept of how this shell work. This section is intended for helping **beginners** (just as me a week ago) grab some basic concept for implementing a shell.

However, some contents such as detailed data structure and marginal logic will be neglected. And the demo code is used for our better understanding instead of doing copy and paste. As a result, the grammar is not strictly follow the C standard. For more detail, we can read the source code directly. It's strongly recommended to understand the concept before doing any coding.

---

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

Assume we've already parsed user input into command `cmd` (we will talk about how to do implement simple parser with `Finite State Machine`in [section 3.5](#35-simple-parsing-with-finite-state-machine)), we have


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

---

### 3.2 Execute a Command: `fork()` and `execvp()` System Calls

#### 3.2.1 What is a `System Call`?

First, what is a system call? In [`Linux manual page`](https://man7.org/linux/man-pages/man2/syscalls.2.html), it writes

> "The system call is the fundamental interface between an application and the Linux kernel."

Basically, if we want our `computer program` requests a service from `the kernal of the operating system`, we use system call.

---

#### 3.2.2 Execute a Command in a Right Way

Just to recap, when we input `ls` in shell, the shell calls the program `/bin/ls`. More precisely, the shell asks `the kernal of the operating system` to execute the program `/bin/ls`.

To do so, we have to use `execvp()`, which is a system call under the library `<unistd.h>`, and it "replaces the current process image with a new process image". When `execvp()` is executed, the program file given by the first argument will be loaded into the caller's address space and over-write the program there.

For example, once the program we call (e.g. `/bin/ls`) starts its execution, the original program in the caller's address space (`mumsh`) is gone and is replaced by the new program (`/bin/ls`). Here "gone" means our `mumsh` just somehow vanished, the only program left is `/bin/ls`. Once `/bin/ls` finishes its jobs, nothing will be left.

Apparently, this should never happen for a shell to run normally. We still need our shell up and running! As a result, we use another system call: `fork()`.

---

#### 3.2.3 What is a `Process` and Why `fork()` before `wxwcvp()`

But before getting into `fork()`, let's first talk about [`process`](https://www.redhat.com/sysadmin/linux-command-basics-7-commands-process-management#:~:text=In%20Linux%2C%20a%20process%20is,you%20have%20created%20a%20process.).

> In Linux, a process is any active (running) instance of a program. But what is a program? Well, technically, a program is any executable file held in storage on your machine.

Aforementioned `execvp()` system call just replace one program with another in a process, but it doesn't create a new process to make both programs execute at the same time. And here comes the `fork()` system call.

The `fork()` system call "creates a new process by duplicating the calling process. The new process is referred to as the `child process`. The calling process is referred to as the `parent process`." In this case, if we execute `/bin/ls` in child process, our `mumsh` process won't vanish.

Now, we have everything prepared:
1. use `fork()` system call to create a child process
2. use `execvp()` system call to execute the program in child process

For more detailed documentations, we can check [`Linux manual page`](https://man7.org/linux/man-pages/man1/man.1.html).

---
##### 3.2.3.1 Reference: `fork()` System Call
> [`fork()`](https://man7.org/linux/man-pages/man2/fork.2.html) - create a child process
>
> **Description**
>
> - creates a new process by duplicating the calling process
>
> ```C
>   #include <unistd.h>
>   pid_t fork(void);
> ```
>
> **Return value**
> - On success
>   - the `PID` of the child process is returned in the parent
>   - `0` is returned in the child
> - On failure
>   - `-1` is returned in the parent
>   - no child process is created
>   - `errno` is set to indicate the error

---
##### 3.2.3.2 Reference: `execvp()` System Call
> [`execvp`](https://man7.org/linux/man-pages/man3/exec.3.html) - execute a filename
>
> **Description**
>
> - replaces the current process image with a new process image
>
> ```C
>   #include <unistd.h>
>   int execvp(const char *file, char *const argv[]);
> ```
>
> **Arguments**
> - `file`: pointer point to the filename associated with the file being executed
> - `argv`: an array of pointers to null-terminated strings that represent the argument list available to the new program
> **Return value**
> - On success: no return
> - On failure
>   - return `-1` if an error has occurred
>   - `errno` is set to indicate the error

---

#### 3.2.4 Basic Construction of `fork()` and `execvp()`

Following the documents, we now can construct the basic `fork and execute structure`.

```C
void execute_cmds(){
  pid_t pid = fork(); // fork() system call
  // child and parent process reach here
  if (pid == 0) {
    // child process starts here
    execvp(cmd, argv); // execvp() system call
    // child process won't reach here!
  } else {
  // parent process jumps here
  }
}
```

You may ask why the child process can't procceed after `execvp()`, and that's because `execvp()` system call have a unique feature: it never return if no error occurs. Basically, it
- replace the program of caller (duplicated instance `mumsh`) on child process created by `fork()` with any program we call
- the program we call is up and running in that child process
- the child process terminated as the program finishes its execution

Now that the child process is gone, of course child process will never run the code after `execvp()`. What only left is the parent process `mumsh`.

---

### 3.3 Built-in Commands
Still remember there's [a question left in 3.1.1](#311-a-question-remains)? Why do I put `check_cmd_exit()` outside of `execute_cmds()` in main loop? That's because `exit` is a `built-in command`.

```C
void check_cmd_exit(){
  if (strcmp(cmd, "exit") == 0) {
    exit(0); // exit the parent process
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

For normal commands like `ls` and `pwd`, our shell `execute commands` by `calling other programs`, which happens in the child process after we `fork()`. However, our `exit` is a `built-in command` which we implement ourselves!

Obviously, `exit` should happen in parent process, because we want to exit `mumsh` once and for all instead of shutting down a duplicated instance of `mumsh` running in child process.

Similarly, we can implement command `cd` (change working directory) as `built-in command` in parent process. If we accidentally run `cd` in child process, we change working directory for child `mumsh` instead of parent `mumsh`. In that case, nothing changes after child process is exited.

```C
void check_cmd_cd(){
  if (strcmp(cmd, "cd") == 0) {
    chdir(argv); // chdir() system call
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

To be clear, `built-in commands` are not equal or related to `commands run in parent process`, they are totally separate things. Here `built-in commands` are just oppsite to `commands that call other programs`.

For example, We can still make our `built-in` commands like `pwd` running in child process, and it has 2 advantages:
- firstly `pwd` doesn't tamper with parent process: it just print something and that's it. Putting it in child process doesn't change its output.
- and secondly, if `pwd` runs in the child process, we can do more fancy operations such as making it parts of a `pipe` or making it a `background job`.

```C
void check_cmd_pwd(){
  if (strcmp(cmd, "pwd") == 0) {
    getcwd(buffer, BUFFER_SIZE); // getcwd() system call
    printf("%s\n", buffer);
    exit(0); // in child process now, don't forget to exit!
  }
}

void execute_cmds(){
  pid_t pid = fork();
  if (pid == 0) {
    check_cmd_pwd(); // if user input "pwd", print working directory here
    execvp(cmd, argv);
  }
}
```

Until now, the basic structure of `mumsh` has been constructed.

```C
void check_cmd_cd(){
  if (strcmp(cmd, "cd") == 0) {
    chdir(argv); // chdir() system call
  }
}

void check_cmd_exit(){
  if (strcmp(cmd, "exit") == 0) {
    exit(0); // exit parent process
  }
}

void check_cmd_pwd(){
  if (strcmp(cmd, "pwd") == 0) {
    getcwd(buffer, BUFFER_SIZE); // getcwd() system call
    printf("%s\n", buffer);
    exit(0); // exit child process
  }
}

void execute_cmds(){
  pid_t pid = fork(); // fork() system call
  if (pid == 0) { // child process
    check_cmd_pwd();
    execvp(cmd, argv); // execvp() system call
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

---

### 3.4 Monitor Our Children: `waitpid()` System Call

However, there's one more thing to do after we create a child process and run a program on it, which is, monitoring our child process.

---

#### 3.4.1 A Simple Metaphor

To be a qualified parent in real world, we should take care of our children. Let's say we have a child playing badminton on the field, we as parent, have simply 3 choices:
1. do nothing and wait for the child until he is done
2. do our own stuff and pick the child up after he is done
3. do our own stuff and leave the child alone forever

In this metaphor, the child is `a child process`, the parent is `mumsh parent process`, and playing badminton is `execution of a program`. The 3 choices are correspondingly:
1. parent process is `blocked` to `wait` the child process to `exit`
2. parent process is `unblocked` and `wait` the child process to `exit` sometime in future
3. parent process is `unblocked` and don't give a sh*t about the status of child process

Apparently, our `mumsh` is a caring parent, so we should choose option 1 or 2.

In both options, we say the child process is `reaped` by the parent process, and the only difference is we `wait blockingly or not` when the child process is executing.

To make us a good parent, `waitpid()` system call is needed.

---
#### 3.4.2 Reference: `waitpid()` System Call

> [`waitpid()`](https://man7.org/linux/man-pages/man2/wait.2.html) - wait for process to change state
>
> **Description**
>
> - Suspends execution of the calling thread until a child specified by pid argument has changed state
>
> ```C
>   #include <sys/wait.h>
>   pid_t waitpid(pid_t pid, int *wstatus, int options);
> ```
>
> **Arguments**
> - `pid`
>   - `> 0`: wait for the child whose `process ID` is equal to the value of `pid`
>   - `0`: wait for any child process whose `process group ID` is equal to that of the calling process at the time of the call to `waitpid()`
>   - `-1`: wait for any child process
>   - `< -1`: wait for any child process whose `process group ID` is equal to the `absolute value of pid`
> - `wstatus`: `int pointer` pointing to the `int` that stores status information
> - `options`
>   - `0`: by default, waits only for terminated children
>   - `WNOHANG`: return immediately if no child has exited
>   - `WUNTRACED`: also return if a child has stopped
>   - `...`
>
> **Return value**
> - On success
>   - the `PID` of the child process is returned in the parent
>   - `0` is returned in the child
> - On failure
>   - `-1` is returned in the parent
>   - no child process is created
>   - `errno` is set to indicate the error

---

#### 3.4.3 Reaping Child Process Blockingly

Following the document, with `waitpid` system call, we now can construct the complete `fork-execute-wait structure`.

```C
void execute_cmds(){
  pid_t pid = fork(); // fork() system call
  if (pid == 0) { // child process
    check_cmd_pwd();
    execvp(cmd, argv); // execvp() system call
  }
  // parent wait here until child exit or interrupted
  waitpid(pid, NULL, WUNTRACED);
}
```

Tips: command `sleep` is useful to observe blocking. Try `sleep 5` for sleeping 5 seconds!

---

### 3.5 Simple Parser via Finite State machine

In [section 3.2](#32-execute-a-command-fork-and-execvp-system-calls), we assumed that we already parsed user input into command `cmd` and execute. Now, it's time for us to implement a parser for real!

(If you are currently a VE482 student, I suggest you to start early on milestone 1 and put more attentions on parser. Without a properly-organized parser, you would suffer a lot from refactoring your code in the future milestones)

---

#### 3.5.0 A Simple Test for Brain-parsing Commands

The command formats are mentioned in [section 2.1](#21-overall-mumsh-grammar-in-backus-naur-form), please check first if you are not familar with the concept.

**Task**:
1. Given a `simple command` below, please describe in what ways our brain is parsing the commands

    ```sh
    echo hello < 1.in world > 1.out &
    ```

2. Given a `cascade command` below, please describe in what ways your brain is parsing

    ```sh
    echo hello world | cat | cat | cat | grep hello
    ```

3. Given a `complicated command` below, please describe in what ways our brain is parsing

    ```sh
    "echo" hello'  <  "'world'"  |  'cat  >  '3.out'
    ```

Please think for yourself first and then check whether it matches the following strategies:

1. Searching for keywords `>`,`<`, then check the first cluster after it
2. Searching for whitespace to separate arguments in left and right
3. Searching for `|` to separate commands in left and right
4. Searching for `"`,`'` and another quotes close to each other

Unfortunately, if you choose any of the strategy listed above, you might be on a wrong tract.

Our brain indeed works fast on identifying keywords like `<`,`>`,`|` or separating arguments by whitespace in the middle, however, they might have no meaning in a command.

For example,
- `echo hello ">" 1.out < 1.in` just print `hello > 1.out` on the screen instead of doing redirection
- `echo "hello' 'world"` only print one argument `hello' 'world` instead of `hello` and `world`

As a result, it doesn't work if we are trying to `match something in the middle` of a command, at least for supporting quotes. And here comes our savior: `Finite State Machine`.

---

#### 3.5.1 What is a Finite State Machine?

(If you are a JI student, you should be familiar with FSM at least in VE270!)

According to [Google searching results](https://medium.com/@mlbors/what-is-a-finite-state-machine-6d8dec727e2c), A `Finite State Machine`, or `FSM`, is

> a computation model that can be used to simulate sequential logic, or, in other words, to represent and control execution flow. Finite State Machines can be used to model problems in many fields, including mathematics, artificial intelligence, games or linguistics.

---

#### 3.5.2 How is a Finite State Machine in `mumsh` Parser?

From above descriptions, the most important term is `simulate sequential logic`, which means, we start from the leftmost character and read one by one to the rightmost character.

For above command `echo hello ">" 1.out`, please follow the steps:

1. we read in `e` and we write on paper: we have `e`
2. we read in `c` and we write on paper: we have `ec`
3. we read in `h` and we write on paper: we have `ech`
4. we read in `o` and we write on paper: we have `echo`

Here comes the critical part,

5. we read in `white space`, and we say: "Nice! `echo` is the first argument".

But wait, some critical questions comes:

- How do you know the `echo` is the whole string?
- Can we have `white space` in a command? Perhaps the first command is something like `echo twice`?

The answer is obvious, we don't meet a quote `'` or `"`.

If I have implemented a built-in command, which is called `echo twice`, then user should input `"echo twice"` in command line instead of `echo twice`.

For now, we can conclude some useful knowledge
- every character including `whitespace` is a meaningful character between quotes
- `whitespace` indicate the end of a argument outside of quotes

(The command to parse: `echo hello ">" 1.out < 1.in`)

Back to step 5, our question is solved, `echo` indeed is the first argument. As we keep moving, similarly, we can find `hello` is the second argument.

After that, we read a double quote `"`, with above knowledge, you say:" Aha! I meet a double quote, let's just keep reading every characters until I meet another double quote`"`".

As a result, we have our third argument: a single char `>`.

As we keep reading, we find a `<` and you are tring to recall whether we are in between a quotation mark or not. And then you look back:" Aha! Last two quotes are matched! We are outside of quotation marks."

As a result, this `<` is a sign of `input redirection`.

According to the grammars, we need a `filename` to perform redirection. So you write on the paper: "I need a `filename`."

Now, whatever we read in next is a `filename` instead of a `argument`.

Then, we find `1.in`, and we know it is the `filename` for `input redirection`.

Finally, we read in `\n`, which is the `newline` inputted by pressing `ENTER` in keyboard, indicating our parsing process comes to an end... or... maybe not?! Consider following cases, what if:

- The quotation marks are not closed? e.g. `echo "hello`
- No `filename` after `<` or `>` outside the quotes? e.g. `echo hello >`
- No `argument` after `|` outside the quotes? e.g. `echo hello |`

Or what if... we have a rookie user who is messing up with our input with wrong grammar:
- Syntax error: e.g. `echo > <`
- Duplicated output direction: e.g. `echo > out | cat`
- Missing program: e.g. `| cat`
- ......

Actually, the answer to the questions are all hiden in FSM. For FSM, we have different `states` to jump from one another according to the `condition`. Here `condition` is defined by `the character we meet now`, and `states` are defined as `different roles toward a character`.

Once we find some unexpected behaviors which are not defined in our FSM, then we raise an error.

For reference, the following FSM defines states and state transitions of a simple parser.

![An FSM Parser](https://z3.ax1x.com/2021/10/01/47n26g.png)

You may notice that the pipe mark `|` and backgound jobs indicator `&` are absent in the above FSM. But once we figure this methodology out, it's nothing big deal but adding a few state transitions.

---

#### 3.5.3 Implement FSM with Code
Commonly, we can use `while loop` together with `switch case` to `switch states of FSM`.

However, for `mumsh`, we use `for loop` with `if else branch` to `switch conditions`. Both of the implementation can serve the needs of a FSM parser.

(Considering `code reuse`, the source code of `mumsh` is a little bit different from the following main structure.)

```C
extern char user_input[BUFFER_SIZE];

void parser(){
  for (size_t i = 0; i < BUFFER_SIZE; i++) {
    if (FSM.in_quotation){
      if (user_input[i] == '\''){
        // do something
      } else if (user_input[i] == '\"'){
        // do something
      } else {
        // do something
      }
    } else {
      if (user_input[i] == ' '){
        // do something
      } else if (user_input[i] == '<'){
        // do something
      } else if (user_input[i] == '>'){
        // do something
      } else if (user_input[i] == '|'){
        // do something
      } else if (user_input[i] == '\''){
        // do something
      } else if (user_input[i] == '\"'){
        // do something
      } else if (user_input[i] == '&'){
        // do something
      } else {
        // do something
      }
    }
  }
}
```

In `mumsh`, we keep the state-related information in a `parser struct`, which is used `locally` in parser:
```C
typedef struct parser {
  size_t buffer_len; // char buffer length
  int is_src; // input redirection state
  int is_dest; // output redirection state
  int is_pipe; // in pipe condition indicator
  int in_single_quote; // in single quote condition indicator
  int in_double_quote; // in double quote condition indicator
  char buffer[BUFFER_SIZE]; // char buffer
} parser_t;
```

In `mumsh`, we save the command-related data in a `cmd struct`, which is used `external-globally` in various functions:
```C
typedef struct token {
  size_t argc; // argument count
  char* argv[TOKEN_SIZE]; // argument list
} token_t;

typedef struct cmd {
  size_t cnt; // command count
  int background; // background job indicator
  int read_file; // input redirection indicator
  int write_file; // output redirection indicator
  int append_file; // append mode indicator
  char src[BUFFER_SIZE]; // input redirection filename
  char dest[BUFFER_SIZE]; // output redirection filename
  token_t cmds[COMMAND_SIZE]; // command list
} cmd_t;

extern cmd_t cmd;
```

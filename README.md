# Mumsh: VE482 Operating System Project 1
This project is a course project in VE482 Operating System at UM-SJTU Joint Institute. In this project, a mini shell `mumsh` is implemented with C. `mumsh` supports some basic shell functions including a read/parse/execute loop, I/O redirection under bash style syntax, pipes, internal cmd `pwd`/`cd`/`jobs`, `CTRL-C` interruption, quotes, incomplete input waiting, syntax error handling and background jobs.

In this `README`, the following content will be included:
1.  files related to this project
2.  how to build and run `mumsh`
3.  how to play with `mumsh`
4.  how `mumsh` is constructed

## Files related to project `mumsh`
We have 4 kinds of files in this project:
- README
  - It's strongly adviced to read `README.md` before running `mumsh` or reading source code, since it may give you more sense of what `mumsh` is doing in each stage.
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

## Build and Run `mumsh`
- build: `$ make`
- run: `$ ./mumsh`

If everything is normal, you can see in the terminal `mumsh $ `, which indicates that `mumsh` is up and running, waiting for your input.

## 
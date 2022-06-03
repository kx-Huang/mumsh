# `mumsh`: VE482 Course Project Version

This release version of `mumsh` meets 100% VE482 course project requirements, including basic and bonus.

## Features

As a result, only features in the criteria of grading (including basic and bonus) are implemented, including:

- Incomplete input waiting
- Syntax error handling
- Quotation mark parsing
- Internal commands `exit`/`pwd`/`cd`/`jobs`
- I/O redirection under `bash` style syntax
- Arbitrarily-deep pipes running in parallel
- `CTRL-C` and `CTRL-D` interruption
- Background jobs

For detailed specifications, please refer to project manual.

However, there is **NO**:

- ~~Tab-triggered auto-completion~~
- ~~Smart history search with up&down key~~
- ~~Left & right cursor switch~~
- ~~Intermediate insert & delete~~
- ~~Colored path prompt in prefix~~

## Build and Run

- Build: `$ make`
- Run: `$ ./mumsh`

Once `mumsh` is launched successfully, it will prompt and waiting for user inputs.

```sh
mumsh $ 
```

Try some basic commands and press `ENTER` key, `mumsh` will prompt

```sh
mumsh $ echo "Hello, world!"
Hello, world!
```

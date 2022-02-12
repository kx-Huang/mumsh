CC = clang
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -Wconversion -pedantic -Wno-unused-result
MUMSH_SRC = *.c
MUMSH_H = *.h
MUMSH = mumsh
MUMSHMC = mumsh_memory_check
MUMSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
TAR_NAME = mumsh.tar

.PHONY: clean

all: $(MUMSH) $(MUMSHMC)
	@echo mumsh successfully constructed

tar:
	tar -cvzf $(TAR_NAME) $(MUMSH_SRC) $(MUMSH_H)

check: $(MUMSH) $(MUMSHMC)
	cpplint --linelength=120 --filter=-legal,-readability/casting,-whitespace,-runtime/printf,-runtime/threadsafe_fn,-readability/todo,-build/include_subdir,-build/header_guard *.c *.h

$(MUMSH): $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSHMC) : $(MUMSH_SRC)
	$(CC) $(CFLAGS) $(MUMSHMC_FLAGS) -o $(MUMSHMC) $(MUMSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.a *~ $(MUMSH) $(MUMSHMC) $(DRIVER)
	$(RM) *.tar

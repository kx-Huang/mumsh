CC = clang
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -pedantic -Wno-unused-result
MUMSH_SRC = *.c
MUMSH = mumsh
MUMSHMC = mumsh_memory_check
MUMSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
JOJ_URL = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6141871a623b5b000610bca9/6141859a623b5b000610bc9d
TAR_NAME = p1_m1.tar
.PHONY: clean

all: $(MUMSH) $(MUMSHMC)
	@echo mumsh successfully constructed

tar:
	find . -name "*.c" -or -name "*.h" -or -name "makefile" | tar -cvzf $(TAR_NAME) -T -

joj: $(MUMSH) $(MUMSHMC) tar
	joj-submit $(JOJ_URL) $(TAR_NAME) make

$(MUMSH): $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSHMC) : $(MUMSH_SRC)
	$(CC) $(CFLAGS) $(MUMSHMC_FLAGS) -o $(MUMSHMC) $(MUMSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.a *~ $(MUMSH) $(MUMSHMC)
	$(RM) *.tar

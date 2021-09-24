CC = clang
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -Wconversion -pedantic -Wno-unused-result
MUMSH_SRC = *.c
MUMSH = mumsh
MUMSHMC = mumsh_memory_check
MUMSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
JOJ_M1 = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6141871a623b5b000610bca9/6141859a623b5b000610bc9d
JOJ_M2_1 = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df247677b50009adb634/6141859a623b5b000610bc9d
JOJ_M1_MC = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df247677b50009adb634/61418609623b5b000610bca3
JOJ_M2 = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df247677b50009adb634/6144c9007677b50009adb5d7
JOJ_M2_MC = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df247677b50009adb634/6144c9137677b50009adb5d8
TAR_NAME = p1_m1.tar
.PHONY: clean

all: $(MUMSH) $(MUMSHMC)
	@echo mumsh successfully constructed

tar:
	find . -name "*.c" -or -name "*.h" -or -name "makefile" | tar -cvzf $(TAR_NAME) -T -

joj: $(MUMSH) $(MUMSHMC) tar
	joj-submit $(JOJ_M2) $(TAR_NAME) make

mc: $(MUMSH) $(MUMSHMC) tar
	joj-submit $(JOJ_M2_MC) $(TAR_NAME) make

check: $(MUMSH) $(MUMSHMC)
	cpplint --linelength=120 --filter=-legal,-readability/casting,-whitespace,-runtime/printf,-runtime/threadsafe_fn,-readability/todo,-build/include_subdir,-build/header_guard *.c *.h

$(MUMSH): $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSHMC) : $(MUMSH_SRC)
	$(CC) $(CFLAGS) $(MUMSHMC_FLAGS) -o $(MUMSHMC) $(MUMSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.a *~ $(MUMSH) $(MUMSHMC)
	$(RM) *.tar

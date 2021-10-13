CC = clang
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -Wconversion -pedantic -Wno-unused-result
MUMSH_SRC = io.c mumsh.c parser.c process.c
MUMSH_H = data.h io.h mumsh.h parser.h process.h
DRIVER_SRC = driver.c
DRIVER = driver
MUMSH = mumsh
MUMSHMC = mumsh_memory_check
MUMSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
JOJ_M1 = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df8f7677b50009adb635/6141859a623b5b000610bc9d
JOJ_M1_MC = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df8f7677b50009adb635/61418609623b5b000610bca3
JOJ_M2 = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df8f7677b50009adb635/6144c9007677b50009adb5d7
JOJ_M2_MC = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df247677b50009adb634/6144c9137677b50009adb5d8
JOJ_M3 = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df8f7677b50009adb635/6144d54e7677b50009adb617
JOJ_M3_MC = https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/6144df8f7677b50009adb635/6144d5647677b50009adb618

TAR_NAME = p1.tar
.PHONY: clean

all: $(MUMSH) $(MUMSHMC)
	@echo mumsh successfully constructed

tarall:
	find . -name "*.c" -or -name "*.h" -or -name "makefile" | tar -cvzf $(TAR_NAME) -T -

tar:
	tar -cvzf $(TAR_NAME) $(MUMSH_SRC) $(MUMSH_H)

joj: $(MUMSH) $(MUMSHMC) tar
	joj-submit $(JOJ_M3) $(TAR_NAME) make

mc: $(MUMSH) $(MUMSHMC) tar
	joj-submit $(JOJ_M3_MC) $(TAR_NAME) make

check: $(MUMSH) $(MUMSHMC)
	cpplint --linelength=120 --filter=-legal,-readability/casting,-whitespace,-runtime/printf,-runtime/threadsafe_fn,-readability/todo,-build/include_subdir,-build/header_guard *.c *.h

$(DRIVER): $(DRIVER_SRC)
	$(CC) $(CFLAGS) -o $(DRIVER) $(DRIVER_SRC)

$(MUMSH): $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSHMC) : $(MUMSH_SRC)
	$(CC) $(CFLAGS) $(MUMSHMC_FLAGS) -o $(MUMSHMC) $(MUMSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.a *~ $(MUMSH) $(MUMSHMC) $(DRIVER)
	$(RM) *.tar

# Makefile for a-shell-kernel
CC = clang
CFLAGS = -Wall -Wextra -g -O2 -I.
LDFLAGS = -lpthread

# Test targets
.PHONY: all test clean

all: test_syscalls test_process

test_syscalls: test_syscalls.c a_shell_error.h a_shell_system.h libc_replacement.c
	$(CC) $(CFLAGS) -DTEST_MODE -c libc_replacement.c -o libc_replacement.o
	$(CC) $(CFLAGS) test_syscalls.c libc_replacement.o $(LDFLAGS) -o test_syscalls

test_process: test_process.c a_shell_process.c a_shell_error.h
	$(CC) $(CFLAGS) test_process.c a_shell_process.c $(LDFLAGS) -o test_process

test: test_syscalls test_process
	./test_syscalls
	./test_process

clean:
	rm -f test_syscalls test_process *.o

//
//  a_shell_system.h
//  a-shell-kernel
//
//  System call replacement macros for iOS
//  Include this header to redirect standard syscalls to a-shell-kernel implementations
//

#ifndef A_SHELL_SYSTEM_H
#define A_SHELL_SYSTEM_H

#include "a_shell_error.h"

// Process management
#define fork ios_fork
#define vfork ios_vfork
#define waitpid ios_waitpid
#define wait ios_wait

// Environment
#define getenv ios_getenv
#define setenv ios_setenv
#define unsetenv ios_unsetenv
#define putenv ios_putenv

// Exit
#define exit ios_exit
#define _exit ios_exit

// Execution
#define execv ios_execv
#define execvp ios_execv
#define execve ios_execve

// Signals
#define signal ios_signal

// File descriptor operations
#define dup2 ios_dup2

// Process info
#define getpid ios_currentPid

#endif /* A_SHELL_SYSTEM_H */

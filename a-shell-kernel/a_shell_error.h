//
//  error.h
//  shell_cmds_ios
//
//  Created by Nicolas Holzschuch on 16/06/2017.
//  Copyright © 2017 Nicolas Holzschuch. All rights reserved.
//

#ifndef ios_error_h
#define ios_error_h

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/signal.h>

/* #define errx compileError
#define err compileError
#define warn compileError
#define warnx compileError
#ifndef printf
#define printf(...) fprintf (thread_stdout, ##__VA_ARGS__)
#endif */

#define putchar(a) fputc(a, thread_stdout)
#define getchar() fgetc(thread_stdin)
// these functions are defined differently in C++. The #define approach breaks things.
#ifndef __cplusplus
  #define getwchar() fgetwc(thread_stdin)
  #define putwchar(a) fputwc(a, thread_stdout)
  // iswprint depends on the given locale, and setlocale() fails on iOS:
  #define iswprint(a) 1
  #define write a_shell_write
  #define fwrite a_shell_fwrite
  #define puts a_shell_puts
  #define fputs a_shell_fputs
  #define fputc a_shell_fputc
  #define putw a_shell_putw
  #define putp a_shell_putp
  #define fflush a_shell_fflush
  #define abort() a_shell_exit(1)
#endif

// Thread-local input and output streams
extern __thread FILE* thread_stdin;
extern __thread FILE* thread_stdout;
extern __thread FILE* thread_stderr;

#define exit a_shell_exit
#define _exit a_shell_exit
#define kill a_shell_killpid
#define _kill a_shell_killpid
#define killpg a_shell_killpid
#define popen a_shell_popen
#define pclose fclose
#define system a_shell_system
#define execv a_shell_execv
#define execvp a_shell_execv
#define execve a_shell_execve
#define dup2 a_shell_dup2
#define getenv a_shell_getenv
#define setenv a_shell_setenv
#define unsetenv a_shell_unsetenv
#define putenv a_shell_putenv
#define fchdir a_shell_fchdir
#define signal a_shell_signal

extern int a_shell_executable(const char* cmd); // is this command part of the "shell" commands?
extern int a_shell_system(const char* inputCmd); // execute this command (executable file or builtin command)
extern FILE *a_shell_popen(const char *command, const char *type); // Execute this command and pipe the result
extern int a_shell_kill(void); // kill the current running command
extern int a_shell_killpid(pid_t pid, int sig); // kill the current running command

extern void a_shell_exit(int errorCode) __dead2; // set error code and exits from the thread.
extern int a_shell_execv(const char *path, char* const argv[]);
extern int a_shell_execve(const char *path, char* const argv[], char** envlist);
extern int a_shell_dup2(int fd1, int fd2);
extern char * a_shell_getenv(const char *name);
extern int a_shell_setenv(const char* variableName, const char* value, int overwrite);
int a_shell_unsetenv(const char* variableName);
extern int a_shell_putenv(char *string);
extern char** environmentVariables(pid_t pid);

extern int a_shell_isatty(int fd);
extern pthread_t a_shell_getLastThreadId(void);  // deprecated
extern pthread_t a_shell_getThreadId(pid_t pid);
extern void a_shell_storeThreadId(pthread_t thread);
extern void a_shell_releaseThread(pthread_t thread);
extern void a_shell_releaseThreadId(pid_t pid);
extern pid_t a_shell_currentPid(void);
extern int a_shell_getCommandStatus(void);
extern const char* a_shell_progname(void);
extern pid_t a_shell_fork(void);
extern void a_shell_waitpid(pid_t pid);
extern pid_t a_shell_full_waitpid(pid_t pid, int *stat_loc, int options);

// Catch signal definition:
extern int canSetSignal(void);
extern sig_t a_shell_signal(int signal, sig_t function);


extern int a_shell_fchdir(const int fd);
extern ssize_t a_shell_write(int fildes, const void *buf, size_t nbyte);
extern size_t a_shell_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
extern int a_shell_puts(const char *s);
extern int a_shell_fputs(const char* s, FILE *stream);
extern int a_shell_fputc(int c, FILE *stream);
extern int a_shell_putw(int w, FILE *stream);
extern int a_shell_fflush(FILE *stream);
extern int a_shell_getstdin(void);
extern int a_shell_gettty(void);
extern int a_shell_opentty(void);
extern void a_shell_closetty(void);
extern void a_shell_stopInteractive(void);
extern void a_shell_startInteractive(void);
extern int a_shell_storeInteractive(void);
// Communication between dash and a_shell_system:
extern const char* a_shell_expandtilde(const char *login);
extern void a_shell_activateChildStreams(FILE** old_stdin, FILE** old_stdout,  FILE ** old_stderr);
extern const char* a_shell_getBookmarkedVersion(const char* p);

#ifdef __cplusplus
}
#endif
#endif /* ios_error_h */

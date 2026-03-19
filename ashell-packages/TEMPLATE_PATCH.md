# a-Shell Package Patch Template

This template shows how to write patches for packages that use the a-Shell kernel APIs.

## API Reference

### Core Syscalls (Process Management)

```c
// Include the a-Shell kernel header
extern "C" {
    #include "a_shell_system.h"  // or <a_shell_system.h> if installed
}

// Process management
pid_t a_shell_fork(void);
void a_shell_waitpid(pid_t pid);
pid_t a_shell_full_waitpid(pid_t pid, int *stat_loc, int options);
int a_shell_execv(const char *path, char* const argv[]);
int a_shell_execve(const char *path, char* const argv[], char** envlist);
void a_shell_exit(int errorCode);

// Signal handling
sig_t a_shell_signal(int signal, sig_t function);
int a_shell_killpid(pid_t pid, int sig);
```

### I/O Functions

```c
// Stream operations
FILE *a_shell_popen(const char *command, const char *type);
FILE* a_shell_stdin(void);
FILE* a_shell_stdout(void);
FILE* a_shell_stderr(void);

// I/O redirection
int a_shell_dup2(int fd1, int fd2);
int a_shell_isatty(int fd);
int a_shell_getstdin(void);
int a_shell_gettty(void);
int a_shell_opentty(void);
void a_shell_closetty(void);

// I/O functions (for #defines)
ssize_t a_shell_write(int fildes, const void *buf, size_t nbyte);
size_t a_shell_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
int a_shell_puts(const char *s);
int a_shell_fputs(const char* s, FILE *stream);
int a_shell_fputc(int c, FILE *stream);
int a_shell_fflush(FILE *stream);
```

### Environment

```c
char *a_shell_getenv(const char *name);
int a_shell_setenv(const char* variableName, const char* value, int overwrite);
int a_shell_unsetenv(const char* variableName);
int a_shell_putenv(char *string);
int a_shell_fchdir(const int fd);
```

### Session Management

```c
void a_shell_switchSession(const void* sessionid);
void a_shell_closeSession(const void* sessionid);
void a_shell_setStreams(FILE* _stdin, FILE* _stdout, FILE* _stderr);
void a_shell_setContext(const void *context);
const void* a_shell_getContext(void);
const char* a_shell_progname(void);
int a_shell_getCommandStatus(void);
```

## Patch Examples

### Example 1: Replace fork/exec with a_shell_* equivalents

```diff
--- a/some_package/executor.c
+++ b/some_package/executor.c
@@ -45,8 +45,17 @@ int execute_command(char *cmd, char **argv)
 {
     int status;
+
+    #ifdef __APPLE__
+    // iOS: Use a-shell-kernel APIs
+    extern "C" { #include "a_shell_system.h" }
+    pid_t pid = a_shell_fork();
+    #else
     pid_t pid = fork();
+    #endif
+
     if (pid == 0) {
         // Child process
         #ifdef __APPLE__
+        a_shell_execv(cmd, argv);
+        #else
         execv(cmd, argv);
+        #endif
         _exit(1);
     } else if (pid > 0) {
         // Parent process
         #ifdef __APPLE__
+        a_shell_waitpid(pid);
+        #else
         waitpid(pid, &status, 0);
+        #endif
     }
     return status;
 }
```

### Example 2: Replace standard I/O with a-shell versions

```diff
--- a/some_package/stdio_patch.c
+++ b/some_package/stdio_patch.c
@@ -20,6 +20,14 @@
  #include <stdio.h>
  #include <stdlib.h>

++#ifdef __APPLE__
++// iOS: Use a-shell-kernel I/O
++#include "a_shell_system.h"
++#define getenv a_shell_getenv
++#define setenv a_shell_setenv
++#define write  a_shell_write
++#endif
++
  int main(int argc, char **argv)
  {
      char *path = getenv("PATH");
```

### Example 3: Session-aware shell

```diff
--- a/some_package/shell.c
+++ b/some_package/shell.c
@@ -100,6 +100,15 @@ void interactive_shell(void)
  {
      char *line;
      int status = 1;
+
+    #ifdef __APPLE__
+    // iOS: Setup streams from a-shell-kernel
+    extern "C" { #include "a_shell_system.h" }
+    FILE* stdin_stream = a_shell_stdin();
+    FILE* stdout_stream = a_shell_stdout();
+    #else
+    FILE* stdin_stream = stdin;
+    FILE* stdout_stream = stdout;
+    #endif

      do {
          printf("> ");
```

## Best Practices

1. **Always use `__APPLE__` guard**: Wrap iOS-specific changes in `#ifdef __APPLE__` blocks
2. **Include a_shell_system.h**: Use `extern "C"` for C++ compatibility
3. **Fallback to standard APIs**: Provide else branches with standard POSIX calls
4. **Test on both iOS and Simulator**: Some behaviors differ between device and simulator
5. **Document why**: Add comments explaining the iOS-specific changes

## Standard Macros (from ios_error.h)

The a-shell-kernel provides these macros for common functions:

```c
#define exit    a_shell_exit
#define _exit   a_shell_exit
#define kill    a_shell_killpid
#define popen   a_shell_popen
#define system  a_shell_system
#define execv   a_shell_execv
#define execvp  a_shell_execv
#define execve  a_shell_execve
#define dup2    a_shell_dup2
#define getenv  a_shell_getenv
#define setenv  a_shell_setenv
#define unsetenv a_shell_unsetenv
#define putenv  a_shell_putenv
#define fchdir  a_shell_fchdir
#define signal  a_shell_signal
```

If you include `ios_error.h` (or let the package include it), you can often use standard function names and the macros will redirect to a_shell_* automatically.

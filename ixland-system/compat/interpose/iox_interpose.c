/* iOS Subsystem for Linux - Symbol Interposition Layer
 *
 * Provides standard Linux syscall names as strong symbols
 * These wrap the iox_* public API
 */

#include <iox/iox_syscalls.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

/* ============================================================================
 * PROCESS MANAGEMENT (16 syscalls)
 * ============================================================================ */

__attribute__((visibility("default"))) pid_t fork(void) {
    return iox_fork();
}

__attribute__((visibility("default"))) int vfork(void) {
    return iox_vfork();
}

__attribute__((visibility("default"))) int execve(const char *p, char *const a[], char *const e[]) {
    return iox_execve(p, a, e);
}

__attribute__((visibility("default"))) int execv(const char *p, char *const a[]) {
    return iox_execv(p, a);
}

__attribute__((visibility("default"))) void exit(int s) {
    iox_exit(s);
    __builtin_unreachable();
}

__attribute__((visibility("default"))) void _exit(int s) {
    iox__exit(s);
    __builtin_unreachable();
}

__attribute__((visibility("default"))) pid_t getpid(void) {
    return iox_getpid();
}

__attribute__((visibility("default"))) pid_t getppid(void) {
    return iox_getppid();
}

__attribute__((visibility("default"))) pid_t getpgrp(void) {
    return iox_getpgrp();
}

__attribute__((visibility("default"))) int setpgrp(void) {
    return iox_setpgrp();
}

__attribute__((visibility("default"))) pid_t getpgid(pid_t p) {
    return iox_getpgid(p);
}

__attribute__((visibility("default"))) int setpgid(pid_t p, pid_t g) {
    return iox_setpgid(p, g);
}

__attribute__((visibility("default"))) pid_t wait(int *s) {
    return iox_wait(s);
}

__attribute__((visibility("default"))) pid_t waitpid(pid_t p, int *s, int o) {
    return iox_waitpid(p, s, o);
}

__attribute__((visibility("default"))) pid_t wait3(int *s, int o, struct rusage *r) {
    return iox_wait3(s, o, r);
}

__attribute__((visibility("default"))) pid_t wait4(pid_t p, int *s, int o, struct rusage *r) {
    return iox_wait4(p, s, o, r);
}

__attribute__((visibility("default"))) int system(const char *c) {
    return iox_system(c);
}

/* ============================================================================
 * FILE OPERATIONS (20 syscalls)
 * ============================================================================ */

__attribute__((visibility("default"))) int open(const char *p, int f, ...) {
    mode_t m = 0;
    if (f & O_CREAT) {
        va_list a;
        va_start(a, f);
        m = va_arg(a, int); /* mode_t promoted to int */
        va_end(a);
    }
    return iox_open(p, f, m);
}

__attribute__((visibility("default"))) int openat(int d, const char *p, int f, ...) {
    mode_t m = 0;
    if (f & O_CREAT) {
        va_list a;
        va_start(a, f);
        m = va_arg(a, int); /* mode_t promoted to int */
        va_end(a);
    }
    return iox_openat(d, p, f, m);
}

__attribute__((visibility("default"))) int creat(const char *p, mode_t m) {
    return iox_creat(p, m);
}

__attribute__((visibility("default"))) ssize_t read(int f, void *b, size_t c) {
    return iox_read(f, b, c);
}

__attribute__((visibility("default"))) ssize_t write(int f, const void *b, size_t c) {
    return iox_write(f, b, c);
}

__attribute__((visibility("default"))) int close(int f) {
    return iox_close(f);
}

__attribute__((visibility("default"))) off_t lseek(int f, off_t o, int w) {
    return iox_lseek(f, o, w);
}

__attribute__((visibility("default"))) ssize_t pread(int f, void *b, size_t c, off_t o) {
    return iox_pread(f, b, c, o);
}

__attribute__((visibility("default"))) ssize_t pwrite(int f, const void *b, size_t c, off_t o) {
    return iox_pwrite(f, b, c, o);
}

__attribute__((visibility("default"))) int dup(int o) {
    return iox_dup(o);
}

__attribute__((visibility("default"))) int dup2(int o, int n) {
    return iox_dup2(o, n);
}

__attribute__((visibility("default"))) int dup3(int o, int n, int f) {
    return iox_dup3(o, n, f);
}

__attribute__((visibility("default"))) int fcntl(int f, int c, ...) {
    va_list a;
    va_start(a, c);
    int arg = va_arg(a, int);
    va_end(a);
    return iox_fcntl(f, c, arg);
}

__attribute__((visibility("default"))) int ioctl(int f, unsigned long r, ...) {
    va_list a;
    va_start(a, r);
    void *arg = va_arg(a, void *);
    va_end(a);
    return iox_ioctl(f, r, arg);
}

__attribute__((visibility("default"))) int access(const char *p, int m) {
    return iox_access(p, m);
}

__attribute__((visibility("default"))) int faccessat(int d, const char *p, int m, int f) {
    return iox_faccessat(d, p, m, f);
}

__attribute__((visibility("default"))) int chdir(const char *p) {
    return iox_chdir(p);
}

__attribute__((visibility("default"))) int fchdir(int f) {
    return iox_fchdir(f);
}

__attribute__((visibility("default"))) char *getcwd(char *b, size_t s) {
    return iox_getcwd(b, s);
}

/* ============================================================================
 * ENVIRONMENT (5 syscalls)
 * ============================================================================ */

__attribute__((visibility("default"))) char *getenv(const char *n) {
    return iox_getenv(n);
}

__attribute__((visibility("default"))) int setenv(const char *n, const char *v, int o) {
    return iox_setenv(n, v, o);
}

__attribute__((visibility("default"))) int unsetenv(const char *n) {
    return iox_unsetenv(n);
}

__attribute__((visibility("default"))) int clearenv(void) {
    return iox_clearenv();
}

/* ============================================================================
 * MISC SYSCALLS - Stubs for unimplemented
 * ============================================================================ */

/* Filesystem stubs */
__attribute__((visibility("default"))) int stat(const char *p, struct stat *s) {
    return iox_stat(p, s);
}

__attribute__((visibility("default"))) int fstat(int f, struct stat *s) {
    return iox_fstat(f, s);
}

__attribute__((visibility("default"))) int lstat(const char *p, struct stat *s) {
    return iox_lstat(p, s);
}

__attribute__((visibility("default"))) int mkdir(const char *p, mode_t m) {
    return iox_mkdir(p, m);
}

__attribute__((visibility("default"))) int rmdir(const char *p) {
    return iox_rmdir(p);
}

__attribute__((visibility("default"))) int unlink(const char *p) {
    return iox_unlink(p);
}

__attribute__((visibility("default"))) int link(const char *o, const char *n) {
    return iox_link(o, n);
}

__attribute__((visibility("default"))) int symlink(const char *t, const char *l) {
    return iox_symlink(t, l);
}

__attribute__((visibility("default"))) ssize_t readlink(const char *p, char *b, size_t s) {
    return iox_readlink(p, b, s);
}

__attribute__((visibility("default"))) int chmod(const char *p, mode_t m) {
    return iox_chmod(p, m);
}

__attribute__((visibility("default"))) int fchmod(int f, mode_t m) {
    return iox_fchmod(f, m);
}

__attribute__((visibility("default"))) int chown(const char *p, uid_t o, gid_t g) {
    return iox_chown(p, o, g);
}

__attribute__((visibility("default"))) int fchown(int f, uid_t o, gid_t g) {
    return iox_fchown(f, o, g);
}

__attribute__((visibility("default"))) int lchown(const char *p, uid_t o, gid_t g) {
    return iox_lchown(p, o, g);
}

__attribute__((visibility("default"))) int chroot(const char *p) {
    return iox_chroot(p);
}

/* Signal stubs */
__attribute__((visibility("default"))) sighandler_t signal(int s, sighandler_t h) {
    return iox_signal(s, h);
}

__attribute__((visibility("default"))) int kill(pid_t p, int s) {
    return iox_kill(p, s);
}

__attribute__((visibility("default"))) int sigaction(int s, const struct sigaction *a,
                                                     struct sigaction *o) {
    return iox_sigaction(s, a, o);
}

unsigned __attribute__((visibility("default"))) int alarm(unsigned int s) {
    return iox_alarm(s);
}

/* Time stubs */
unsigned __attribute__((visibility("default"))) int sleep(unsigned int s) {
    return iox_sleep(s);
}

__attribute__((visibility("default"))) int usleep(useconds_t u) {
    return iox_usleep(u);
}

__attribute__((visibility("default"))) int nanosleep(const struct timespec *r, struct timespec *o) {
    return iox_nanosleep(r, o);
}

/* Memory stubs */
__attribute__((visibility("default"))) void *mmap(void *a, size_t l, int p, int f, int fd,
                                                  off_t o) {
    return iox_mmap(a, l, p, f, fd, o);
}

__attribute__((visibility("default"))) int munmap(void *a, size_t l) {
    return iox_munmap(a, l);
}

__attribute__((visibility("default"))) int mprotect(void *a, size_t l, int p) {
    return iox_mprotect(a, l, p);
}

/* Pipe stubs */
__attribute__((visibility("default"))) int pipe(int p[2]) {
    return iox_pipe(p);
}

/* TTY stubs */
__attribute__((visibility("default"))) int isatty(int f) {
    return iox_isatty(f);
}

__attribute__((visibility("default"))) int tcgetattr(int f, struct termios *t) {
    return iox_tcgetattr(f, t);
}

__attribute__((visibility("default"))) int tcsetattr(int f, int o, const struct termios *t) {
    return iox_tcsetattr(f, o, t);
}

/* iOS Subsystem for Linux - Stub Functions
 *
 * Placeholder implementations for unimplemented syscalls
 * These return ENOSYS (function not implemented)
 */

#include "../internal/iox_internal.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>

/* Define sighandler_t if needed */
#ifndef __sighandler_t
#define __sighandler_t sighandler_t
typedef void (*sighandler_t)(int);
#endif

/* ============================================================================
 * FILESYSTEM STUBS
 * ============================================================================ */

int __iox_stat_impl(const char *pathname, struct stat *statbuf) {
    /* For now, use real stat */
    return stat(pathname, statbuf);
}

int __iox_fstat_impl(int fd, struct stat *statbuf) {
    return fstat(fd, statbuf);
}

int __iox_lstat_impl(const char *pathname, struct stat *statbuf) {
    return lstat(pathname, statbuf);
}

int __iox_mkdir_impl(const char *pathname, mode_t mode) {
    return mkdir(pathname, mode);
}

int __iox_rmdir_impl(const char *pathname) {
    return rmdir(pathname);
}

int __iox_unlink_impl(const char *pathname) {
    return unlink(pathname);
}

int __iox_link_impl(const char *oldpath, const char *newpath) {
    return link(oldpath, newpath);
}

int __iox_symlink_impl(const char *target, const char *linkpath) {
    return symlink(target, linkpath);
}

ssize_t __iox_readlink_impl(const char *pathname, char *buf, size_t bufsiz) {
    return readlink(pathname, buf, bufsiz);
}

int __iox_chmod_impl(const char *pathname, mode_t mode) {
    return chmod(pathname, mode);
}

int __iox_fchmod_impl(int fd, mode_t mode) {
    return fchmod(fd, mode);
}

int __iox_chown_impl(const char *pathname, uid_t owner, gid_t group) {
    return chown(pathname, owner, group);
}

int __iox_fchown_impl(int fd, uid_t owner, gid_t group) {
    return fchown(fd, owner, group);
}

int __iox_lchown_impl(const char *pathname, uid_t owner, gid_t group) {
    return lchown(pathname, owner, group);
}

int __iox_chroot_impl(const char *path) {
    (void)path;
    errno = EPERM;
    return -1;
}

/* ============================================================================
 * SIGNAL STUBS
 * 
 * Note: __iox_kill_impl, __iox_signal_impl, __iox_sigaction_impl
 * are implemented in iox_process.c with full functionality
 * ============================================================================ */

int __iox_sigprocmask_impl(int how, const sigset_t *set, sigset_t *oldset) {
    return sigprocmask(how, set, oldset);
}

int __iox_sigpending_impl(sigset_t *set) {
    return sigpending(set);
}

int __iox_sigsuspend_impl(const sigset_t *mask) {
    return sigsuspend(mask);
}

int __iox_sigemptyset_impl(sigset_t *set) {
    return sigemptyset(set);
}

int __iox_sigfillset_impl(sigset_t *set) {
    return sigfillset(set);
}

int __iox_sigaddset_impl(sigset_t *set, int signum) {
    return sigaddset(set, signum);
}

int __iox_sigdelset_impl(sigset_t *set, int signum) {
    return sigdelset(set, signum);
}

int __iox_sigismember_impl(const sigset_t *set, int signum) {
    return sigismember(set, signum);
}

unsigned int __iox_alarm_impl(unsigned int seconds) {
    return alarm(seconds);
}

int __iox_setitimer_impl(int which, const struct itimerval *new_value, struct itimerval *old_value) {
    return setitimer(which, new_value, old_value);
}

int __iox_getitimer_impl(int which, struct itimerval *curr_value) {
    return getitimer(which, curr_value);
}

int __iox_pause_impl(void) {
    return pause();
}

/* ============================================================================
 * TIME STUBS
 * ============================================================================ */

unsigned int __iox_sleep_impl(unsigned int seconds) {
    return sleep(seconds);
}

int __iox_usleep_impl(useconds_t usec) {
    return usleep(usec);
}

int __iox_nanosleep_impl(const struct timespec *req, struct timespec *rem) {
    return nanosleep(req, rem);
}

int __iox_gettimeofday_impl(struct timeval *tv, struct timezone *tz) {
    return gettimeofday(tv, tz);
}

int __iox_settimeofday_impl(const struct timeval *tv, const struct timezone *tz) {
    return settimeofday(tv, tz);
}

int __iox_clock_gettime_impl(clockid_t clk_id, struct timespec *tp) {
    return clock_gettime(clk_id, tp);
}

time_t __iox_time_impl(time_t *tloc) {
    return time(tloc);
}

/* ============================================================================
 * ENVIRONMENT STUBS
 * ============================================================================ */

/* Use original libc functions to avoid recursion with interposed versions */
#include <dlfcn.h>

static char *(*orig_getenv)(const char *) = NULL;
static int (*orig_setenv)(const char *, const char *, int) = NULL;
static int (*orig_unsetenv)(const char *) = NULL;

static void init_orig_env_funcs(void) {
    static int initialized = 0;
    if (!initialized) {
        orig_getenv = dlsym(RTLD_NEXT, "getenv");
        orig_setenv = dlsym(RTLD_NEXT, "setenv");
        orig_unsetenv = dlsym(RTLD_NEXT, "unsetenv");
        initialized = 1;
    }
}

char *__iox_getenv_impl(const char *name) {
    init_orig_env_funcs();
    return orig_getenv ? orig_getenv(name) : NULL;
}

int __iox_setenv_impl(const char *name, const char *value, int overwrite) {
    init_orig_env_funcs();
    return orig_setenv ? orig_setenv(name, value, overwrite) : -1;
}

int __iox_unsetenv_impl(const char *name) {
    init_orig_env_funcs();
    return orig_unsetenv ? orig_unsetenv(name) : -1;
}

int __iox_clearenv_impl(void) {
    /* Not standard, just clear environ */
    extern char **environ;
    environ = NULL;
    return 0;
}

/* ============================================================================
 * MEMORY STUBS
 * ============================================================================ */

void *__iox_mmap_impl(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return mmap(addr, length, prot, flags, fd, offset);
}

int __iox_munmap_impl(void *addr, size_t length) {
    return munmap(addr, length);
}

int __iox_mprotect_impl(void *addr, size_t len, int prot) {
    return mprotect(addr, len, prot);
}

/* ============================================================================
 * PIPE STUBS
 * ============================================================================ */

int __iox_pipe_impl(int pipefd[2]) {
    return pipe(pipefd);
}

/* ============================================================================
 * TTY STUBS
 * ============================================================================ */

int __iox_isatty_impl(int fd) {
    return isatty(fd);
}

int __iox_tcgetattr_impl(int fd, struct termios *termios_p) {
    return tcgetattr(fd, termios_p);
}

int __iox_tcsetattr_impl(int fd, int optional_actions, const struct termios *termios_p) {
    return tcsetattr(fd, optional_actions, termios_p);
}

/* ============================================================================
 * PUBLIC API - STUBS
 * ============================================================================ */

/* Filesystem */
int iox_stat(const char *pathname, struct stat *statbuf) { return __iox_stat_impl(pathname, statbuf); }
int iox_fstat(int fd, struct stat *statbuf) { return __iox_fstat_impl(fd, statbuf); }
int iox_lstat(const char *pathname, struct stat *statbuf) { return __iox_lstat_impl(pathname, statbuf); }
int iox_mkdir(const char *pathname, mode_t mode) { return __iox_mkdir_impl(pathname, mode); }
int iox_rmdir(const char *pathname) { return __iox_rmdir_impl(pathname); }
int iox_unlink(const char *pathname) { return __iox_unlink_impl(pathname); }
int iox_link(const char *oldpath, const char *newpath) { return __iox_link_impl(oldpath, newpath); }
int iox_symlink(const char *target, const char *linkpath) { return __iox_symlink_impl(target, linkpath); }
ssize_t iox_readlink(const char *pathname, char *buf, size_t bufsiz) { return __iox_readlink_impl(pathname, buf, bufsiz); }
int iox_chmod(const char *pathname, mode_t mode) { return __iox_chmod_impl(pathname, mode); }
int iox_fchmod(int fd, mode_t mode) { return __iox_fchmod_impl(fd, mode); }
int iox_chown(const char *pathname, uid_t owner, gid_t group) { return __iox_chown_impl(pathname, owner, group); }
int iox_fchown(int fd, uid_t owner, gid_t group) { return __iox_fchown_impl(fd, owner, group); }
int iox_lchown(const char *pathname, uid_t owner, gid_t group) { return __iox_lchown_impl(pathname, owner, group); }
int iox_chroot(const char *path) { return __iox_chroot_impl(path); }

/* Signal wrappers - main implementations in iox_process.c */
/* __sighandler_t iox_signal(int signum, __sighandler_t handler) - in iox_process.c */
/* int iox_kill(pid_t pid, int sig) - in iox_process.c */
/* int iox_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) - in iox_process.c */
int iox_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) { return __iox_sigprocmask_impl(how, set, oldset); }
int iox_sigpending(sigset_t *set) { return __iox_sigpending_impl(set); }
int iox_sigsuspend(const sigset_t *mask) { return __iox_sigsuspend_impl(mask); }
int iox_sigemptyset(sigset_t *set) { return __iox_sigemptyset_impl(set); }
int iox_sigfillset(sigset_t *set) { return __iox_sigfillset_impl(set); }
int iox_sigaddset(sigset_t *set, int signum) { return __iox_sigaddset_impl(set, signum); }
int iox_sigdelset(sigset_t *set, int signum) { return __iox_sigdelset_impl(set, signum); }
int iox_sigismember(const sigset_t *set, int signum) { return __iox_sigismember_impl(set, signum); }
unsigned int iox_alarm(unsigned int seconds) { return __iox_alarm_impl(seconds); }
int iox_setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value) { return __iox_setitimer_impl(which, new_value, old_value); }
int iox_getitimer(int which, struct itimerval *curr_value) { return __iox_getitimer_impl(which, curr_value); }
int iox_pause(void) { return __iox_pause_impl(); }

/* Time */
unsigned int iox_sleep(unsigned int seconds) { return __iox_sleep_impl(seconds); }
int iox_usleep(useconds_t usec) { return __iox_usleep_impl(usec); }
int iox_nanosleep(const struct timespec *req, struct timespec *rem) { return __iox_nanosleep_impl(req, rem); }
int iox_gettimeofday(struct timeval *tv, struct timezone *tz) { return __iox_gettimeofday_impl(tv, tz); }
int iox_settimeofday(const struct timeval *tv, const struct timezone *tz) { return __iox_settimeofday_impl(tv, tz); }
int iox_clock_gettime(clockid_t clk_id, struct timespec *tp) { return __iox_clock_gettime_impl(clk_id, tp); }
time_t iox_time(time_t *tloc) { return __iox_time_impl(tloc); }

/* Environment */
char *iox_getenv(const char *name) { return __iox_getenv_impl(name); }
int iox_setenv(const char *name, const char *value, int overwrite) { return __iox_setenv_impl(name, value, overwrite); }
int iox_unsetenv(const char *name) { return __iox_unsetenv_impl(name); }
int iox_clearenv(void) { return __iox_clearenv_impl(); }

/* Memory */
void *iox_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) { return __iox_mmap_impl(addr, length, prot, flags, fd, offset); }
int iox_munmap(void *addr, size_t length) { return __iox_munmap_impl(addr, length); }
int iox_mprotect(void *addr, size_t len, int prot) { return __iox_mprotect_impl(addr, len, prot); }

/* Pipe */
int iox_pipe(int pipefd[2]) { return __iox_pipe_impl(pipefd); }

/* TTY */
int iox_isatty(int fd) { return __iox_isatty_impl(fd); }
int iox_tcgetattr(int fd, struct termios *termios_p) { return __iox_tcgetattr_impl(fd, termios_p); }
int iox_tcsetattr(int fd, int optional_actions, const struct termios *termios_p) { return __iox_tcsetattr_impl(fd, optional_actions, termios_p); }

/* ============================================================================
 * DIRECTORY STUBS
 * ============================================================================ */

int __iox_chdir_impl(const char *path) {
    return chdir(path);
}

int __iox_fchdir_impl(int fd) {
    return fchdir(fd);
}

char *__iox_getcwd_impl(char *buf, size_t size) {
    return getcwd(buf, size);
}

/* ============================================================================
 * PUBLIC API - DIRECTORY
 * ============================================================================ */

int iox_chdir(const char *path) { return __iox_chdir_impl(path); }
int iox_fchdir(int fd) { return __iox_fchdir_impl(fd); }
char *iox_getcwd(char *buf, size_t size) { return __iox_getcwd_impl(buf, size); }

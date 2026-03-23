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
#include <dlfcn.h>

/* Define sighandler_t if needed */
#ifndef __sighandler_t
#define __sighandler_t sighandler_t
typedef void (*sighandler_t)(int);
#endif

/* Original libc function pointers for interposition safety */
static int (*orig_chdir)(const char *path) = NULL;
static int (*orig_fchdir)(int fd) = NULL;
static char *(*orig_getcwd)(char *buf, size_t size) = NULL;

/* Environment */
static char *(*orig_getenv)(const char *) = NULL;
static int (*orig_setenv)(const char *, const char *, int) = NULL;
static int (*orig_unsetenv)(const char *) = NULL;

/* Filesystem */
static int (*orig_stat)(const char *, struct stat *) = NULL;
static int (*orig_fstat)(int, struct stat *) = NULL;
static int (*orig_lstat)(const char *, struct stat *) = NULL;
static int (*orig_mkdir)(const char *, mode_t) = NULL;
static int (*orig_rmdir)(const char *) = NULL;
static int (*orig_unlink)(const char *) = NULL;
static int (*orig_chmod)(const char *, mode_t) = NULL;
static int (*orig_fchmod)(int, mode_t) = NULL;
static int (*orig_chown)(const char *, uid_t, gid_t) = NULL;
static int (*orig_fchown)(int, uid_t, gid_t) = NULL;
static int (*orig_lchown)(const char *, uid_t, gid_t) = NULL;

/* Time */
static unsigned int (*orig_alarm)(unsigned int) = NULL;
static unsigned int (*orig_sleep)(unsigned int) = NULL;
static int (*orig_usleep)(useconds_t) = NULL;
static int (*orig_nanosleep)(const struct timespec *, struct timespec *) = NULL;

/* Memory */
static void *(*orig_mmap)(void *, size_t, int, int, int, off_t) = NULL;
static int (*orig_munmap)(void *, size_t) = NULL;
static int (*orig_mprotect)(void *, size_t, int) = NULL;

static void init_orig_funcs(void) {
    static int initialized = 0;
    if (initialized) return;
    initialized = 1;
    
    /* Directory */
    orig_chdir = (int (*)(const char *))dlsym(RTLD_NEXT, "chdir");
    orig_fchdir = (int (*)(int))dlsym(RTLD_NEXT, "fchdir");
    orig_getcwd = (char *(*)(char *, size_t))dlsym(RTLD_NEXT, "getcwd");
    
    /* Filesystem */
    orig_stat = (int (*)(const char *, struct stat *))dlsym(RTLD_NEXT, "stat");
    orig_fstat = (int (*)(int, struct stat *))dlsym(RTLD_NEXT, "fstat");
    orig_lstat = (int (*)(const char *, struct stat *))dlsym(RTLD_NEXT, "lstat");
    orig_mkdir = (int (*)(const char *, mode_t))dlsym(RTLD_NEXT, "mkdir");
    orig_rmdir = (int (*)(const char *))dlsym(RTLD_NEXT, "rmdir");
    orig_unlink = (int (*)(const char *))dlsym(RTLD_NEXT, "unlink");
    orig_chmod = (int (*)(const char *, mode_t))dlsym(RTLD_NEXT, "chmod");
    orig_fchmod = (int (*)(int, mode_t))dlsym(RTLD_NEXT, "fchmod");
    orig_chown = (int (*)(const char *, uid_t, gid_t))dlsym(RTLD_NEXT, "chown");
    orig_fchown = (int (*)(int, uid_t, gid_t))dlsym(RTLD_NEXT, "fchown");
    orig_lchown = (int (*)(const char *, uid_t, gid_t))dlsym(RTLD_NEXT, "lchown");
    
    /* Time */
    orig_alarm = (unsigned int (*)(unsigned int))dlsym(RTLD_NEXT, "alarm");
    orig_sleep = (unsigned int (*)(unsigned int))dlsym(RTLD_NEXT, "sleep");
    orig_usleep = (int (*)(useconds_t))dlsym(RTLD_NEXT, "usleep");
    orig_nanosleep = (int (*)(const struct timespec *, struct timespec *))dlsym(RTLD_NEXT, "nanosleep");
    
    /* Memory */
    orig_mmap = (void *(*)(void *, size_t, int, int, int, off_t))dlsym(RTLD_NEXT, "mmap");
    orig_munmap = (int (*)(void *, size_t))dlsym(RTLD_NEXT, "munmap");
    orig_mprotect = (int (*)(void *, size_t, int))dlsym(RTLD_NEXT, "mprotect");
}

/* ============================================================================
 * FILESYSTEM STUBS
 * ============================================================================ */

int __iox_stat_impl(const char *pathname, struct stat *statbuf) {
    init_orig_funcs();
    return orig_stat ? orig_stat(pathname, statbuf) : -1;
}

int __iox_fstat_impl(int fd, struct stat *statbuf) {
    init_orig_funcs();
    return orig_fstat ? orig_fstat(fd, statbuf) : -1;
}

int __iox_lstat_impl(const char *pathname, struct stat *statbuf) {
    init_orig_funcs();
    return orig_lstat ? orig_lstat(pathname, statbuf) : -1;
}

int __iox_mkdir_impl(const char *pathname, mode_t mode) {
    init_orig_funcs();
    return orig_mkdir ? orig_mkdir(pathname, mode) : -1;
}

int __iox_rmdir_impl(const char *pathname) {
    init_orig_funcs();
    return orig_rmdir ? orig_rmdir(pathname) : -1;
}

int __iox_unlink_impl(const char *pathname) {
    init_orig_funcs();
    return orig_unlink ? orig_unlink(pathname) : -1;
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
    init_orig_funcs();
    return orig_chmod ? orig_chmod(pathname, mode) : -1;
}

int __iox_fchmod_impl(int fd, mode_t mode) {
    init_orig_funcs();
    return orig_fchmod ? orig_fchmod(fd, mode) : -1;
}

int __iox_chown_impl(const char *pathname, uid_t owner, gid_t group) {
    init_orig_funcs();
    return orig_chown ? orig_chown(pathname, owner, group) : -1;
}

int __iox_fchown_impl(int fd, uid_t owner, gid_t group) {
    init_orig_funcs();
    return orig_fchown ? orig_fchown(fd, owner, group) : -1;
}

int __iox_lchown_impl(const char *pathname, uid_t owner, gid_t group) {
    init_orig_funcs();
    return orig_lchown ? orig_lchown(pathname, owner, group) : -1;
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
    init_orig_funcs();
    return orig_alarm ? orig_alarm(seconds) : 0;
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
    init_orig_funcs();
    return orig_sleep ? orig_sleep(seconds) : 0;
}

int __iox_usleep_impl(useconds_t usec) {
    init_orig_funcs();
    return orig_usleep ? orig_usleep(usec) : -1;
}

int __iox_nanosleep_impl(const struct timespec *req, struct timespec *rem) {
    init_orig_funcs();
    return orig_nanosleep ? orig_nanosleep(req, rem) : -1;
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

/* Environment function pointers declared with others at top of file */

char *__iox_getenv_impl(const char *name) {
    init_orig_funcs();
    return orig_getenv ? orig_getenv(name) : NULL;
}

int __iox_setenv_impl(const char *name, const char *value, int overwrite) {
    init_orig_funcs();
    return orig_setenv ? orig_setenv(name, value, overwrite) : -1;
}

int __iox_unsetenv_impl(const char *name) {
    init_orig_funcs();
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
    init_orig_funcs();
    return orig_mmap ? orig_mmap(addr, length, prot, flags, fd, offset) : MAP_FAILED;
}

int __iox_munmap_impl(void *addr, size_t length) {
    init_orig_funcs();
    return orig_munmap ? orig_munmap(addr, length) : -1;
}

int __iox_mprotect_impl(void *addr, size_t len, int prot) {
    init_orig_funcs();
    return orig_mprotect ? orig_mprotect(addr, len, prot) : -1;
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
    init_orig_funcs();
    if (!orig_chdir) {
        errno = ENOSYS;
        return -1;
    }
    return orig_chdir(path);
}

int __iox_fchdir_impl(int fd) {
    init_orig_funcs();
    if (!orig_fchdir) {
        errno = ENOSYS;
        return -1;
    }
    return orig_fchdir(fd);
}

char *__iox_getcwd_impl(char *buf, size_t size) {
    init_orig_funcs();
    if (!orig_getcwd) {
        errno = ENOSYS;
        return NULL;
    }
    return orig_getcwd(buf, size);
}

/* ============================================================================
 * PUBLIC API - DIRECTORY
 * ============================================================================ */

int iox_chdir(const char *path) { return __iox_chdir_impl(path); }
int iox_fchdir(int fd) { return __iox_fchdir_impl(fd); }
char *iox_getcwd(char *buf, size_t size) { return __iox_getcwd_impl(buf, size); }

/* ============================================================================
 * PUBLIC API - USER/GROUP
 * ============================================================================ */

static uid_t (*orig_getuid)(void) = NULL;
static gid_t (*orig_getgid)(void) = NULL;

uid_t iox_getuid(void) {
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        orig_getuid = (uid_t (*)(void))dlsym(RTLD_NEXT, "getuid");
    }
    if (orig_getuid) return orig_getuid();
    /* Return iOS default uid */
    return 501;
}

gid_t iox_getgid(void) {
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        orig_getgid = (gid_t (*)(void))dlsym(RTLD_NEXT, "getgid");
    }
    if (orig_getgid) return orig_getgid();
    /* Return iOS default gid */
    return 501;
}

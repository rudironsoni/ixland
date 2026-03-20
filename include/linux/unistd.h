/*
 * linux/unistd.h - Standard symbolic constants and types
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: fork, exec, alarm, basic I/O, process identity
 */

#ifndef _LINUX_UNISTD_H
#define _LINUX_UNISTD_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Process Management (POSIX requires these in unistd.h)
 * ============================================================================ */

extern pid_t a_shell_fork(void);
extern pid_t a_shell_vfork(void);
extern int a_shell_execve(const char *pathname, char *const argv[], char *const envp[]);
extern int a_shell_execv(const char *pathname, char *const argv[]);
extern int a_shell_execvp(const char *file, char *const argv[]);
extern void a_shell_exit(int status);
extern void a_shell__exit(int status);
extern pid_t a_shell_getpid(void);
extern pid_t a_shell_getppid(void);
extern pid_t a_shell_getpgrp(void);
extern pid_t a_shell_getpgid(pid_t pid);
extern int a_shell_setpgid(pid_t pid, pid_t pgid);
extern pid_t a_shell_getsid(pid_t pid);
extern pid_t a_shell_setsid(void);

/* ============================================================================
 * Directory Operations (POSIX requires these in unistd.h)
 * ============================================================================ */

extern int a_shell_chdir(const char *path);
extern int a_shell_fchdir(int fd);
extern char *a_shell_getcwd(char *buf, size_t size);
extern int a_shell_chroot(const char *path);

/* ============================================================================
 * File Operations (POSIX requires these in unistd.h)
 * ============================================================================ */

extern ssize_t a_shell_read(int fd, void *buf, size_t count);
extern ssize_t a_shell_write(int fd, const void *buf, size_t count);
extern int a_shell_close(int fd);
extern off_t a_shell_lseek(int fd, off_t offset, int whence);

/* ============================================================================
 * File Descriptor Operations (POSIX requires these in unistd.h)
 * ============================================================================ */

extern int a_shell_dup(int oldfd);
extern int a_shell_dup2(int oldfd, int newfd);
extern int a_shell_dup3(int oldfd, int newfd, int flags);
extern int a_shell_pipe(int pipefd[2]);
extern int a_shell_pipe2(int pipefd[2], int flags);

/* ============================================================================
 * Timing (POSIX requires these in unistd.h)
 * ============================================================================ */

extern unsigned int a_shell_alarm(unsigned int seconds);
extern int a_shell_pause(void);
extern unsigned int a_shell_sleep(unsigned int seconds);
extern int a_shell_nanosleep(const struct timespec *req, struct timespec *rem);

/* ============================================================================
 * User and Group Identity (POSIX requires these in unistd.h)
 * ============================================================================ */

extern uid_t a_shell_getuid(void);
extern uid_t a_shell_geteuid(void);
extern int a_shell_setuid(uid_t uid);
extern int a_shell_seteuid(uid_t euid);
extern int a_shell_setreuid(uid_t ruid, uid_t euid);

extern gid_t a_shell_getgid(void);
extern gid_t a_shell_getegid(void);
extern int a_shell_setgid(gid_t gid);
extern int a_shell_setegid(gid_t egid);
extern int a_shell_setregid(gid_t rgid, gid_t egid);

/* ============================================================================
 * File System Sync
 * ============================================================================ */

extern void a_shell_sync(void);
extern int a_shell_fsync(int fd);
extern int a_shell_fdatasync(int fd);

/* ============================================================================
 * Access and Permissions
 * ============================================================================ */

extern int a_shell_access(const char *pathname, int mode);
extern int a_shell_faccessat(int dirfd, const char *pathname, int mode, int flags);

/* ============================================================================
 * Truncate
 * ============================================================================ */

extern int a_shell_truncate(const char *path, off_t length);
extern int a_shell_ftruncate(int fd, off_t length);

/* ============================================================================
 * Link Operations
 * ============================================================================ */

extern int a_shell_link(const char *oldpath, const char *newpath);
extern int a_shell_linkat(int olddirfd, const char *oldpath,
                          int newdirfd, const char *newpath, int flags);
extern int a_shell_unlink(const char *pathname);
extern int a_shell_unlinkat(int dirfd, const char *pathname, int flags);
extern int a_shell_symlink(const char *target, const char *linkpath);
extern int a_shell_symlinkat(const char *target, int newdirfd, const char *linkpath);
extern ssize_t a_shell_readlink(const char *pathname, char *buf, size_t bufsiz);
extern ssize_t a_shell_readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);

/* ============================================================================
 * Directory Operations
 * ============================================================================ */

extern int a_shell_rmdir(const char *pathname);

/* ============================================================================
 * Hostname
 * ============================================================================ */

extern int a_shell_gethostname(char *name, size_t len);
extern int a_shell_sethostname(const char *name, size_t len);
extern int a_shell_getdomainname(char *name, size_t len);
extern int a_shell_setdomainname(const char *name, size_t len);

/* ============================================================================
 * Login Name
 * ============================================================================ */

extern char *a_shell_getlogin(void);
extern int a_shell_getlogin_r(char *buf, size_t bufsize);

/* ============================================================================
 * Process Priority
 * ============================================================================ */

extern int a_shell_nice(int inc);

/* ============================================================================
 * Legacy Compatibility Macros
 * Each function mapped exactly once in its Linux-standard location
 * ============================================================================ */

#define fork        a_shell_fork
#define vfork       a_shell_vfork
#define execve      a_shell_execve
#define execv       a_shell_execv
#define execvp      a_shell_execvp
#define exit        a_shell_exit
#define _exit       a_shell__exit
#define getpid      a_shell_getpid
#define getppid     a_shell_getppid
#define getpgrp     a_shell_getpgrp
#define getpgid     a_shell_getpgid
#define setpgid     a_shell_setpgid
#define getsid      a_shell_getsid
#define setsid      a_shell_setsid

#define chdir       a_shell_chdir
#define fchdir      a_shell_fchdir
#define getcwd      a_shell_getcwd
#define chroot      a_shell_chroot

#define read        a_shell_read
#define write       a_shell_write
#define close       a_shell_close
#define lseek       a_shell_lseek

#define dup         a_shell_dup
#define dup2        a_shell_dup2
#define dup3        a_shell_dup3
#define pipe        a_shell_pipe
#define pipe2       a_shell_pipe2

#define alarm       a_shell_alarm
#define pause       a_shell_pause
#define sleep       a_shell_sleep
#define nanosleep   a_shell_nanosleep

#define getuid      a_shell_getuid
#define geteuid     a_shell_geteuid
#define setuid      a_shell_setuid
#define seteuid     a_shell_seteuid
#define setreuid    a_shell_setreuid
#define getgid      a_shell_getgid
#define getegid     a_shell_getegid
#define setgid      a_shell_setgid
#define setegid     a_shell_setegid
#define setregid    a_shell_setregid

#define sync        a_shell_sync
#define fsync       a_shell_fsync
#define fdatasync   a_shell_fdatasync

#define access      a_shell_access
#define faccessat   a_shell_faccessat

#define truncate    a_shell_truncate
#define ftruncate   a_shell_ftruncate

#define link        a_shell_link
#define linkat      a_shell_linkat
#define unlink      a_shell_unlink
#define unlinkat    a_shell_unlinkat
#define symlink     a_shell_symlink
#define symlinkat   a_shell_symlinkat
#define readlink    a_shell_readlink
#define readlinkat  a_shell_readlinkat

#define rmdir       a_shell_rmdir

#define gethostname a_shell_gethostname
#define sethostname a_shell_sethostname
#define getdomainname a_shell_getdomainname
#define setdomainname a_shell_setdomainname

#define getlogin    a_shell_getlogin
#define getlogin_r  a_shell_getlogin_r

#define nice        a_shell_nice

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_UNISTD_H */

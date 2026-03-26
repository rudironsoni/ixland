#ifndef IOX_SYSCALLS_H
#define IOX_SYSCALLS_H

/* iOS Subsystem for Linux - Syscall API
 * 
 * Public syscall API - matches Linux syscall signatures
 * All functions are implemented via symbol interposition
 */

#include "iox_types.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>

/* Define sighandler_t if not defined */
#ifndef __sighandler_t
#define __sighandler_t sighandler_t
typedef void (*sighandler_t)(int);
#endif

/* Syscall numbers for statistics */
typedef enum {
    IOX_SYSCALL_FORK = 0,
    IOX_SYSCALL_EXECVE,
    IOX_SYSCALL_EXIT,
    IOX_SYSCALL_OPEN,
    IOX_SYSCALL_READ,
    IOX_SYSCALL_WRITE,
    IOX_SYSCALL_CLOSE,
    IOX_SYSCALL_MMAP,
    IOX_SYSCALL_MUNMAP,
    IOX_SYSCALL_KILL,
    IOX_SYSCALL_SIGACTION,
    IOX_SYSCALL_GETPID,
    IOX_SYSCALL_GETPPID,
    IOX_SYSCALL_WAITPID,
    IOX_SYSCALL_CLONE,
    IOX_SYSCALL_SOCKET,
    IOX_SYSCALL_CONNECT,
    IOX_SYSCALL_BIND,
    IOX_SYSCALL_LISTEN,
    IOX_SYSCALL_ACCEPT,
    IOX_SYSCALL_SEND,
    IOX_SYSCALL_RECV,
    IOX_SYSCALL_SELECT,
    IOX_SYSCALL_POLL,
    IOX_SYSCALL_STAT,
    IOX_SYSCALL_FSTAT,
    IOX_SYSCALL_LSTAT,
    IOX_SYSCALL_CHDIR,
    IOX_SYSCALL_GETCWD,
    IOX_SYSCALL_MKDIR,
    IOX_SYSCALL_RMDIR,
    IOX_SYSCALL_UNLINK,
    IOX_SYSCALL_RENAME,
    IOX_SYSCALL_ACCESS,
    IOX_SYSCALL_CHMOD,
    IOX_SYSCALL_CHOWN,
    IOX_SYSCALL_DUP,
    IOX_SYSCALL_DUP2,
    IOX_SYSCALL_FCNTL,
    IOX_SYSCALL_IOCTL,
    IOX_SYSCALL_PIPE,
    IOX_SYSCALL_ALARM,
    IOX_SYSCALL_SETITIMER,
    IOX_SYSCALL_GETITIMER,
    IOX_SYSCALL_TIME,
    IOX_SYSCALL_GETTIMEOFDAY,
    IOX_SYSCALL_SETTIMEOFDAY,
    IOX_SYSCALL_NANOSLEEP,
    IOX_SYSCALL_PAUSE,
    IOX_SYSCALL_SIGPROCMASK,
    IOX_SYSCALL_SIGPENDING,
    IOX_SYSCALL_SIGSUSPEND,
    IOX_SYSCALL_MPROTECT,
    IOX_SYSCALL_MSYNC,
    IOX_SYSCALL_MLOCK,
    IOX_SYSCALL_MUNLOCK,
    IOX_SYSCALL_SYMLINK,
    IOX_SYSCALL_READLINK,
    IOX_SYSCALL_LSEEK,
    IOX_SYSCALL_FSYNC,
    IOX_SYSCALL_FDATASYNC,
    IOX_SYSCALL_TRUNCATE,
    IOX_SYSCALL_FTRUNCATE,
    IOX_SYSCALL_GETRLIMIT,
    IOX_SYSCALL_SETRLIMIT,
    IOX_SYSCALL_GETRUSAGE,
    IOX_SYSCALL_TIMES,
    IOX_SYSCALL_PTRACE,
    IOX_SYSCALL_GETUID,
    IOX_SYSCALL_GETEUID,
    IOX_SYSCALL_GETGID,
    IOX_SYSCALL_GETEGID,
    IOX_SYSCALL_SETUID,
    IOX_SYSCALL_SETEUID,
    IOX_SYSCALL_SETGID,
    IOX_SYSCALL_SETEGID,
    IOX_SYSCALL_GETGROUPS,
    IOX_SYSCALL_SETGROUPS,
    IOX_SYSCALL_GETPGRP,
    IOX_SYSCALL_SETPGRP,
    IOX_SYSCALL_GETPGID,
    IOX_SYSCALL_SETPGID,
    IOX_SYSCALL_GETSID,
    IOX_SYSCALL_SETSID,
    IOX_SYSCALL_SETREUID,
    IOX_SYSCALL_SETREGID,
    IOX_SYSCALL_GETRESUID,
    IOX_SYSCALL_GETRESGID,
    IOX_SYSCALL_SETRESUID,
    IOX_SYSCALL_SETRESGID,
    IOX_SYSCALL_UMASK,
    IOX_SYSCALL_GETPRIORITY,
    IOX_SYSCALL_SETPRIORITY,
    IOX_SYSCALL_SCHED_GETSCHEDULER,
    IOX_SYSCALL_SCHED_SETSCHEDULER,
    IOX_SYSCALL_SCHED_GETPARAM,
    IOX_SYSCALL_SCHED_SETPARAM,
    IOX_SYSCALL_SCHED_GET_PRIORITY_MAX,
    IOX_SYSCALL_SCHED_GET_PRIORITY_MIN,
    IOX_SYSCALL_SCHED_RR_GET_INTERVAL,
    IOX_SYSCALL_SCHED_YIELD,
    IOX_SYSCALL_PRCTL,
    IOX_SYSCALL_ARCH_PRCTL,
    IOX_SYSCALL_PERSONALITY,
    IOX_SYSCALL_CAPGET,
    IOX_SYSCALL_CAPSET,
    IOX_SYSCALL_QUOTACTL,
    IOX_SYSCALL_MOUNT,
    IOX_SYSCALL_UMOUNT,
    IOX_SYSCALL_SWAPON,
    IOX_SYSCALL_SWAPOFF,
    IOX_SYSCALL_REBOOT,
    IOX_SYSCALL_SETHOSTNAME,
    IOX_SYSCALL_SETDOMAINNAME,
    IOX_SYSCALL_FLOCK,
    IOX_SYSCALL_SYNC,
    IOX_SYSCALL_SYNCFS,
    IOX_SYSCALL_PREAD64,
    IOX_SYSCALL_PWRITE64,
    IOX_SYSCALL_READV,
    IOX_SYSCALL_WRITEV,
    IOX_SYSCALL_PREADV,
    IOX_SYSCALL_PWRITEV,
    IOX_SYSCALL_SENDFILE,
    IOX_SYSCALL_SPLICE,
    IOX_SYSCALL_TEE,
    IOX_SYSCALL_VMSPLICE,
    IOX_SYSCALL_PROCESS_VM_READV,
    IOX_SYSCALL_PROCESS_VM_WRITEV,
    IOX_SYSCALL_FUTEX,
    IOX_SYSCALL_SET_TID_ADDRESS,
    IOX_SYSCALL_EXIT_GROUP,
    IOX_SYSCALL_WAITID,
    IOX_SYSCALL_SEMGET,
    IOX_SYSCALL_SEMOP,
    IOX_SYSCALL_SEMCTL,
    IOX_SYSCALL_MSGGET,
    IOX_SYSCALL_MSGSND,
    IOX_SYSCALL_MSGRCV,
    IOX_SYSCALL_MSGCTL,
    IOX_SYSCALL_SHMGET,
    IOX_SYSCALL_SHMAT,
    IOX_SYSCALL_SHMDT,
    IOX_SYSCALL_SHMCTL,
    IOX_SYSCALL_MQ_OPEN,
    IOX_SYSCALL_MQ_UNLINK,
    IOX_SYSCALL_MQ_TIMEDSEND,
    IOX_SYSCALL_MQ_TIMEDRECEIVE,
    IOX_SYSCALL_MQ_NOTIFY,
    IOX_SYSCALL_MQ_GETSETATTR,
    IOX_SYSCALL_IO_SETUP,
    IOX_SYSCALL_IO_DESTROY,
    IOX_SYSCALL_IO_SUBMIT,
    IOX_SYSCALL_IO_CANCEL,
    IOX_SYSCALL_IO_GETEVENTS,
    IOX_SYSCALL_IOURING_SETUP,
    IOX_SYSCALL_IOURING_ENTER,
    IOX_SYSCALL_IOURING_REGISTER,
    IOX_SYSCALL_EPOLL_CREATE1,
    IOX_SYSCALL_EPOLL_CTL,
    IOX_SYSCALL_EPOLL_WAIT,
    IOX_SYSCALL_EPOLL_PWAIT,
    IOX_SYSCALL_EVENTFD2,
    IOX_SYSCALL_TIMERFD_CREATE,
    IOX_SYSCALL_TIMERFD_SETTIME,
    IOX_SYSCALL_TIMERFD_GETTIME,
    IOX_SYSCALL_SIGNFD4,
    IOX_SYSCALL_USERFAULTFD,
    IOX_SYSCALL_PERF_EVENT_OPEN,
    IOX_SYSCALL_BPF,
    IOX_SYSCALL_BPF_OBJ_PIN,
    IOX_SYSCALL_BPF_OBJ_GET,
    IOX_SYSCALL_KCMP,
    IOX_SYSCALL_FANOTIFY_INIT,
    IOX_SYSCALL_FANOTIFY_MARK,
    IOX_SYSCALL_MEMFD_CREATE,
    IOX_SYSCALL_MEMBARRIER,
    IOX_SYSCALL_PKEY_ALLOC,
    IOX_SYSCALL_PKEY_FREE,
    IOX_SYSCALL_PKEY_MPROTECT,
    IOX_SYSCALL_STATX,
    IOX_SYSCALL_RSEQ,
    IOX_SYSCALL_CLONE3,
    IOX_SYSCALL_OPENAT2,
    IOX_SYSCALL_PIDFD_OPEN,
    IOX_SYSCALL_PIDFD_GETFD,
    IOX_SYSCALL_FACCESSAT2,
    IOX_SYSCALL_PROCESS_MADVISE,
    IOX_SYSCALL_EPOLL_PWAIT2,
    IOX_SYSCALL_MOUNT_SETATTR,
    IOX_SYSCALL_LANDLOCK_CREATE_RULESET,
    IOX_SYSCALL_LANDLOCK_ADD_RULE,
    IOX_SYSCALL_LANDLOCK_RESTRICT_SELF,
    IOX_SYSCALL_PROCESS_MRELEASE,
    IOX_SYSCALL_FUTEX_WAITV,
    IOX_SYSCALL_SET_MEMPOLICY_HOME_NODE,
    IOX_SYSCALL_COUNT
} iox_syscall_t;

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * PROCESS MANAGEMENT (16 syscalls)
 * ============================================================================ */

/* Create new process (thread-based simulation) */
pid_t iox_fork(void);

/* Create new process (shares memory) */
int iox_vfork(void);

/* Execute program with environment */
int iox_execve(const char *pathname, char *const argv[], char *const envp[]);

/* Execute program */
int iox_execv(const char *pathname, char *const argv[]);

/* Terminate process */
void iox_exit(int status);

/* Terminate process immediately */
void iox__exit(int status) __attribute__((noreturn));

/* Get process ID */
pid_t iox_getpid(void);

/* Get parent process ID */
pid_t iox_getppid(void);

/* Get process group ID */
pid_t iox_getpgrp(void);

/* Set process group ID */
int iox_setpgrp(void);

/* Get process group ID for pid */
pid_t iox_getpgid(pid_t pid);

/* Set process group ID for pid */
int iox_setpgid(pid_t pid, pid_t pgid);

/* Wait for any child */
pid_t iox_wait(int *stat_loc);

/* Wait for specific child */
pid_t iox_waitpid(pid_t pid, int *stat_loc, int options);

/* Wait for child with resource usage */
pid_t iox_wait3(int *stat_loc, int options, struct rusage *rusage);

/* Wait for specific child with resource usage */
pid_t iox_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage);

/* Execute shell command */
int iox_system(const char *command);

/* ============================================================================
 * FILE OPERATIONS (20 syscalls)
 * ============================================================================ */

/* Open file */
int iox_open(const char *pathname, int flags, ...);

/* Open file relative to directory */
int iox_openat(int dirfd, const char *pathname, int flags, ...);

/* Create file */
int iox_creat(const char *pathname, mode_t mode);

/* Read from file descriptor */
ssize_t iox_read(int fd, void *buf, size_t count);

/* Write to file descriptor */
ssize_t iox_write(int fd, const void *buf, size_t count);

/* Close file descriptor */
int iox_close(int fd);

/* Reposition file offset */
off_t iox_lseek(int fd, off_t offset, int whence);

/* Read from position */
ssize_t iox_pread(int fd, void *buf, size_t count, off_t offset);

/* Write at position */
ssize_t iox_pwrite(int fd, const void *buf, size_t count, off_t offset);

/* Duplicate file descriptor */
int iox_dup(int oldfd);

/* Duplicate file descriptor to specific number */
int iox_dup2(int oldfd, int newfd);

/* Duplicate file descriptor (extended) */
int iox_dup3(int oldfd, int newfd, int flags);

/* File control operations */
int iox_fcntl(int fd, int cmd, ...);

/* Device-specific I/O control */
int iox_ioctl(int fd, unsigned long request, ...);

/* Check access permissions */
int iox_access(const char *pathname, int mode);

/* Check access (extended) */
int iox_faccessat(int dirfd, const char *pathname, int mode, int flags);

/* Change working directory */
int iox_chdir(const char *path);

/* Change working directory via fd */
int iox_fchdir(int fd);

/* Get current working directory */
char *iox_getcwd(char *buf, size_t size);

/* ============================================================================
 * FILESYSTEM (24 syscalls)
 * ============================================================================ */

/* Get file status */
int iox_stat(const char *pathname, struct stat *statbuf);

/* Get file status via fd */
int iox_fstat(int fd, struct stat *statbuf);

/* Get file status (don't follow symlinks) */
int iox_lstat(const char *pathname, struct stat *statbuf);

/* Get file status relative to directory */
int iox_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);

/* Create directory */
int iox_mkdir(const char *pathname, mode_t mode);

/* Create directory relative to directory fd */
int iox_mkdirat(int dirfd, const char *pathname, mode_t mode);

/* Remove directory */
int iox_rmdir(const char *pathname);

/* Remove file */
int iox_unlink(const char *pathname);

/* Remove file relative to directory fd */
int iox_unlinkat(int dirfd, const char *pathname, int flags);

/* Create hard link */
int iox_link(const char *oldpath, const char *newpath);

/* Create hard link relative to directories */
int iox_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);

/* Create symbolic link */
int iox_symlink(const char *target, const char *linkpath);

/* Create symbolic link relative to directory */
int iox_symlinkat(const char *target, int newdirfd, const char *linkpath);

/* Read symbolic link */
ssize_t iox_readlink(const char *pathname, char *buf, size_t bufsiz);

/* Read symbolic link relative to directory */
ssize_t iox_readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);

/* Change file permissions */
int iox_chmod(const char *pathname, mode_t mode);

/* Change file permissions via fd */
int iox_fchmod(int fd, mode_t mode);

/* Change file permissions relative to directory fd */
int iox_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);

/* Change file owner */
int iox_chown(const char *pathname, uid_t owner, gid_t group);

/* Change file owner via fd */
int iox_fchown(int fd, uid_t owner, gid_t group);

/* Change file owner (don't follow symlinks) */
int iox_lchown(const char *pathname, uid_t owner, gid_t group);

/* Change file owner relative to directory fd */
int iox_fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);

/* Change root directory */
int iox_chroot(const char *path);

/* ============================================================================
 * SIGNAL HANDLING (16 syscalls)
 * ============================================================================ */

/* Set signal handler */
__sighandler_t iox_signal(int signum, __sighandler_t handler);

/* Send signal to process */
int iox_kill(pid_t pid, int sig);

/* Send signal to process group */
int iox_killpg(int pgrp, int sig);

/* Send signal to current process */
int iox_raise(int sig);

/* Examine and change signal action */
int iox_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

/* Examine and change blocked signals */
int iox_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

/* Get set of pending signals */
int iox_sigpending(sigset_t *set);

/* Wait for signal */
int iox_sigsuspend(const sigset_t *mask);

/* Initialize empty signal set */
int iox_sigemptyset(sigset_t *set);

/* Initialize full signal set */
int iox_sigfillset(sigset_t *set);

/* Add signal to set */
int iox_sigaddset(sigset_t *set, int signum);

/* Remove signal from set */
int iox_sigdelset(sigset_t *set, int signum);

/* Test if signal is in set */
int iox_sigismember(const sigset_t *set, int signum);

/* Schedule alarm */
unsigned int iox_alarm(unsigned int seconds);

/* Set interval timer */
int iox_setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);

/* Get interval timer value */
int iox_getitimer(int which, struct itimerval *curr_value);

/* Wait for signal */
int iox_pause(void);

/* ============================================================================
 * MEMORY MANAGEMENT (6 syscalls)
 * ============================================================================ */

/* Map memory */
void *iox_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

/* Unmap memory */
int iox_munmap(void *addr, size_t length);

/* Set memory protection */
int iox_mprotect(void *addr, size_t len, int prot);

/* Synchronize memory with physical storage */
int iox_msync(void *addr, size_t length, int flags);

/* Lock memory pages */
int iox_mlock(const void *addr, size_t len);

/* Unlock memory pages */
int iox_munlock(const void *addr, size_t len);

/* ============================================================================
 * TIME (7 syscalls)
 * ============================================================================ */

/* Sleep for seconds */
unsigned int iox_sleep(unsigned int seconds);

/* Sleep for microseconds */
int iox_usleep(useconds_t usec);

/* Sleep for nanoseconds */
int iox_nanosleep(const struct timespec *req, struct timespec *rem);

/* Get time of day */
int iox_gettimeofday(struct timeval *tv, struct timezone *tz);

/* Set time of day */
int iox_settimeofday(const struct timeval *tv, const struct timezone *tz);

/* Get clock time */
int iox_clock_gettime(clockid_t clk_id, struct timespec *tp);

/* Get current time */
time_t iox_time(time_t *tloc);

/* ============================================================================
 * ENVIRONMENT (5 syscalls)
 * ============================================================================ */

/* Get environment variable */
char *iox_getenv(const char *name);

/* Set environment variable */
int iox_setenv(const char *name, const char *value, int overwrite);

/* Unset environment variable */
int iox_unsetenv(const char *name);

/* Clear environment */
int iox_clearenv(void);

/* Get environment array */
char **iox_environ(void);

/* ============================================================================
 * NETWORK (20 syscalls - passthrough to iOS)
 * ============================================================================ */

/* Create socket */
int iox_socket(int domain, int type, int protocol);

/* Create socket pair */
int iox_socketpair(int domain, int type, int protocol, int sv[2]);

/* Bind socket to address */
int iox_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/* Connect socket to address */
int iox_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/* Listen for connections */
int iox_listen(int sockfd, int backlog);

/* Accept connection */
int iox_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/* Accept connection (extended) */
int iox_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);

/* Send data */
ssize_t iox_send(int sockfd, const void *buf, size_t len, int flags);

/* Send data to address */
ssize_t iox_sendto(int sockfd, const void *buf, size_t len, int flags,
                     const struct sockaddr *dest_addr, socklen_t addrlen);

/* Send message */
ssize_t iox_sendmsg(int sockfd, const struct msghdr *msg, int flags);

/* Receive data */
ssize_t iox_recv(int sockfd, void *buf, size_t len, int flags);

/* Receive data with sender address */
ssize_t iox_recvfrom(int sockfd, void *buf, size_t len, int flags,
                       struct sockaddr *src_addr, socklen_t *addrlen);

/* Receive message */
ssize_t iox_recvmsg(int sockfd, struct msghdr *msg, int flags);

/* Shutdown socket */
int iox_shutdown(int sockfd, int how);

/* Get socket name */
int iox_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/* Get peer name */
int iox_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/* Get socket option */
int iox_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);

/* Set socket option */
int iox_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

/* ============================================================================
 * PIPES (4 syscalls)
 * ============================================================================ */

/* Create pipe */
int iox_pipe(int pipefd[2]);

/* Create pipe (extended) */
int iox_pipe2(int pipefd[2], int flags);

/* Create FIFO */
int iox_mkfifo(const char *pathname, mode_t mode);

/* Create FIFO relative to directory */
int iox_mkfifoat(int dirfd, const char *pathname, mode_t mode);

/* ============================================================================
 * SELECT/POLL (4 syscalls)
 * ============================================================================ */

/* Monitor file descriptors */
int iox_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
               struct timeval *timeout);

/* Monitor file descriptors (signal-safe) */
int iox_pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                const struct timespec *timeout, const sigset_t *sigmask);

/* Wait for file descriptor events */
int iox_poll(struct pollfd *fds, nfds_t nfds, int timeout);

/* Wait for events (signal-safe) */
int iox_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask);

/* ============================================================================
 * TTY (7 syscalls)
 * ============================================================================ */

/* Test if file descriptor is a terminal */
int iox_isatty(int fd);

/* Get terminal name */
int iox_ttyname_r(int fd, char *buf, size_t buflen);

/* Get terminal attributes */
int iox_tcgetattr(int fd, struct termios *termios_p);

/* Set terminal attributes */
int iox_tcsetattr(int fd, int optional_actions, const struct termios *termios_p);

/* Send break condition */
int iox_tcsendbreak(int fd, int duration);

/* Wait for output to drain */
int iox_tcdrain(int fd);

/* Discard terminal data */
int iox_tcflush(int fd, int queue_selector);

/* Suspend/restart terminal */
int iox_tcflow(int fd, int action);

/* ============================================================================
 * MISC (3 syscalls)
 * ============================================================================ */

/* Raw syscall dispatcher */
long iox__syscall(long number, ...);

/* Print error message */
void iox_perror(const char *s);

/* Get error string */
char *iox_strerror(int errnum);

/* ============================================================================
 * IOX-SPECIFIC EXTENSIONS
 * ============================================================================ */

/* Initialize iox library - call before any other iox functions */
int iox_init(const iox_config_t *config);

/* Cleanup iox library */
void iox_cleanup(void);

/* Get iox version */
const char *iox_version(void);

/* Check if initialized */
int iox_is_initialized(void);

/* Get system information */
iox_sys_info_t iox_get_sys_info(void);

/* Get process information */
iox_proc_info_t *iox_get_proc_info(pid_t pid);

/* Free process info */
void iox_free_proc_info(iox_proc_info_t *info);

/* Get thread information */
iox_thread_info_t *iox_get_thread_info(pthread_t tid);

/* Free thread info */
void iox_free_thread_info(iox_thread_info_t *info);

/* Set process callback */
void iox_set_proc_callback(iox_proc_callback_t callback, void *userdata);

/* Set thread callback */
void iox_set_thread_callback(iox_thread_callback_t callback, void *userdata);

/* Set file callback */
void iox_set_file_callback(iox_file_callback_t callback, void *userdata);

/* Get syscall statistics */
void iox_get_syscall_stats(iox_syscall_t syscall, iox_syscall_stat_t *stats);

/* Get memory statistics */
void iox_get_memory_stats(iox_memory_stat_t *stats);

/* Get process statistics */
void iox_get_process_stats(iox_process_stat_t *stats);

/* Dump debug information */
void iox_debug_dump(FILE *stream);

/* Enable/disable syscall tracing */
void iox_set_tracing(int enabled);

#ifdef __cplusplus
}
#endif

#endif /* IOX_SYSCALLS_H */

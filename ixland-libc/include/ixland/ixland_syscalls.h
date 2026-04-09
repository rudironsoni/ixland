#ifndef IXLAND_SYSCALLS_H
#define IXLAND_SYSCALLS_H

/* iXland libc - Public Syscall API
 *
 * Public syscall API - matches Linux syscall signatures.
 * This header defines the interface; implementations live elsewhere.
 *
 * Extracted from ixland-system as part of libc boundary formation.
 */

#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "ixland_types.h"

/* Define sighandler_t if not defined */
#ifndef __sighandler_t
#define __sighandler_t sighandler_t
typedef void (*sighandler_t)(int);
#endif

/* Syscall numbers for statistics */
typedef enum {
    IXLAND_SYSCALL_FORK = 0,
    IXLAND_SYSCALL_EXECVE,
    IXLAND_SYSCALL_EXIT,
    IXLAND_SYSCALL_OPEN,
    IXLAND_SYSCALL_READ,
    IXLAND_SYSCALL_WRITE,
    IXLAND_SYSCALL_CLOSE,
    IXLAND_SYSCALL_MMAP,
    IXLAND_SYSCALL_MUNMAP,
    IXLAND_SYSCALL_KILL,
    IXLAND_SYSCALL_SIGACTION,
    IXLAND_SYSCALL_GETPID,
    IXLAND_SYSCALL_GETPPID,
    IXLAND_SYSCALL_WAITPID,
    IXLAND_SYSCALL_CLONE,
    IXLAND_SYSCALL_SOCKET,
    IXLAND_SYSCALL_CONNECT,
    IXLAND_SYSCALL_BIND,
    IXLAND_SYSCALL_LISTEN,
    IXLAND_SYSCALL_ACCEPT,
    IXLAND_SYSCALL_SEND,
    IXLAND_SYSCALL_RECV,
    IXLAND_SYSCALL_SELECT,
    IXLAND_SYSCALL_POLL,
    IXLAND_SYSCALL_STAT,
    IXLAND_SYSCALL_FSTAT,
    IXLAND_SYSCALL_LSTAT,
    IXLAND_SYSCALL_CHDIR,
    IXLAND_SYSCALL_GETCWD,
    IXLAND_SYSCALL_MKDIR,
    IXLAND_SYSCALL_RMDIR,
    IXLAND_SYSCALL_UNLINK,
    IXLAND_SYSCALL_RENAME,
    IXLAND_SYSCALL_ACCESS,
    IXLAND_SYSCALL_CHMOD,
    IXLAND_SYSCALL_CHOWN,
    IXLAND_SYSCALL_DUP,
    IXLAND_SYSCALL_DUP2,
    IXLAND_SYSCALL_FCNTL,
    IXLAND_SYSCALL_IOCTL,
    IXLAND_SYSCALL_PIPE,
    IXLAND_SYSCALL_ALARM,
    IXLAND_SYSCALL_SETITIMER,
    IXLAND_SYSCALL_GETITIMER,
    IXLAND_SYSCALL_TIME,
    IXLAND_SYSCALL_GETTIMEOFDAY,
    IXLAND_SYSCALL_SETTIMEOFDAY,
    IXLAND_SYSCALL_NANOSLEEP,
    IXLAND_SYSCALL_PAUSE,
    IXLAND_SYSCALL_SIGPROCMASK,
    IXLAND_SYSCALL_SIGPENDING,
    IXLAND_SYSCALL_SIGSUSPEND,
    IXLAND_SYSCALL_MPROTECT,
    IXLAND_SYSCALL_MSYNC,
    IXLAND_SYSCALL_MLOCK,
    IXLAND_SYSCALL_MUNLOCK,
    IXLAND_SYSCALL_SYMLINK,
    IXLAND_SYSCALL_READLINK,
    IXLAND_SYSCALL_LSEEK,
    IXLAND_SYSCALL_FSYNC,
    IXLAND_SYSCALL_FDATASYNC,
    IXLAND_SYSCALL_TRUNCATE,
    IXLAND_SYSCALL_FTRUNCATE,
    IXLAND_SYSCALL_GETRLIMIT,
    IXLAND_SYSCALL_SETRLIMIT,
    IXLAND_SYSCALL_GETRUSAGE,
    IXLAND_SYSCALL_TIMES,
    IXLAND_SYSCALL_PTRACE,
    IXLAND_SYSCALL_GETUID,
    IXLAND_SYSCALL_GETEUID,
    IXLAND_SYSCALL_GETGID,
    IXLAND_SYSCALL_GETEGID,
    IXLAND_SYSCALL_SETUID,
    IXLAND_SYSCALL_SETEUID,
    IXLAND_SYSCALL_SETGID,
    IXLAND_SYSCALL_SETEGID,
    IXLAND_SYSCALL_GETGROUPS,
    IXLAND_SYSCALL_SETGROUPS,
    IXLAND_SYSCALL_GETPGRP,
    IXLAND_SYSCALL_SETPGRP,
    IXLAND_SYSCALL_GETPGID,
    IXLAND_SYSCALL_SETPGID,
    IXLAND_SYSCALL_GETSID,
    IXLAND_SYSCALL_SETSID,
    IXLAND_SYSCALL_SETREUID,
    IXLAND_SYSCALL_SETREGID,
    IXLAND_SYSCALL_GETRESUID,
    IXLAND_SYSCALL_GETRESGID,
    IXLAND_SYSCALL_SETRESUID,
    IXLAND_SYSCALL_SETRESGID,
    IXLAND_SYSCALL_UMASK,
    IXLAND_SYSCALL_GETPRIORITY,
    IXLAND_SYSCALL_SETPRIORITY,
    IXLAND_SYSCALL_SCHED_GETSCHEDULER,
    IXLAND_SYSCALL_SCHED_SETSCHEDULER,
    IXLAND_SYSCALL_SCHED_GETPARAM,
    IXLAND_SYSCALL_SCHED_SETPARAM,
    IXLAND_SYSCALL_SCHED_GET_PRIORITY_MAX,
    IXLAND_SYSCALL_SCHED_GET_PRIORITY_MIN,
    IXLAND_SYSCALL_SCHED_RR_GET_INTERVAL,
    IXLAND_SYSCALL_SCHED_YIELD,
    IXLAND_SYSCALL_PRCTL,
    IXLAND_SYSCALL_ARCH_PRCTL,
    IXLAND_SYSCALL_PERSONALITY,
    IXLAND_SYSCALL_CAPGET,
    IXLAND_SYSCALL_CAPSET,
    IXLAND_SYSCALL_QUOTACTL,
    IXLAND_SYSCALL_MOUNT,
    IXLAND_SYSCALL_UMOUNT,
    IXLAND_SYSCALL_SWAPON,
    IXLAND_SYSCALL_SWAPOFF,
    IXLAND_SYSCALL_REBOOT,
    IXLAND_SYSCALL_SETHOSTNAME,
    IXLAND_SYSCALL_SETDOMAINNAME,
    IXLAND_SYSCALL_FLOCK,
    IXLAND_SYSCALL_SYNC,
    IXLAND_SYSCALL_SYNCFS,
    IXLAND_SYSCALL_PREAD64,
    IXLAND_SYSCALL_PWRITE64,
    IXLAND_SYSCALL_READV,
    IXLAND_SYSCALL_WRITEV,
    IXLAND_SYSCALL_PREADV,
    IXLAND_SYSCALL_PWRITEV,
    IXLAND_SYSCALL_SENDFILE,
    IXLAND_SYSCALL_SPLICE,
    IXLAND_SYSCALL_TEE,
    IXLAND_SYSCALL_VMSPLICE,
    IXLAND_SYSCALL_PROCESS_VM_READV,
    IXLAND_SYSCALL_PROCESS_VM_WRITEV,
    IXLAND_SYSCALL_FUTEX,
    IXLAND_SYSCALL_SET_TID_ADDRESS,
    IXLAND_SYSCALL_EXIT_GROUP,
    IXLAND_SYSCALL_WAITID,
    IXLAND_SYSCALL_SEMGET,
    IXLAND_SYSCALL_SEMOP,
    IXLAND_SYSCALL_SEMCTL,
    IXLAND_SYSCALL_MSGGET,
    IXLAND_SYSCALL_MSGSND,
    IXLAND_SYSCALL_MSGRCV,
    IXLAND_SYSCALL_MSGCTL,
    IXLAND_SYSCALL_SHMGET,
    IXLAND_SYSCALL_SHMAT,
    IXLAND_SYSCALL_SHMDT,
    IXLAND_SYSCALL_SHMCTL,
    IXLAND_SYSCALL_MQ_OPEN,
    IXLAND_SYSCALL_MQ_UNLINK,
    IXLAND_SYSCALL_MQ_TIMEDSEND,
    IXLAND_SYSCALL_MQ_TIMEDRECEIVE,
    IXLAND_SYSCALL_MQ_NOTIFY,
    IXLAND_SYSCALL_MQ_GETSETATTR,
    IXLAND_SYSCALL_IO_SETUP,
    IXLAND_SYSCALL_IO_DESTROY,
    IXLAND_SYSCALL_IO_SUBMIT,
    IXLAND_SYSCALL_IO_CANCEL,
    IXLAND_SYSCALL_IO_GETEVENTS,
    IXLAND_SYSCALL_IOURING_SETUP,
    IXLAND_SYSCALL_IOURING_ENTER,
    IXLAND_SYSCALL_IOURING_REGISTER,
    IXLAND_SYSCALL_EPOLL_CREATE1,
    IXLAND_SYSCALL_EPOLL_CTL,
    IXLAND_SYSCALL_EPOLL_WAIT,
    IXLAND_SYSCALL_EPOLL_PWAIT,
    IXLAND_SYSCALL_EVENTFD2,
    IXLAND_SYSCALL_TIMERFD_CREATE,
    IXLAND_SYSCALL_TIMERFD_SETTIME,
    IXLAND_SYSCALL_TIMERFD_GETTIME,
    IXLAND_SYSCALL_SIGNFD4,
    IXLAND_SYSCALL_USERFAULTFD,
    IXLAND_SYSCALL_PERF_EVENT_OPEN,
    IXLAND_SYSCALL_BPF,
    IXLAND_SYSCALL_BPF_OBJ_PIN,
    IXLAND_SYSCALL_BPF_OBJ_GET,
    IXLAND_SYSCALL_KCMP,
    IXLAND_SYSCALL_FANOTIFY_INIT,
    IXLAND_SYSCALL_FANOTIFY_MARK,
    IXLAND_SYSCALL_MEMFD_CREATE,
    IXLAND_SYSCALL_MEMBARRIER,
    IXLAND_SYSCALL_PKEY_ALLOC,
    IXLAND_SYSCALL_PKEY_FREE,
    IXLAND_SYSCALL_PKEY_MPROTECT,
    IXLAND_SYSCALL_STATX,
    IXLAND_SYSCALL_RSEQ,
    IXLAND_SYSCALL_CLONE3,
    IXLAND_SYSCALL_OPENAT2,
    IXLAND_SYSCALL_PIDFD_OPEN,
    IXLAND_SYSCALL_PIDFD_GETFD,
    IXLAND_SYSCALL_FACCESSAT2,
    IXLAND_SYSCALL_PROCESS_MADVISE,
    IXLAND_SYSCALL_EPOLL_PWAIT2,
    IXLAND_SYSCALL_MOUNT_SETATTR,
    IXLAND_SYSCALL_LANDLOCK_CREATE_RULESET,
    IXLAND_SYSCALL_LANDLOCK_ADD_RULE,
    IXLAND_SYSCALL_LANDLOCK_RESTRICT_SELF,
    IXLAND_SYSCALL_PROCESS_MRELEASE,
    IXLAND_SYSCALL_FUTEX_WAITV,
    IXLAND_SYSCALL_SET_MEMPOLICY_HOME_NODE,
    IXLAND_SYSCALL_COUNT
} ixland_syscall_t;

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * PROCESS MANAGEMENT
 * ============================================================================ */

pid_t ixland_fork(void);
int ixland_vfork(void);
int ixland_execve(const char *pathname, char *const argv[], char *const envp[]);
int ixland_execv(const char *pathname, char *const argv[]);
void ixland_exit(int status);
void ixland__exit(int status) __attribute__((noreturn));
pid_t ixland_getpid(void);
pid_t ixland_getppid(void);
pid_t ixland_getpgrp(void);
int ixland_setpgrp(void);
pid_t ixland_getpgid(pid_t pid);
int ixland_setpgid(pid_t pid, pid_t pgid);
pid_t ixland_getsid(pid_t pid);
pid_t ixland_setsid(void);
pid_t ixland_wait(int *stat_loc);
pid_t ixland_waitpid(pid_t pid, int *stat_loc, int options);
pid_t ixland_wait3(int *stat_loc, int options, struct rusage *rusage);
pid_t ixland_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage);
int ixland_system(const char *command);

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */

int ixland_open(const char *pathname, int flags, ...);
int ixland_openat(int dirfd, const char *pathname, int flags, ...);
int ixland_creat(const char *pathname, mode_t mode);
ssize_t ixland_read(int fd, void *buf, size_t count);
ssize_t ixland_write(int fd, const void *buf, size_t count);
int ixland_close(int fd);
off_t ixland_lseek(int fd, off_t offset, int whence);
ssize_t ixland_pread(int fd, void *buf, size_t count, off_t offset);
ssize_t ixland_pwrite(int fd, const void *buf, size_t count, off_t offset);
int ixland_dup(int oldfd);
int ixland_dup2(int oldfd, int newfd);
int ixland_dup3(int oldfd, int newfd, int flags);
int ixland_fcntl(int fd, int cmd, ...);
int ixland_ioctl(int fd, unsigned long request, ...);
int ixland_access(const char *pathname, int mode);
int ixland_faccessat(int dirfd, const char *pathname, int mode, int flags);
int ixland_chdir(const char *path);
int ixland_fchdir(int fd);
char *ixland_getcwd(char *buf, size_t size);

/* ============================================================================
 * FILESYSTEM
 * ============================================================================ */

int ixland_stat(const char *pathname, struct stat *statbuf);
int ixland_fstat(int fd, struct stat *statbuf);
int ixland_lstat(const char *pathname, struct stat *statbuf);
int ixland_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);
int ixland_mkdir(const char *pathname, mode_t mode);
int ixland_mkdirat(int dirfd, const char *pathname, mode_t mode);
int ixland_rmdir(const char *pathname);
int ixland_unlink(const char *pathname);
int ixland_unlinkat(int dirfd, const char *pathname, int flags);
int ixland_link(const char *oldpath, const char *newpath);
int ixland_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
int ixland_symlink(const char *target, const char *linkpath);
int ixland_symlinkat(const char *target, int newdirfd, const char *linkpath);
ssize_t ixland_readlink(const char *pathname, char *buf, size_t bufsiz);
ssize_t ixland_readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
int ixland_chmod(const char *pathname, mode_t mode);
int ixland_fchmod(int fd, mode_t mode);
int ixland_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
int ixland_chown(const char *pathname, uid_t owner, gid_t group);
int ixland_fchown(int fd, uid_t owner, gid_t group);
int ixland_lchown(const char *pathname, uid_t owner, gid_t group);
int ixland_fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);
int ixland_chroot(const char *path);

/* ============================================================================
 * SIGNAL HANDLING
 * ============================================================================ */

__sighandler_t ixland_signal(int signum, __sighandler_t handler);
int ixland_kill(pid_t pid, int sig);
int ixland_killpg(int pgrp, int sig);
int ixland_raise(int sig);
int ixland_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int ixland_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int ixland_sigpending(sigset_t *set);
int ixland_sigsuspend(const sigset_t *mask);
int ixland_sigemptyset(sigset_t *set);
int ixland_sigfillset(sigset_t *set);
int ixland_sigaddset(sigset_t *set, int signum);
int ixland_sigdelset(sigset_t *set, int signum);
int ixland_sigismember(const sigset_t *set, int signum);
unsigned int ixland_alarm(unsigned int seconds);
int ixland_setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
int ixland_getitimer(int which, struct itimerval *curr_value);
int ixland_pause(void);

/* ============================================================================
 * MEMORY MANAGEMENT
 * ============================================================================ */

void *ixland_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int ixland_munmap(void *addr, size_t length);
int ixland_mprotect(void *addr, size_t len, int prot);
int ixland_msync(void *addr, size_t length, int flags);
int ixland_mlock(const void *addr, size_t len);
int ixland_munlock(const void *addr, size_t len);

/* ============================================================================
 * TIME
 * ============================================================================ */

unsigned int ixland_sleep(unsigned int seconds);
int ixland_usleep(useconds_t usec);
int ixland_nanosleep(const struct timespec *req, struct timespec *rem);
int ixland_gettimeofday(struct timeval *tv, struct timezone *tz);
int ixland_settimeofday(const struct timeval *tv, const struct timezone *tz);
int ixland_clock_gettime(clockid_t clk_id, struct timespec *tp);
time_t ixland_time(time_t *tloc);

/* ============================================================================
 * ENVIRONMENT
 * ============================================================================ */

char *ixland_getenv(const char *name);
int ixland_setenv(const char *name, const char *value, int overwrite);
int ixland_unsetenv(const char *name);
int ixland_clearenv(void);
char **ixland_environ(void);

/* ============================================================================
 * NETWORK
 * ============================================================================ */

int ixland_socket(int domain, int type, int protocol);
int ixland_socketpair(int domain, int type, int protocol, int sv[2]);
int ixland_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int ixland_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int ixland_listen(int sockfd, int backlog);
int ixland_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int ixland_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
ssize_t ixland_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t ixland_sendto(int sockfd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t ixland_sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t ixland_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t ixland_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr,
                        socklen_t *addrlen);
ssize_t ixland_recvmsg(int sockfd, struct msghdr *msg, int flags);
int ixland_shutdown(int sockfd, int how);
int ixland_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int ixland_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int ixland_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int ixland_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

/* ============================================================================
 * PIPES
 * ============================================================================ */

int ixland_pipe(int pipefd[2]);
int ixland_pipe2(int pipefd[2], int flags);
int ixland_mkfifo(const char *pathname, mode_t mode);
int ixland_mkfifoat(int dirfd, const char *pathname, mode_t mode);

/* ============================================================================
 * SELECT/POLL
 * ============================================================================ */

int ixland_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                  struct timeval *timeout);
int ixland_pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                   const struct timespec *timeout, const sigset_t *sigmask);
int ixland_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int ixland_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p,
                 const sigset_t *sigmask);

/* ============================================================================
 * TTY
 * ============================================================================ */

int ixland_isatty(int fd);
int ixland_ttyname_r(int fd, char *buf, size_t buflen);
int ixland_tcgetattr(int fd, struct termios *termios_p);
int ixland_tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
int ixland_tcsendbreak(int fd, int duration);
int ixland_tcdrain(int fd);
int ixland_tcflush(int fd, int queue_selector);
int ixland_tcflow(int fd, int action);

/* ============================================================================
 * MISC
 * ============================================================================ */

long ixland__syscall(long number, ...);
void ixland_perror(const char *s);
const char *ixland_strerror(int errnum);

/* ============================================================================
 * IXLAND-SPECIFIC EXTENSIONS
 * ============================================================================ */

int ixland_init(const ixland_config_t *config);
void ixland_cleanup(void);
const char *ixland_version(void);
int ixland_is_initialized(void);
ixland_sys_info_t ixland_get_sys_info(void);
ixland_proc_info_t *ixland_get_proc_info(pid_t pid);
void ixland_free_proc_info(ixland_proc_info_t *info);
ixland_thread_info_t *ixland_get_thread_info(pthread_t tid);
void ixland_free_thread_info(ixland_thread_info_t *info);
void ixland_set_proc_callback(ixland_proc_callback_t callback, void *userdata);
void ixland_set_thread_callback(ixland_thread_callback_t callback, void *userdata);
void ixland_set_file_callback(ixland_file_callback_t callback, void *userdata);
void ixland_get_syscall_stats(ixland_syscall_t syscall, ixland_syscall_stat_t *stats);
void ixland_get_memory_stats(ixland_memory_stat_t *stats);
void ixland_get_process_stats(ixland_process_stat_t *stats);
void ixland_debug_dump(FILE *stream);
void ixland_set_tracing(int enabled);

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_SYSCALLS_H */

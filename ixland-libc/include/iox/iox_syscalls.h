#ifndef IOX_SYSCALLS_H
#define IOX_SYSCALLS_H

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

#include "iox_types.h"

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
 * PROCESS MANAGEMENT
 * ============================================================================ */

pid_t iox_fork(void);
int iox_vfork(void);
int iox_execve(const char *pathname, char *const argv[], char *const envp[]);
int iox_execv(const char *pathname, char *const argv[]);
void iox_exit(int status);
void iox__exit(int status) __attribute__((noreturn));
pid_t iox_getpid(void);
pid_t iox_getppid(void);
pid_t iox_getpgrp(void);
int iox_setpgrp(void);
pid_t iox_getpgid(pid_t pid);
int iox_setpgid(pid_t pid, pid_t pgid);
pid_t iox_wait(int *stat_loc);
pid_t iox_waitpid(pid_t pid, int *stat_loc, int options);
pid_t iox_wait3(int *stat_loc, int options, struct rusage *rusage);
pid_t iox_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage);
int iox_system(const char *command);

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */

int iox_open(const char *pathname, int flags, ...);
int iox_openat(int dirfd, const char *pathname, int flags, ...);
int iox_creat(const char *pathname, mode_t mode);
ssize_t iox_read(int fd, void *buf, size_t count);
ssize_t iox_write(int fd, const void *buf, size_t count);
int iox_close(int fd);
off_t iox_lseek(int fd, off_t offset, int whence);
ssize_t iox_pread(int fd, void *buf, size_t count, off_t offset);
ssize_t iox_pwrite(int fd, const void *buf, size_t count, off_t offset);
int iox_dup(int oldfd);
int iox_dup2(int oldfd, int newfd);
int iox_dup3(int oldfd, int newfd, int flags);
int iox_fcntl(int fd, int cmd, ...);
int iox_ioctl(int fd, unsigned long request, ...);
int iox_access(const char *pathname, int mode);
int iox_faccessat(int dirfd, const char *pathname, int mode, int flags);
int iox_chdir(const char *path);
int iox_fchdir(int fd);
char *iox_getcwd(char *buf, size_t size);

/* ============================================================================
 * FILESYSTEM
 * ============================================================================ */

int iox_stat(const char *pathname, struct stat *statbuf);
int iox_fstat(int fd, struct stat *statbuf);
int iox_lstat(const char *pathname, struct stat *statbuf);
int iox_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);
int iox_mkdir(const char *pathname, mode_t mode);
int iox_mkdirat(int dirfd, const char *pathname, mode_t mode);
int iox_rmdir(const char *pathname);
int iox_unlink(const char *pathname);
int iox_unlinkat(int dirfd, const char *pathname, int flags);
int iox_link(const char *oldpath, const char *newpath);
int iox_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
int iox_symlink(const char *target, const char *linkpath);
int iox_symlinkat(const char *target, int newdirfd, const char *linkpath);
ssize_t iox_readlink(const char *pathname, char *buf, size_t bufsiz);
ssize_t iox_readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
int iox_chmod(const char *pathname, mode_t mode);
int iox_fchmod(int fd, mode_t mode);
int iox_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
int iox_chown(const char *pathname, uid_t owner, gid_t group);
int iox_fchown(int fd, uid_t owner, gid_t group);
int iox_lchown(const char *pathname, uid_t owner, gid_t group);
int iox_fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);
int iox_chroot(const char *path);

/* ============================================================================
 * SIGNAL HANDLING
 * ============================================================================ */

__sighandler_t iox_signal(int signum, __sighandler_t handler);
int iox_kill(pid_t pid, int sig);
int iox_killpg(int pgrp, int sig);
int iox_raise(int sig);
int iox_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int iox_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int iox_sigpending(sigset_t *set);
int iox_sigsuspend(const sigset_t *mask);
int iox_sigemptyset(sigset_t *set);
int iox_sigfillset(sigset_t *set);
int iox_sigaddset(sigset_t *set, int signum);
int iox_sigdelset(sigset_t *set, int signum);
int iox_sigismember(const sigset_t *set, int signum);
unsigned int iox_alarm(unsigned int seconds);
int iox_setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
int iox_getitimer(int which, struct itimerval *curr_value);
int iox_pause(void);

/* ============================================================================
 * MEMORY MANAGEMENT
 * ============================================================================ */

void *iox_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int iox_munmap(void *addr, size_t length);
int iox_mprotect(void *addr, size_t len, int prot);
int iox_msync(void *addr, size_t length, int flags);
int iox_mlock(const void *addr, size_t len);
int iox_munlock(const void *addr, size_t len);

/* ============================================================================
 * TIME
 * ============================================================================ */

unsigned int iox_sleep(unsigned int seconds);
int iox_usleep(useconds_t usec);
int iox_nanosleep(const struct timespec *req, struct timespec *rem);
int iox_gettimeofday(struct timeval *tv, struct timezone *tz);
int iox_settimeofday(const struct timeval *tv, const struct timezone *tz);
int iox_clock_gettime(clockid_t clk_id, struct timespec *tp);
time_t iox_time(time_t *tloc);

/* ============================================================================
 * ENVIRONMENT
 * ============================================================================ */

char *iox_getenv(const char *name);
int iox_setenv(const char *name, const char *value, int overwrite);
int iox_unsetenv(const char *name);
int iox_clearenv(void);
char **iox_environ(void);

/* ============================================================================
 * NETWORK
 * ============================================================================ */

int iox_socket(int domain, int type, int protocol);
int iox_socketpair(int domain, int type, int protocol, int sv[2]);
int iox_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int iox_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int iox_listen(int sockfd, int backlog);
int iox_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int iox_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
ssize_t iox_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t iox_sendto(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t iox_sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t iox_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t iox_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr,
                     socklen_t *addrlen);
ssize_t iox_recvmsg(int sockfd, struct msghdr *msg, int flags);
int iox_shutdown(int sockfd, int how);
int iox_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int iox_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int iox_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int iox_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

/* ============================================================================
 * PIPES
 * ============================================================================ */

int iox_pipe(int pipefd[2]);
int iox_pipe2(int pipefd[2], int flags);
int iox_mkfifo(const char *pathname, mode_t mode);
int iox_mkfifoat(int dirfd, const char *pathname, mode_t mode);

/* ============================================================================
 * SELECT/POLL
 * ============================================================================ */

int iox_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
               struct timeval *timeout);
int iox_pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                const struct timespec *timeout, const sigset_t *sigmask);
int iox_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int iox_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p,
              const sigset_t *sigmask);

/* ============================================================================
 * TTY
 * ============================================================================ */

int iox_isatty(int fd);
int iox_ttyname_r(int fd, char *buf, size_t buflen);
int iox_tcgetattr(int fd, struct termios *termios_p);
int iox_tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
int iox_tcsendbreak(int fd, int duration);
int iox_tcdrain(int fd);
int iox_tcflush(int fd, int queue_selector);
int iox_tcflow(int fd, int action);

/* ============================================================================
 * MISC
 * ============================================================================ */

long iox__syscall(long number, ...);
void iox_perror(const char *s);
const char *iox_strerror(int errnum);

/* ============================================================================
 * IOX-SPECIFIC EXTENSIONS
 * ============================================================================ */

int iox_init(const iox_config_t *config);
void iox_cleanup(void);
const char *iox_version(void);
int iox_is_initialized(void);
iox_sys_info_t iox_get_sys_info(void);
iox_proc_info_t *iox_get_proc_info(pid_t pid);
void iox_free_proc_info(iox_proc_info_t *info);
iox_thread_info_t *iox_get_thread_info(pthread_t tid);
void iox_free_thread_info(iox_thread_info_t *info);
void iox_set_proc_callback(iox_proc_callback_t callback, void *userdata);
void iox_set_thread_callback(iox_thread_callback_t callback, void *userdata);
void iox_set_file_callback(iox_file_callback_t callback, void *userdata);
void iox_get_syscall_stats(iox_syscall_t syscall, iox_syscall_stat_t *stats);
void iox_get_memory_stats(iox_memory_stat_t *stats);
void iox_get_process_stats(iox_process_stat_t *stats);
void iox_debug_dump(FILE *stream);
void iox_set_tracing(int enabled);

#ifdef __cplusplus
}
#endif

#endif /* IOX_SYSCALLS_H */

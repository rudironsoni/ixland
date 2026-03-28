/* iOS Subsystem for Linux - Internal Header
 *
 * Unified internal header with all declarations
 * This is the single source of truth for internal APIs
 */

#ifndef IOX_INTERNAL_H
#define IOX_INTERNAL_H

#define _DARWIN_C_SOURCE

/* Standard headers */
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>

/* Include iox types - Makefile adds -I./include */
#ifndef IOX_TYPES_H
#include <iox/iox_types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#ifndef RLIMIT_NLIMITS
#define RLIMIT_NLIMITS 16
#endif

#define IOX_MAX_PROCESSES     1024
#define IOX_MAX_THREADS       1024
#define IOX_MAX_SESSIONS      64
#define IOX_MAX_PATH          1024
#define IOX_MAX_NAME          256
#define IOX_MIN_PID           1000
#define IOX_MAX_FD            256
#define IOX_NSIG              64
#define IOX_SIGQUEUE_MAX      1024

/* Process states */
#define IOX_PROC_RUNNING      0
#define IOX_PROC_STOPPED      1
#define IOX_PROC_ZOMBIE       2
#define IOX_PROC_DEAD         3

/* Task states (Linux-compatible) */
#define IOX_TASK_RUNNING        0x0000
#define IOX_TASK_INTERRUPTIBLE  0x0001
#define IOX_TASK_UNINTERRUPTIBLE 0x0002
#define IOX_TASK_STOPPED        0x0004
#define IOX_TASK_TRACED         0x0008
#define IOX_EXIT_ZOMBIE         0x0010
#define IOX_EXIT_DEAD           0x0020

/* VFS mount flags */
#define IOX_VFS_BIND        0x0001
#define IOX_VFS_RDONLY      0x0002
#define IOX_VFS_NOSUID      0x0004
#define IOX_VFS_NOEXEC      0x0008
#define IOX_VFS_RECURSIVE   0x0010

/* ============================================================================
 * TYPE DEFINITIONS
 * ============================================================================ */

typedef _Atomic pid_t atomic_pid_t;
typedef _Atomic uint64_t atomic_uint64_t;
typedef _Atomic uint32_t atomic_uint32_t;

/* Define sighandler_t if not present */
#ifndef __sighandler_t
typedef void (*__sighandler_t)(int);
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct __iox_process_s __iox_process_t;
typedef struct __iox_sigqueue_entry_s __iox_sigqueue_entry_t;
typedef struct __iox_sigqueue_s __iox_sigqueue_t;
typedef struct __iox_wait_entry_s __iox_wait_entry_t;
typedef struct iox_vfs_mount_s iox_vfs_mount_t;
typedef struct iox_process_context iox_context_t;

/* Additional process-related types (used in tables) */
typedef struct __iox_thread_s __iox_thread_t;
typedef struct __iox_session_s __iox_session_t;
typedef struct __iox_pgroup_s __iox_pgroup_t;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/* Signal queue entry */
struct __iox_sigqueue_entry_s {
    int sig;
    siginfo_t info;
    struct __iox_sigqueue_entry_s *next;
};

/* Signal queue */
struct __iox_sigqueue_s {
    __iox_sigqueue_entry_t *head;
    __iox_sigqueue_entry_t *tail;
    int count;
    pthread_mutex_t lock;
};

/* Wait entry for waitpid */
struct __iox_wait_entry_s {
    pid_t pid;
    int options;
    int *status;
    bool done;
    pthread_cond_t cond;
    struct __iox_wait_entry_s *next;
};

/* File descriptor table entry */
struct __iox_fd_entry_s {
    int fd;
    int real_fd;
    int flags;
    off_t offset;
    char path[IOX_MAX_PATH];
    bool used;
    FILE *stream;
};

/* Process group structure */
struct __iox_pgroup_s {
    pid_t pgid;
    pid_t sid;
    atomic_int refcount;
    pthread_mutex_t lock;
    struct __iox_pgroup_s *next;
    struct __iox_pgroup_s *prev;
};

/* Session structure */
struct __iox_session_s {
    pid_t sid;
    pid_t leader;
    atomic_int refcount;
    pthread_mutex_t lock;
    struct __iox_session_s *next;
    struct __iox_session_s *prev;
};

/* Thread structure (placeholder for future expansion) */
struct __iox_thread_s {
    pid_t tid;
    pthread_t thread;
    __iox_process_t *process;
    struct __iox_thread_s *next;
};

/* Main process structure */
struct __iox_process_s {
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    pid_t sid;
    atomic_int state;
    int exit_status;
    pthread_t thread;
    pthread_mutex_t thread_lock;
    char name[IOX_MAX_NAME];
    char cwd[IOX_MAX_PATH];

    /* Signal handling */
    struct sigaction sig_actions[IOX_NSIG];
    sigset_t sig_mask;
    sigset_t sig_pending;
    __iox_sigqueue_t sigqueue;
    pthread_mutex_t sig_lock;

    /* Waiters */
    __iox_wait_entry_t *waiters;
    bool waited_on;
    pthread_mutex_t wait_lock;
    pthread_cond_t wait_cond;

    /* File descriptors */
    struct __iox_fd_entry_s fd_table[IOX_MAX_FD];
    int fd_count;
    pthread_mutex_t fd_lock;

    /* Environment and arguments */
    char **argv;
    char **envp;

    /* Resource limits */
    struct rlimit rlimits[RLIMIT_NLIMITS];

    /* Links */
    __iox_process_t *parent;
    __iox_process_t *children;
    __iox_process_t *next_sibling;
    struct __iox_process_s *hash_next;
    struct __iox_process_s *sibling;

    /* Reference counting */
    atomic_int refcount;
    atomic_int children_count;
    atomic_int children_exited;

    /* Timing */
    struct timespec start_time;

    /* Execution info */
    char exe[IOX_MAX_PATH];
    atomic_bool exited;
    atomic_bool signaled;
    atomic_int termsig;
    atomic_int exit_code;
    int status;
};

/* VFS mount structure */
struct iox_vfs_mount_s {
    bool active;
    unsigned long flags;
    char mountpoint[IOX_MAX_PATH];
    char target[IOX_MAX_PATH];
    struct iox_vfs_mount_s *next;
};

/* Process context for thread-based simulation */
struct iox_process_context {
    /* Identity */
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    pid_t sid;
    char name[IOX_MAX_NAME];

    /* State */
    atomic_int state;
    int exit_status;

    /* Thread */
    pthread_t thread;
    pthread_mutex_t lock;

    /* Working directory */
    char cwd[IOX_MAX_PATH];
    char root[IOX_MAX_PATH];

    /* File descriptor table */
    struct {
        int real_fd;
        int flags;
        off_t offset;
        char path[IOX_MAX_PATH];
        bool used;
    } fd_table[IOX_MAX_FD];
    pthread_mutex_t fd_lock;

    /* Environment */
    char **env;
    int env_count;
    pthread_mutex_t env_lock;

    /* Signal state */
    struct sigaction sig_actions[IOX_NSIG];
    sigset_t sig_mask;
    sigset_t sig_pending;
    pthread_mutex_t sig_lock;

    /* Resource limits */
    struct rlimit rlimits[RLIMIT_NLIMITS];

    /* Parent/child */
    struct iox_process_context *parent;
    struct iox_process_context *children;
    struct iox_process_context *next_sibling;

    /* Wait queue */
    pthread_cond_t wait_cond;
    bool waited_on;

    /* Reference counting */
    atomic_int refs;
};

/* ============================================================================
 * GLOBAL VARIABLES (extern declarations)
 * ============================================================================ */

extern pthread_mutex_t __iox_process_table_lock;
extern __iox_process_t *__iox_process_table[IOX_MAX_PROCESSES];
extern pid_t __iox_next_pid;

extern __thread __iox_process_t *__iox_current_process;
extern __thread iox_context_t *__iox_current_ctx;

extern atomic_bool __iox_initialized;
extern pthread_once_t __iox_init_once;

extern atomic_uint64_t __iox_process_created;
extern atomic_uint64_t __iox_process_exited;
extern atomic_uint32_t __iox_active_processes;

/* VFS globals */
extern pthread_mutex_t vfs_lock;
extern iox_vfs_mount_t vfs_mount_table[64];
extern int vfs_mount_count;
extern char ios_sandbox_root[IOX_MAX_PATH];
extern char ios_home_dir[IOX_MAX_PATH];
extern char ios_tmp_dir[IOX_MAX_PATH];

/* ============================================================================
 * FUNCTION DECLARATIONS - Process Management
 * ============================================================================ */

/* Initialization */
int __iox_init(void);
void __iox_deinit(void);
void __iox_process_init_once(void);

/* PID management */
pid_t __iox_alloc_pid(void);
void __iox_free_pid(pid_t pid);

/* Process operations */
__iox_process_t *__iox_process_create(const char *name, pid_t ppid);
void __iox_process_destroy(__iox_process_t *proc);
__iox_process_t *__iox_get_process(pid_t pid);
void __iox_process_exit(__iox_process_t *proc, int status);
int __iox_process_wait(__iox_process_t *proc, int *status, int options);
__iox_process_t *__iox_get_current_process(void);
void __iox_set_current_process(__iox_process_t *proc);

/* Internal implementations */
pid_t __iox_fork_impl(void);
int __iox_execve_impl(const char *pathname, char *const argv[], char *const envp[]);
void __iox_exit_impl(int status);
pid_t __iox_getpid_impl(void);
pid_t __iox_getppid_impl(void);
pid_t __iox_waitpid_impl(pid_t pid, int *stat_loc, int options);

/* Context system */
int iox_context_init(void);
void iox_context_deinit(void);
iox_context_t *iox_ctx_alloc(pid_t pid, iox_context_t *parent);
void iox_ctx_free(iox_context_t *ctx);
iox_context_t *iox_ctx_get(pid_t pid);
void iox_ctx_put(iox_context_t *ctx);

pid_t iox_fork_full(void);
int iox_execve_full(const char *pathname, char *const argv[], char *const envp[]);
pid_t iox_waitpid_full(pid_t pid, int *wstatus, int options);
void iox_exit_full(int status);

int iox_open_in_ctx(const char *pathname, int flags, mode_t mode);
ssize_t iox_read_in_ctx(int fd, void *buf, size_t count);
ssize_t iox_write_in_ctx(int fd, const void *buf, size_t count);
int iox_close_in_ctx(int fd);
off_t iox_lseek_in_ctx(int fd, off_t offset, int whence);

/* ============================================================================
 * FUNCTION DECLARATIONS - Signal Handling
 * ============================================================================ */

int __iox_sigqueue_enqueue(__iox_sigqueue_t *queue, int sig, siginfo_t *info);
int __iox_sigqueue_dequeue(__iox_sigqueue_t *queue, int *sig, siginfo_t *info);
void __iox_sigqueue_flush(__iox_sigqueue_t *queue);
int __iox_kill_impl(pid_t pid, int sig);
__sighandler_t __iox_signal_impl(int signum, __sighandler_t handler);
int __iox_sigaction_impl(int signum, const struct sigaction *act, struct sigaction *oldact);

/* ============================================================================
 * FUNCTION DECLARATIONS - VFS
 * ============================================================================ */

int iox_vfs_init(void);
void iox_vfs_deinit(void);
int iox_vfs_translate(const char *vpath, char *ios_path, size_t ios_path_len);
int iox_vfs_reverse_translate(const char *ios_path, char *vpath, size_t vpath_len);
int iox_vfs_mount(const char *source, const char *target, unsigned long flags);
int iox_vfs_umount(const char *target);
int iox_vfs_umount2(const char *target, int flags);
bool iox_vfs_in_sandbox(const char *path);
int iox_vfs_chroot(const char *path);

int iox_vfs_open(const char *pathname, int flags, mode_t mode);
int iox_vfs_stat(const char *pathname, struct stat *statbuf);
int iox_vfs_lstat(const char *pathname, struct stat *statbuf);
int iox_vfs_access(const char *pathname, int mode);
int iox_vfs_mkdir(const char *pathname, mode_t mode);
int iox_vfs_rmdir(const char *pathname);
int iox_vfs_unlink(const char *pathname);
int iox_vfs_rename(const char *oldpath, const char *newpath);
int iox_vfs_symlink(const char *target, const char *linkpath);
ssize_t iox_vfs_readlink(const char *pathname, char *buf, size_t bufsiz);
int iox_vfs_chmod(const char *pathname, mode_t mode);
int iox_vfs_chown(const char *pathname, uid_t owner, gid_t group);
int iox_vfs_link(const char *oldpath, const char *newpath);

const char *iox_vfs_get_sandbox_root(void);
const char *iox_vfs_get_home(void);
const char *iox_vfs_get_tmp(void);

/* ============================================================================
 * FUNCTION DECLARATIONS - Path Utilities
 * ============================================================================ */

int __iox_path_resolve(const char *path, char *resolved, size_t resolved_len);
void __iox_path_normalize(char *path);
void __iox_path_join(const char *base, const char *rel, char *result, size_t result_len);
bool __iox_path_in_sandbox(const char *path);

/* Path classification for hybrid iOS/Linux architecture */
typedef enum {
    IOX_PATH_INVALID = 0,
    IOX_PATH_VIRTUAL_LINUX,    /* /home, /tmp, /etc - needs VFS translation */
    IOX_PATH_OWN_SANDBOX,      /* Within app container - direct access */
    IOX_PATH_EXTERNAL          /* Security-scoped external paths */
} iox_path_type_t;

bool __iox_path_is_virtual_linux(const char *path);
bool __iox_path_is_own_sandbox(const char *path);
bool __iox_path_is_external(const char *path);
iox_path_type_t __iox_path_classify(const char *path);
int __iox_path_virtual_to_ios(const char *vpath, char *ios_path, size_t ios_path_len);
bool __iox_path_is_direct(const char *path);

/* ============================================================================
 * FUNCTION DECLARATIONS - File Operations (Internal)
 * ============================================================================ */

int __iox_open_impl(const char *pathname, int flags, mode_t mode);
int __iox_openat_impl(int dirfd, const char *pathname, int flags, mode_t mode);
ssize_t __iox_read_impl(int fd, void *buf, size_t count);
ssize_t __iox_write_impl(int fd, const void *buf, size_t count);
int __iox_close_impl(int fd);
off_t __iox_lseek_impl(int fd, off_t offset, int whence);
ssize_t __iox_pread_impl(int fd, void *buf, size_t count, off_t offset);
ssize_t __iox_pwrite_impl(int fd, const void *buf, size_t count, off_t offset);
int __iox_dup_impl(int oldfd);
int __iox_dup2_impl(int oldfd, int newfd);
int __iox_dup3_impl(int oldfd, int newfd, int flags);
int __iox_fcntl_impl(int fd, int cmd, ...);
int __iox_ioctl_impl(int fd, unsigned long request, ...);
int __iox_access_impl(const char *pathname, int mode);
int __iox_faccessat_impl(int dirfd, const char *pathname, int mode, int flags);
int __iox_creat_impl(const char *pathname, mode_t mode);

/* File descriptor table initialization */
void __iox_file_init_impl(void);

/* ============================================================================
 * FUNCTION DECLARATIONS - Filesystem (Internal)
 * ============================================================================ */

int __iox_stat_impl(const char *pathname, struct stat *statbuf);
int __iox_fstat_impl(int fd, struct stat *statbuf);
int __iox_lstat_impl(const char *pathname, struct stat *statbuf);
int __iox_mkdir_impl(const char *pathname, mode_t mode);
int __iox_rmdir_impl(const char *pathname);
int __iox_unlink_impl(const char *pathname);
int __iox_link_impl(const char *oldpath, const char *newpath);
int __iox_symlink_impl(const char *target, const char *linkpath);
ssize_t __iox_readlink_impl(const char *pathname, char *buf, size_t bufsiz);
int __iox_chmod_impl(const char *pathname, mode_t mode);
int __iox_fchmod_impl(int fd, mode_t mode);
int __iox_chown_impl(const char *pathname, uid_t owner, gid_t group);
int __iox_fchown_impl(int fd, uid_t owner, gid_t group);
int __iox_lchown_impl(const char *pathname, uid_t owner, gid_t group);
int __iox_chroot_impl(const char *path);

/* ============================================================================
 * FUNCTION DECLARATIONS - Signal (Internal)
 * ============================================================================ */

int __iox_kill_impl(pid_t pid, int sig);
int __iox_killpg_impl(int pgrp, int sig);
int __iox_raise_impl(int sig);
unsigned int __iox_alarm_impl(unsigned int seconds);
int __iox_setitimer_impl(int which, const struct itimerval *new_value, struct itimerval *old_value);
int __iox_getitimer_impl(int which, struct itimerval *curr_value);
int __iox_pause_impl(void);
int __iox_sigprocmask_impl(int how, const sigset_t *set, sigset_t *oldset);
int __iox_sigpending_impl(sigset_t *set);
int __iox_sigsuspend_impl(const sigset_t *mask);
int __iox_sigemptyset_impl(sigset_t *set);
int __iox_sigfillset_impl(sigset_t *set);
int __iox_sigaddset_impl(sigset_t *set, int signum);
int __iox_sigdelset_impl(sigset_t *set, int signum);
int __iox_sigismember_impl(const sigset_t *set, int signum);

/* ============================================================================
 * FUNCTION DECLARATIONS - Time (Internal)
 * ============================================================================ */

unsigned int __iox_sleep_impl(unsigned int seconds);
int __iox_usleep_impl(useconds_t usec);
int __iox_nanosleep_impl(const struct timespec *req, struct timespec *rem);
int __iox_gettimeofday_impl(struct timeval *tv, struct timezone *tz);
int __iox_settimeofday_impl(const struct timeval *tv, const struct timezone *tz);
int __iox_clock_gettime_impl(clockid_t clk_id, struct timespec *tp);
time_t __iox_time_impl(time_t *tloc);

/* ============================================================================
 * FUNCTION DECLARATIONS - Environment (Internal)
 * ============================================================================ */

char *__iox_getenv_impl(const char *name);
int __iox_setenv_impl(const char *name, const char *value, int overwrite);
int __iox_unsetenv_impl(const char *name);
int __iox_clearenv_impl(void);

/* ============================================================================
 * FUNCTION DECLARATIONS - Memory (Internal)
 * ============================================================================ */

void *__iox_mmap_impl(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int __iox_munmap_impl(void *addr, size_t length);
int __iox_mprotect_impl(void *addr, size_t len, int prot);
int __iox_msync_impl(void *addr, size_t length, int flags);
int __iox_mlock_impl(const void *addr, size_t len);
int __iox_munlock_impl(const void *addr, size_t len);

/* ============================================================================
 * ORIGINAL LIBC FUNCTION POINTERS (for internal use to avoid recursion)
 * ============================================================================ */

void init_orig_funcs(void);

extern int (*libc_open)(const char *, int, mode_t);
extern int (*libc_close)(int);
extern ssize_t (*libc_read)(int, void *, size_t);
extern ssize_t (*libc_write)(int, const void *, size_t);
extern off_t (*libc_lseek)(int, off_t, int);

/* ============================================================================
 * FUNCTION DECLARATIONS - Pipes (Internal)
 * ============================================================================ */

int __iox_pipe_impl(int pipefd[2]);
int __iox_pipe2_impl(int pipefd[2], int flags);
int __iox_mkfifo_impl(const char *pathname, mode_t mode);
int __iox_mkfifoat_impl(int dirfd, const char *pathname, mode_t mode);

/* ============================================================================
 * FUNCTION DECLARATIONS - TTY (Internal)
 * ============================================================================ */

int __iox_isatty_impl(int fd);
int __iox_ttyname_r_impl(int fd, char *buf, size_t buflen);
int __iox_tcgetattr_impl(int fd, struct termios *termios_p);
int __iox_tcsetattr_impl(int fd, int optional_actions, const struct termios *termios_p);
int __iox_tcsendbreak_impl(int fd, int duration);
int __iox_tcdrain_impl(int fd);
int __iox_tcflush_impl(int fd, int queue_selector);
int __iox_tcflow_impl(int fd, int action);

/* ============================================================================
 * FUNCTION DECLARATIONS - Directory (Internal)
 * ============================================================================ */

int __iox_chdir_impl(const char *path);
int __iox_fchdir_impl(int fd);
char *__iox_getcwd_impl(char *buf, size_t size);

/* ============================================================================
 * FUNCTION DECLARATIONS - Logging/Debugging
 * ============================================================================ */

typedef enum {
    IOX_LOG_NONE = 0,
    IOX_LOG_ERROR,
    IOX_LOG_WARN,
    IOX_LOG_INFO,
    IOX_LOG_DEBUG,
    IOX_LOG_TRACE
} __iox_log_level_t;

void __iox_set_log_level(__iox_log_level_t level);
void __iox_log(__iox_log_level_t level, const char *fmt, ...);

#define IOX_ERROR(...) __iox_log(IOX_LOG_ERROR, __VA_ARGS__)
#define IOX_WARN(...)  __iox_log(IOX_LOG_WARN, __VA_ARGS__)
#define IOX_INFO(...)  __iox_log(IOX_LOG_INFO, __VA_ARGS__)
#define IOX_DEBUG(...) __iox_log(IOX_LOG_DEBUG, __VA_ARGS__)
#define IOX_TRACE(...) __iox_log(IOX_LOG_TRACE, __VA_ARGS__)

/* ============================================================================
 * FUNCTION DECLARATIONS - Public API (needed for interposition)
 * ============================================================================ */

/* Process management */
pid_t iox_fork(void);
int iox_vfork(void);
int iox_execve(const char *pathname, char *const argv[], char *const envp[]);
int iox_execv(const char *pathname, char *const argv[]);
void iox_exit(int status);
void iox__exit(int status);
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

/* File operations */
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

/* Filesystem */
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

/* Signal handling */
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

/* Memory management */
void *iox_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int iox_munmap(void *addr, size_t length);
int iox_mprotect(void *addr, size_t len, int prot);
int iox_msync(void *addr, size_t length, int flags);
int iox_mlock(const void *addr, size_t len);
int iox_munlock(const void *addr, size_t len);

/* Time */
unsigned int iox_sleep(unsigned int seconds);
int iox_usleep(useconds_t usec);
int iox_nanosleep(const struct timespec *req, struct timespec *rem);
int iox_gettimeofday(struct timeval *tv, struct timezone *tz);
int iox_settimeofday(const struct timeval *tv, const struct timezone *tz);
int iox_clock_gettime(clockid_t clk_id, struct timespec *tp);
time_t iox_time(time_t *tloc);

/* Environment */
char *iox_getenv(const char *name);
int iox_setenv(const char *name, const char *value, int overwrite);
int iox_unsetenv(const char *name);
int iox_clearenv(void);
char **iox_environ(void);

/* Directory */
int iox_chdir(const char *path);
int iox_fchdir(int fd);
char *iox_getcwd(char *buf, size_t size);

/* Pipes */
int iox_pipe(int pipefd[2]);
int iox_pipe2(int pipefd[2], int flags);
int iox_mkfifo(const char *pathname, mode_t mode);
int iox_mkfifoat(int dirfd, const char *pathname, mode_t mode);

/* TTY */
int iox_isatty(int fd);
int iox_ttyname_r(int fd, char *buf, size_t buflen);
int iox_tcgetattr(int fd, struct termios *termios_p);
int iox_tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
int iox_tcsendbreak(int fd, int duration);
int iox_tcdrain(int fd);
int iox_tcflush(int fd, int queue_selector);
int iox_tcflow(int fd, int action);

/* Identity - User/Group */
uid_t iox_getuid(void);
uid_t iox_geteuid(void);
gid_t iox_getgid(void);
gid_t iox_getegid(void);
int iox_setuid(uid_t uid);
int iox_setgid(gid_t gid);

/* Initialization */
int iox_init(const iox_config_t *config);
int iox_is_initialized(void);
const char *iox_version(void);

#ifdef __cplusplus
}
#endif

#endif /* IOX_INTERNAL_H */

/* iOS Subsystem for Linux - Internal Header
 *
 * Unified internal header with all declarations
 * This is the single source of truth for internal APIs
 */

#ifndef IXLAND_INTERNAL_H
#define IXLAND_INTERNAL_H

#define _DARWIN_C_SOURCE

/* Standard headers */
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/* Include ixland types - Makefile adds -I./include */
#ifndef IXLAND_TYPES_H
#include <ixland/ixland_types.h>
#endif

/* Include poll/epoll headers from ixland-libc boundary
 * These are public headers installed to include/linux/
 * Include paths are set by CMake: ixland-libc/include */
#include <linux/epoll.h>
#include <linux/poll.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#ifndef RLIMIT_NLIMITS
#define RLIMIT_NLIMITS 16
#endif

#define IXLAND_MAX_PROCESSES 1024
#define IXLAND_MAX_THREADS 1024
#define IXLAND_MAX_SESSIONS 64
#define IXLAND_MAX_PATH 1024
#define IXLAND_MAX_NAME 256
#define IXLAND_MIN_PID 1000
#define IXLAND_MAX_FD 256
#define IXLAND_NSIG 64
#define IXLAND_SIGQUEUE_MAX 1024

/* Process states */
#define IXLAND_PROC_RUNNING 0
#define IXLAND_PROC_STOPPED 1
#define IXLAND_PROC_ZOMBIE 2
#define IXLAND_PROC_DEAD 3

/* Task states (Linux-compatible) */
#define IXLAND_TASK_RUNNING 0x0000
#define IXLAND_TASK_INTERRUPTIBLE 0x0001
#define IXLAND_TASK_UNINTERRUPTIBLE 0x0002
#define IXLAND_TASK_STOPPED 0x0004
#define IXLAND_TASK_TRACED 0x0008
#define IXLAND_EXIT_ZOMBIE 0x0010
#define IXLAND_EXIT_DEAD 0x0020

/* VFS mount flags */
#define IXLAND_VFS_BIND 0x0001
#define IXLAND_VFS_RDONLY 0x0002
#define IXLAND_VFS_NOSUID 0x0004
#define IXLAND_VFS_NOEXEC 0x0008
#define IXLAND_VFS_RECURSIVE 0x0010

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

typedef struct __ixland_process_s __ixland_process_t;
typedef struct __ixland_sigqueue_entry_s __ixland_sigqueue_entry_t;
typedef struct __ixland_sigqueue_s __ixland_sigqueue_t;
typedef struct __ixland_wait_entry_s __ixland_wait_entry_t;
typedef struct ixland_vfs_mount_s ixland_vfs_mount_t;
typedef struct ixland_process_context ixland_context_t;

/* Additional process-related types (used in tables) */
typedef struct __ixland_thread_s __ixland_thread_t;
typedef struct __ixland_session_s __ixland_session_t;
typedef struct __ixland_pgroup_s __ixland_pgroup_t;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/* Signal queue entry */
struct __ixland_sigqueue_entry_s {
    int sig;
    siginfo_t info;
    struct __ixland_sigqueue_entry_s *next;
};

/* Signal queue */
struct __ixland_sigqueue_s {
    __ixland_sigqueue_entry_t *head;
    __ixland_sigqueue_entry_t *tail;
    int count;
    pthread_mutex_t lock;
};

/* Wait entry for waitpid */
struct __ixland_wait_entry_s {
    pid_t pid;
    int options;
    int *status;
    bool done;
    pthread_cond_t cond;
    struct __ixland_wait_entry_s *next;
};

/* File descriptor table entry */
struct __ixland_fd_entry_s {
    int fd;
    int real_fd;
    int flags;
    off_t offset;
    char path[IXLAND_MAX_PATH];
    bool used;
    FILE *stream;
};

/* Process group structure */
struct __ixland_pgroup_s {
    pid_t pgid;
    pid_t sid;
    atomic_int refcount;
    pthread_mutex_t lock;
    struct __ixland_pgroup_s *next;
    struct __ixland_pgroup_s *prev;
};

/* Session structure */
struct __ixland_session_s {
    pid_t sid;
    pid_t leader;
    atomic_int refcount;
    pthread_mutex_t lock;
    struct __ixland_session_s *next;
    struct __ixland_session_s *prev;
};

/* Thread structure (placeholder for future expansion) */
struct __ixland_thread_s {
    pid_t tid;
    pthread_t thread;
    __ixland_process_t *process;
    struct __ixland_thread_s *next;
};

/* Main process structure */
struct __ixland_process_s {
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    pid_t sid;
    atomic_int state;
    int exit_status;
    pthread_t thread;
    pthread_mutex_t thread_lock;
    char name[IXLAND_MAX_NAME];
    char cwd[IXLAND_MAX_PATH];

    /* Signal handling */
    struct sigaction sig_actions[IXLAND_NSIG];
    sigset_t sig_mask;
    sigset_t sig_pending;
    __ixland_sigqueue_t sigqueue;
    pthread_mutex_t sig_lock;

    /* Waiters */
    __ixland_wait_entry_t *waiters;
    bool waited_on;
    pthread_mutex_t wait_lock;
    pthread_cond_t wait_cond;

    /* File descriptors */
    struct __ixland_fd_entry_s fd_table[IXLAND_MAX_FD];
    int fd_count;
    pthread_mutex_t fd_lock;

    /* Environment and arguments */
    char **argv;
    char **envp;

    /* Resource limits */
    struct rlimit rlimits[RLIMIT_NLIMITS];

    /* Links */
    __ixland_process_t *parent;
    __ixland_process_t *children;
    __ixland_process_t *next_sibling;
    struct __ixland_process_s *hash_next;
    struct __ixland_process_s *sibling;

    /* Reference counting */
    atomic_int refcount;
    atomic_int children_count;
    atomic_int children_exited;

    /* Timing */
    struct timespec start_time;

    /* Execution info */
    char exe[IXLAND_MAX_PATH];
    atomic_bool exited;
    atomic_bool signaled;
    atomic_int termsig;
    atomic_int exit_code;
    int status;
};

/* VFS mount structure */
struct ixland_vfs_mount_s {
    bool active;
    unsigned long flags;
    char mountpoint[IXLAND_MAX_PATH];
    char target[IXLAND_MAX_PATH];
    struct ixland_vfs_mount_s *next;
};

/* Process context for thread-based simulation */
struct ixland_process_context {
    /* Identity */
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    pid_t sid;
    char name[IXLAND_MAX_NAME];

    /* State */
    atomic_int state;
    int exit_status;

    /* Thread */
    pthread_t thread;
    pthread_mutex_t lock;

    /* Working directory */
    char cwd[IXLAND_MAX_PATH];
    char root[IXLAND_MAX_PATH];

    /* File descriptor table */
    struct {
        int real_fd;
        int flags;
        off_t offset;
        char path[IXLAND_MAX_PATH];
        bool used;
    } fd_table[IXLAND_MAX_FD];
    pthread_mutex_t fd_lock;

    /* Environment */
    char **env;
    int env_count;
    pthread_mutex_t env_lock;

    /* Signal state */
    struct sigaction sig_actions[IXLAND_NSIG];
    sigset_t sig_mask;
    sigset_t sig_pending;
    pthread_mutex_t sig_lock;

    /* Resource limits */
    struct rlimit rlimits[RLIMIT_NLIMITS];

    /* Parent/child */
    struct ixland_process_context *parent;
    struct ixland_process_context *children;
    struct ixland_process_context *next_sibling;

    /* Wait queue */
    pthread_cond_t wait_cond;
    bool waited_on;

    /* Reference counting */
    atomic_int refs;
};

/* ============================================================================
 * GLOBAL VARIABLES (extern declarations)
 * ============================================================================ */

extern pthread_mutex_t __ixland_process_table_lock;
extern __ixland_process_t *__ixland_process_table[IXLAND_MAX_PROCESSES];
extern pid_t __ixland_next_pid;

extern __thread __ixland_process_t *__ixland_current_process;
extern __thread ixland_context_t *__ixland_current_ctx;

extern atomic_bool __ixland_initialized;
extern pthread_once_t __ixland_init_once;

extern atomic_uint64_t __ixland_process_created;
extern atomic_uint64_t __ixland_process_exited;
extern atomic_uint32_t __ixland_active_processes;

/* VFS globals */
extern pthread_mutex_t vfs_lock;
extern ixland_vfs_mount_t vfs_mount_table[64];
extern int vfs_mount_count;
extern char ios_sandbox_root[IXLAND_MAX_PATH];
extern char ios_home_dir[IXLAND_MAX_PATH];
extern char ios_tmp_dir[IXLAND_MAX_PATH];

/* ============================================================================
 * FUNCTION DECLARATIONS - Process Management
 * ============================================================================ */

/* Initialization */
int __ixland_init(void);
void __ixland_deinit(void);
void __ixland_process_init_once(void);

/* PID management */
pid_t __ixland_alloc_pid(void);
void __ixland_free_pid(pid_t pid);

/* Process operations */
__ixland_process_t *__ixland_process_create(const char *name, pid_t ppid);
void __ixland_process_destroy(__ixland_process_t *proc);
__ixland_process_t *__ixland_get_process(pid_t pid);
void __ixland_process_exit(__ixland_process_t *proc, int status);
int __ixland_process_wait(__ixland_process_t *proc, int *status, int options);
__ixland_process_t *__ixland_get_current_process(void);
void __ixland_set_current_process(__ixland_process_t *proc);

/* Internal implementations */
pid_t __ixland_fork_impl(void);
int __ixland_execve_impl(const char *pathname, char *const argv[], char *const envp[]);
void __ixland_exit_impl(int status);
pid_t __ixland_getpid_impl(void);
pid_t __ixland_getppid_impl(void);
pid_t __ixland_waitpid_impl(pid_t pid, int *stat_loc, int options);

/* Context system */
int ixland_context_init(void);
void ixland_context_deinit(void);
ixland_context_t *ixland_ctx_alloc(pid_t pid, ixland_context_t *parent);
void ixland_ctx_free(ixland_context_t *ctx);
ixland_context_t *ixland_ctx_get(pid_t pid);
void ixland_ctx_put(ixland_context_t *ctx);

pid_t ixland_fork_full(void);
int ixland_execve_full(const char *pathname, char *const argv[], char *const envp[]);
pid_t ixland_waitpid_full(pid_t pid, int *wstatus, int options);
void ixland_exit_full(int status);

int ixland_open_in_ctx(const char *pathname, int flags, mode_t mode);
ssize_t ixland_read_in_ctx(int fd, void *buf, size_t count);
ssize_t ixland_write_in_ctx(int fd, const void *buf, size_t count);
int ixland_close_in_ctx(int fd);
off_t ixland_lseek_in_ctx(int fd, off_t offset, int whence);

/* ============================================================================
 * FUNCTION DECLARATIONS - Signal Handling
 * ============================================================================ */

int __ixland_sigqueue_enqueue(__ixland_sigqueue_t *queue, int sig, siginfo_t *info);
int __ixland_sigqueue_dequeue(__ixland_sigqueue_t *queue, int *sig, siginfo_t *info);
void __ixland_sigqueue_flush(__ixland_sigqueue_t *queue);
int __ixland_kill_impl(pid_t pid, int sig);
__sighandler_t __ixland_signal_impl(int signum, __sighandler_t handler);
int __ixland_sigaction_impl(int signum, const struct sigaction *act, struct sigaction *oldact);

/* ============================================================================
 * FUNCTION DECLARATIONS - VFS
 * ============================================================================ */

int ixland_vfs_init(void);
void ixland_vfs_deinit(void);
int ixland_vfs_translate(const char *vpath, char *ios_path, size_t ios_path_len);
int ixland_vfs_reverse_translate(const char *ios_path, char *vpath, size_t vpath_len);
int ixland_vfs_mount(const char *source, const char *target, unsigned long flags);
int ixland_vfs_umount(const char *target);
int ixland_vfs_umount2(const char *target, int flags);
bool ixland_vfs_in_sandbox(const char *path);
int ixland_vfs_chroot(const char *path);

int ixland_vfs_open(const char *pathname, int flags, mode_t mode);
int ixland_vfs_stat(const char *pathname, struct stat *statbuf);
int ixland_vfs_lstat(const char *pathname, struct stat *statbuf);
int ixland_vfs_access(const char *pathname, int mode);
int ixland_vfs_mkdir(const char *pathname, mode_t mode);
int ixland_vfs_rmdir(const char *pathname);
int ixland_vfs_unlink(const char *pathname);
int ixland_vfs_rename(const char *oldpath, const char *newpath);
int ixland_vfs_symlink(const char *target, const char *linkpath);
ssize_t ixland_vfs_readlink(const char *pathname, char *buf, size_t bufsiz);
int ixland_vfs_chmod(const char *pathname, mode_t mode);
int ixland_vfs_chown(const char *pathname, uid_t owner, gid_t group);
int ixland_vfs_link(const char *oldpath, const char *newpath);

const char *ixland_vfs_get_sandbox_root(void);
const char *ixland_vfs_get_home(void);
const char *ixland_vfs_get_tmp(void);

/* ============================================================================
 * FUNCTION DECLARATIONS - Path Utilities
 * ============================================================================ */

int __ixland_path_resolve(const char *path, char *resolved, size_t resolved_len);
void __ixland_path_normalize(char *path);
void __ixland_path_join(const char *base, const char *rel, char *result, size_t result_len);
bool __ixland_path_in_sandbox(const char *path);

/* Path classification for hybrid iOS/Linux architecture */
typedef enum {
    IXLAND_PATH_INVALID = 0,
    IXLAND_PATH_VIRTUAL_LINUX, /* /home, /tmp, /etc - needs VFS translation */
    IXLAND_PATH_OWN_SANDBOX,   /* Within app container - direct access */
    IXLAND_PATH_EXTERNAL       /* Security-scoped external paths */
} ixland_path_type_t;

bool __ixland_path_is_virtual_linux(const char *path);
bool __ixland_path_is_own_sandbox(const char *path);
bool __ixland_path_is_external(const char *path);
ixland_path_type_t __ixland_path_classify(const char *path);
int __ixland_path_virtual_to_ios(const char *vpath, char *ios_path, size_t ios_path_len);
bool __ixland_path_is_direct(const char *path);

/* ============================================================================
 * FUNCTION DECLARATIONS - File Operations (Internal)
 * ============================================================================ */

int __ixland_open_impl(const char *pathname, int flags, mode_t mode);
int __ixland_openat_impl(int dirfd, const char *pathname, int flags, mode_t mode);
ssize_t __ixland_read_impl(int fd, void *buf, size_t count);
ssize_t __ixland_write_impl(int fd, const void *buf, size_t count);
int __ixland_close_impl(int fd);
off_t __ixland_lseek_impl(int fd, off_t offset, int whence);
ssize_t __ixland_pread_impl(int fd, void *buf, size_t count, off_t offset);
ssize_t __ixland_pwrite_impl(int fd, const void *buf, size_t count, off_t offset);
int __ixland_dup_impl(int oldfd);
int __ixland_dup2_impl(int oldfd, int newfd);
int __ixland_dup3_impl(int oldfd, int newfd, int flags);
int __ixland_fcntl_impl(int fd, int cmd, ...);
int __ixland_ioctl_impl(int fd, unsigned long request, ...);
int __ixland_access_impl(const char *pathname, int mode);
int __ixland_faccessat_impl(int dirfd, const char *pathname, int mode, int flags);
int __ixland_creat_impl(const char *pathname, mode_t mode);

/* File descriptor table initialization */
void __ixland_file_init_impl(void);

/* ============================================================================
 * FUNCTION DECLARATIONS - Filesystem (Internal)
 * ============================================================================ */

int __ixland_stat_impl(const char *pathname, struct stat *statbuf);
int __ixland_fstat_impl(int fd, struct stat *statbuf);
int __ixland_lstat_impl(const char *pathname, struct stat *statbuf);
int __ixland_mkdir_impl(const char *pathname, mode_t mode);
int __ixland_rmdir_impl(const char *pathname);
int __ixland_unlink_impl(const char *pathname);
int __ixland_link_impl(const char *oldpath, const char *newpath);
int __ixland_symlink_impl(const char *target, const char *linkpath);
ssize_t __ixland_readlink_impl(const char *pathname, char *buf, size_t bufsiz);
int __ixland_chmod_impl(const char *pathname, mode_t mode);
int __ixland_fchmod_impl(int fd, mode_t mode);
int __ixland_chown_impl(const char *pathname, uid_t owner, gid_t group);
int __ixland_fchown_impl(int fd, uid_t owner, gid_t group);
int __ixland_lchown_impl(const char *pathname, uid_t owner, gid_t group);
int __ixland_chroot_impl(const char *path);

/* ============================================================================
 * FUNCTION DECLARATIONS - Signal (Internal)
 * ============================================================================ */

int __ixland_kill_impl(pid_t pid, int sig);
int __ixland_killpg_impl(int pgrp, int sig);
int __ixland_raise_impl(int sig);
unsigned int __ixland_alarm_impl(unsigned int seconds);
int __ixland_setitimer_impl(int which, const struct itimerval *new_value,
                            struct itimerval *old_value);
int __ixland_getitimer_impl(int which, struct itimerval *curr_value);
int __ixland_pause_impl(void);
int __ixland_sigprocmask_impl(int how, const sigset_t *set, sigset_t *oldset);
int __ixland_sigpending_impl(sigset_t *set);
int __ixland_sigsuspend_impl(const sigset_t *mask);
int __ixland_sigemptyset_impl(sigset_t *set);
int __ixland_sigfillset_impl(sigset_t *set);
int __ixland_sigaddset_impl(sigset_t *set, int signum);
int __ixland_sigdelset_impl(sigset_t *set, int signum);
int __ixland_sigismember_impl(const sigset_t *set, int signum);

/* ============================================================================
 * FUNCTION DECLARATIONS - Time (Internal)
 * ============================================================================ */

unsigned int __ixland_sleep_impl(unsigned int seconds);
int __ixland_usleep_impl(useconds_t usec);
int __ixland_nanosleep_impl(const struct timespec *req, struct timespec *rem);
int __ixland_gettimeofday_impl(struct timeval *tv, struct timezone *tz);
int __ixland_settimeofday_impl(const struct timeval *tv, const struct timezone *tz);
int __ixland_clock_gettime_impl(clockid_t clk_id, struct timespec *tp);
time_t __ixland_time_impl(time_t *tloc);

/* ============================================================================
 * FUNCTION DECLARATIONS - Environment (Internal)
 * ============================================================================ */

char *__ixland_getenv_impl(const char *name);
int __ixland_setenv_impl(const char *name, const char *value, int overwrite);
int __ixland_unsetenv_impl(const char *name);
int __ixland_clearenv_impl(void);

/* ============================================================================
 * FUNCTION DECLARATIONS - Memory (Internal)
 * ============================================================================ */

void *__ixland_mmap_impl(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int __ixland_munmap_impl(void *addr, size_t length);
int __ixland_mprotect_impl(void *addr, size_t len, int prot);
int __ixland_msync_impl(void *addr, size_t length, int flags);
int __ixland_mlock_impl(const void *addr, size_t len);
int __ixland_munlock_impl(const void *addr, size_t len);

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

int __ixland_pipe_impl(int pipefd[2]);
int __ixland_pipe2_impl(int pipefd[2], int flags);
int __ixland_mkfifo_impl(const char *pathname, mode_t mode);
int __ixland_mkfifoat_impl(int dirfd, const char *pathname, mode_t mode);

/* ============================================================================
 * FUNCTION DECLARATIONS - TTY (Internal)
 * ============================================================================ */

int __ixland_isatty_impl(int fd);
int __ixland_ttyname_r_impl(int fd, char *buf, size_t buflen);
int __ixland_tcgetattr_impl(int fd, struct termios *termios_p);
int __ixland_tcsetattr_impl(int fd, int optional_actions, const struct termios *termios_p);
int __ixland_tcsendbreak_impl(int fd, int duration);
int __ixland_tcdrain_impl(int fd);
int __ixland_tcflush_impl(int fd, int queue_selector);
int __ixland_tcflow_impl(int fd, int action);

/* ============================================================================
 * FUNCTION DECLARATIONS - Directory (Internal)
 * ============================================================================ */

int __ixland_chdir_impl(const char *path);
int __ixland_fchdir_impl(int fd);
char *__ixland_getcwd_impl(char *buf, size_t size);

/* ============================================================================
 * FUNCTION DECLARATIONS - Logging/Debugging
 * ============================================================================ */

typedef enum {
    IXLAND_LOG_NONE = 0,
    IXLAND_LOG_ERROR,
    IXLAND_LOG_WARN,
    IXLAND_LOG_INFO,
    IXLAND_LOG_DEBUG,
    IXLAND_LOG_TRACE
} __ixland_log_level_t;

void __ixland_set_log_level(__ixland_log_level_t level);
void __ixland_log(__ixland_log_level_t level, const char *fmt, ...);

#define IXLAND_ERROR(...) __ixland_log(IXLAND_LOG_ERROR, __VA_ARGS__)
#define IXLAND_WARN(...) __ixland_log(IXLAND_LOG_WARN, __VA_ARGS__)
#define IXLAND_INFO(...) __ixland_log(IXLAND_LOG_INFO, __VA_ARGS__)
#define IXLAND_DEBUG(...) __ixland_log(IXLAND_LOG_DEBUG, __VA_ARGS__)
#define IXLAND_TRACE(...) __ixland_log(IXLAND_LOG_TRACE, __VA_ARGS__)

/* ============================================================================
 * FUNCTION DECLARATIONS - Public API (needed for interposition)
 * ============================================================================ */

/* Process management */
pid_t ixland_fork(void);
int ixland_vfork(void);
int ixland_execve(const char *pathname, char *const argv[], char *const envp[]);
int ixland_execv(const char *pathname, char *const argv[]);
void ixland_exit(int status);
void ixland__exit(int status);
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

/* File operations */
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

/* Filesystem */
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

/* Signal handling */
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

/* Memory management */
void *ixland_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int ixland_munmap(void *addr, size_t length);
int ixland_mprotect(void *addr, size_t len, int prot);
int ixland_msync(void *addr, size_t length, int flags);
int ixland_mlock(const void *addr, size_t len);
int ixland_munlock(const void *addr, size_t len);

/* Time */
unsigned int ixland_sleep(unsigned int seconds);
int ixland_usleep(useconds_t usec);
int ixland_nanosleep(const struct timespec *req, struct timespec *rem);
int ixland_gettimeofday(struct timeval *tv, struct timezone *tz);
int ixland_settimeofday(const struct timeval *tv, const struct timezone *tz);
int ixland_clock_gettime(clockid_t clk_id, struct timespec *tp);
time_t ixland_time(time_t *tloc);

/* Environment */
char *ixland_getenv(const char *name);
int ixland_setenv(const char *name, const char *value, int overwrite);
int ixland_unsetenv(const char *name);
int ixland_clearenv(void);
char **ixland_environ(void);

/* Directory */
int ixland_chdir(const char *path);
int ixland_fchdir(int fd);
char *ixland_getcwd(char *buf, size_t size);

/* I/O Multiplexing */
int ixland_select(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
                  linux_fd_set_t *exceptfds, struct linux_timeval *timeout);
int ixland_pselect(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
                   linux_fd_set_t *exceptfds, const struct linux_timespec *timeout,
                   const linux_sigset_t *sigmask);
int ixland_poll(struct linux_pollfd *fds, unsigned int nfds, int timeout);
int ixland_ppoll(struct linux_pollfd *fds, unsigned int nfds, const struct linux_timespec *timeout,
                 const linux_sigset_t *sigmask);
int ixland_epoll_create(int size);
int ixland_epoll_create1(int flags);
int ixland_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int ixland_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
int ixland_epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout,
                       const struct ixland_sigset *sigmask);
int ixland_epoll_pwait2(int epfd, struct epoll_event *events, int maxevents,
                        const struct ixland_timespec *timeout, const struct ixland_sigset *sigmask);

/* Pipes */
int ixland_pipe(int pipefd[2]);
int ixland_pipe2(int pipefd[2], int flags);
int ixland_mkfifo(const char *pathname, mode_t mode);
int ixland_mkfifoat(int dirfd, const char *pathname, mode_t mode);

/* TTY */
int ixland_isatty(int fd);
int ixland_ttyname_r(int fd, char *buf, size_t buflen);
int ixland_tcgetattr(int fd, struct termios *termios_p);
int ixland_tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
int ixland_tcsendbreak(int fd, int duration);
int ixland_tcdrain(int fd);
int ixland_tcflush(int fd, int queue_selector);
int ixland_tcflow(int fd, int action);

/* Identity - User/Group */
uid_t ixland_getuid(void);
uid_t ixland_geteuid(void);
gid_t ixland_getgid(void);
gid_t ixland_getegid(void);
int ixland_setuid(uid_t uid);
int ixland_setgid(gid_t gid);

/* Initialization */
int ixland_init(const ixland_config_t *config);
int ixland_is_initialized(void);
const char *ixland_version(void);

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_INTERNAL_H */

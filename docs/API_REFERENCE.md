# iXland Kernel API Reference

**Version**: 1.0.0
**Date**: 2026-03-28
**Status**: Stable

---

## Table of Contents

1. [Public API Overview](#public-api-overview)
2. [Type Definitions](#type-definitions)
3. [Error Codes](#error-codes)
4. [Process Management](#process-management)
5. [File Operations](#file-operations)
6. [Signal Handling](#signal-handling)
7. [Memory Management](#memory-management)
8. [Time Functions](#time-functions)
9. [Identity Functions](#identity-functions)
10. [Environment Functions](#environment-functions)
11. [Network Functions](#network-functions)
12. [IXLAND-Specific Extensions](#ixland-specific-extensions)

---

## Public API Overview

The iXland public API consists of three main headers:

| Header | Purpose |
|--------|---------|
| `<ixland/ixland_types.h>` | Type definitions, constants, structures |
| `<ixland/ixland_syscalls.h>` | Syscall function declarations |
| `<ixland/ixland.h>` | Umbrella header (includes both above) |

### Usage

```c
#include <ixland/ixland.h>  // Include all public APIs

// Or individually:
#include <ixland/ixland_types.h>
#include <ixland/ixland_syscalls.h>
```

---

## Type Definitions

### Basic Types

```c
#include <sys/types.h>

typedef int32_t  pid_t;     /* Process ID */
typedef uint32_t uid_t;     /* User ID */
typedef uint32_t gid_t;     /* Group ID */
typedef uint32_t mode_t;    /* File mode */
typedef uint64_t off_t;     /* File offset */
typedef int64_t  ssize_t;   /* Signed size */
```

### Error Type

```c
typedef enum {
    IXLAND_OK           = 0,      /* Success */
    IXLAND_ERROR        = -1,     /* Generic error */
    IXLAND_EINVAL       = -22,    /* Invalid argument */
    IXLAND_ENOMEM       = -12,    /* Out of memory */
    IXLAND_EACCES       = -13,    /* Permission denied */
    IXLAND_EEXIST       = -17,    /* File exists */
    IXLAND_ENOENT       = -2,     /* No such file or directory */
    IXLAND_EAGAIN       = -11,    /* Resource temporarily unavailable */
    IXLAND_EBUSY        = -16,    /* Device or resource busy */
    IXLAND_ECHILD       = -10,    /* No child processes */
    IXLAND_EINTR        = -4,     /* Interrupted system call */
    IXLAND_EIO          = -5,     /* I/O error */
    IXLAND_EISDIR       = -21,    /* Is a directory */
    IXLAND_EMFILE       = -24,    /* Too many open files */
    IXLAND_ENAMETOOLONG = -63,    /* File name too long */
    IXLAND_ENOTDIR      = -20,    /* Not a directory */
    IXLAND_ENOTEMPTY    = -39,    /* Directory not empty */
    IXLAND_EOPNOTSUPP   = -95,    /* Operation not supported */
    IXLAND_EOVERFLOW    = -75,    /* Value too large */
    IXLAND_EPERM        = -1,     /* Operation not permitted */
    IXLAND_ERANGE       = -34,    /* Result too large */
    IXLAND_ESPIPE       = -29,    /* Invalid seek */
    IXLAND_ESRCH        = -3,     /* No such process */
    IXLAND_ETIMEDOUT    = -60,    /* Connection timed out */
} ixland_error_t;
```

### Configuration Structure

```c
typedef struct {
    bool debug_enabled;         /* Enable debug logging */
    bool trace_syscalls;        /* Trace all syscalls */
    bool check_sandbox;         /* Validate all paths */
    size_t max_processes;       /* Maximum virtual processes */
    size_t max_threads;         /* Maximum threads */
    size_t max_memory;          /* Maximum memory per process */
    const char *home_directory; /* Home directory path */
    const char *tmp_directory;  /* Temporary directory path */
} ixland_config_t;
```

### Process Information

```c
typedef struct {
    pid_t pid;           /* Process ID */
    pid_t ppid;          /* Parent process ID */
    pid_t pgid;          /* Process group ID */
    uid_t uid;           /* User ID */
    gid_t gid;           /* Group ID */
    char name[256];      /* Process name */
    char state;          /* Process state (R, S, D, T, Z) */
    uint64_t memory_rss; /* Resident memory (bytes) */
    uint64_t memory_vms; /* Virtual memory (bytes) */
    uint64_t cpu_time;   /* CPU time (nanoseconds) */
    uint64_t start_time; /* Start time (epoch nanoseconds) */
} ixland_proc_info_t;
```

### System Information

```c
typedef struct {
    char sysname[256];   /* Operating system name (iOS) */
    char nodename[256];  /* Node name */
    char release[256];  /* OS release */
    char version[256];   /* OS version */
    char machine[256];   /* Hardware identifier (arm64) */

    uint64_t total_memory;  /* Total physical memory (bytes) */
    uint64_t free_memory;   /* Free memory (bytes) */
    uint64_t avail_memory;  /* Available memory (bytes) */
    uint64_t shared_memory; /* Shared memory (bytes) */
    uint64_t buffer_memory; /* Buffer cache (bytes) */

    uint32_t cpu_count;  /* Number of CPUs */
    uint32_t cpu_online; /* Online CPUs */
    uint64_t cpu_freq;   /* CPU frequency (Hz) */
    char cpu_arch[64];   /* CPU architecture (arm64) */
} ixland_sys_info_t;
```

### Resource Limits

```c
#define IXLAND_RLIM_INFINITY ((uint64_t)-1)

typedef struct {
    uint64_t rlim_cur; /* Current (soft) limit */
    uint64_t rlim_max; /* Hard limit */
} ixland_rlimit_t;

typedef enum {
    IXLAND_RLIMIT_CPU = 0,         /* CPU time in seconds */
    IXLAND_RLIMIT_FSIZE = 1,       /* Maximum file size */
    IXLAND_RLIMIT_DATA = 2,        /* Maximum data size */
    IXLAND_RLIMIT_STACK = 3,       /* Maximum stack size */
    IXLAND_RLIMIT_CORE = 4,        /* Maximum core file size */
    IXLAND_RLIMIT_RSS = 5,         /* Maximum resident set size */
    IXLAND_RLIMIT_NPROC = 6,       /* Maximum number of processes */
    IXLAND_RLIMIT_NOFILE = 7,      /* Maximum number of open files */
    IXLAND_RLIMIT_MEMLOCK = 8,     /* Maximum locked-in-memory space */
    IXLAND_RLIMIT_AS = 9,          /* Maximum address space */
    IXLAND_RLIMIT_LOCKS = 10,      /* Maximum file locks */
    IXLAND_RLIMIT_SIGPENDING = 11, /* Maximum queued signals */
    IXLAND_RLIMIT_MSGQUEUE = 12,   /* Maximum POSIX message queue bytes */
    IXLAND_RLIMIT_NICE = 13,       /* Maximum nice priority */
    IXLAND_RLIMIT_RTPRIO = 14,     /* Maximum real-time priority */
    IXLAND_RLIMIT_RTTIME = 15,     /* Maximum real-time timeout */
    IXLAND_RLIMIT_NLIMITS = 16     /* Number of resource limits */
} ixland_rlimit_resource_t;
```

### Callback Types

```c
/* Process state change callback */
typedef void (*ixland_proc_callback_t)(pid_t pid, int event, void *userdata);

/* Thread state change callback */
typedef void (*ixland_thread_callback_t)(pthread_t tid, int event, void *userdata);

/* Signal delivery callback */
typedef void (*ixland_signal_callback_t)(pid_t pid, int sig, void *userdata);

/* File operation callback */
typedef void (*ixland_file_callback_t)(const char *path, int op, void *userdata);
```

### Callback Events

```c
/* Process events */
#define IXLAND_PROC_CREATE   0x01
#define IXLAND_PROC_START    0x02
#define IXLAND_PROC_EXIT     0x04
#define IXLAND_PROC_SIGNAL   0x08
#define IXLAND_PROC_STOP     0x10
#define IXLAND_PROC_CONTINUE 0x20

/* Thread events */
#define IXLAND_THREAD_CREATE 0x01
#define IXLAND_THREAD_START  0x02
#define IXLAND_THREAD_EXIT   0x04
#define IXLAND_THREAD_DETACH 0x08

/* File operation events */
#define IXLAND_FILE_OPEN     0x01
#define IXLAND_FILE_CLOSE    0x02
#define IXLAND_FILE_READ     0x04
#define IXLAND_FILE_WRITE    0x08
#define IXLAND_FILE_TRUNCATE 0x10
#define IXLAND_FILE_DELETE   0x20
#define IXLAND_FILE_RENAME   0x40
#define IXLAND_FILE_CHMOD    0x80
```

---

## Error Codes

Standard Linux/POSIX error codes are returned as negative values:

| Constant | Value | Description |
|----------|-------|-------------|
| `IXLAND_OK` | 0 | Success |
| `IXLAND_EPERM` | -1 | Operation not permitted |
| `IXLAND_ENOENT` | -2 | No such file or directory |
| `IXLAND_ESRCH` | -3 | No such process |
| `IXLAND_EINTR` | -4 | Interrupted system call |
| `IXLAND_EIO` | -5 | I/O error |
| `IXLAND_ENXIO` | -6 | No such device or address |
| `IXLAND_E2BIG` | -7 | Argument list too long |
| `IXLAND_ENOEXEC` | -8 | Exec format error |
| `IXLAND_EBADF` | -9 | Bad file number |
| `IXLAND_ECHILD` | -10 | No child processes |
| `IXLAND_EAGAIN` | -11 | Try again |
| `IXLAND_ENOMEM` | -12 | Out of memory |
| `IXLAND_EACCES` | -13 | Permission denied |
| `IXLAND_EFAULT` | -14 | Bad address |
| `IXLAND_ENOTBLK` | -15 | Block device required |
| `IXLAND_EBUSY` | -16 | Device or resource busy |
| `IXLAND_EEXIST` | -17 | File exists |
| `IXLAND_EXDEV` | -18 | Cross-device link |
| `IXLAND_ENODEV` | -19 | No such device |
| `IXLAND_ENOTDIR` | -20 | Not a directory |
| `IXLAND_EISDIR` | -21 | Is a directory |
| `IXLAND_EINVAL` | -22 | Invalid argument |
| `IXLAND_ENFILE` | -23 | File table overflow |
| `IXLAND_EMFILE` | -24 | Too many open files |
| `IXLAND_ENOTTY` | -25 | Not a typewriter |

### Checking for Errors

```c
pid_t pid = ixland_fork();
if (pid < 0) {
    // Error occurred
    perror("fork");  // Uses errno
    return 1;
}
```

---

## Process Management

### ixland_fork

Create a new process by duplicating the calling process.

```c
pid_t ixland_fork(void);
```

**Returns**:
- On success: PID of child process to parent, 0 to child
- On error: -1, errno set

**Example**:
```c
pid_t pid = ixland_fork();
if (pid == 0) {
    // Child process
    printf("Child PID: %d\n", ixland_getpid());
    ixland_exit(0);
} else if (pid > 0) {
    // Parent process
    printf("Child created: %d\n", pid);
    ixland_waitpid(pid, NULL, 0);
}
```

---

### ixland_vfork

Create a child process and block parent until child execs or exits.

```c
int ixland_vfork(void);
```

**Returns**:
- On success: 0 to child, parent's PID to parent (after unblock)
- On error: -1

**Example**:
```c
pid_t pid = ixland_vfork();
if (pid == 0) {
    // Child - shares parent's memory
    char *argv[] = {"/bin/ls", NULL};
    ixland_execve("/bin/ls", argv, NULL);
    ixland_exit(1);
}
// Parent resumes after child execs/exits
```

---

### ixland_execve

Execute a program.

```c
int ixland_execve(const char *pathname, char *const argv[], char *const envp[]);
```

**Parameters**:
- `pathname`: Path to executable
- `argv`: Argument vector (NULL-terminated)
- `envp`: Environment variables (NULL-terminated)

**Returns**:
- On success: Does not return
- On error: -1, errno set

**Example**:
```c
char *argv[] = {"/bin/ls", "-la", NULL};
char *envp[] = {"PATH=/bin:/usr/bin", NULL};
ixland_execve("/bin/ls", argv, envp);
// If we get here, execve failed
perror("execve");
```

---

### ixland_execv

Execute a program with current environment.

```c
int ixland_execv(const char *pathname, char *const argv[]);
```

**Example**:
```c
char *argv[] = {"ls", "-la", NULL};
ixland_execv("/bin/ls", argv);
```

---

### ixland_exit

Terminate the calling process.

```c
void ixland_exit(int status);
```

**Parameters**:
- `status`: Exit status (0-255)

**Example**:
```c
ixland_exit(0);  // Success
ixland_exit(1);  // Error
```

---

### ixland__exit

Terminate immediately without cleanup.

```c
void ixland__exit(int status) __attribute__((noreturn));
```

**Example**:
```c
// Use in signal handlers or after fork failure
ixland__exit(1);
```

---

### ixland_getpid

Get process ID.

```c
pid_t ixland_getpid(void);
```

**Returns**: Process ID (always >= 1000 for iXland processes)

**Example**:
```c
printf("PID: %d\n", ixland_getpid());
```

---

### ixland_getppid

Get parent process ID.

```c
pid_t ixland_getppid(void);
```

**Returns**: Parent process ID

---

### ixland_getpgrp

Get process group ID.

```c
pid_t ixland_getpgrp(void);
```

**Returns**: Process group ID

---

### ixland_setpgrp

Set process group ID.

```c
int ixland_setpgrp(void);
```

**Returns**: 0 on success, -1 on error

---

### ixland_getpgid

Get process group ID of a process.

```c
pid_t ixland_getpgid(pid_t pid);
```

**Parameters**:
- `pid`: Process ID (0 for current process)

**Returns**: Process group ID, or -1 on error

---

### ixland_setpgid

Set process group ID.

```c
int ixland_setpgid(pid_t pid, pid_t pgid);
```

**Parameters**:
- `pid`: Process ID (0 for current process)
- `pgid`: Process group ID (0 to use pid)

**Returns**: 0 on success, -1 on error

**Example**:
```c
// Create new process group
ixland_setpgid(0, 0);
```

---

### ixland_getsid

Get session ID.

```c
pid_t ixland_getsid(pid_t pid);
```

**Parameters**:
- `pid`: Process ID (0 for current process)

**Returns**: Session ID

---

### ixland_setsid

Create a new session and set process group ID.

```c
pid_t ixland_setsid(void);
```

**Returns**: New session ID on success, -1 on error

**Example**:
```c
pid_t sid = ixland_setsid();
if (sid < 0) {
    perror("setsid");
}
```

---

### ixland_wait

Wait for any child process to terminate.

```c
pid_t ixland_wait(int *stat_loc);
```

**Parameters**:
- `stat_loc`: Pointer to store exit status (can be NULL)

**Returns**: PID of terminated child, or -1 on error

**Example**:
```c
int status;
pid_t pid = ixland_wait(&status);
if (pid > 0) {
    printf("Child %d exited with status %d\n",
           pid, WEXITSTATUS(status));
}
```

---

### ixland_waitpid

Wait for a specific child process.

```c
pid_t ixland_waitpid(pid_t pid, int *stat_loc, int options);
```

**Parameters**:
- `pid`: Child PID to wait for:
  - `-1`: Wait for any child
  - `0`: Wait for any child in same process group
  - `>0`: Wait for specific child
- `stat_loc`: Pointer to store exit status
- `options`: Bitmask:
  - `WNOHANG`: Return immediately if no child exited
  - `WUNTRACED`: Also return if child stopped
  - `WCONTINUED`: Also return if child continued

**Returns**: PID of child, 0 if WNOHANG and no child exited, -1 on error

**Example**:
```c
// Non-blocking wait
pid_t pid = ixland_waitpid(-1, &status, WNOHANG);
if (pid > 0) {
    // Child exited
} else if (pid == 0) {
    // No children exited yet
}
```

---

### ixland_wait3

Wait for child, with resource usage.

```c
pid_t ixland_wait3(int *stat_loc, int options, struct rusage *rusage);
```

**Parameters**:
- `rusage`: Resource usage structure (can be NULL)

**Returns**: PID of child, or -1 on error

---

### ixland_wait4

Wait for specific child, with resource usage.

```c
pid_t ixland_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage);
```

**Example**:
```c
struct rusage rusage;
pid_t child = ixland_wait4(-1, &status, 0, &rusage);
printf("User time: %ld.%06d\n",
       rusage.ru_utime.tv_sec, rusage.ru_utime.tv_usec);
```

---

### ixland_system

Execute a shell command.

```c
int ixland_system(const char *command);
```

**Parameters**:
- `command`: Shell command to execute

**Returns**: Exit status of command

**Example**:
```c
int status = ixland_system("ls -la");
```

---

## File Operations

### ixland_open

Open a file.

```c
int ixland_open(const char *pathname, int flags, ...);
```

**Parameters**:
- `pathname`: Path to file
- `flags`: Open flags (O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, etc.)
- `...`: Mode (required if O_CREAT is set)

**Returns**: File descriptor, or -1 on error

**Example**:
```c
int fd = ixland_open("/tmp/file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (fd < 0) {
    perror("open");
}
```

---

### ixland_openat

Open a file relative to directory.

```c
int ixland_openat(int dirfd, const char *pathname, int flags, ...);
```

**Parameters**:
- `dirfd`: Directory file descriptor (or AT_FDCWD)

---

### ixland_creat

Create a file.

```c
int ixland_creat(const char *pathname, mode_t mode);
```

**Example**:
```c
int fd = ixland_creat("/tmp/newfile", 0644);
```

---

### ixland_read

Read from a file descriptor.

```c
ssize_t ixland_read(int fd, void *buf, size_t count);
```

**Parameters**:
- `fd`: File descriptor
- `buf`: Buffer to read into
- `count`: Number of bytes to read

**Returns**: Number of bytes read, 0 at EOF, -1 on error

**Example**:
```c
char buffer[1024];
ssize_t n = ixland_read(fd, buffer, sizeof(buffer));
if (n > 0) {
    // Process buffer[0..n-1]
}
```

---

### ixland_write

Write to a file descriptor.

```c
ssize_t ixland_write(int fd, const void *buf, size_t count);
```

**Parameters**:
- `fd`: File descriptor
- `buf`: Buffer to write
- `count`: Number of bytes to write

**Returns**: Number of bytes written, -1 on error

**Example**:
```c
const char *msg = "Hello\n";
ixland_write(fd, msg, strlen(msg));
```

---

### ixland_close

Close a file descriptor.

```c
int ixland_close(int fd);
```

**Returns**: 0 on success, -1 on error

---

### ixland_lseek

Reposition read/write file offset.

```c
off_t ixland_lseek(int fd, off_t offset, int whence);
```

**Parameters**:
- `whence`: SEEK_SET, SEEK_CUR, or SEEK_END

**Returns**: New file offset, or -1 on error

**Example**:
```c
ixland_lseek(fd, 0, SEEK_SET);   // Seek to beginning
ixland_lseek(fd, -100, SEEK_END); // Seek 100 bytes from end
```

---

### ixland_dup

Duplicate a file descriptor.

```c
int ixland_dup(int oldfd);
```

**Returns**: New file descriptor, -1 on error

---

### ixland_dup2

Duplicate a file descriptor to a specific number.

```c
int ixland_dup2(int oldfd, int newfd);
```

**Example**:
```c
// Redirect stdout
ixland_dup2(fd, STDOUT_FILENO);
```

---

### ixland_dup3

Duplicate file descriptor with flags.

```c
int ixland_dup3(int oldfd, int newfd, int flags);
```

**Parameters**:
- `flags`: O_CLOEXEC or 0

---

### ixland_fcntl

File control operations.

```c
int ixland_fcntl(int fd, int cmd, ...);
```

**Commands**:
- `F_DUPFD`: Duplicate file descriptor
- `F_GETFD/F_SETFD`: File descriptor flags
- `F_GETFL/F_SETFL`: File status flags
- `F_GETOWN/F_SETOWN`: Owner
- `F_GETLK/F_SETLK/F_SETLKW`: Locks

**Example**:
```c
// Set close-on-exec
int flags = ixland_fcntl(fd, F_GETFD);
ixland_fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

// Set non-blocking
flags = ixland_fcntl(fd, F_GETFL);
ixland_fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

---

### ixland_ioctl

Device control.

```c
int ixland_ioctl(int fd, unsigned long request, ...);
```

---

### ixland_chdir

Change working directory.

```c
int ixland_chdir(const char *path);
```

---

### ixland_fchdir

Change working directory via file descriptor.

```c
int ixland_fchdir(int fd);
```

---

### ixland_getcwd

Get current working directory.

```c
char *ixland_getcwd(char *buf, size_t size);
```

**Returns**: Pointer to buf, or NULL on error

**Example**:
```c
char cwd[1024];
if (ixland_getcwd(cwd, sizeof(cwd))) {
    printf("CWD: %s\n", cwd);
}
```

---

### ixland_access

Check file accessibility.

```c
int ixland_access(const char *pathname, int mode);
```

**Parameters**:
- `mode`: R_OK, W_OK, X_OK, or F_OK

**Returns**: 0 if accessible, -1 on error

---

### ixland_faccessat

Check file accessibility relative to directory.

```c
int ixland_faccessat(int dirfd, const char *pathname, int mode, int flags);
```

---

## Signal Handling

### ixland_signal

Set signal handler (simplified interface).

```c
__sighandler_t ixland_signal(int signum, __sighandler_t handler);
```

**Parameters**:
- `signum`: Signal number (SIGINT, SIGTERM, etc.)
- `handler`: SIG_DFL, SIG_IGN, or function pointer

**Returns**: Previous handler, or SIG_ERR on error

**Example**:
```c
void handler(int sig) {
    printf("Caught signal %d\n", sig);
}

ixland_signal(SIGINT, handler);
ixland_signal(SIGTERM, SIG_IGN);  // Ignore SIGTERM
```

---

### ixland_kill

Send signal to a process.

```c
int ixland_kill(pid_t pid, int sig);
```

**Parameters**:
- `pid`: Process ID:
  - `>0`: Specific process
  - `0`: All processes in process group
  - `-1`: All processes (privileged)
  - `<-1`: Process group -pid
- `sig`: Signal number (0 to check if process exists)

**Returns**: 0 on success, -1 on error

**Example**:
```c
// Check if process exists
if (ixland_kill(pid, 0) == 0) {
    // Process exists
}

// Send SIGTERM
ixland_kill(pid, SIGTERM);

// Kill process group
ixland_kill(-pgid, SIGTERM);
```

---

### ixland_killpg

Send signal to a process group.

```c
int ixland_killpg(int pgrp, int sig);
```

**Example**:
```c
ixland_killpg(pgid, SIGHUP);
```

---

### ixland_raise

Send signal to self.

```c
int ixland_raise(int sig);
```

---

### ixland_sigaction

Examine and change signal action.

```c
int ixland_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```

**Parameters**:
- `signum`: Signal number
- `act`: New action (can be NULL)
- `oldact`: Previous action (can be NULL)

**Example**:
```c
struct sigaction sa;
sa.sa_handler = SIG_IGN;
sigemptyset(&sa.sa_mask);
sa.sa_flags = 0;
ixland_sigaction(SIGPIPE, &sa, NULL);

// With SA_SIGINFO
sa.sa_sigaction = advanced_handler;
sa.sa_flags = SA_SIGINFO;
ixland_sigaction(SIGUSR1, &sa, NULL);
```

---

### ixland_sigprocmask

Examine and change blocked signals.

```c
int ixland_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
```

**Parameters**:
- `how`: SIG_BLOCK, SIG_UNBLOCK, or SIG_SETMASK

**Example**:
```c
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);
ixland_sigprocmask(SIG_BLOCK, &set, NULL);  // Block SIGINT

// Later...
ixland_sigprocmask(SIG_UNBLOCK, &set, NULL); // Unblock
```

---

### ixland_sigpending

Get set of pending signals.

```c
int ixland_sigpending(sigset_t *set);
```

**Example**:
```c
sigset_t pending;
ixland_sigpending(&pending);
if (sigismember(&pending, SIGINT)) {
    printf("SIGINT is pending\n");
}
```

---

### ixland_sigsuspend

Wait for signal.

```c
int ixland_sigsuspend(const sigset_t *mask);
```

**Parameters**:
- `mask`: Temporary signal mask

**Returns**: -1 with errno EINTR

---

### Signal Set Operations

```c
int ixland_sigemptyset(sigset_t *set);
int ixland_sigfillset(sigset_t *set);
int ixland_sigaddset(sigset_t *set, int signum);
int ixland_sigdelset(sigset_t *set, int signum);
int ixland_sigismember(const sigset_t *set, int signum);
```

**Example**:
```c
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);
sigaddset(&set, SIGTERM);
if (sigismember(&set, SIGINT)) {
    // true
}
```

---

### ixland_alarm

Set an alarm clock for delivery of a signal.

```c
unsigned int ixland_alarm(unsigned int seconds);
```

**Returns**: Seconds remaining from previous alarm, or 0

**Example**:
```c
ixland_signal(SIGALRM, timeout_handler);
ixland_alarm(5);  // 5 second timeout
```

---

### ixland_setitimer

Set value of an interval timer.

```c
int ixland_setitimer(int which, const struct itimerval *new_value,
                  struct itimerval *old_value);
```

**Parameters**:
- `which`: ITIMER_REAL, ITIMER_VIRTUAL, or ITIMER_PROF

---

### ixland_getitimer

Get value of an interval timer.

```c
int ixland_getitimer(int which, struct itimerval *curr_value);
```

---

### ixland_pause

Wait for signal.

```c
int ixland_pause(void);
```

**Returns**: -1 with errno EINTR

---

## Memory Management

### ixland_mmap

Map files or devices into memory.

```c
void *ixland_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```

**Parameters**:
- `addr`: Preferred address (NULL for automatic)
- `length`: Mapping length
- `prot`: PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE
- `flags`: MAP_SHARED, MAP_PRIVATE, MAP_ANONYMOUS, etc.
- `fd`: File descriptor (-1 for anonymous)
- `offset`: File offset

**Returns**: Mapped address, or MAP_FAILED on error

**Example**:
```c
// Anonymous mapping
void *mem = ixland_mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
if (mem == MAP_FAILED) {
    perror("mmap");
}
```

---

### ixland_munmap

Unmap memory.

```c
int ixland_munmap(void *addr, size_t length);
```

---

### ixland_mprotect

Set protection on a region of memory.

```c
int ixland_mprotect(void *addr, size_t len, int prot);
```

**Example**:
```c
// Make memory read-only
ixland_mprotect(mem, len, PROT_READ);
```

---

### ixland_msync

Synchronize a file with a memory map.

```c
int ixland_msync(void *addr, size_t length, int flags);
```

---

### ixland_mlock

Lock memory.

```c
int ixland_mlock(const void *addr, size_t len);
```

---

### ixland_munlock

Unlock memory.

```c
int ixland_munlock(const void *addr, size_t len);
```

---

## Time Functions

### ixland_sleep

Sleep for specified seconds.

```c
unsigned int ixland_sleep(unsigned int seconds);
```

**Returns**: 0 on success, seconds remaining if interrupted

---

### ixland_usleep

Sleep for specified microseconds.

```c
int ixland_usleep(useconds_t usec);
```

---

### ixland_nanosleep

High-resolution sleep.

```c
int ixland_nanosleep(const struct timespec *req, struct timespec *rem);
```

**Example**:
```c
struct timespec req = {1, 500000000};  // 1.5 seconds
ixland_nanosleep(&req, NULL);
```

---

### ixland_gettimeofday

Get time.

```c
int ixland_gettimeofday(struct timeval *tv, struct timezone *tz);
```

**Example**:
```c
struct timeval tv;
ixland_gettimeofday(&tv, NULL);
printf("Seconds: %ld\n", tv.tv_sec);
```

---

### ixland_time

Get time in seconds.

```c
time_t ixland_time(time_t *tloc);
```

**Example**:
```c
time_t now = ixland_time(NULL);
```

---

### ixland_clock_gettime

Get time from a clock.

```c
int ixland_clock_gettime(clockid_t clk_id, struct timespec *tp);
```

**Clock IDs**:
- `CLOCK_REALTIME`
- `CLOCK_MONOTONIC`
- `CLOCK_PROCESS_CPUTIME_ID`

---

## Identity Functions

### ixland_getuid

Get real user ID.

```c
uid_t ixland_getuid(void);
```

---

### ixland_geteuid

Get effective user ID.

```c
uid_t ixland_geteuid(void);
```

---

### ixland_getgid

Get real group ID.

```c
gid_t ixland_getgid(void);
```

---

### ixland_getegid

Get effective group ID.

```c
gid_t ixland_getegid(void);
```

---

### ixland_setuid

Set user ID.

```c
int ixland_setuid(uid_t uid);
```

**Note**: Returns EPERM on iOS (not permitted)

---

### ixland_seteuid

Set effective user ID.

```c
int ixland_seteuid(uid_t uid);
```

---

### ixland_setgid

Set group ID.

```c
int ixland_setgid(gid_t gid);
```

---

### ixland_setegid

Set effective group ID.

```c
int ixland_setegid(gid_t gid);
```

---

### ixland_getgroups

Get supplementary group IDs.

```c
int ixland_getgroups(int size, gid_t list[]);
```

---

### ixland_setgroups

Set supplementary group IDs.

```c
int ixland_setgroups(size_t size, const gid_t *list);
```

---

## Environment Functions

### ixland_getenv

Get environment variable.

```c
char *ixland_getenv(const char *name);
```

**Example**:
```c
char *path = ixland_getenv("PATH");
if (path) {
    printf("PATH=%s\n", path);
}
```

---

### ixland_setenv

Set environment variable.

```c
int ixland_setenv(const char *name, const char *value, int overwrite);
```

**Example**:
```c
ixland_setenv("MYVAR", "myvalue", 1);  // Overwrite if exists
```

---

### ixland_unsetenv

Unset environment variable.

```c
int ixland_unsetenv(const char *name);
```

---

### ixland_clearenv

Clear environment.

```c
int ixland_clearenv(void);
```

---

### ixland_environ

Get environment pointer.

```c
char **ixland_environ(void);
```

---

## Network Functions

### ixland_socket

Create a socket.

```c
int ixland_socket(int domain, int type, int protocol);
```

**Example**:
```c
int sock = ixland_socket(AF_INET, SOCK_STREAM, 0);
```

---

### ixland_connect

Connect to a socket.

```c
int ixland_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

---

### ixland_bind

Bind a socket.

```c
int ixland_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

---

### ixland_listen

Listen for connections.

```c
int ixland_listen(int sockfd, int backlog);
```

---

### ixland_accept

Accept a connection.

```c
int ixland_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

---

### ixland_send / ixland_recv

Send/receive data.

```c
ssize_t ixland_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t ixland_recv(int sockfd, void *buf, size_t len, int flags);
```

---

### ixland_sendto / ixland_recvfrom

Send/receive datagrams.

```c
ssize_t ixland_sendto(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t ixland_recvfrom(int sockfd, void *buf, size_t len, int flags,
                     struct sockaddr *src_addr, socklen_t *addrlen);
```

---

### ixland_shutdown

Shutdown socket.

```c
int ixland_shutdown(int sockfd, int how);
```

---

### ixland_setsockopt / ixland_getsockopt

Socket options.

```c
int ixland_setsockopt(int sockfd, int level, int optname,
                   const void *optval, socklen_t optlen);
int ixland_getsockopt(int sockfd, int level, int optname,
                   void *optval, socklen_t *optlen);
```

---

## IXLAND-Specific Extensions

### Initialization and Cleanup

```c
int ixland_init(const ixland_config_t *config);
void ixland_cleanup(void);
```

**Example**:
```c
ixland_config_t config = {
    .debug_enabled = true,
    .home_directory = "/home/user",
    .max_processes = 256
};
ixland_init(&config);

// ... use ixland syscalls ...

ixland_cleanup();
```

---

### Version Information

```c
const char *ixland_version(void);
int ixland_is_initialized(void);
```

**Example**:
```c
printf("iXland version: %s\n", ixland_version());
// Output: iXland version: 1.0.0
```

---

### System Information

```c
ixland_sys_info_t ixland_get_sys_info(void);
```

**Example**:
```c
ixland_sys_info_t info = ixland_get_sys_info();
printf("OS: %s %s\n", info.sysname, info.release);
printf("Memory: %llu MB total\n", info.total_memory / (1024 * 1024));
```

---

### Process Information

```c
ixland_proc_info_t *ixland_get_proc_info(pid_t pid);
void ixland_free_proc_info(ixland_proc_info_t *info);
```

**Example**:
```c
ixland_proc_info_t *info = ixland_get_proc_info(ixland_getpid());
if (info) {
    printf("Process: %s (PID %d)\n", info->name, info->pid);
    printf("State: %c\n", info->state);
    ixland_free_proc_info(info);
}
```

---

### Statistics

```c
void ixland_get_syscall_stats(ixland_syscall_t syscall, ixland_syscall_stat_t *stats);
void ixland_get_memory_stats(ixland_memory_stat_t *stats);
void ixland_get_process_stats(ixland_process_stat_t *stats);
```

**Example**:
```c
ixland_syscall_stat_t stats;
ixland_get_syscall_stats(IXLAND_SYSCALL_FORK, &stats);
printf("fork() called %llu times\n", stats.calls);
```

---

### Callbacks

```c
void ixland_set_proc_callback(ixland_proc_callback_t callback, void *userdata);
void ixland_set_thread_callback(ixland_thread_callback_t callback, void *userdata);
void ixland_set_file_callback(ixland_file_callback_t callback, void *userdata);
```

**Example**:
```c
void on_proc_change(pid_t pid, int event, void *userdata) {
    if (event & IXLAND_PROC_CREATE) {
        printf("Process %d created\n", pid);
    }
    if (event & IXLAND_PROC_EXIT) {
        printf("Process %d exited\n", pid);
    }
}

ixland_set_proc_callback(on_proc_change, NULL);
```

---

### Debugging

```c
void ixland_debug_dump(FILE *stream);
void ixland_set_tracing(int enabled);
```

**Example**:
```c
ixland_set_tracing(1);  // Enable syscall tracing
// ... syscalls are logged ...
ixland_set_tracing(0);  // Disable

ixland_debug_dump(stderr);  // Dump internal state
```

---

## Error Handling

### ixland_strerror

Get error description.

```c
const char *ixland_strerror(int errnum);
```

**Example**:
```c
int fd = ixland_open("/nonexistent", O_RDONLY);
if (fd < 0) {
    printf("Error: %s\n", ixland_strerror(errno));
    // Output: Error: No such file or directory
}
```

---

### ixland_perror

Print error message.

```c
void ixland_perror(const char *s);
```

**Example**:
```c
if (ixland_open("/file", O_RDONLY) < 0) {
    ixland_perror("open");
    // Output: open: No such file or directory
}
```

---

## Summary

The iXland API provides:

- **100+ syscalls** covering process, file, signal, memory, and network operations
- **POSIX compatibility** with Linux-compatible semantics
- **Clean naming** with `ixland_` prefix for all functions
- **iOS-specific extensions** for initialization, debugging, and introspection
- **Type safety** with proper struct definitions
- **Error handling** using standard errno conventions

All functions are thread-safe where applicable and use virtual PIDs starting at 1000.

---

*End of API Reference*

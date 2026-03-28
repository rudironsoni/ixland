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
12. [IOX-Specific Extensions](#iox-specific-extensions)

---

## Public API Overview

The iXland public API consists of three main headers:

| Header | Purpose |
|--------|---------|
| `<iox/iox_types.h>` | Type definitions, constants, structures |
| `<iox/iox_syscalls.h>` | Syscall function declarations |
| `<iox/iox.h>` | Umbrella header (includes both above) |

### Usage

```c
#include <iox/iox.h>  // Include all public APIs

// Or individually:
#include <iox/iox_types.h>
#include <iox/iox_syscalls.h>
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
    IOX_OK           = 0,      /* Success */
    IOX_ERROR        = -1,     /* Generic error */
    IOX_EINVAL       = -22,    /* Invalid argument */
    IOX_ENOMEM       = -12,    /* Out of memory */
    IOX_EACCES       = -13,    /* Permission denied */
    IOX_EEXIST       = -17,    /* File exists */
    IOX_ENOENT       = -2,     /* No such file or directory */
    IOX_EAGAIN       = -11,    /* Resource temporarily unavailable */
    IOX_EBUSY        = -16,    /* Device or resource busy */
    IOX_ECHILD       = -10,    /* No child processes */
    IOX_EINTR        = -4,     /* Interrupted system call */
    IOX_EIO          = -5,     /* I/O error */
    IOX_EISDIR       = -21,    /* Is a directory */
    IOX_EMFILE       = -24,    /* Too many open files */
    IOX_ENAMETOOLONG = -63,    /* File name too long */
    IOX_ENOTDIR      = -20,    /* Not a directory */
    IOX_ENOTEMPTY    = -39,    /* Directory not empty */
    IOX_EOPNOTSUPP   = -95,    /* Operation not supported */
    IOX_EOVERFLOW    = -75,    /* Value too large */
    IOX_EPERM        = -1,     /* Operation not permitted */
    IOX_ERANGE       = -34,    /* Result too large */
    IOX_ESPIPE       = -29,    /* Invalid seek */
    IOX_ESRCH        = -3,     /* No such process */
    IOX_ETIMEDOUT    = -60,    /* Connection timed out */
} iox_error_t;
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
} iox_config_t;
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
} iox_proc_info_t;
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
} iox_sys_info_t;
```

### Resource Limits

```c
#define IOX_RLIM_INFINITY ((uint64_t)-1)

typedef struct {
    uint64_t rlim_cur; /* Current (soft) limit */
    uint64_t rlim_max; /* Hard limit */
} iox_rlimit_t;

typedef enum {
    IOX_RLIMIT_CPU = 0,         /* CPU time in seconds */
    IOX_RLIMIT_FSIZE = 1,       /* Maximum file size */
    IOX_RLIMIT_DATA = 2,        /* Maximum data size */
    IOX_RLIMIT_STACK = 3,       /* Maximum stack size */
    IOX_RLIMIT_CORE = 4,        /* Maximum core file size */
    IOX_RLIMIT_RSS = 5,         /* Maximum resident set size */
    IOX_RLIMIT_NPROC = 6,       /* Maximum number of processes */
    IOX_RLIMIT_NOFILE = 7,      /* Maximum number of open files */
    IOX_RLIMIT_MEMLOCK = 8,     /* Maximum locked-in-memory space */
    IOX_RLIMIT_AS = 9,          /* Maximum address space */
    IOX_RLIMIT_LOCKS = 10,      /* Maximum file locks */
    IOX_RLIMIT_SIGPENDING = 11, /* Maximum queued signals */
    IOX_RLIMIT_MSGQUEUE = 12,   /* Maximum POSIX message queue bytes */
    IOX_RLIMIT_NICE = 13,       /* Maximum nice priority */
    IOX_RLIMIT_RTPRIO = 14,     /* Maximum real-time priority */
    IOX_RLIMIT_RTTIME = 15,     /* Maximum real-time timeout */
    IOX_RLIMIT_NLIMITS = 16     /* Number of resource limits */
} iox_rlimit_resource_t;
```

### Callback Types

```c
/* Process state change callback */
typedef void (*iox_proc_callback_t)(pid_t pid, int event, void *userdata);

/* Thread state change callback */
typedef void (*iox_thread_callback_t)(pthread_t tid, int event, void *userdata);

/* Signal delivery callback */
typedef void (*iox_signal_callback_t)(pid_t pid, int sig, void *userdata);

/* File operation callback */
typedef void (*iox_file_callback_t)(const char *path, int op, void *userdata);
```

### Callback Events

```c
/* Process events */
#define IOX_PROC_CREATE   0x01
#define IOX_PROC_START    0x02
#define IOX_PROC_EXIT     0x04
#define IOX_PROC_SIGNAL   0x08
#define IOX_PROC_STOP     0x10
#define IOX_PROC_CONTINUE 0x20

/* Thread events */
#define IOX_THREAD_CREATE 0x01
#define IOX_THREAD_START  0x02
#define IOX_THREAD_EXIT   0x04
#define IOX_THREAD_DETACH 0x08

/* File operation events */
#define IOX_FILE_OPEN     0x01
#define IOX_FILE_CLOSE    0x02
#define IOX_FILE_READ     0x04
#define IOX_FILE_WRITE    0x08
#define IOX_FILE_TRUNCATE 0x10
#define IOX_FILE_DELETE   0x20
#define IOX_FILE_RENAME   0x40
#define IOX_FILE_CHMOD    0x80
```

---

## Error Codes

Standard Linux/POSIX error codes are returned as negative values:

| Constant | Value | Description |
|----------|-------|-------------|
| `IOX_OK` | 0 | Success |
| `IOX_EPERM` | -1 | Operation not permitted |
| `IOX_ENOENT` | -2 | No such file or directory |
| `IOX_ESRCH` | -3 | No such process |
| `IOX_EINTR` | -4 | Interrupted system call |
| `IOX_EIO` | -5 | I/O error |
| `IOX_ENXIO` | -6 | No such device or address |
| `IOX_E2BIG` | -7 | Argument list too long |
| `IOX_ENOEXEC` | -8 | Exec format error |
| `IOX_EBADF` | -9 | Bad file number |
| `IOX_ECHILD` | -10 | No child processes |
| `IOX_EAGAIN` | -11 | Try again |
| `IOX_ENOMEM` | -12 | Out of memory |
| `IOX_EACCES` | -13 | Permission denied |
| `IOX_EFAULT` | -14 | Bad address |
| `IOX_ENOTBLK` | -15 | Block device required |
| `IOX_EBUSY` | -16 | Device or resource busy |
| `IOX_EEXIST` | -17 | File exists |
| `IOX_EXDEV` | -18 | Cross-device link |
| `IOX_ENODEV` | -19 | No such device |
| `IOX_ENOTDIR` | -20 | Not a directory |
| `IOX_EISDIR` | -21 | Is a directory |
| `IOX_EINVAL` | -22 | Invalid argument |
| `IOX_ENFILE` | -23 | File table overflow |
| `IOX_EMFILE` | -24 | Too many open files |
| `IOX_ENOTTY` | -25 | Not a typewriter |

### Checking for Errors

```c
pid_t pid = iox_fork();
if (pid < 0) {
    // Error occurred
    perror("fork");  // Uses errno
    return 1;
}
```

---

## Process Management

### iox_fork

Create a new process by duplicating the calling process.

```c
pid_t iox_fork(void);
```

**Returns**:
- On success: PID of child process to parent, 0 to child
- On error: -1, errno set

**Example**:
```c
pid_t pid = iox_fork();
if (pid == 0) {
    // Child process
    printf("Child PID: %d\n", iox_getpid());
    iox_exit(0);
} else if (pid > 0) {
    // Parent process
    printf("Child created: %d\n", pid);
    iox_waitpid(pid, NULL, 0);
}
```

---

### iox_vfork

Create a child process and block parent until child execs or exits.

```c
int iox_vfork(void);
```

**Returns**:
- On success: 0 to child, parent's PID to parent (after unblock)
- On error: -1

**Example**:
```c
pid_t pid = iox_vfork();
if (pid == 0) {
    // Child - shares parent's memory
    char *argv[] = {"/bin/ls", NULL};
    iox_execve("/bin/ls", argv, NULL);
    iox_exit(1);
}
// Parent resumes after child execs/exits
```

---

### iox_execve

Execute a program.

```c
int iox_execve(const char *pathname, char *const argv[], char *const envp[]);
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
iox_execve("/bin/ls", argv, envp);
// If we get here, execve failed
perror("execve");
```

---

### iox_execv

Execute a program with current environment.

```c
int iox_execv(const char *pathname, char *const argv[]);
```

**Example**:
```c
char *argv[] = {"ls", "-la", NULL};
iox_execv("/bin/ls", argv);
```

---

### iox_exit

Terminate the calling process.

```c
void iox_exit(int status);
```

**Parameters**:
- `status`: Exit status (0-255)

**Example**:
```c
iox_exit(0);  // Success
iox_exit(1);  // Error
```

---

### iox__exit

Terminate immediately without cleanup.

```c
void iox__exit(int status) __attribute__((noreturn));
```

**Example**:
```c
// Use in signal handlers or after fork failure
iox__exit(1);
```

---

### iox_getpid

Get process ID.

```c
pid_t iox_getpid(void);
```

**Returns**: Process ID (always >= 1000 for iXland processes)

**Example**:
```c
printf("PID: %d\n", iox_getpid());
```

---

### iox_getppid

Get parent process ID.

```c
pid_t iox_getppid(void);
```

**Returns**: Parent process ID

---

### iox_getpgrp

Get process group ID.

```c
pid_t iox_getpgrp(void);
```

**Returns**: Process group ID

---

### iox_setpgrp

Set process group ID.

```c
int iox_setpgrp(void);
```

**Returns**: 0 on success, -1 on error

---

### iox_getpgid

Get process group ID of a process.

```c
pid_t iox_getpgid(pid_t pid);
```

**Parameters**:
- `pid`: Process ID (0 for current process)

**Returns**: Process group ID, or -1 on error

---

### iox_setpgid

Set process group ID.

```c
int iox_setpgid(pid_t pid, pid_t pgid);
```

**Parameters**:
- `pid`: Process ID (0 for current process)
- `pgid`: Process group ID (0 to use pid)

**Returns**: 0 on success, -1 on error

**Example**:
```c
// Create new process group
iox_setpgid(0, 0);
```

---

### iox_getsid

Get session ID.

```c
pid_t iox_getsid(pid_t pid);
```

**Parameters**:
- `pid`: Process ID (0 for current process)

**Returns**: Session ID

---

### iox_setsid

Create a new session and set process group ID.

```c
pid_t iox_setsid(void);
```

**Returns**: New session ID on success, -1 on error

**Example**:
```c
pid_t sid = iox_setsid();
if (sid < 0) {
    perror("setsid");
}
```

---

### iox_wait

Wait for any child process to terminate.

```c
pid_t iox_wait(int *stat_loc);
```

**Parameters**:
- `stat_loc`: Pointer to store exit status (can be NULL)

**Returns**: PID of terminated child, or -1 on error

**Example**:
```c
int status;
pid_t pid = iox_wait(&status);
if (pid > 0) {
    printf("Child %d exited with status %d\n",
           pid, WEXITSTATUS(status));
}
```

---

### iox_waitpid

Wait for a specific child process.

```c
pid_t iox_waitpid(pid_t pid, int *stat_loc, int options);
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
pid_t pid = iox_waitpid(-1, &status, WNOHANG);
if (pid > 0) {
    // Child exited
} else if (pid == 0) {
    // No children exited yet
}
```

---

### iox_wait3

Wait for child, with resource usage.

```c
pid_t iox_wait3(int *stat_loc, int options, struct rusage *rusage);
```

**Parameters**:
- `rusage`: Resource usage structure (can be NULL)

**Returns**: PID of child, or -1 on error

---

### iox_wait4

Wait for specific child, with resource usage.

```c
pid_t iox_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage);
```

**Example**:
```c
struct rusage rusage;
pid_t child = iox_wait4(-1, &status, 0, &rusage);
printf("User time: %ld.%06d\n",
       rusage.ru_utime.tv_sec, rusage.ru_utime.tv_usec);
```

---

### iox_system

Execute a shell command.

```c
int iox_system(const char *command);
```

**Parameters**:
- `command`: Shell command to execute

**Returns**: Exit status of command

**Example**:
```c
int status = iox_system("ls -la");
```

---

## File Operations

### iox_open

Open a file.

```c
int iox_open(const char *pathname, int flags, ...);
```

**Parameters**:
- `pathname`: Path to file
- `flags`: Open flags (O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, etc.)
- `...`: Mode (required if O_CREAT is set)

**Returns**: File descriptor, or -1 on error

**Example**:
```c
int fd = iox_open("/tmp/file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (fd < 0) {
    perror("open");
}
```

---

### iox_openat

Open a file relative to directory.

```c
int iox_openat(int dirfd, const char *pathname, int flags, ...);
```

**Parameters**:
- `dirfd`: Directory file descriptor (or AT_FDCWD)

---

### iox_creat

Create a file.

```c
int iox_creat(const char *pathname, mode_t mode);
```

**Example**:
```c
int fd = iox_creat("/tmp/newfile", 0644);
```

---

### iox_read

Read from a file descriptor.

```c
ssize_t iox_read(int fd, void *buf, size_t count);
```

**Parameters**:
- `fd`: File descriptor
- `buf`: Buffer to read into
- `count`: Number of bytes to read

**Returns**: Number of bytes read, 0 at EOF, -1 on error

**Example**:
```c
char buffer[1024];
ssize_t n = iox_read(fd, buffer, sizeof(buffer));
if (n > 0) {
    // Process buffer[0..n-1]
}
```

---

### iox_write

Write to a file descriptor.

```c
ssize_t iox_write(int fd, const void *buf, size_t count);
```

**Parameters**:
- `fd`: File descriptor
- `buf`: Buffer to write
- `count`: Number of bytes to write

**Returns**: Number of bytes written, -1 on error

**Example**:
```c
const char *msg = "Hello\n";
iox_write(fd, msg, strlen(msg));
```

---

### iox_close

Close a file descriptor.

```c
int iox_close(int fd);
```

**Returns**: 0 on success, -1 on error

---

### iox_lseek

Reposition read/write file offset.

```c
off_t iox_lseek(int fd, off_t offset, int whence);
```

**Parameters**:
- `whence`: SEEK_SET, SEEK_CUR, or SEEK_END

**Returns**: New file offset, or -1 on error

**Example**:
```c
iox_lseek(fd, 0, SEEK_SET);   // Seek to beginning
iox_lseek(fd, -100, SEEK_END); // Seek 100 bytes from end
```

---

### iox_dup

Duplicate a file descriptor.

```c
int iox_dup(int oldfd);
```

**Returns**: New file descriptor, -1 on error

---

### iox_dup2

Duplicate a file descriptor to a specific number.

```c
int iox_dup2(int oldfd, int newfd);
```

**Example**:
```c
// Redirect stdout
iox_dup2(fd, STDOUT_FILENO);
```

---

### iox_dup3

Duplicate file descriptor with flags.

```c
int iox_dup3(int oldfd, int newfd, int flags);
```

**Parameters**:
- `flags`: O_CLOEXEC or 0

---

### iox_fcntl

File control operations.

```c
int iox_fcntl(int fd, int cmd, ...);
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
int flags = iox_fcntl(fd, F_GETFD);
iox_fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

// Set non-blocking
flags = iox_fcntl(fd, F_GETFL);
iox_fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

---

### iox_ioctl

Device control.

```c
int iox_ioctl(int fd, unsigned long request, ...);
```

---

### iox_chdir

Change working directory.

```c
int iox_chdir(const char *path);
```

---

### iox_fchdir

Change working directory via file descriptor.

```c
int iox_fchdir(int fd);
```

---

### iox_getcwd

Get current working directory.

```c
char *iox_getcwd(char *buf, size_t size);
```

**Returns**: Pointer to buf, or NULL on error

**Example**:
```c
char cwd[1024];
if (iox_getcwd(cwd, sizeof(cwd))) {
    printf("CWD: %s\n", cwd);
}
```

---

### iox_access

Check file accessibility.

```c
int iox_access(const char *pathname, int mode);
```

**Parameters**:
- `mode`: R_OK, W_OK, X_OK, or F_OK

**Returns**: 0 if accessible, -1 on error

---

### iox_faccessat

Check file accessibility relative to directory.

```c
int iox_faccessat(int dirfd, const char *pathname, int mode, int flags);
```

---

## Signal Handling

### iox_signal

Set signal handler (simplified interface).

```c
__sighandler_t iox_signal(int signum, __sighandler_t handler);
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

iox_signal(SIGINT, handler);
iox_signal(SIGTERM, SIG_IGN);  // Ignore SIGTERM
```

---

### iox_kill

Send signal to a process.

```c
int iox_kill(pid_t pid, int sig);
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
if (iox_kill(pid, 0) == 0) {
    // Process exists
}

// Send SIGTERM
iox_kill(pid, SIGTERM);

// Kill process group
iox_kill(-pgid, SIGTERM);
```

---

### iox_killpg

Send signal to a process group.

```c
int iox_killpg(int pgrp, int sig);
```

**Example**:
```c
iox_killpg(pgid, SIGHUP);
```

---

### iox_raise

Send signal to self.

```c
int iox_raise(int sig);
```

---

### iox_sigaction

Examine and change signal action.

```c
int iox_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
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
iox_sigaction(SIGPIPE, &sa, NULL);

// With SA_SIGINFO
sa.sa_sigaction = advanced_handler;
sa.sa_flags = SA_SIGINFO;
iox_sigaction(SIGUSR1, &sa, NULL);
```

---

### iox_sigprocmask

Examine and change blocked signals.

```c
int iox_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
```

**Parameters**:
- `how`: SIG_BLOCK, SIG_UNBLOCK, or SIG_SETMASK

**Example**:
```c
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);
iox_sigprocmask(SIG_BLOCK, &set, NULL);  // Block SIGINT

// Later...
iox_sigprocmask(SIG_UNBLOCK, &set, NULL); // Unblock
```

---

### iox_sigpending

Get set of pending signals.

```c
int iox_sigpending(sigset_t *set);
```

**Example**:
```c
sigset_t pending;
iox_sigpending(&pending);
if (sigismember(&pending, SIGINT)) {
    printf("SIGINT is pending\n");
}
```

---

### iox_sigsuspend

Wait for signal.

```c
int iox_sigsuspend(const sigset_t *mask);
```

**Parameters**:
- `mask`: Temporary signal mask

**Returns**: -1 with errno EINTR

---

### Signal Set Operations

```c
int iox_sigemptyset(sigset_t *set);
int iox_sigfillset(sigset_t *set);
int iox_sigaddset(sigset_t *set, int signum);
int iox_sigdelset(sigset_t *set, int signum);
int iox_sigismember(const sigset_t *set, int signum);
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

### iox_alarm

Set an alarm clock for delivery of a signal.

```c
unsigned int iox_alarm(unsigned int seconds);
```

**Returns**: Seconds remaining from previous alarm, or 0

**Example**:
```c
iox_signal(SIGALRM, timeout_handler);
iox_alarm(5);  // 5 second timeout
```

---

### iox_setitimer

Set value of an interval timer.

```c
int iox_setitimer(int which, const struct itimerval *new_value,
                  struct itimerval *old_value);
```

**Parameters**:
- `which`: ITIMER_REAL, ITIMER_VIRTUAL, or ITIMER_PROF

---

### iox_getitimer

Get value of an interval timer.

```c
int iox_getitimer(int which, struct itimerval *curr_value);
```

---

### iox_pause

Wait for signal.

```c
int iox_pause(void);
```

**Returns**: -1 with errno EINTR

---

## Memory Management

### iox_mmap

Map files or devices into memory.

```c
void *iox_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
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
void *mem = iox_mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
if (mem == MAP_FAILED) {
    perror("mmap");
}
```

---

### iox_munmap

Unmap memory.

```c
int iox_munmap(void *addr, size_t length);
```

---

### iox_mprotect

Set protection on a region of memory.

```c
int iox_mprotect(void *addr, size_t len, int prot);
```

**Example**:
```c
// Make memory read-only
iox_mprotect(mem, len, PROT_READ);
```

---

### iox_msync

Synchronize a file with a memory map.

```c
int iox_msync(void *addr, size_t length, int flags);
```

---

### iox_mlock

Lock memory.

```c
int iox_mlock(const void *addr, size_t len);
```

---

### iox_munlock

Unlock memory.

```c
int iox_munlock(const void *addr, size_t len);
```

---

## Time Functions

### iox_sleep

Sleep for specified seconds.

```c
unsigned int iox_sleep(unsigned int seconds);
```

**Returns**: 0 on success, seconds remaining if interrupted

---

### iox_usleep

Sleep for specified microseconds.

```c
int iox_usleep(useconds_t usec);
```

---

### iox_nanosleep

High-resolution sleep.

```c
int iox_nanosleep(const struct timespec *req, struct timespec *rem);
```

**Example**:
```c
struct timespec req = {1, 500000000};  // 1.5 seconds
iox_nanosleep(&req, NULL);
```

---

### iox_gettimeofday

Get time.

```c
int iox_gettimeofday(struct timeval *tv, struct timezone *tz);
```

**Example**:
```c
struct timeval tv;
iox_gettimeofday(&tv, NULL);
printf("Seconds: %ld\n", tv.tv_sec);
```

---

### iox_time

Get time in seconds.

```c
time_t iox_time(time_t *tloc);
```

**Example**:
```c
time_t now = iox_time(NULL);
```

---

### iox_clock_gettime

Get time from a clock.

```c
int iox_clock_gettime(clockid_t clk_id, struct timespec *tp);
```

**Clock IDs**:
- `CLOCK_REALTIME`
- `CLOCK_MONOTONIC`
- `CLOCK_PROCESS_CPUTIME_ID`

---

## Identity Functions

### iox_getuid

Get real user ID.

```c
uid_t iox_getuid(void);
```

---

### iox_geteuid

Get effective user ID.

```c
uid_t iox_geteuid(void);
```

---

### iox_getgid

Get real group ID.

```c
gid_t iox_getgid(void);
```

---

### iox_getegid

Get effective group ID.

```c
gid_t iox_getegid(void);
```

---

### iox_setuid

Set user ID.

```c
int iox_setuid(uid_t uid);
```

**Note**: Returns EPERM on iOS (not permitted)

---

### iox_seteuid

Set effective user ID.

```c
int iox_seteuid(uid_t uid);
```

---

### iox_setgid

Set group ID.

```c
int iox_setgid(gid_t gid);
```

---

### iox_setegid

Set effective group ID.

```c
int iox_setegid(gid_t gid);
```

---

### iox_getgroups

Get supplementary group IDs.

```c
int iox_getgroups(int size, gid_t list[]);
```

---

### iox_setgroups

Set supplementary group IDs.

```c
int iox_setgroups(size_t size, const gid_t *list);
```

---

## Environment Functions

### iox_getenv

Get environment variable.

```c
char *iox_getenv(const char *name);
```

**Example**:
```c
char *path = iox_getenv("PATH");
if (path) {
    printf("PATH=%s\n", path);
}
```

---

### iox_setenv

Set environment variable.

```c
int iox_setenv(const char *name, const char *value, int overwrite);
```

**Example**:
```c
iox_setenv("MYVAR", "myvalue", 1);  // Overwrite if exists
```

---

### iox_unsetenv

Unset environment variable.

```c
int iox_unsetenv(const char *name);
```

---

### iox_clearenv

Clear environment.

```c
int iox_clearenv(void);
```

---

### iox_environ

Get environment pointer.

```c
char **iox_environ(void);
```

---

## Network Functions

### iox_socket

Create a socket.

```c
int iox_socket(int domain, int type, int protocol);
```

**Example**:
```c
int sock = iox_socket(AF_INET, SOCK_STREAM, 0);
```

---

### iox_connect

Connect to a socket.

```c
int iox_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

---

### iox_bind

Bind a socket.

```c
int iox_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

---

### iox_listen

Listen for connections.

```c
int iox_listen(int sockfd, int backlog);
```

---

### iox_accept

Accept a connection.

```c
int iox_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

---

### iox_send / iox_recv

Send/receive data.

```c
ssize_t iox_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t iox_recv(int sockfd, void *buf, size_t len, int flags);
```

---

### iox_sendto / iox_recvfrom

Send/receive datagrams.

```c
ssize_t iox_sendto(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t iox_recvfrom(int sockfd, void *buf, size_t len, int flags,
                     struct sockaddr *src_addr, socklen_t *addrlen);
```

---

### iox_shutdown

Shutdown socket.

```c
int iox_shutdown(int sockfd, int how);
```

---

### iox_setsockopt / iox_getsockopt

Socket options.

```c
int iox_setsockopt(int sockfd, int level, int optname,
                   const void *optval, socklen_t optlen);
int iox_getsockopt(int sockfd, int level, int optname,
                   void *optval, socklen_t *optlen);
```

---

## IOX-Specific Extensions

### Initialization and Cleanup

```c
int iox_init(const iox_config_t *config);
void iox_cleanup(void);
```

**Example**:
```c
iox_config_t config = {
    .debug_enabled = true,
    .home_directory = "/home/user",
    .max_processes = 256
};
iox_init(&config);

// ... use iox syscalls ...

iox_cleanup();
```

---

### Version Information

```c
const char *iox_version(void);
int iox_is_initialized(void);
```

**Example**:
```c
printf("iXland version: %s\n", iox_version());
// Output: iXland version: 1.0.0
```

---

### System Information

```c
iox_sys_info_t iox_get_sys_info(void);
```

**Example**:
```c
iox_sys_info_t info = iox_get_sys_info();
printf("OS: %s %s\n", info.sysname, info.release);
printf("Memory: %llu MB total\n", info.total_memory / (1024 * 1024));
```

---

### Process Information

```c
iox_proc_info_t *iox_get_proc_info(pid_t pid);
void iox_free_proc_info(iox_proc_info_t *info);
```

**Example**:
```c
iox_proc_info_t *info = iox_get_proc_info(iox_getpid());
if (info) {
    printf("Process: %s (PID %d)\n", info->name, info->pid);
    printf("State: %c\n", info->state);
    iox_free_proc_info(info);
}
```

---

### Statistics

```c
void iox_get_syscall_stats(iox_syscall_t syscall, iox_syscall_stat_t *stats);
void iox_get_memory_stats(iox_memory_stat_t *stats);
void iox_get_process_stats(iox_process_stat_t *stats);
```

**Example**:
```c
iox_syscall_stat_t stats;
iox_get_syscall_stats(IOX_SYSCALL_FORK, &stats);
printf("fork() called %llu times\n", stats.calls);
```

---

### Callbacks

```c
void iox_set_proc_callback(iox_proc_callback_t callback, void *userdata);
void iox_set_thread_callback(iox_thread_callback_t callback, void *userdata);
void iox_set_file_callback(iox_file_callback_t callback, void *userdata);
```

**Example**:
```c
void on_proc_change(pid_t pid, int event, void *userdata) {
    if (event & IOX_PROC_CREATE) {
        printf("Process %d created\n", pid);
    }
    if (event & IOX_PROC_EXIT) {
        printf("Process %d exited\n", pid);
    }
}

iox_set_proc_callback(on_proc_change, NULL);
```

---

### Debugging

```c
void iox_debug_dump(FILE *stream);
void iox_set_tracing(int enabled);
```

**Example**:
```c
iox_set_tracing(1);  // Enable syscall tracing
// ... syscalls are logged ...
iox_set_tracing(0);  // Disable

iox_debug_dump(stderr);  // Dump internal state
```

---

## Error Handling

### iox_strerror

Get error description.

```c
const char *iox_strerror(int errnum);
```

**Example**:
```c
int fd = iox_open("/nonexistent", O_RDONLY);
if (fd < 0) {
    printf("Error: %s\n", iox_strerror(errno));
    // Output: Error: No such file or directory
}
```

---

### iox_perror

Print error message.

```c
void iox_perror(const char *s);
```

**Example**:
```c
if (iox_open("/file", O_RDONLY) < 0) {
    iox_perror("open");
    // Output: open: No such file or directory
}
```

---

## Summary

The iXland API provides:

- **100+ syscalls** covering process, file, signal, memory, and network operations
- **POSIX compatibility** with Linux-compatible semantics
- **Clean naming** with `iox_` prefix for all functions
- **iOS-specific extensions** for initialization, debugging, and introspection
- **Type safety** with proper struct definitions
- **Error handling** using standard errno conventions

All functions are thread-safe where applicable and use virtual PIDs starting at 1000.

---

*End of API Reference*

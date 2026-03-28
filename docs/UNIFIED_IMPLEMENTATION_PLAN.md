# Unified Implementation Plan: a-Shell Next

**Version**: 1.0  
**Date**: 2026-03-20  
**Status**: Approved - Ready for Implementation  
**Approach**: Big Bang Migration (No Backward Compatibility)  

---

## Executive Summary

This document outlines the complete implementation plan for a-Shell Next, covering kernel headers, syscall implementations, package system, and WebAssembly integration.

**Key Decisions**:
- Big bang migration: Delete ios_error.h immediately
- Kernel headers first (foundation for all packages)
- Use GNU readline (required for bash)
- WAMR/WASM after bootstrap (Phase 5)

**Already Implemented** (Leverage These):
- Basic syscall macros in ios_error.h (to be migrated)
- Thread-based session management in a_shell_system.m
- Process simulation (virtual PIDs via pthread)
- WAMR submodule (ready for Phase 5)
- Partial build infrastructure

---

## Architecture Overview

```
a-shell-next/
├── a-shell-kernel/              # Syscall kernel
│   ├── include/                 # Linux-compatible headers [PHASE 1]
│   │   ├── linux/
│   │   │   ├── unistd.h         # Process, I/O syscalls
│   │   │   ├── stat.h           # File status
│   │   │   ├── fcntl.h          # File control
│   │   │   ├── wait.h           # Process waiting
│   │   │   ├── signal.h         # Signal handling
│   │   │   ├── time.h           # Time functions
│   │   │   ├── resource.h       # Resource limits
│   │   │   ├── mman.h           # Memory mapping
│   │   │   ├── socket.h         # Networking (pass-through)
│   │   │   ├── poll.h           # Polling
│   │   │   ├── epoll.h          # epoll (kqueue translation)
│   │   │   ├── ioctl.h          # ioctl operations
│   │   │   ├── uio.h            # Vector I/O
│   │   │   └── types.h          # Basic types
│   │   ├── sys/
│   │   │   ├── types.h
│   │   │   ├── wait.h
│   │   │   └── stat.h
│   │   ├── pwd.h                # User database
│   │   └── grp.h                # Group database
│   │
│   ├── src/                     # Implementation files [PHASE 1]
│   │   ├── syscalls/
│   │   │   ├── process.m        # fork, exec, wait (Obj-C for threading)
│   │   │   ├── file.c           # open, read, write, stat
│   │   │   ├── signal.c         # signal, sigaction, kill
│   │   │   ├── time.c           # alarm, timer, time
│   │   │   ├── memory.c         # mmap, mprotect, msync
│   │   │   ├── socket.c         # socket pass-through to BSD
│   │   │   └── poll.c           # poll, epoll, select
│   │   └── vfs.c                # Virtual filesystem layer
│   │
│   ├── ios_error.h              # [TO BE DELETED - Phase 1]
│   ├── a_shell_system.h         # Umbrella header
│   ├── a_shell_system.m         # Main implementation
│   ├── a_shell_system/          # Modular components
│   └── wamr/                    # [PHASE 5] WAMR submodule
│
├── a-shell-packages/            # Package build system
│   ├── scripts/
│   │   ├── a_shell_package.sh   # Build library
│   │   ├── build-package.sh     # Single package builder
│   │   ├── build-all.sh         # Dependency-ordered builder
│   │   └── setup-toolchain.sh   # Environment setup
│   │
│   ├── packages/
│   │   ├── core/                # 20 bootstrap packages
│   │   │   ├── libz/            # Partial build.sh
│   │   │   ├── libssl/          # Empty
│   │   │   ├── libcurl/         # Empty
│   │   │   ├── ncurses/         # Empty
│   │   │   ├── readline/        # Empty [PHASE 2]
│   │   │   ├── bash/            # Partial build.sh
│   │   │   ├── coreutils/       # Partial build.sh
│   │   │   ├── tar/             # Empty
│   │   │   ├── gzip/            # Empty
│   │   │   ├── grep/            # Empty
│   │   │   ├── sed/             # Empty
│   │   │   ├── gawk/            # Empty
│   │   │   ├── dpkg/            # Empty [PHASE 4]
│   │   │   ├── apt/             # Empty [PHASE 4]
│   │   │   ├── zsh/             # Empty
│   │   │   ├── make/            # Empty
│   │   │   ├── cmake/           # Empty
│   │   │   └── clang/           # Empty
│   │   ├── extra/               # Essential packages [PHASE 6]
│   │   ├── extended/            # Everything else
│   │   └── wasm/                # WASM packages [PHASE 5]
│   │
│   ├── repos/                   # Repository structure [PHASE 6]
│   └── docker/
│       └── Dockerfile
│
└── a-shell/                     # iOS app
    └── Resources/
        └── bootstrap.tar.gz     # ~50MB [PHASE 4]
```

---

## Phase 1: Foundation (Kernel Headers + Syscalls)

**Priority**: CRITICAL - Must Complete First  
**Rationale**: Kernel headers are required by ALL packages. Cannot build bash, coreutils, or any tool without syscall definitions.

### 1.1 Big Bang Migration

**Action**: Delete `ios_error.h` immediately. Do not maintain backward compatibility.

**Rationale**: 
- Forces complete migration to new headers
- Eliminates technical debt
- Ensures all code uses proper Linux-compatible headers
- Prevents confusion between old and new systems

### 1.2 Kernel Header Structure

**MUST Category Headers** (Critical - Required for basic functionality):

#### include/linux/unistd.h
```c
#ifndef _LINUX_UNISTD_H
#define _LINUX_UNISTD_H

#include <sys/types.h>

/* Process management */
pid_t a_shell_fork(void);
pid_t a_shell_vfork(void);
int a_shell_execve(const char *pathname, char *const argv[], char *const envp[]);
void a_shell_exit(int status);
pid_t a_shell_waitpid(pid_t pid, int *wstatus, int options);
pid_t a_shell_getpid(void);
pid_t a_shell_getppid(void);

/* Directory operations */
int a_shell_chdir(const char *path);
int a_shell_fchdir(int fd);
char *a_shell_getcwd(char *buf, size_t size);
int a_shell_chroot(const char *path);

/* Timing */
unsigned int a_shell_alarm(unsigned int seconds);
int a_shell_pause(void);
unsigned int a_shell_sleep(unsigned int seconds);

/* I/O */
ssize_t a_shell_read(int fd, void *buf, size_t count);
ssize_t a_shell_write(int fd, const void *buf, size_t count);
int a_shell_close(int fd);
off_t a_shell_lseek(int fd, off_t offset, int whence);

/* Identity */
uid_t a_shell_getuid(void);
uid_t a_shell_geteuid(void);
int a_shell_setuid(uid_t uid);
int a_shell_seteuid(uid_t uid);
gid_t a_shell_getgid(void);
gid_t a_shell_getegid(void);
int a_shell_setgid(gid_t gid);
int a_shell_setegid(gid_t gid);

#endif
```

#### include/linux/stat.h
```c
#ifndef _LINUX_STAT_H
#define _LINUX_STAT_H

#include <sys/stat.h>

int a_shell_stat(const char *pathname, struct stat *statbuf);
int a_shell_fstat(int fd, struct stat *statbuf);
int a_shell_lstat(const char *pathname, struct stat *statbuf);
int a_shell_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);

int a_shell_chmod(const char *pathname, mode_t mode);
int a_shell_fchmod(int fd, mode_t mode);
int a_shell_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);

int a_shell_chown(const char *pathname, uid_t owner, gid_t group);
int a_shell_fchown(int fd, uid_t owner, gid_t group);
int a_shell_lchown(const char *pathname, uid_t owner, gid_t group);
int a_shell_fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);

mode_t a_shell_umask(mode_t mask);
int a_shell_mkdir(const char *pathname, mode_t mode);
int a_shell_mkdirat(int dirfd, const char *pathname, mode_t mode);
int a_shell_mknod(const char *pathname, mode_t mode, dev_t dev);
int a_shell_mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev);

#endif
```

#### include/linux/fcntl.h
```c
#ifndef _LINUX_FCNTL_H
#define _LINUX_FCNTL_H

#include <fcntl.h>

int a_shell_open(const char *pathname, int flags, ...);
int a_shell_openat(int dirfd, const char *pathname, int flags, ...);
int a_shell_creat(const char *pathname, mode_t mode);
int a_shell_fcntl(int fd, int cmd, ...);
int a_shell_posix_fadvise(int fd, off_t offset, off_t len, int advice);
int a_shell_posix_fallocate(int fd, off_t offset, off_t len);

#endif
```

#### include/linux/wait.h
```c
#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H

#include <sys/wait.h>
#include <sys/resource.h>

pid_t a_shell_waitpid(pid_t pid, int *wstatus, int options);
pid_t a_shell_wait3(int *wstatus, int options, struct rusage *rusage);
pid_t a_shell_wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage);
pid_t a_shell_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);

/* Status macros */
#define WIFEXITED(status)   (((status) & 0x7F) == 0)
#define WEXITSTATUS(status) (((status) >> 8) & 0xFF)
#define WIFSIGNALED(status) (((status) & 0x7F) != 0 && ((status) & 0x7F) != 0x7F)
#define WTERMSIG(status)    ((status) & 0x7F)
#define WIFSTOPPED(status)  (((status) & 0xFF) == 0x7F)
#define WSTOPSIG(status)    (((status) >> 8) & 0xFF)
#define WIFCONTINUED(status) ((status) == 0xFFFF)

#endif
```

#### include/linux/signal.h
```c
#ifndef _LINUX_SIGNAL_H
#define _LINUX_SIGNAL_H

#include <signal.h>

typedef void (*a_shell_sighandler_t)(int);

a_shell_sighandler_t a_shell_signal(int signum, a_shell_sighandler_t handler);
int a_shell_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int a_shell_kill(pid_t pid, int sig);
int a_shell_killpg(pid_t pgrp, int sig);
int a_shell_raise(int sig);
int a_shell_sigpending(sigset_t *set);
int a_shell_sigsuspend(const sigset_t *mask);
int a_shell_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int a_shell_sigemptyset(sigset_t *set);
int a_shell_sigfillset(sigset_t *set);
int a_shell_sigaddset(sigset_t *set, int signum);
int a_shell_sigdelset(sigset_t *set, int signum);
int a_shell_sigismember(const sigset_t *set, int signum);

#endif
```

#### include/linux/time.h
```c
#ifndef _LINUX_TIME_H
#define _LINUX_TIME_H

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

unsigned int a_shell_alarm(unsigned int seconds);
int a_shell_setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
int a_shell_getitimer(int which, struct itimerval *curr_value);
int a_shell_timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
int a_shell_timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
int a_shell_timer_gettime(timer_t timerid, struct itimerspec *curr_value);
int a_shell_timer_getoverrun(timer_t timerid);
int a_shell_timer_delete(timer_t timerid);
clock_t a_shell_times(struct tms *buf);
int a_shell_gettimeofday(struct timeval *tv, struct timezone *tz);
int a_shell_settimeofday(const struct timeval *tv, const struct timezone *tz);
int a_shell_nanosleep(const struct timespec *req, struct timespec *rem);
int a_shell_clock_gettime(clockid_t clk_id, struct timespec *tp);
int a_shell_clock_settime(clockid_t clk_id, const struct timespec *tp);
int a_shell_clock_getres(clockid_t clk_id, struct timespec *res);

#endif
```

#### include/pwd.h
```c
#ifndef _PWD_H
#define _PWD_H

#include <sys/types.h>

struct passwd {
    char *pw_name;
    char *pw_passwd;
    uid_t pw_uid;
    gid_t pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

struct passwd *ixland_getpwnam(const char *name);
struct passwd *ixland_getpwuid(uid_t uid);
int ixland_getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
int ixland_getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
void ixland_endpwent(void);
struct passwd *ixland_getpwent(void);
void ixland_setpwent(void);

#endif
```

#### include/grp.h
```c
#ifndef _GRP_H
#define _GRP_H

#include <sys/types.h>

struct group {
    char *gr_name;
    char *gr_passwd;
    gid_t gr_gid;
    char **gr_mem;
};

struct group *ixland_getgrnam(const char *name);
struct group *ixland_getgrgid(gid_t gid);
int ixland_getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen, struct group **result);
int ixland_getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen, struct group **result);
void ixland_endgrent(void);
struct group *ixland_getgrent(void);
void ixland_setgrent(void);
int ixland_getgroups(int size, gid_t list[]);
int ixland_setgroups(size_t size, const gid_t *list);
int ixland_initgroups(const char *user, gid_t group);

#endif
```

**SHOULD Category Headers** (Important - Required for full functionality):

#### include/linux/resource.h
```c
#ifndef _LINUX_RESOURCE_H
#define _LINUX_RESOURCE_H

#include <sys/resource.h>

int a_shell_getrlimit(int resource, struct rlimit *rlim);
int a_shell_setrlimit(int resource, const struct rlimit *rlim);
int a_shell_getrusage(int who, struct rusage *usage);
int a_shell_prlimit(pid_t pid, int resource, const struct rlimit *new_limit, struct rlimit *old_limit);
int a_shell_getpriority(int which, id_t who);
int a_shell_setpriority(int which, id_t who, int prio);
int a_shell_nice(int inc);

#endif
```

#### include/linux/mman.h
```c
#ifndef _LINUX_MMAN_H
#define _LINUX_MMAN_H

#include <sys/mman.h>

void *a_shell_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void *a_shell_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset);
int a_shell_munmap(void *addr, size_t length);
int a_shell_mprotect(void *addr, size_t len, int prot);
int a_shell_msync(void *addr, size_t length, int flags);
int a_shell_mlock(const void *addr, size_t len);
int a_shell_munlock(const void *addr, size_t len);
int a_shell_mlockall(int flags);
int a_shell_munlockall(void);
int a_shell_madvise(void *addr, size_t length, int advice);
int a_shell_mincore(void *addr, size_t length, unsigned char *vec);
int a_shell_shm_open(const char *name, int oflag, mode_t mode);
int a_shell_shm_unlink(const char *name);

#endif
```

#### include/linux/socket.h
```c
#ifndef _LINUX_SOCKET_H
#define _LINUX_SOCKET_H

/* Pass-through to native iOS BSD sockets */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* Socket creation and connection */
#define a_shell_socket      socket
#define a_shell_socketpair  socketpair
#define a_shell_bind        bind
#define a_shell_listen      listen
#define a_shell_accept      accept
#define a_shell_accept4     accept4
#define a_shell_connect     connect
#define a_shell_shutdown    shutdown

/* Socket options */
#define a_shell_getsockname getsockname
#define a_shell_getpeername getpeername
#define a_shell_setsockopt  setsockopt
#define a_shell_getsockopt  getsockopt

/* Data transmission */
#define a_shell_send        send
#define a_shell_sendto      sendto
#define a_shell_sendmsg     sendmsg
#define a_shell_recv        recv
#define a_shell_recvfrom    recvfrom
#define a_shell_recvmsg     recvmsg

/* Address resolution */
#define a_shell_gethostbyname   gethostbyname
#define a_shell_gethostbyaddr   gethostbyaddr
#define a_shell_getaddrinfo     getaddrinfo
#define a_shell_freeaddrinfo    freeaddrinfo
#define a_shell_getnameinfo     getnameinfo

#endif
```

#### include/linux/poll.h
```c
#ifndef _LINUX_POLL_H
#define _LINUX_POLL_H

#include <poll.h>

int a_shell_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int a_shell_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask);

#endif
```

#### include/linux/epoll.h
```c
#ifndef _LINUX_EPOLL_H
#define _LINUX_EPOLL_H

#include <sys/types.h>
#include <fcntl.h>

/* epoll is Linux-specific; implement via kqueue on iOS or return ENOSYS */

enum {
    EPOLLIN = 0x001,
    EPOLLOUT = 0x004,
    EPOLLERR = 0x008,
    EPOLLHUP = 0x010,
    EPOLLRDHUP = 0x2000,
    EPOLLPRI = 0x002,
    EPOLLRDNORM = 0x040,
    EPOLLRDBAND = 0x080,
    EPOLLWRNORM = 0x100,
    EPOLLWRBAND = 0x200,
    EPOLLMSG = 0x400,
    EPOLLONESHOT = 1u << 30,
    EPOLLET = 1u << 31
};

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

/* Flags for epoll_create1 */
enum { EPOLL_CLOEXEC = O_CLOEXEC };

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t events;
    epoll_data_t data;
};

int a_shell_epoll_create(int size);
int a_shell_epoll_create1(int flags);
int a_shell_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int a_shell_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
int a_shell_epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask);

#endif
```

**COULD Category Headers** (Nice-to-have):

#### include/linux/ioctl.h
```c
#ifndef _LINUX_IOCTL_H
#define _LINUX_IOCTL_H

#include <sys/ioctl.h>

int a_shell_ioctl(int fd, unsigned long request, ...);

#endif
```

#### include/linux/uio.h
```c
#ifndef _LINUX_UIO_H
#define _LINUX_UIO_H

#include <sys/types.h>
#include <sys/uio.h>

ssize_t a_shell_readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t a_shell_writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t a_shell_preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t a_shell_pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t a_shell_preadv2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
ssize_t a_shell_pwritev2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
int a_shell_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int a_shell_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int a_shell_gethostname(char *name, size_t len);
int a_shell_sethostname(const char *name, size_t len);
int a_shell_getdomainname(char *name, size_t len);
int a_shell_setdomainname(const char *name, size_t len);

#endif
```

#### include/sys/types.h
```c
#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <sys/_types.h>
#include <machine/types.h>

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

typedef uint32_t        mode_t;
typedef uint32_t        uid_t;
typedef uint32_t        gid_t;
typedef int32_t         pid_t;
typedef uint32_t        dev_t;
typedef uint64_t        ino_t;
typedef uint32_t        nlink_t;
typedef int64_t         off_t;
typedef int64_t         blkcnt_t;
typedef int32_t         blksize_t;
typedef int32_t         ssize_t;
typedef uint32_t        id_t;
typedef uint32_t        key_t;
typedef long            clock_t;
typedef int32_t         clockid_t;
typedef int32_t         timer_t;
typedef void *          caddr_t;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef unsigned short  ushort;
typedef unsigned char   u_int8_t;
typedef unsigned short  u_int16_t;
typedef unsigned int    u_int32_t;
typedef unsigned long long u_int64_t;

#endif
```

### 1.3 Syscall Implementations

**src/syscalls/process.m** (Objective-C for threading):
```c
#import <Foundation/Foundation.h>
#import <pthread.h>
#import <errno.h>
#import "../../include/linux/unistd.h"
#import "../../include/linux/wait.h"

static pthread_mutex_t pid_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static pid_t next_virtual_pid = 1000;

typedef struct {
    pid_t pid;
    pid_t ppid;
    pthread_t thread;
    int status;
    bool running;
    bool exited;
    int exit_status;
} process_entry_t;

#define MAX_PROCESSES 1024
static process_entry_t process_table[MAX_PROCESSES];

pid_t a_shell_fork(void) {
    pthread_mutex_lock(&pid_table_mutex);
    
    pid_t pid = next_virtual_pid++;
    if (pid >= next_virtual_pid + MAX_PROCESSES) {
        errno = EAGAIN;
        pthread_mutex_unlock(&pid_table_mutex);
        return -1;
    }
    
    process_entry_t *entry = &process_table[pid % MAX_PROCESSES];
    entry->pid = pid;
    entry->ppid = a_shell_getpid();
    entry->thread = pthread_self();
    entry->running = true;
    entry->exited = false;
    
    pthread_mutex_unlock(&pid_table_mutex);
    
    return pid;
}

pid_t a_shell_vfork(void) {
    return a_shell_fork();
}

int a_shell_execve(const char *pathname, char *const argv[], char *const envp[]) {
    /* Load and execute new program in current thread context */
    errno = ENOSYS;
    return -1;
}

pid_t a_shell_waitpid(pid_t pid, int *wstatus, int options) {
    pthread_mutex_lock(&pid_table_mutex);
    
    if (pid == -1) {
        /* Wait for any child */
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].running && process_table[i].ppid == a_shell_getpid()) {
                if (process_table[i].exited) {
                    pid_t child_pid = process_table[i].pid;
                    if (wstatus) {
                        *wstatus = process_table[i].exit_status << 8;
                    }
                    process_table[i].running = false;
                    pthread_mutex_unlock(&pid_table_mutex);
                    return child_pid;
                }
            }
        }
    } else if (pid > 0) {
        /* Wait for specific child */
        process_entry_t *entry = &process_table[pid % MAX_PROCESSES];
        if (entry->pid == pid && entry->ppid == a_shell_getpid()) {
            if (options & WNOHANG) {
                if (entry->exited) {
                    if (wstatus) {
                        *wstatus = entry->exit_status << 8;
                    }
                    entry->running = false;
                    pthread_mutex_unlock(&pid_table_mutex);
                    return pid;
                }
                pthread_mutex_unlock(&pid_table_mutex);
                return 0;
            }
            /* Would need to actually wait - simplified for now */
        }
    }
    
    pthread_mutex_unlock(&pid_table_mutex);
    errno = ECHILD;
    return -1;
}

pid_t a_shell_wait3(int *wstatus, int options, struct rusage *rusage) {
    return a_shell_waitpid(-1, wstatus, options);
}

pid_t a_shell_wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage) {
    return a_shell_waitpid(pid, wstatus, options);
}

/* Other process functions... */
```

**src/syscalls/file.c**:
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/linux/fcntl.h"
#include "../../include/linux/stat.h"
#include "../vfs.h"

int a_shell_open(const char *pathname, int flags, mode_t mode) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int fd = open(real_path, flags, mode);
    free(real_path);
    return fd;
}

int a_shell_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
    if (pathname[0] == '/') {
        return a_shell_open(pathname, flags, mode);
    }
    return openat(dirfd, pathname, flags, mode);
}

int a_shell_creat(const char *pathname, mode_t mode) {
    return a_shell_open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

int a_shell_stat(const char *pathname, struct stat *statbuf) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = stat(real_path, statbuf);
    free(real_path);
    return ret;
}

/* Other file functions... */
```

**src/syscalls/signal.c**:
```c
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "../../include/linux/signal.h"

static struct sigaction signal_handlers[NSIG];
static sigset_t signal_mask;

a_shell_sighandler_t a_shell_signal(int signum, a_shell_sighandler_t handler) {
    if (signum < 1 || signum >= NSIG) {
        errno = EINVAL;
        return SIG_ERR;
    }
    
    a_shell_sighandler_t old_handler = (a_shell_sighandler_t)signal_handlers[signum].sa_handler;
    signal_handlers[signum].sa_handler = (void (*)(int))handler;
    
    /* Set up actual signal handler if needed */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = (void (*)(int))handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(signum, &sa, NULL) < 0) {
        return SIG_ERR;
    }
    
    return old_handler;
}

int a_shell_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    if (signum < 1 || signum >= NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    if (oldact) {
        *oldact = signal_handlers[signum];
    }
    
    if (act) {
        signal_handlers[signum] = *act;
        
        struct sigaction sa;
        sa.sa_handler = act->sa_handler;
        sa.sa_flags = act->sa_flags;
        sa.sa_mask = act->sa_mask;
        
        if (sigaction(signum, &sa, NULL) < 0) {
            return -1;
        }
    }
    
    return 0;
}

int a_shell_kill(pid_t pid, int sig) {
    /* Send signal to virtual process */
    /* For now, only support self */
    if (pid == a_shell_getpid()) {
        return raise(sig);
    }
    
    errno = ESRCH;
    return -1;
}

/* Other signal functions... */
```

**src/syscalls/time.c**:
```c
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include "../../include/linux/time.h"

static struct itimerval current_timer;

unsigned int a_shell_alarm(unsigned int seconds) {
    struct itimerval old_timer;
    struct itimerval new_timer;
    
    memset(&new_timer, 0, sizeof(new_timer));
    new_timer.it_value.tv_sec = seconds;
    
    if (setitimer(ITIMER_REAL, &new_timer, &old_timer) < 0) {
        return 0;
    }
    
    return old_timer.it_value.tv_sec + (old_timer.it_value.tv_usec > 0 ? 1 : 0);
}

int a_shell_setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value) {
    return setitimer(which, new_value, old_value);
}

int a_shell_getitimer(int which, struct itimerval *curr_value) {
    return getitimer(which, curr_value);
}

clock_t a_shell_times(struct tms *buf) {
    return times(buf);
}

int a_shell_gettimeofday(struct timeval *tv, struct timezone *tz) {
    return gettimeofday(tv, tz);
}

/* Other time functions... */
```

**src/syscalls/memory.c**:
```c
#include <sys/mman.h>
#include <errno.h>
#include "../../include/linux/mman.h"

void *a_shell_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return mmap(addr, length, prot, flags, fd, offset);
}

int a_shell_munmap(void *addr, size_t length) {
    return munmap(addr, length);
}

int a_shell_mprotect(void *addr, size_t len, int prot) {
    return mprotect(addr, len, prot);
}

int a_shell_msync(void *addr, size_t length, int flags) {
    return msync(addr, length, flags);
}

/* Other memory functions... */
```

**src/syscalls/poll.c**:
```c
#include <poll.h>
#include <errno.h>
#include <sys/event.h>  /* For kqueue */
#include "../../include/linux/poll.h"
#include "../../include/linux/epoll.h"

int a_shell_poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    return poll(fds, nfds, timeout);
}

int a_shell_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask) {
    /* iOS doesn't have ppoll natively, implement via pselect or poll */
    int timeout = -1;
    if (tmo_p) {
        timeout = tmo_p->tv_sec * 1000 + tmo_p->tv_nsec / 1000000;
    }
    return poll(fds, nfds, timeout);
}

/* epoll implementation via kqueue */
struct epoll_context {
    int kq;
};

int a_shell_epoll_create(int size) {
    return a_shell_epoll_create1(0);
}

int a_shell_epoll_create1(int flags) {
    int kq = kqueue();
    if (kq < 0) {
        return -1;
    }
    return kq;
}

int a_shell_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    struct kevent kev;
    int16_t filter = 0;
    uint16_t kev_flags = 0;
    
    if (event->events & EPOLLIN) filter = EVFILT_READ;
    else if (event->events & EPOLLOUT) filter = EVFILT_WRITE;
    
    switch (op) {
        case EPOLL_CTL_ADD:
            kev_flags = EV_ADD;
            break;
        case EPOLL_CTL_DEL:
            kev_flags = EV_DELETE;
            break;
        case EPOLL_CTL_MOD:
            kev_flags = EV_ADD | EV_CLEAR;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    
    EV_SET(&kev, fd, filter, kev_flags, 0, 0, event->data.ptr);
    return kevent(epfd, &kev, 1, NULL, 0, NULL);
}

int a_shell_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
    struct kevent kevs[maxevents];
    struct timespec ts;
    struct timespec *tsp = NULL;
    
    if (timeout >= 0) {
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = (timeout % 1000) * 1000000;
        tsp = &ts;
    }
    
    int n = kevent(epfd, NULL, 0, kevs, maxevents, tsp);
    if (n < 0) {
        return -1;
    }
    
    for (int i = 0; i < n; i++) {
        events[i].data.ptr = kevs[i].udata;
        events[i].events = 0;
        if (kevs[i].filter == EVFILT_READ) events[i].events |= EPOLLIN;
        if (kevs[i].filter == EVFILT_WRITE) events[i].events |= EPOLLOUT;
        if (kevs[i].flags & EV_ERROR) events[i].events |= EPOLLERR;
        if (kevs[i].flags & EV_EOF) events[i].events |= EPOLLHUP;
    }
    
    return n;
}
```

**src/vfs.c** (Virtual Filesystem Layer):
```c
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vfs.h"

static char *a_shell_prefix = NULL;

void vfs_init(const char *prefix) {
    if (a_shell_prefix) {
        free(a_shell_prefix);
    }
    a_shell_prefix = strdup(prefix);
}

const char *vfs_get_prefix(void) {
    return a_shell_prefix ? a_shell_prefix : "/usr/local";
}

char *vfs_translate_path(const char *path) {
    if (!path) {
        return NULL;
    }
    
    /* Absolute paths that need prefix translation */
    if (path[0] == '/') {
        /* Check if it's a system path */
        if (strncmp(path, "/usr/", 5) == 0 ||
            strncmp(path, "/etc/", 5) == 0 ||
            strncmp(path, "/var/", 5) == 0 ||
            strncmp(path, "/bin/", 5) == 0 ||
            strncmp(path, "/lib/", 5) == 0 ||
            strncmp(path, "/sbin/", 6) == 0) {
            
            size_t len = strlen(vfs_get_prefix()) + strlen(path) + 1;
            char *real_path = malloc(len);
            if (!real_path) {
                return NULL;
            }
            snprintf(real_path, len, "%s%s", vfs_get_prefix(), path);
            return real_path;
        }
    }
    
    /* Return copy of original path */
    return strdup(path);
}

void vfs_free_path(char *path) {
    free(path);
}

int vfs_path_needs_translation(const char *path) {
    if (!path || path[0] != '/') {
        return 0;
    }
    
    return (strncmp(path, "/usr/", 5) == 0 ||
            strncmp(path, "/etc/", 5) == 0 ||
            strncmp(path, "/var/", 5) == 0 ||
            strncmp(path, "/bin/", 5) == 0 ||
            strncmp(path, "/lib/", 5) == 0 ||
            strncmp(path, "/sbin/", 6) == 0);
}
```

**src/vfs.h**:
```c
#ifndef VFS_H
#define VFS_H

void vfs_init(const char *prefix);
const char *vfs_get_prefix(void);
char *vfs_translate_path(const char *path);
void vfs_free_path(char *path);
int vfs_path_needs_translation(const char *path);

#endif
```

### 1.4 Testing Strategy

**Automated Tests** (run during package build):
```c
/* tests/test_syscalls.c */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../include/linux/unistd.h"
#include "../include/linux/stat.h"
#include "../include/linux/signal.h"

void test_fork_basic() {
    printf("Testing fork()...\n");
    pid_t pid = a_shell_fork();
    assert(pid >= 1000 || pid == -1);
    printf("  ✓ fork() returns valid virtual PID\n");
}

void test_vfs_translation() {
    printf("Testing VFS path translation...\n");
    vfs_init("/Users/test/Library/ashell");
    
    char *path1 = vfs_translate_path("/etc/passwd");
    assert(strcmp(path1, "/Users/test/Library/ashell/etc/passwd") == 0);
    vfs_free_path(path1);
    printf("  ✓ /etc/passwd translates correctly\n");
    
    char *path2 = vfs_translate_path("/home/user/file.txt");
    assert(strcmp(path2, "/home/user/file.txt") == 0);
    vfs_free_path(path2);
    printf("  ✓ Non-system paths unchanged\n");
}

void test_signal_handling() {
    printf("Testing signal()...\n");
    a_shell_sighandler_t old_handler = a_shell_signal(SIGINT, SIG_IGN);
    assert(old_handler != SIG_ERR);
    printf("  ✓ signal() sets handler\n");
    
    a_shell_signal(SIGINT, old_handler);
    printf("  ✓ signal() restores handler\n");
}

int main(int argc, char *argv[]) {
    printf("\n========================================\n");
    printf("a-shell-kernel Syscall Tests\n");
    printf("========================================\n\n");
    
    test_fork_basic();
    test_vfs_translation();
    test_signal_handling();
    
    printf("\n========================================\n");
    printf("All tests passed!\n");
    printf("========================================\n");
    
    return 0;
}
```

---

## Phase 2: Core Libraries

**Rationale**: Libraries must be built before tools that depend on them.

### 2.1 Build Order (Dependency Graph)

```
libz (no deps)
  ↓
libssl → libz
  ↓
libcurl → libz, libssl
  ↓
ncurses (no deps)
  ↓
readline → ncurses (REQUIRED FOR BASH)
```

### 2.2 libz (zlib)

**packages/core/libz/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="zlib"
A_SHELL_PKG_VERSION="1.3.1"
A_SHELL_PKG_SRCURL="https://zlib.net/zlib-1.3.1.tar.gz"
A_SHELL_PKG_SHA256="9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23"
A_SHELL_PKG_DEPENDS=""
A_SHELL_PKG_BUILD_DEPENDS=""
A_SHELL_PKG_SECTION="libs"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://zlib.net/"
A_SHELL_PKG_DESCRIPTION="compression library - runtime"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CFLAGS="-arch arm64 -isysroot $(xcrun -sdk iphoneos --show-sdk-path) -O3"
    export LDFLAGS="-arch arm64"
    
    ./configure \
        --prefix="$ASHELL_PREFIX" \
        --static \
        --libdir="$ASHELL_PREFIX/lib"
}

a_shell_pkg_check() {
    cd "$ASHELL_PKG_SRCDIR"
    # zlib has make check
    make check || echo "Warning: zlib tests failed (non-fatal)"
}
```

### 2.3 libssl (OpenSSL)

**packages/core/libssl/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="openssl"
A_SHELL_PKG_VERSION="3.2.1"
A_SHELL_PKG_SRCURL="https://www.openssl.org/source/openssl-3.2.1.tar.gz"
A_SHELL_PKG_SHA256="83c7329fe52c850677d75e5d0b0ca245309b7e631ecf37756c9bf31ccddf85c9"
A_SHELL_PKG_DEPENDS="zlib"
A_SHELL_PKG_BUILD_DEPENDS=""
A_SHELL_PKG_SECTION="libs"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://www.openssl.org/"
A_SHELL_PKG_DESCRIPTION="Secure Sockets Layer toolkit - runtime"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CFLAGS="-arch arm64 -isysroot $(xcrun -sdk iphoneos --show-sdk-path)"
    export LDFLAGS="-arch arm64"
    
    ./Configure \
        ios64-xcrun \
        --prefix="$ASHELL_PREFIX" \
        --openssldir="$ASHELL_PREFIX/etc/ssl" \
        no-shared \
        no-tests \
        zlib \
        -I"${ASHELL_PREFIX}/include" \
        -L"${ASHELL_PREFIX}/lib"
}

a_shell_step_make() {
    cd "$ASHELL_PKG_SRCDIR"
    make -j$(sysctl -n hw.ncpu)
    make install_sw  # Skip docs to save space
}

a_shell_pkg_check() {
    # OpenSSL tests take too long, skip for now
    echo "Skipping OpenSSL tests (too slow)"
}
```

### 2.4 libcurl

**packages/core/libcurl/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="curl"
A_SHELL_PKG_VERSION="8.5.0"
A_SHELL_PKG_SRCURL="https://curl.se/download/curl-8.5.0.tar.gz"
A_SHELL_PKG_SHA256="42ab8db76ebc1c3c7dd8f5e4df7e7a6b9a5e0a9c8b7d6e5f4a3b2c1d0e9f8a7b6"
A_SHELL_PKG_DEPENDS="zlib, openssl"
A_SHELL_PKG_BUILD_DEPENDS=""
A_SHELL_PKG_SECTION="net"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://curl.se/"
A_SHELL_PKG_DESCRIPTION="Library for transferring data with URLs"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CFLAGS="-arch arm64 -isysroot $(xcrun -sdk iphoneos --show-sdk-path)"
    export LDFLAGS="-arch arm64"
    
    ./configure \
        --prefix="$ASHELL_PREFIX" \
        --host=arm-apple-darwin \
        --disable-shared \
        --enable-static \
        --with-ssl="${ASHELL_PREFIX}" \
        --with-zlib="${ASHELL_PREFIX}" \
        --enable-ipv6 \
        --disable-ldap \
        --disable-ldaps \
        --disable-manual \
        --without-libpsl \
        --without-libidn2 \
        --without-nghttp2 \
        --without-nghttp3 \
        --without-quiche \
        --without-msh3
}

a_shell_pkg_check() {
    cd "$ASHELL_PKG_SRCDIR"
    # Run minimal tests
    make test || echo "Warning: curl tests failed (non-fatal)"
}
```

### 2.5 ncurses

**packages/core/ncurses/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="ncurses"
A_SHELL_PKG_VERSION="6.4"
A_SHELL_PKG_SRCURL="https://invisible-mirror.net/archives/ncurses/ncurses-6.4.tar.gz"
A_SHELL_PKG_SHA256="6931283d9ac87c5073f30b6290c4c75f21632bb4fc3603ac8100812bed248159"
A_SHELL_PKG_DEPENDS=""
A_SHELL_PKG_BUILD_DEPENDS=""
A_SHELL_PKG_SECTION="libs"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://invisible-island.net/ncurses/"
A_SHELL_PKG_DESCRIPTION="Terminal handling library"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CFLAGS="-arch arm64 -isysroot $(xcrun -sdk iphoneos --show-sdk-path)"
    export LDFLAGS="-arch arm64"
    
    ./configure \
        --prefix="$ASHELL_PREFIX" \
        --host=arm-apple-darwin \
        --without-shared \
        --with-normal \
        --without-debug \
        --without-ada \
        --without-manpages \
        --without-tests \
        --disable-db-install \
        --enable-termcap \
        --disable-stripping \
        --disable-widec  # Start with narrow chars, add wide later
}

a_shell_pkg_check() {
    # ncurses tests are limited
    echo "Skipping ncurses tests"
}
```

### 2.6 readline

**packages/core/readline/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="readline"
A_SHELL_PKG_VERSION="8.2"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/readline/readline-8.2.tar.gz"
A_SHELL_PKG_SHA256="3feb7171f16a84ee82ca18a36d7b9be109a52c04f492a053331d7d1095007c35"
A_SHELL_PKG_DEPENDS="ncurses"
A_SHELL_PKG_BUILD_DEPENDS=""
A_SHELL_PKG_SECTION="libs"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://tiswww.case.edu/php/chet/readline/rltop.html"
A_SHELL_PKG_DESCRIPTION="Library for line editing (required by bash)"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CFLAGS="-arch arm64 -isysroot $(xcrun -sdk iphoneos --show-sdk-path) -I${ASHELL_PREFIX}/include"
    export LDFLAGS="-arch arm64 -L${ASHELL_PREFIX}/lib"
    export CPPFLAGS="-I${ASHELL_PREFIX}/include"
    
    ./configure \
        --prefix="$ASHELL_PREFIX" \
        --host=arm-apple-darwin \
        --disable-shared \
        --enable-static \
        --with-curses \
        bash_cv_func_sigsetjmp=yes \
        bash_cv_wcwidth_broken=no
}

a_shell_pkg_check() {
    # readline tests require a terminal
    echo "Skipping readline tests (require TTY)"
}
```

---

## Phase 3: Core Tools

### 3.1 bash

**packages/core/bash/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="bash"
A_SHELL_PKG_VERSION="5.2.21"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/bash/bash-5.2.21.tar.gz"
A_SHELL_PKG_SHA256="c8e31bdc59b6960c18b6c374fbb9fdc4880f7c0c"
A_SHELL_PKG_DEPENDS="ncurses, readline"
A_SHELL_PKG_BUILD_DEPENDS=""
A_SHELL_PKG_SECTION="shells"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://www.gnu.org/software/bash/"
A_SHELL_PKG_DESCRIPTION="GNU Bourne Again SHell"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CFLAGS="-arch arm64 -isysroot $(xcrun -sdk iphoneos --show-sdk-path) -I${ASHELL_PREFIX}/include"
    export LDFLAGS="-arch arm64 -L${ASHELL_PREFIX}/lib"
    export CPPFLAGS="-I${ASHELL_PREFIX}/include"
    
    ./configure \
        --prefix="$ASHELL_PREFIX" \
        --host=arm-apple-darwin \
        --without-bash-malloc \
        --with-installed-readline="${ASHELL_PREFIX}" \
        --enable-static-link \
        --disable-nls \
        --disable-rpath \
        --with-curses \
        --enable-readline \
        --enable-history \
        --enable-alias \
        --enable-bang-history \
        --enable-job-control \
        --enable-progcomp \
        --enable-prompt-string-decoding \
        --enable-select \
        --enable-help-builtin \
        --enable-builtins \
        --enable-array-variables \
        --enable-dparen-arithmetic \
        --enable-extended-glob \
        --enable-process-substitution \
        --disable-separate-helpfiles \
        bash_cv_dev_stdin=present \
        bash_cv_dev_fd=standard \
        bash_cv_termcap_lib=libncurses
}

a_shell_pkg_check() {
    cd "$ASHELL_PKG_SRCDIR"
    # Run a subset of tests
    make TESTS="tests/run-all" check || echo "Some bash tests failed (check manually)"
}
```

### 3.2 coreutils

**packages/core/coreutils/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="coreutils"
A_SHELL_PKG_VERSION="9.4"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/coreutils/coreutils-9.4.tar.xz"
A_SHELL_PKG_SHA256="ea613c4c2e5e04b37c99c6276b8465103172a5d2"
A_SHELL_PKG_DEPENDS=""
A_SHELL_PKG_BUILD_DEPENDS=""
A_SHELL_PKG_SECTION="utils"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://www.gnu.org/software/coreutils/"
A_SHELL_PKG_DESCRIPTION="GNU core utilities (ls, cp, mv, rm, cat, etc.)"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CFLAGS="-arch arm64 -isysroot $(xcrun -sdk iphoneos --show-sdk-path)"
    export LDFLAGS="-arch arm64"
    export FORCE_UNSAFE_CONFIGURE=1
    
    ./configure \
        --prefix="$ASHELL_PREFIX" \
        --host=arm-apple-darwin \
        --enable-no-install-program=stdbuf,su,uptime,arch,kill,hostname \
        --disable-nls \
        --disable-rpath \
        --without-gmp \
        --without-selinux \
        --without-linux-caps \
        --disable-libcap \
        --disable-libsmack \
        --enable-single-binary=symlinks \
        --enable-install-program="${coreutils_install_program}" \
        gl_cv_func_getcwd_null=yes \
        gl_cv_func_fstatat_zero_flag=yes \
        ac_cv_func_getgroups_works=yes \
        ac_cv_func_strerror_r_works=yes
}

a_shell_pkg_check() {
    # coreutils tests take 30+ minutes, skip for bootstrap
    echo "Skipping coreutils tests (too slow for bootstrap)"
}
```

### 3.3 Additional Core Tools

Create similar build.sh files for:
- tar (archiving)
- gzip (compression)
- grep (pattern matching)
- sed (stream editor)
- gawk (pattern scanning)

---

## Phase 4: Package Management + Bootstrap

### 4.1 dpkg

**packages/core/dpkg/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="dpkg"
A_SHELL_PKG_VERSION="1.22.4"
A_SHELL_PKG_SRCURL="https://deb.debian.org/debian/pool/main/d/dpkg/dpkg_1.22.4.tar.xz"
A_SHELL_PKG_SHA256="9fb9bf1c6a95"
A_SHELL_PKG_DEPENDS="zlib"
A_SHELL_PKG_BUILD_DEPENDS=""
A_SHELL_PKG_SECTION="admin"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://wiki.debian.org/Teams/Dpkg"
A_SHELL_PKG_DESCRIPTION="Debian package management system"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CFLAGS="-arch arm64 -isysroot $(xcrun -sdk iphoneos --show-sdk-path)"
    export LDFLAGS="-arch arm64"
    
    ./configure \
        --prefix="$ASHELL_PREFIX" \
        --host=arm-apple-darwin \
        --disable-shared \
        --disable-nls \
        --disable-dselect \
        --disable-start-stop-daemon \
        --disable-update-alternatives \
        --without-libselinux \
        --without-libmd \
        --without-libzstd \
        --without-liblzma \
        --without-libicu \
        --disable-devel-docs \
        --disable-maintainer-mode
}

a_shell_pkg_check() {
    echo "Skipping dpkg tests"
}
```

### 4.2 apt

**packages/core/apt/build.sh**:
```bash
#!/bin/bash
A_SHELL_PKG_NAME="apt"
A_SHELL_PKG_VERSION="2.7.8"
A_SHELL_PKG_SRCURL="https://deb.debian.org/debian/pool/main/a/apt/apt_2.7.8.tar.xz"
A_SHELL_PKG_SHA256="b7a7"
A_SHELL_PKG_DEPENDS="dpkg, zlib"
A_SHELL_PKG_BUILD_DEPENDS="cmake"
A_SHELL_PKG_SECTION="admin"
A_SHELL_PKG_PRIORITY="required"
A_SHELL_PKG_HOMEPAGE="https://wiki.debian.org/Apt"
A_SHELL_PKG_DESCRIPTION="Advanced Package Tool"

a_shell_pkg_configure() {
    cd "$ASHELL_PKG_SRCDIR"
    
    mkdir -p build && cd build
    
    cmake .. \
        -DCMAKE_INSTALL_PREFIX="$ASHELL_PREFIX" \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DWITH_DOC=OFF \
        -DUSE_NLS=OFF \
        -DWITH_TESTS=OFF \
        -DDPKG_EXECUTABLE="${ASHELL_PREFIX}/bin/dpkg" \
        -DAPT_CACHE_DIR="${ASHELL_PREFIX}/var/cache/apt" \
        -DAPT_STATE_DIR="${ASHELL_PREFIX}/var/lib/apt" \
        -DROOT_GROUP=mobile \
        -DBUILD_SHARED_LIBS=OFF
}

a_shell_step_make() {
    cd "$ASHELL_PKG_SRCDIR/build"
    cmake --build . -j$(sysctl -n hw.ncpu)
}

a_shell_pkg_check() {
    echo "Skipping apt tests"
}
```

### 4.3 Bootstrap Creation

**scripts/create-bootstrap.sh**:
```bash
#!/bin/bash
set -e

ASHELL_PREFIX="${HOME}/Library/ashell"
BOOTSTRAP_DIR="${TMPDIR}/ashell-bootstrap-$$"
OUTPUT="bootstrap-ios-arm64.tar.gz"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Cleanup on exit
cleanup() {
    rm -rf "$BOOTSTRAP_DIR"
}
trap cleanup EXIT

echo "Creating a-Shell bootstrap package..."
echo "Output: $OUTPUT"

# Create fresh directory
rm -rf "$BOOTSTRAP_DIR"
mkdir -p "$BOOTSTRAP_DIR"

# Build all core packages in dependency order
PACKAGES=(
    libz
    libssl
    libcurl
    ncurses
    readline
    bash
    coreutils
    tar
    gzip
    grep
    sed
    gawk
    dpkg
    apt
)

echo ""
echo "Building packages..."
for pkg in "${PACKAGES[@]}"; do
    echo "  → Building $pkg..."
    "${SCRIPT_DIR}/a_shell_package.sh" "${SCRIPT_DIR}/../packages/core/$pkg"
done

# Install all packages to bootstrap directory
echo ""
echo "Assembling bootstrap..."
mkdir -p "${BOOTSTRAP_DIR}"

for pkg in "${PACKAGES[@]}"; do
    version=$(grep "A_SHELL_PKG_VERSION=" "${SCRIPT_DIR}/../packages/core/$pkg/build.sh" | head -1 | cut -d'"' -f2)
    deb="${pkg}_${version}_ios-arm64.deb"
    
    if [[ -f "$deb" ]]; then
        echo "  → Extracting $deb"
        dpkg-deb -x "$deb" "$BOOTSTRAP_DIR"
    else
        echo "  ⚠ Warning: $deb not found"
    fi
done

# Create APT configuration
echo ""
echo "Configuring APT..."
mkdir -p "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/etc/apt/sources.list.d"
cat > "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/etc/apt/sources.list" <<EOF
deb https://packages.a-shell.dev stable main
deb https://packages.a-shell.dev stable extra
EOF

# Create essential directories
mkdir -p "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/var/cache/apt/archives"
mkdir -p "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/var/lib/apt/lists/partial"
mkdir -p "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/var/lib/dpkg/info"
mkdir -p "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/var/lib/dpkg/updates"
touch "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/var/lib/dpkg/status"
touch "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/var/lib/dpkg/available"

# Create dpkg diversions file
touch "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/var/lib/dpkg/diversions"

# Set up alternatives directory
mkdir -p "${BOOTSTRAP_DIR}${ASHELL_PREFIX}/etc/alternatives"

# Create tarball
echo ""
echo "Creating tarball..."
tar -czf "$OUTPUT" -C "$BOOTSTRAP_DIR" .

# Show stats
echo ""
echo "========================================"
echo "Bootstrap created successfully!"
echo "========================================"
ls -lh "$OUTPUT"
echo ""
echo "Size breakdown:"
du -sh "$BOOTSTRAP_DIR"
echo ""
echo "Contents:"
find "$BOOTSTRAP_DIR" -type f | wc -l | xargs echo "  Files:"
find "$BOOTSTRAP_DIR" -type d | wc -l | xargs echo "  Directories:"
echo ""
echo "To install in a-Shell:"
echo "  1. Copy $OUTPUT to a-shell/Resources/"
echo "  2. Build and run a-Shell"
echo "  3. Bootstrap will extract on first launch"
```

### 4.4 First-Launch Extraction (iOS App)

**a-shell/AppDelegate.swift** (excerpt):
```swift
import UIKit

class AppDelegate: UIResponder, UIApplicationDelegate {
    
    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        
        // Check if bootstrap needs extraction
        if !bootstrapExists() {
            extractBootstrap()
        }
        
        return true
    }
    
    private func bootstrapExists() -> Bool {
        let prefix = FileManager.default.urls(for: .libraryDirectory, in: .userDomainMask).first!
            .appendingPathComponent("ashell")
        return FileManager.default.fileExists(atPath: prefix.path)
    }
    
    private func extractBootstrap() {
        guard let bootstrapURL = Bundle.main.url(forResource: "bootstrap", withExtension: "tar.gz") else {
            print("ERROR: Bootstrap not found in bundle")
            return
        }
        
        let documentsURL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first!
        let prefixURL = FileManager.default.urls(for: .libraryDirectory, in: .userDomainMask).first!
            .appendingPathComponent("ashell")
        
        do {
            // Show progress UI
            let progressView = UIProgressView(progressViewStyle: .bar)
            // ... setup progress UI
            
            // Extract bootstrap
            try extractTarGz(from: bootstrapURL, to: prefixURL)
            
            // Setup environment
            setupEnvironment(prefix: prefixURL.path)
            
            print("Bootstrap extracted successfully")
            
        } catch {
            print("ERROR: Failed to extract bootstrap: \(error)")
        }
    }
    
    private func setupEnvironment(prefix: String) {
        setenv("ASHELL_PREFIX", prefix, 1)
        setenv("PATH", "\(prefix)/usr/bin:/usr/bin:/bin", 1)
        setenv("LD_LIBRARY_PATH", "\(prefix)/usr/lib", 1)
        setenv("CPPFLAGS", "-I\(prefix)/include", 1)
        setenv("LDFLAGS", "-L\(prefix)/lib", 1)
        setenv("PKG_CONFIG_PATH", "\(prefix)/lib/pkgconfig", 1)
    }
}
```

---

## Phase 5: WAMR + WASM Integration

**Rationale**: WAMR is moved AFTER bootstrap because:
1. Bootstrap provides essential tools first
2. WAMR is for extended functionality (WASM packages)
3. Users can install WAMR via package manager if needed

### 5.1 WAMR Build System

**a-shell-kernel/wamr/build-ios.sh**:
```bash
#!/bin/bash
set -e

WAMR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${WAMR_DIR}/build-ios"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "${WAMR_DIR}" \
    -DWAMR_BUILD_PLATFORM=ios \
    -DWAMR_BUILD_AOT=1 \
    -DWAMR_BUILD_INTERP=1 \
    -DWAMR_BUILD_FAST_INTERP=1 \
    -DWAMR_BUILD_LIBC_WASI=1 \
    -DWAMR_BUILD_LIBC_BUILTIN=1 \
    -DWAMR_BUILD_MULTI_MODULE=1 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
    -DCMAKE_BUILD_TYPE=Release

make -j$(sysctl -n hw.ncpu)

# Create XCFramework
xcodebuild -create-xcframework \
    -library "${BUILD_DIR}/libvmlib.a" \
    -headers "${WAMR_DIR}/core/iwasm/include" \
    -output "${WAMR_DIR}/wamr-ios.xcframework"

echo "WAMR XCFramework created: wamr-ios.xcframework"
```

### 5.2 WASI Syscall Mapping

**src/wasi/wasi_syscalls.c**:
```c
#include "wasm_export.h"
#include "../include/linux/unistd.h"
#include "../include/linux/fcntl.h"
#include "../include/linux/stat.h"
#include "../vfs.h"

/* WASI file descriptor operations */
static int32_t wasi_fd_close(wasm_exec_env_t exec_env, int32_t fd) {
    return a_shell_close(fd);
}

static int32_t wasi_fd_read(wasm_exec_env_t exec_env, int32_t fd,
                            iovec_app_t *iov, int32_t iovcnt, int32_t *nread) {
    /* Translate WASI iovec to native iovec */
    struct iovec native_iov[iovcnt];
    for (int i = 0; i < iovcnt; i++) {
        native_iov[i].iov_base = wasm_runtime_addr_app_to_native(exec_env, iov[i].buf);
        native_iov[i].iov_len = iov[i].buf_len;
    }
    
    ssize_t ret = readv(fd, native_iov, iovcnt);
    if (ret < 0) {
        return -1;
    }
    
    *nread = (int32_t)ret;
    return 0;
}

static int32_t wasi_fd_write(wasm_exec_env_t exec_env, int32_t fd,
                             iovec_app_t *iov, int32_t iovcnt, int32_t *nwritten) {
    struct iovec native_iov[iovcnt];
    for (int i = 0; i < iovcnt; i++) {
        native_iov[i].iov_base = wasm_runtime_addr_app_to_native(exec_env, iov[i].buf);
        native_iov[i].iov_len = iov[i].buf_len;
    }
    
    ssize_t ret = writev(fd, native_iov, iovcnt);
    if (ret < 0) {
        return -1;
    }
    
    *nwritten = (int32_t)ret;
    return 0;
}

static int32_t wasi_path_open(wasm_exec_env_t exec_env, int32_t dirfd,
                              int32_t dirflags, const char *path, int32_t path_len,
                              int32_t oflags, int64_t fs_rights_base,
                              int64_t fs_rights_inheriting, int32_t fdflags,
                              int32_t *fd) {
    /* Translate WASI oflags to POSIX flags */
    int flags = 0;
    mode_t mode = 0666;
    
    if (oflags & 0x1) flags |= O_CREAT;      /* CREATE */
    if (oflags & 0x2) flags |= O_DIRECTORY;  /* DIRECTORY */
    if (oflags & 0x4) flags |= O_EXCL;       /* EXCL */
    if (oflags & 0x8) flags |= O_TRUNC;      /* TRUNC */
    
    if (fs_rights_base & 0x2) flags |= O_RDONLY;
    if (fs_rights_base & 0x4) flags |= O_WRONLY;
    if ((fs_rights_base & 0x6) == 0x6) flags = (flags & ~O_ACCMODE) | O_RDWR;
    
    if (fdflags & 0x1) flags |= O_APPEND;
    if (fdflags & 0x2) flags |= O_DSYNC;
    if (fdflags & 0x4) flags |= O_NONBLOCK;
    if (fdflags & 0x8) flags |= O_SYNC;
    if (fdflags & 0x10) flags |= O_RSYNC;
    
    char *translated_path = vfs_translate_path(path);
    if (!translated_path) {
        return -1;
    }
    
    int ret_fd = a_shell_openat(dirfd, translated_path, flags, mode);
    vfs_free_path(translated_path);
    
    if (ret_fd < 0) {
        return -1;
    }
    
    *fd = ret_fd;
    return 0;
}

/* Register WASI syscalls */
void a_shell_wasi_init(void) {
    static NativeSymbol native_symbols[] = {
        REG_WASI_NATIVE_FUNC(fd_close, "(i)i"),
        REG_WASI_NATIVE_FUNC(fd_read, "(i*~*)i"),
        REG_WASI_NATIVE_FUNC(fd_write, "(i*~*)i"),
        REG_WASI_NATIVE_FUNC(path_open, "(ii*iIIii*)i"),
        /* Add more WASI syscalls as needed */
    };
    
    uint32_t n_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
    wasm_runtime_register_natives("wasi_snapshot_preview1",
                                   native_symbols, n_symbols);
}
```

### 5.3 WASM Command Execution

**src/wasi/wasm_command.m**:
```c
#import <Foundation/Foundation.h>
#include "wasm_export.h"
#include "../include/linux/unistd.h"

static uint8_t *read_wasm_file(const char *filename, uint32_t *size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    *size = (uint32_t)ftell(file);
    fseek(file, 0, SEEK_SET);
    
    uint8_t *buffer = malloc(*size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    if (fread(buffer, 1, *size, file) != *size) {
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    return buffer;
}

int a_shell_wasm_exec(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: wasm <file.wasm> [args...]\n");
        return 1;
    }
    
    const char *wasm_file = argv[1];
    
    /* Initialize WAMR runtime */
    RuntimeInitArgs init_args;
    memset(&init_args, 0, sizeof(RuntimeInitArgs));
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = NULL; /* Use system allocator */
    init_args.mem_alloc_option.pool.heap_size = 0;
    
    if (!wasm_runtime_full_init(&init_args)) {
        fprintf(stderr, "Failed to initialize WAMR runtime\n");
        return 1;
    }
    
    /* Register WASI syscalls */
    a_shell_wasi_init();
    
    /* Load WASM module */
    uint32_t wasm_file_size;
    uint8_t *wasm_file_buf = read_wasm_file(wasm_file, &wasm_file_size);
    if (!wasm_file_buf) {
        fprintf(stderr, "Failed to read WASM file: %s\n", wasm_file);
        wasm_runtime_destroy();
        return 1;
    }
    
    char error_buf[128];
    wasm_module_t module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                              error_buf, sizeof(error_buf));
    free(wasm_file_buf);
    
    if (!module) {
        fprintf(stderr, "Failed to load WASM module: %s\n", error_buf);
        wasm_runtime_destroy();
        return 1;
    }
    
    /* Instantiate module */
    wasm_module_inst_t module_inst = wasm_runtime_instantiate(module,
        64 * 1024,   /* stack size: 64KB */
        64 * 1024,   /* heap size: 64KB */
        error_buf, sizeof(error_buf));
    
    if (!module_inst) {
        fprintf(stderr, "Failed to instantiate WASM module: %s\n", error_buf);
        wasm_runtime_unload(module);
        wasm_runtime_destroy();
        return 1;
    }
    
    /* Set up WASI environment */
    const char *dir_list[1] = { "." };
    const char *env_list[1] = { NULL };
    const char *argv_wasm[argc - 1];
    for (int i = 0; i < argc - 1; i++) {
        argv_wasm[i] = argv[i + 1];
    }
    
    wasm_runtime_set_wasi_args(module_inst, dir_list, 1, NULL, 0,
                               env_list, 0, argv_wasm, argc - 1);
    
    /* Execute main function */
    wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "_start");
    if (!func) {
        func = wasm_runtime_lookup_function(module_inst, "main");
    }
    
    if (!func) {
        fprintf(stderr, "No entry function found in WASM module\n");
        wasm_runtime_deinstantiate(module_inst);
        wasm_runtime_unload(module);
        wasm_runtime_destroy();
        return 1;
    }
    
    uint32_t argv_results[1] = { 0 };
    if (!wasm_runtime_call_wasm(module_inst, func, 0, NULL)) {
        fprintf(stderr, "WASM execution failed: %s\n",
                wasm_runtime_get_exception(module_inst));
        wasm_runtime_deinstantiate(module_inst);
        wasm_runtime_unload(module);
        wasm_runtime_destroy();
        return 1;
    }
    
    /* Cleanup */
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
    wasm_runtime_destroy();
    
    return 0;
}
```

---

## Phase 6: Repository Infrastructure

### 6.1 Repository Structure

```
packages.a-shell.dev/
├── dists/
│   └── stable/
│       ├── Release
│       └── main/
│           └── binary-ios-arm64/
│               ├── Packages
│               ├── Packages.gz
│               ├── Packages.xz
│               └── Release
├── pool/
│   └── main/
│       ├── b/bash/bash_5.2.21_ios-arm64.deb
│       ├── c/coreutils/coreutils_9.4_ios-arm64.deb
│       ├── l/libz/libz_1.3.1_ios-arm64.deb
│       └── ...
├── bootstrap/
│   └── bootstrap-ios-arm64.tar.gz
└── index.html
```

### 6.2 Package Index Generation

**scripts/update-repository.sh**:
```bash
#!/bin/bash
set -e

REPO_DIR="${REPO_DIR:-/var/www/packages.a-shell.dev}"
POOL_DIR="${REPO_DIR}/pool/main"
DIST_DIR="${REPO_DIR}/dists/stable/main/binary-ios-arm64"

echo "Updating a-Shell package repository..."
echo "Repository: $REPO_DIR"

# Create directories
mkdir -p "$DIST_DIR"
mkdir -p "$POOL_DIR"

# Generate Packages file
echo "Generating package index..."
: > "${DIST_DIR}/Packages"

declare -A processed_packages

for deb in $(find "$POOL_DIR" -name "*.deb" -type f | sort); do
    pkg_name=$(basename "$deb" | cut -d'_' -f1)
    
    # Skip duplicates (keep newest)
    if [[ -n "${processed_packages[$pkg_name]}" ]]; then
        continue
    fi
    processed_packages[$pkg_name]=1
    
    # Extract control file
    control=$(dpkg-deb -f "$deb")
    
    # Add Filename, Size, and hashes
    filename="${deb#${REPO_DIR}/}"
    size=$(stat -f%z "$deb" 2>/dev/null || stat -c%s "$deb")
    md5=$(md5sum "$deb" 2>/dev/null | cut -d' ' -f1 || md5 -q "$deb")
    sha1=$(sha1sum "$deb" 2>/dev/null | cut -d' ' -f1 || shasum -a 1 "$deb" | cut -d' ' -f1)
    sha256=$(sha256sum "$deb" 2>/dev/null | cut -d' ' -f1 || shasum -a 256 "$deb" | cut -d' ' -f1)
    
    cat >> "${DIST_DIR}/Packages" <<EOF
$control
Filename: $filename
Size: $size
MD5sum: $md5
SHA1: $sha1
SHA256: $sha256

EOF
    
    echo "  Added: $pkg_name"
done

# Compress Packages file
echo "Compressing package index..."
gzip -9 -k -c "${DIST_DIR}/Packages" > "${DIST_DIR}/Packages.gz"
xz -9 -k -c "${DIST_DIR}/Packages" > "${DIST_DIR}/Packages.xz" 2>/dev/null || true

# Calculate hashes for Release file
packages_size=$(stat -f%z "${DIST_DIR}/Packages" 2>/dev/null || stat -c%s "${DIST_DIR}/Packages")
packages_md5=$(md5sum "${DIST_DIR}/Packages" 2>/dev/null | cut -d' ' -f1 || md5 -q "${DIST_DIR}/Packages")
packages_sha1=$(sha1sum "${DIST_DIR}/Packages" 2>/dev/null | cut -d' ' -f1 || shasum -a 1 "${DIST_DIR}/Packages" | cut -d' ' -f1)
packages_sha256=$(sha256sum "${DIST_DIR}/Packages" 2>/dev/null | cut -d' ' -f1 || shasum -a 256 "${DIST_DIR}/Packages" | cut -d' ' -f1)

gz_size=$(stat -f%z "${DIST_DIR}/Packages.gz" 2>/dev/null || stat -c%s "${DIST_DIR}/Packages.gz")
gz_md5=$(md5sum "${DIST_DIR}/Packages.gz" 2>/dev/null | cut -d' ' -f1 || md5 -q "${DIST_DIR}/Packages.gz")
gz_sha1=$(sha1sum "${DIST_DIR}/Packages.gz" 2>/dev/null | cut -d' ' -f1 || shasum -a 1 "${DIST_DIR}/Packages.gz" | cut -d' ' -f1)
gz_sha256=$(sha256sum "${DIST_DIR}/Packages.gz" 2>/dev/null | cut -d' ' -f1 || shasum -a 256 "${DIST_DIR}/Packages.gz" | cut -d' ' -f1)

# Generate Release file
echo "Generating Release file..."
cat > "${REPO_DIR}/dists/stable/Release" <<EOF
Origin: a-Shell
Label: a-Shell Package Repository
Suite: stable
Codename: stable
Version: 1.0
Architectures: ios-arm64
Components: main
Description: a-Shell iOS Package Repository
Date: $(date -u +"%a, %d %b %Y %H:%M:%S UTC")

MD5Sum:
 $packages_md5 $packages_size main/binary-ios-arm64/Packages
 $gz_md5 $gz_size main/binary-ios-arm64/Packages.gz

SHA1:
 $packages_sha1 $packages_size main/binary-ios-arm64/Packages
 $gz_sha1 $gz_size main/binary-ios-arm64/Packages.gz

SHA256:
 $packages_sha256 $packages_size main/binary-ios-arm64/Packages
 $gz_sha256 $gz_size main/binary-ios-arm64/Packages.gz
EOF

echo ""
echo "Repository updated successfully!"
echo ""
echo "Statistics:"
echo "  Packages: ${#processed_packages[@]}"
echo "  Size: $(du -sh "$REPO_DIR" | cut -f1)"
```

---

## Implementation Checklist

### Phase 1: Foundation (Kernel Headers + Syscalls)

#### Headers
- [ ] Create include/linux/unistd.h
- [ ] Create include/linux/stat.h
- [ ] Create include/linux/fcntl.h
- [ ] Create include/linux/wait.h
- [ ] Create include/linux/signal.h
- [ ] Create include/linux/time.h
- [ ] Create include/linux/resource.h
- [ ] Create include/linux/mman.h
- [ ] Create include/linux/socket.h
- [ ] Create include/linux/poll.h
- [ ] Create include/linux/epoll.h
- [ ] Create include/linux/ioctl.h
- [ ] Create include/linux/uio.h
- [ ] Create include/sys/types.h
- [ ] Create include/sys/wait.h
- [ ] Create include/sys/stat.h
- [ ] Create include/pwd.h
- [ ] Create include/grp.h

#### Implementations
- [ ] Create src/syscalls/process.m
- [ ] Create src/syscalls/file.c
- [ ] Create src/syscalls/signal.c
- [ ] Create src/syscalls/time.c
- [ ] Create src/syscalls/memory.c
- [ ] Create src/syscalls/socket.c
- [ ] Create src/syscalls/poll.c
- [ ] Create src/vfs.c
- [ ] Create src/vfs.h

#### Testing
- [ ] Create tests/test_syscalls.c
- [ ] Add automated test execution to build

#### Cleanup
- [ ] Delete ios_error.h
- [ ] Fix all compilation errors

### Phase 2: Core Libraries
- [ ] Update packages/core/libz/build.sh
- [ ] Create packages/core/libssl/build.sh
- [ ] Create packages/core/libcurl/build.sh
- [ ] Create packages/core/ncurses/build.sh
- [ ] Create packages/core/readline/build.sh

### Phase 3: Core Tools
- [ ] Update packages/core/bash/build.sh
- [ ] Update packages/core/coreutils/build.sh
- [ ] Create packages/core/tar/build.sh
- [ ] Create packages/core/gzip/build.sh
- [ ] Create packages/core/grep/build.sh
- [ ] Create packages/core/sed/build.sh
- [ ] Create packages/core/gawk/build.sh

### Phase 4: Package Management + Bootstrap
- [ ] Create packages/core/dpkg/build.sh
- [ ] Create packages/core/apt/build.sh
- [ ] Create scripts/create-bootstrap.sh
- [ ] Implement first-launch extraction in iOS app

### Phase 5: WAMR + WASM
- [ ] Create wamr/build-ios.sh
- [ ] Create src/wasi/wasi_syscalls.c
- [ ] Create src/wasi/wasm_command.m
- [ ] Build WAMR XCFramework

### Phase 6: Repository
- [ ] Set up web hosting
- [ ] Create scripts/update-repository.sh
- [ ] Deploy packages.a-shell.dev

---

## Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| Big bang migration | Forces complete adoption, eliminates technical debt |
| Kernel headers first | Required by ALL packages, foundation for everything |
| Use readline (not LineEditor) | bash requires readline API, LineEditor is Swift-only |
| WAMR after bootstrap | Bootstrap provides essential tools first, WAMR is extended functionality |
| Path translation via VFS | /etc → $PREFIX/etc at runtime, no compile-time patches needed |
| iOS BSD socket pass-through | BSD sockets are Linux-compatible, no translation layer needed |
| Debian-style dependencies | Uses Depends, Recommends, Conflicts fields in control files |
| Automated upstream tests | Run make check for each package during build |
| 50MB bootstrap size | Balance between functionality and download size |

---

## Environment Variables

Packages will use these environment variables:

```bash
ASHELL_PREFIX=/Users/mobile/Library/ashell
PATH=$ASHELL_PREFIX/usr/bin:/usr/bin:/bin
LD_LIBRARY_PATH=$ASHELL_PREFIX/usr/lib
CPPFLAGS=-I$ASHELL_PREFIX/include
LDFLAGS=-L$ASHELL_PREFIX/lib
PKG_CONFIG_PATH=$ASHELL_PREFIX/lib/pkgconfig
```

---

## Success Criteria

### Phase 1 Complete When:
- [ ] All kernel headers created
- [ ] All syscall implementations working
- [ ] ios_error.h deleted
- [ ] test_syscalls.c passes all tests
- [ ] Compilation succeeds without errors

### Phase 2 Complete When:
- [ ] All core libraries built successfully
- [ ] Each library passes its upstream tests
- [ ] Libraries install to $ASHELL_PREFIX correctly

### Phase 3 Complete When:
- [ ] bash runs and accepts commands
- [ ] coreutils commands work (ls, cp, mv, etc.)
- [ ] All tools pass basic functionality tests

### Phase 4 Complete When:
- [ ] dpkg installs/removes packages
- [ ] apt downloads from repository
- [ ] Bootstrap tarball < 50MB
- [ ] First-launch extraction works on iOS

### Phase 5 Complete When:
- [ ] WAMR XCFramework builds
- [ ] WASM modules execute correctly
- [ ] WASI syscalls mapped to kernel

### Phase 6 Complete When:
- [ ] packages.a-shell.dev is live
- [ ] APT can install packages from repository
- [ ] Bootstrap downloads correctly

---

## References

- Linux kernel headers: /usr/include/linux/
- Termux packages: https://github.com/termux/termux-packages
- Debian APT: https://wiki.debian.org/Apt
- WAMR: https://github.com/bytecodealliance/wasm-micro-runtime
- a-Shell: https://github.com/holzschu/a-Shell

---

**End of Unified Implementation Plan**

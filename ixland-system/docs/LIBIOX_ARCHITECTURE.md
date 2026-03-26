# libiox Architecture Specification

## Overview

**libiox** is the iOS eXtension subsystem - a complete Linux syscall compatibility layer for iOS. It enables unmodified Linux binaries to run on iOS through static symbol interposition and optional WebAssembly support.

## Naming Convention

| Level | Pattern | Example |
|-------|---------|---------|
| Internal Implementation | `__iox_*_impl()` | `__iox_fork_impl()` |
| Public API | `iox_*()` | `iox_fork()` |
| Linux Standard | `#define syscall iox_syscall` | `#define fork iox_fork` |
| Tools | `iox-*` | `iox-cc`, `iox-pkg` |

## Directory Structure

```
a-shell-kernel/
├── include/iox/              # Public headers
│   ├── iox.h                 # Master umbrella header
│   ├── iox_syscalls.h        # Syscall prototypes (public)
│   ├── iox_types.h           # iox-specific types
│   ├── iox_error.h           # Error codes
│   ├── iox_wamr.h            # WAMR integration API
│   └── sys/                  # Linux-compatible system headers
│       ├── unistd.h          # fork, exec, getpid...
│       ├── wait.h            # waitpid...
│       ├── time.h            # time functions
│       ├── stat.h            # stat structures
│       ├── types.h           # pid_t, off_t...
│       ├── socket.h          # BSD sockets
│       ├── signal.h          # signal handling
│       ├── mman.h            # mmap, mprotect
│       ├── fcntl.h           # open flags
│       ├── ioctl.h           # ioctl
│       └── errno.h           # errno codes
│
├── src/iox/                  # ALL implementations
│   ├── core/                 # Core syscall implementations
│   │   ├── iox_process.c     # fork, exec, wait, pid
│   │   ├── iox_file.c        # open, read, write, close
│   │   ├── iox_signal.c      # signal handling
│   │   ├── iox_memory.c      # mmap, munmap, mprotect
│   │   ├── iox_time.c        # sleep, gettimeofday
│   │   ├── iox_network.c     # socket passthrough
│   │   └── iox_thread.c      # pthread utilities
│   │
│   ├── interpose/            # Symbol interposition layer
│   │   └── iox_interpose.c   # All 300+ syscall wrappers
│   │
│   ├── runtime/              # C runtime (from libc_replacement)
│   │   ├── iox_stdio.c       # printf, fprintf, etc.
│   │   ├── iox_stdlib.c      # malloc, free wrappers
│   │   ├── iox_env.c         # getenv, setenv
│   │   ├── iox_string.c      # string functions
│   │   └── iox_crt0.c        # C startup code
│   │
│   ├── wamr/                 # WAMR integration
│   │   ├── iox_wamr_runtime.c     # WAMR initialization
│   │   ├── iox_wamr_wasi.c        # WASI syscall bridge
│   │   ├── iox_wamr_aot.c         # AoT loader
│   │   └── iox_wamr_native.c      # Native symbol exports
│   │
│   ├── util/                 # Utilities
│   │   ├── iox_vfs.c         # Virtual filesystem
│   │   ├── iox_path.c        # Path utilities
│   │   └── iox_debug.c       # Debug/trace functions
│   │
│   └── internal/             # Private headers
│       └── iox_internal.h      # Internal data structures
│
├── lib/                      # Build outputs (gitignore)
│   ├── libiox.a
│   └── libiox.dylib
│
├── bin/                      # Tools (generated)
│   └── iox-cc                # Compiler wrapper
│
├── deps/                     # Dependencies
│   └── wamr/                 # WAMR submodule
│
├── tests/                    # Test suite
│   ├── unit/
│   ├── integration/
│   └── fixtures/
│
├── CMakeLists.txt            # Primary build system
└── Makefile                  # Convenience wrapper
```

## Architecture Layers

### Layer 1: Symbol Interposition (Strong Symbols)

Linux programs call standard syscalls (fork, exec, open) which are intercepted at link time:

```c
// libiox.a provides these as strong symbols:
pid_t fork(void) { return iox_fork(); }
int execve(...) { return iox_execve(...); }
int open(...) { return iox_open(...); }
```

**Compile command:**
```bash
iox-cc program.c -o program_ios
# Internally: -I/path/to/iox/include -liox
```

### Layer 2: Public API (iox_*)

Clean public interface, no macro pollution:

```c
pid_t iox_fork(void);
int iox_execve(const char *pathname, char *const argv[], char *const envp[]);
int iox_open(const char *pathname, int flags, mode_t mode);
```

### Layer 3: Internal Implementation (__iox_*_impl)

Actual implementation with iOS-safe code:

```c
pid_t __iox_fork_impl(void) {
    // Thread-based process simulation
    // Virtual PID allocation
    // iOS-safe implementation
}
```

## Dual Execution Model

### Track A: Native (iox-compiled)

```
Linux Source ──► iox-cc ──► libiox.a ──► Native ARM64 Binary
                (compile)   (syscalls)    (100% performance)
```

### Track B: WASM (WAMR AoT)

```
WASM Binary ──► iox-wamr ──► WAMR AoT ──► 90-95% performance
                (runner)    (runtime)
```

## Syscall Coverage

### Process Management (16 syscalls)
- fork, vfork, execve, execv, exit, _exit
- getpid, getppid, getpgrp, setpgrp
- getpgid, setpgid, wait, waitpid, wait3, wait4
- system

### File Operations (20 syscalls)
- open, openat, creat, read, write, close
- lseek, pread, pwrite, dup, dup2, dup3
- fcntl, ioctl, access, faccessat
- chdir, fchdir, getcwd

### Filesystem (24 syscalls)
- stat, fstat, lstat, fstatat
- mkdir, mkdirat, rmdir, unlink, unlinkat
- link, linkat, symlink, symlinkat
- readlink, readlinkat
- chmod, fchmod, fchmodat
- chown, fchown, lchown, fchownat
- chroot

### Signal Handling (16 syscalls)
- signal, kill, killpg, raise, sigaction
- sigprocmask, sigpending, sigsuspend
- sigemptyset, sigfillset, sigaddset, sigdelset, sigismember
- alarm, setitimer, getitimer, pause

### Memory Management (6 syscalls)
- mmap, munmap, mprotect, msync, mlock, munlock

### Time (7 syscalls)
- sleep, usleep, nanosleep, gettimeofday, settimeofday
- clock_gettime, time

### Environment (5 syscalls)
- getenv, setenv, unsetenv, clearenv, environ

### Network (20 syscalls - passthrough)
- socket, socketpair, bind, connect, listen, accept, accept4
- send, sendto, sendmsg, recv, recvfrom, recvmsg
- shutdown, getsockname, getpeername
- getsockopt, setsockopt

### Pipes (4 syscalls)
- pipe, pipe2, mkfifo, mkfifoat

### Select/Poll (4 syscalls)
- select, pselect, poll, ppoll

### TTY (7 syscalls)
- isatty, ttyname_r, tcgetattr, tcsetattr
- tcsendbreak, tcdrain, tcflush, tcflow

## WAMR Integration

### WASI Syscalls (100+)

WAMR runs WebAssembly binaries with WASI syscalls bridged to iox:

```c
static NativeSymbol iox_wasi_symbols[] = {
    {"fd_write", iox_wasi_fd_write, "(i****)"},
    {"fd_read", iox_wasi_fd_read, "(i****)"},
    {"proc_exit", iox_wasi_exit, "(i)"},
    // ... 100+ more
};
```

### Performance

| Mode | Speed | Use Case |
|------|-------|----------|
| Native (iox) | 100% | Primary - recompiled tools |
| WAMR AoT | 90-95% | Fallback - prebuilt WASM |
| WAMR Interpreter | 10-20% | Debug only |

## Build System

### CMake

```cmake
add_library(iox STATIC
    src/iox/core/*.c
    src/iox/interpose/*.c
    src/iox/runtime/*.c
    src/iox/wamr/*.c
)

target_link_libraries(iox PUBLIC
    pthread
    ${WAMR_LIBS}
)
```

### Compiler Wrapper (iox-cc)

```bash
#!/bin/bash
clang \
    -target arm64-apple-ios16.0 \
    -isystem /path/to/iox/include \
    -L/path/to/iox/lib -liox \
    "$@"
```

## Migration from a-shell-kernel

### Old to New Mapping

| Old | New |
|-----|-----|
| `a_shell_fork()` | `__iox_fork_impl()` (internal) |
| | `iox_fork()` (public) |
| `a_shell_system.m` | `src/iox/core/iox_process.c` |
| `libc_replacement.c` | `src/iox/runtime/iox_*.c` |
| `include/linux/unistd.h` | `include/iox/sys/unistd.h` |
| `#define fork a_shell_fork` | `pid_t fork(void) { return iox_fork(); }` |

### Migration Script

```bash
./scripts/migrate_big_bang.sh
```

This script:
1. Creates new directory structure
2. Moves old files to backup locations
3. Deletes old directories
4. Leaves manual refactoring to developer

## Security Considerations

- **iOS Sandbox**: All syscalls respect app sandbox boundaries
- **No JIT**: WAMR uses AoT (ahead-of-time) compilation - App Store compliant
- **Path Validation**: All paths validated before access
- **No fork()**: Thread-based process simulation
- **W^X Policy**: No executable memory modifications

## App Store Compliance

✅ **Static Library**: libiox.a - no dynamic code generation
✅ **AoT Compilation**: WASM → ARM64 at build time
✅ **Sandboxed**: All file/network access within iOS sandbox
✅ **No Private APIs**: Only public iOS APIs used

## Future Work

- CHROOT support for namespace isolation
- Network sandboxing compliance
- Performance benchmarks
- Package manager (iox-pkg)
- CI/CD for automatic package building

---

**Version**: 1.0.0  
**Last Updated**: 2025-01-XX  
**Status**: Foundation Complete, Implementation In Progress

# a-Shell Next: Clean Architecture Plan

## Overview

**Goal**: Build a Debian-like Linux distribution for iOS with:
- Debian apt 3.1.16 as the package manager (patched for iOS)
- WebAssembly as the primary App Store-compliant extension mechanism
- Single app that detects distribution channel (App Store vs AltStore)
- Termux-style build system with git diff patches

**Core Philosophy**:
- Keep what works: ios_system's drop-in `#define system ios_system` pattern
- Extend capabilities: Add more syscall replacements (fork, exec, getenv, etc.)
- Don't reinvent: Use wasi-libc for POSIX, build on top of it
- Clear separation: root-packages (native core) vs packages (WASM installable)

---

## 1. Architecture

### Single App, Dual Capability

One binary detects its distribution channel at runtime:

```
┌─────────────────────────────────────────────────────────────────────┐
│                         a-Shell App                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  Distribution Detection:                                             │
│  - Checks entitlements/provisioning profile                          │
│  - App Store: WASM-only package installation                         │
│  - AltStore: Full native package installation                        │
│                                                                      │
├─────────────────────────────────────────────────────────────────────┤
│  Package Manager: Debian apt 3.1.16 (patched)                       │
│  - Built from upstream with iOS patches                              │
│  - Patches use git diffs (Termux-style)                              │
│  - No Swift replacement - actual apt binary                          │
│                                                                      │
├─────────────────────────────────────────────────────────────────────┤
│  Package Types:                                                      │
│  • Native (root-packages): Pre-bundled, always available            │
│  • Native (installable): AltStore only, apt install downloads .deb  │
│  • WASM (installable): Both stores, apt install downloads .wasm     │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Package Categories

**root-packages/** (Core Native - Opinionated essentials):
- apt, dpkg
- coreutils
- python-runtime (BeeWare)
- libz, libssl, libcurl
- These are the "distro" - what we think a Linux system needs
- Always native, always available, cannot be uninstalled

**packages/** (User-installable):
- vim, git, node, etc.
- WASM versions for App Store
- Native versions for AltStore
- User choice, can be installed/removed

---

## 2. ashell-system (ios_system evolution)

### What It Is

ashell-system is the **C library** that provides iOS-compatible replacements for standard Unix system calls. It's what makes a-Shell feel like Linux despite iOS restrictions.

**Core insight**: Packages compile normally with standard headers (`#include <stdlib.h>`), but at compile time, calls like `fork()` get redirected to `ios_fork()` which handles iOS limitations gracefully.

### Current State (ios_system)

Today, ios_system provides one replacement:

```c
// ios_system.h
extern int ios_system(char* cmd);

// ios_error.h - included by every package
#define system ios_system
```

Packages include `ios_error.h`, and their `system("ls")` becomes `ios_system("ls")` automatically.

### Extended State (ashell-system)

Expand the same pattern to all syscalls that need iOS adaptation:

```c
// ashell_error.h - drop-in replacements (NEW)
#ifndef ASHELL_ERROR_H
#define ASHELL_ERROR_H

// Process management
#define fork ios_fork
#define execv ios_execv
#define execvp ios_execvp
#define execve ios_execve

// Environment
#define getenv ios_getenv
#define setenv ios_setenv
#define putenv ios_putenv

// Exit
#define exit ios_exit

// Signals (limited support)
#define signal ios_signal
#define raise ios_raise

#endif
```

### Implementation Strategy

```c
// ashell_syscalls.c - Implementation of replacements

#include <errno.h>
#include <pthread.h>

// fork() doesn't exist on iOS - return error
int ios_fork(void) {
    errno = ENOSYS;  // "Function not implemented"
    return -1;
}

// execv() redirects to ios_system() with the command
int ios_execv(const char *path, char *const argv[]) {
    // Build command string from argv
    // Call ios_system() to execute
    // Never returns on success (matches execv semantics)
    return -1;
}

// getenv() returns per-session environment variables
// (not process-global, since we're single-process)
char *ios_getenv(const char *name) {
    // Look up in thread-local environment storage
    // Returns value or NULL
}

// exit() becomes pthread_exit() for threads
void ios_exit(int status) {
    // If main thread: clean exit
    // If worker thread: pthread_exit
}
```

### How Packages Use It

```c
// Package source code (vim, git, etc.)
// They write standard C code:

#include <stdlib.h>   // Normal system header
#include "ios_system.h"   // From ashell-system

int main(int argc, char **argv) {
    // These are redirected to ios_* versions at compile time:
    fork();           // Becomes ios_fork()
    execv(path, argv); // Becomes ios_execv()
    system(cmd);      // Becomes ios_system()

    // Thread-local I/O already provided:
    fprintf(stdout, "hello");  // Goes to ios_stdout()
}
```

### Build Integration

Packages compile with forced header inclusion:

```bash
# In package build.sh
export CFLAGS="$CFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"
```

This ensures ALL source files get the macro replacements, even system headers.

### Session Management

Since we can't fork(), we simulate sessions:

```c
// Session state per "shell session"
typedef struct {
    pid_t session_id;           // Simulated PID
    char cwd[PATH_MAX];         // Current directory
    char **env;                 // Environment variables
    FILE *stdin, *stdout, *stderr;  // Thread-local I/O
} ashell_session_t;

// Thread-local storage for current session
_Thread_local ashell_session_t *current_session;

// Get current session info
pid_t ios_getpid(void) {
    return current_session ? current_session->session_id : 0;
}

// Switch sessions (called when user switches tabs)
void ios_switchSession(const char *session_id) {
    // Save current session state
    // Load new session state
}
```

### PID Simulation

```c
// Simulated PIDs for compatibility
typedef uint32_t pid_t;

pid_t ios_fork(void) {
    // Can't actually fork on iOS
    // Return error, but set up a "virtual PID" for tracking
    static pid_t next_pid = 1000;
    pid_t virtual_pid = __atomic_fetch_add(&next_pid, 1, __ATOMIC_SEQ_CST);

    // Log the "fork" for compatibility
    // Some tools check pid to detect if they're child/parent
    errno = ENOSYS;
    return -1;
}
```

---

## 3. WASM Runtime (First-Class)

**Goal**: Make WASM feel native, not like a fallback.

### Architecture

```
WASM Command (vim.wasm)
    │
    ▼
┌──────────────────────────────┐
│  POSIX Layer                 │
│  • Signal emulation          │
│  • Process model             │
│  • Extended file descriptors │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│  wasi-libc                   │
│  • Standard C library        │
│  • Basic POSIX               │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│  Custom WASI syscalls        │
│  • ios_open_tty()            │
│  • ios_get_window_size()     │
│  • pipe2()                   │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│  WASM Engine                 │
│  • wasm3 OR WAMR             │
│  • AOT for hot paths         │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│  ios_system integration      │
│  • Direct stdio bridge       │
│  • No JavaScript layer       │
│  • Command registration      │
└──────────────────────────────┘
```

### Key Features

1. **Signal Emulation**: setjmp/longjmp at safe points
2. **Process Model**: WASM instance per "fork"
3. **TTY Passthrough**: Custom WASI syscalls to iOS terminal
4. **Zero-copy Pipes**: Shared buffers between WASM and native
5. **AOT Compilation**: For frequently used packages

---

## 4. Python

**Distribution**: Native pip with custom iOS wheel index

**Architecture**:
```
pip install numpy
    │
    ▼
Custom iOS wheel index (pip.ashell.dev)
    │
    ▼
Pre-built iOS wheels:
• numpy-1.26.0-cp312-cp312-ios_13_0_arm64_iphoneos.whl
• Built with cibuildwheel
• Platform tags: arm64_iphoneos, arm64_iphonesimulator, x86_64_iphonesimulator
```

**NOT through apt**: Python packages stay in Python ecosystem (pip)

---

## 5. Build System (ashell-packages)

**Style**: Termux-inspired

**Directory Structure**:
```
ashell-packages/
├── ashell_package.sh              # Build library
├── build.sh                       # Orchestration
├── clean.sh
├── root-packages/                 # Core distro (REQUIRED)
│   ├── apt/ (MOVED HERE)         # apt 3.1.16
│   │   ├── build.sh
│   │   └── patches/
│   │       ├── 01-prefix.patch
│   │       └── 02-ios-sandbox.patch
│   ├── dpkg/
│   ├── coreutils/
│   └── ...
├── packages/                      # User packages
│   ├── vim/                      # Builds vim.wasm
│   ├── git/
│   └── ...
└── scripts/
    └── ashell-fix-shebang
```

**Patch Philosophy**:
- Git diff patches apply to upstream source
- No forking entire repositories
- Substitute @ASHELL_PREFIX@ at build time
- Minimal patches - only what's needed for iOS

---

## 6. Immediate Actions

1. **Merge apt directories**: Move `ashell-packages/apt/` → `ashell-packages/root-packages/apt/`

2. **Update apt to 3.1.16**: Edit `root-packages/apt/build.sh`

3. **Remove Swift apt**: Deprecate `ashell-core/Sources/Commands/apt.swift` and PackageManager.swift

4. **Build real apt**: `./build.sh apt` should produce apt.framework

5. **Update documentation**: Clarify root-packages vs packages

---

## 7. Decisions Summary

| Topic | Decision |
|-------|----------|
| Package manager | Debian apt 3.1.16 (latest), patched |
| Patch style | Git diff patches (Termux-style) |
| apt location | root-packages/ (core system component) |
| Swift apt | REMOVE - use real apt binary |
| Distribution | Single app, detects App Store vs AltStore |
| App Store extensibility | WASM packages |
| AltStore extensibility | Native .deb packages |
| Python | pip + custom iOS wheel index |
| WASM engine | wasm3 or WAMR, wasi-libc base |
| POSIX layer | wasi-libc + custom extensions |
| Build system | Termux-style (bash + patches) |

---

## 8. What Was Removed/Fixed

**Removed**:
- Swift curated package manager (was never the goal)
- "apt for pre-bundled only" option (user rejected as "idiotic")
- Separate App Store vs AltStore apps (single app instead)
- Complex syscall replacement schemes (keep simple #define pattern)
- Multiple WASM strategies (chosen: wasi-libc + POSIX layer)

**Fixed**:
- ashell-system now extends ios_system pattern, doesn't replace it
- Clear distinction: root-packages (essential) vs packages (user choice)
- WASM is primary App Store mechanism, not fallback
- Python via pip (not apt) - proper separation

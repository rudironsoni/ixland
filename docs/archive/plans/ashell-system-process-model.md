# ashell-system Process Model: Termux vs iOS Analysis

**Date**: 2026-03-19
**Purpose**: Compare Termux's process management (Android) with ashell-system requirements (iOS)

---

## Executive Summary

| Aspect | Termux (Android) | ashell-system (iOS) |
|--------|------------------|---------------------|
| **Fork/Exec** | Real `fork()`/`exec()` via JNI | ❌ Not available - macro-based simulation |
| **Process Model** | True multi-process | Single-process, thread-based |
| **PID** | Real kernel PIDs | Virtual PIDs (simulated) |
| **Child Limit** | 32 (Android 12+ phantom killer) | Unlimited threads (memory-bound) |
| **Isolation** | Process boundaries | Thread-local storage |
| **Signal Handling** | Real signals | Emulated via setjmp/longjmp |
| **Drop-in Replacement** | N/A (native) | `#define fork ios_fork` |

---

## 1. Termux Process Architecture (Android)

### 1.1 Real fork()/exec() via JNI

Termux uses actual Linux system calls for process creation:

```c
// terminal-emulator/src/main/jni/termux.c
pid_t fork_result = fork();
if (fork_result == 0) {
    // Child process
    execvp(cmd, args);  // Replace process image
}
// Parent continues...
```

**Call Stack:**
```
Runtime.exec() (Java)
  → ProcessBuilder.start()
  → UNIXProcess_md.forkAndExec() (JNI)
  → fork() (bionic libc)
  → execvp() (bionic libc)
  → Linux kernel syscall
```

### 1.2 Phantom Process Killer (Android 12+)

Android 12 introduced `PhantomProcessList` that kills child processes:

- **Limit**: 32 phantom processes across ALL apps
- **Trigger**: Excessive CPU or process count
- **Mechanism**: `ActivityManagerService` monitors and kills

```java
// PhantomProcessList.trimPhantomProcessesIfNecessary()
if (mPhantomProcesses.size() > MAX_PHANTOM_PROCESSES) {
    // Kill oldest processes first
    proc.killLocked("Trimming phantom processes", true);
}
```

**Impact on Termux:**
- Commands like `make -j8` or many parallel jobs get killed
- Workaround: Users must disable phantom process killing via ADB/root
- Shows fork() model has limits even on Android

---

## 2. iOS Constraints (ashell-system Reality)

### 2.1 No fork()/exec() Available

iOS apps are **single-process by design**:

```c
// fork() returns -1 on iOS
pid_t fork(void) {
    errno = ENOSYS;  // "Function not implemented"
    return -1;
}

// exec() family also blocked
int execve(const char *path, char *const argv[], char *const envp[]) {
    errno = ENOSYS;
    return -1;
}
```

**Why:**
- iOS security model prohibits process creation
- App Sandbox prevents spawning external processes
- Code signing would fail for child processes

### 2.2 What iOS Allows

| Operation | iOS Support | Notes |
|-----------|-------------|-------|
| `pthread_create()` | ✅ Yes | Unlimited threads (memory bound) |
| `NSThread` | ✅ Yes | Objective-C wrapper |
| `dispatch_async()` | ✅ Yes | GCD queues |
| `Task` (Swift) | ✅ Yes | Swift concurrency |
| `system()` | ⚠️ Limited | Can call registered commands only |

---

## 3. ashell-system: Drop-in Replacement Pattern

### 3.1 Core Design: Macros in ashell_error.h

Following the ios_system pattern of `#define system ios_system`:

```c
// ashell_error.h - Drop-in syscall replacements
// Packages include this header, syscalls become ios_* calls

#ifndef ASHELL_ERROR_H
#define ASHELL_ERROR_H

#include <errno.h>
#include <sys/types.h>

// Process management
#define fork    ios_fork
#define vfork   ios_vfork
#define execv   ios_execv
#define execvp  ios_execvp
#define execve  ios_execve
#define waitpid ios_waitpid
#define wait    ios_wait
#define wait3   ios_wait3
#define wait4   ios_wait4

// Signals
#define signal  ios_signal
#define sigaction ios_sigaction
#define raise   ios_raise
#define kill    ios_kill

// Environment
#define getenv  ios_getenv
#define setenv  ios_setenv
#define putenv  ios_putenv
#define unsetenv ios_unsetenv

// Exit
#define exit    ios_exit
#define _exit   ios__exit

// Process info
#define getpid  ios_getpid
#define getppid ios_getppid
#define getpgrp ios_getpgrp
#define setsid  ios_setsid

#endif // ASHELL_ERROR_H
```

### 3.2 How Packages Use It

```bash
# In package build.sh, force include the header
export CFLAGS="$CFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"
export CPPFLAGS="$CPPFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"
```

**Result:**
```c
// Package source code (UNMODIFIED)
#include <unistd.h>   // Normal system header

int main() {
    pid_t pid = fork();  // Becomes ios_fork() at compile time
    if (pid == 0) {
        execvp("ls", args);  // Becomes ios_execvp()
    }
    waitpid(pid, &status, 0);  // Becomes ios_waitpid()
}
```

**No source patching required!**

### 3.3 ios_fork() Implementation

```c
// ashell_syscalls.c
#include <pthread.h>
#include <errno.h>

// Virtual PID system
static pid_t next_vpid = 1000;
static _Thread_local pid_t current_vpid = 0;

typedef struct {
    pid_t pid;
    pid_t ppid;
    pthread_t thread;
    char name[64];
    int status;
    bool running;
    void* stack;
} ashell_process_t;

static ashell_process_t* process_table[1024];

pid_t ios_fork(void) {
    // iOS doesn't support real fork()
    // Return ENOSYS per POSIX
    errno = ENOSYS;
    return -1;
}

pid_t ios_vfork(void) {
    // Allocate virtual PID for "process" tracking
    // No actual thread created yet - that's done on exec
    pid_t vpid = __atomic_fetch_add(&next_vpid, 1, __ATOMIC_SEQ_CST);

    ashell_process_t* proc = calloc(1, sizeof(ashell_process_t));
    proc->pid = vpid;
    proc->ppid = current_vpid ? current_vpid : 1000;
    proc->running = false;
    strncpy(proc->name, "vfork-child", 63);

    process_table[vpid % 1024] = proc;

    // For vfork, parent blocks until child execs or exits
    // In our model, we return and track it
    return vpid;
}

pid_t ios_getpid(void) {
    return current_vpid ? current_vpid : 1000;
}

pid_t ios_getppid(void) {
    ashell_process_t* proc = process_table[current_vpid % 1024];
    if (proc && proc->pid == current_vpid) {
        return proc->ppid;
    }
    return 1;  // init
}
```

### 3.4 ios_exec*() Family

```c
// exec() redirects to ios_system()
int ios_execve(const char *path, char *const argv[], char *const envp[]) {
    // Build command from argv
    char cmd[4096] = {0};
    for (int i = 0; argv[i]; i++) {
        if (i > 0) strncat(cmd, " ", sizeof(cmd)-1);
        strncat(cmd, argv[i], sizeof(cmd)-strlen(cmd)-1);
    }

    // Execute via ios_system (in same thread)
    int result = ios_system(cmd);

    // exec never returns on success
    // If we get here, it failed
    ios_exit(result);
    return -1;  // Never reached
}

int ios_execvp(const char *file, char *const argv[]) {
    // Search PATH
    char* path = find_in_path(file);
    if (!path) {
        errno = ENOENT;
        return -1;
    }
    return ios_execve(path, argv, environ);
}

int ios_execv(const char *path, char *const argv[]) {
    return ios_execve(path, argv, environ);
}
```

### 3.5 ios_waitpid() Implementation

```c
pid_t ios_waitpid(pid_t pid, int *status, int options) {
    if (pid < 0) {
        // Wait for any child - poll process table
        while (true) {
            for (int i = 0; i < 1024; i++) {
                ashell_process_t* proc = process_table[i];
                if (proc && proc->ppid == current_vpid && !proc->running) {
                    // Child completed
                    *status = proc->status;
                    pid_t result = proc->pid;
                    free(proc);
                    process_table[i] = NULL;
                    return result;
                }
            }

            if (options & WNOHANG) {
                return 0;
            }

            // Yield thread
            usleep(10000);  // 10ms
        }
    }

    // Wait for specific PID
    ashell_process_t* proc = process_table[pid % 1024];
    if (!proc || proc->ppid != current_vpid) {
        errno = ECHILD;
        return -1;
    }

    while (proc->running) {
        usleep(10000);
    }

    *status = proc->status;
    free(proc);
    process_table[pid % 1024] = NULL;
    return pid;
}
```

### 3.6 Session Management

```c
// Session = Simulated shell environment
typedef struct {
    pid_t session_pid;             // Virtual session PID
    char cwd[PATH_MAX];            // Current directory
    char** env;                    // Environment variables
    FILE* stdin;                   // Thread-local stdin
    FILE* stdout;                  // Thread-local stdout
    FILE* stderr;                  // Thread-local stderr
} ashell_session_t;

// Thread-local current session
_Thread_local ashell_session_t* current_session;
_Thread_local pid_t current_vpid = 0;

// Session switching (when user switches tabs)
void ios_switchSession(const char* session_id) {
    // Save current session state
    // Load new session
    // Update thread-local current_session
}

// Get/set environment (thread-local, not process-global)
char* ios_getenv(const char* name) {
    if (!current_session) return NULL;

    for (int i = 0; current_session->env[i]; i++) {
        char* eq = strchr(current_session->env[i], '=');
        if (eq && strncmp(current_session->env[i], name, eq - current_session->env[i]) == 0) {
            return eq + 1;
        }
    }
    return NULL;
}

int ios_setenv(const char* name, const char* value, int overwrite) {
    if (!current_session) {
        errno = EINVAL;
        return -1;
    }
    // Update session-local environment
    // ...
    return 0;
}
```

---

## 4. Shell Support (bash + zsh)

### 4.1 No Source Patching Required

**Build configuration only:**

```bash
# bash/build.sh
export CFLAGS="$CFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"
export CPPFLAGS="$CPPFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"
export LDFLAGS="$LDFLAGS -L$ASHELL_PREFIX/lib -lashell-system"

./configure --host=arm64-apple-ios \
            --prefix=$ASHELL_PREFIX \
            --without-bash-malloc \
            --disable-nls
```

**Result:**
- Shell compiles unmodified from upstream source
- All fork/exec become ios_* calls automatically
- Only minimal patches needed for iOS-specific issues

### 4.2 Minimal Patches Needed

| Patch | Purpose |
|-------|---------|
| `01-configure-ios.patch` | Skip feature checks that fail on iOS |
| `02-no-procfs.patch` | Use ashell_proc.h instead of /proc |
| `03-static-users.patch` | Static user/group (no /etc/passwd) |
| `04-tty-handling.patch` | Use ios_opentty() for terminal |

**NO patches for fork/exec** - handled by macros!

### 4.3 Process Substitution

zsh's `<(cmd)` and bash's process substitution need special handling:

```c
// ios_process_substitution.c
// Creates a named pipe, runs command in background thread,
// returns pipe fd for reading

int ios_process_substitution(const char* cmd, int mode) {
    // Create pipe
    int pipefd[2];
    pipe(pipefd);

    // Create background "process" (actually thread)
    pthread_t thread;
    struct proc_sub_args* args = malloc(sizeof(*args));
    args->cmd = strdup(cmd);
    args->write_fd = pipefd[1];

    pthread_create(&thread, NULL, proc_sub_thread, args);

    // Return read end
    return pipefd[0];
}
```

---

## 5. Comparison: Termux vs ashell-system

### 5.1 Scenarios That Work

| Scenario | Termux | ashell-system |
|----------|--------|---------------|
| `ls -la` | ✅ Works | ✅ Works |
| `cat file.txt` | ✅ Works | ✅ Works |
| `grep pattern file` | ✅ Works | ✅ Works |
| `cmd1 \| cmd2` | ✅ Pipe between processes | ✅ Pipe within threads |
| Shell scripts | ✅ fork/exec | ✅ ios_system() dispatch |

### 5.2 What Breaks

| Scenario | Termux | ashell-system |
|----------|--------|---------------|
| `fork()` in C code | ✅ Works | ❌ Returns ENOSYS |
| `daemon()` function | ✅ Detaches | ❌ Cannot detach |
| Real `/proc` filesystem | ✅ Available | ❌ Emulated only |
| `ptrace()` debugging | ✅ Works | ❌ Not available |

---

## 6. Key Design Principles

### 6.1 Drop-in Replacement

**User requirement:**
```c
// Package code writes STANDARD C
#include <unistd.h>
int main() {
    fork();  // Works via macro
    return 0;
}
```

**NOT:**
```c
// DON'T require packages to change their code
#include "ashell_system.h"  // ❌ Bad - non-standard
ios_fork();  // ❌ Bad - proprietary API
```

### 6.2 Header Inclusion Strategy

**Force inclusion at compile time:**
```bash
# Package build.sh
export CFLAGS="$CFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"
```

**This ensures:**
- All source files get the macros
- Even system headers get redirected
- No code changes in packages

### 6.3 Two-Layer Approach (Optional Enhancement)

**Layer 1: Header macros** (compile-time)
```c
#define fork ios_fork
```

**Layer 2: Dyld interposition** (runtime)
```c
// For prebuilt binaries that weren't compiled with headers
// Uses __dyld_insert_libraries on iOS (jailbreak only)
```

**App Store**: Layer 1 only (sufficient for our build system)
**Jailbreak**: Can add Layer 2 for binaries we didn't build

---

## 7. Implementation Priority

### Phase 1: Core Syscalls (Week 1)
- `ios_fork()` - Returns ENOSYS
- `ios_vfork()` - Virtual PID allocation
- `ios_execv*()` - Redirect to ios_system()
- `ios_waitpid()` - Poll process table
- `ios_getenv/setenv()` - Session-local

### Phase 2: Signal Emulation (Week 2)
- `ios_signal()` - Register handlers
- `ios_sigaction()` - POSIX signal API
- `ios_raise()` - Trigger signals
- setjmp/longjmp integration

### Phase 3: Process Table (Week 2)
- Virtual PID allocation
- Process tracking structure
- Parent/child relationships

### Phase 4: Session Management (Week 3)
- Thread-local sessions
- Environment isolation
- Session switching

### Phase 5: Shell Testing (Week 4)
- Compile dash (minimal shell)
- Compile bash
- Compile zsh
- Test job control

---

## 8. References

1. **Termux Execution Environment**: https://github.com/termux/termux-packages/wiki/Termux-execution-environment
2. **Phantom Process Killer**: https://github.com/agnostic-apollo/Android-Docs/blob/master/en/docs/apps/processes/phantom-cached-and-empty-processes.md
3. **iOS System Services**: Apple Developer Documentation
4. **ios_system**: https://github.com/holzschu/ios_system


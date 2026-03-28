# iXland Architecture

## Overview

iXland is a Linux-like virtual kernel subsystem for iOS. It simulates Linux syscalls using pthreads since iOS forbids fork()/exec().

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        iXland App                           │
│                    (iOS Terminal UI)                        │
└─────────────────────┬───────────────────────────────────────┘
                      │ Swift/Obj-C Bridge
┌─────────────────────▼───────────────────────────────────────┐
│                     iXland System                           │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐ │
│  │   Kernel     │ │   Runtime    │ │   File System        │ │
│  │  ┌────────┐  │ │  ┌────────┐  │ │  ┌────────────────┐  │ │
│  │  │ Task   │  │ │  │Native  │  │ │  │ VFS Layer      │  │ │
│  │  │ Signal │  │ │  │WASI    │  │ │  │ FD Table       │  │ │
│  │  │ Exec   │  │ │  │Script  │  │ │  │ Path Translate │  │ │
│  │  └────────┘  │ │  └────────┘  │ │  └────────────────┘  │ │
│  └──────────────┘ └──────────────┘ └──────────────────────┘ │
└─────────────────────┬───────────────────────────────────────┘
                      │ C ABI
┌─────────────────────▼───────────────────────────────────────┐
│                     iXland libc                             │
│         (Public Headers + Core API Functions)               │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐ │
│  │linux/*.h     │ │iox/*.h       │ │ Core Functions       │ │
│  │  unistd.h    │ │  iox.h       │ │  iox_version()       │ │
│  │  stat.h      │ │  iox_types.h │ │  iox_strerror()      │ │
│  │  signal.h    │ │  syscalls.h  │ │  iox_perror()        │ │
│  └──────────────┘ └──────────────┘ └──────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Component Responsibilities

### ixland-libc

**Purpose:** Public ABI boundary - headers and minimal runtime

**Key Files:**
- `include/iox/iox.h` - Umbrella header
- `include/iox/iox_types.h` - Type definitions
- `include/iox/iox_syscalls.h` - Syscall prototypes
- `include/linux/*.h` - Linux-compatible headers

**Targets:**
- `ixland-libc-headers` (INTERFACE) - Public headers only
- `ixland-libc-core` (STATIC) - Core API functions
- `ixland-libc-usersdb` (STATIC) - User/group stubs

### ixland-system

**Purpose:** Kernel implementation - process management, syscalls, VFS

**Subsystems:**

#### Kernel (`kernel/`)
- **task/** - Process lifecycle (fork, exec, exit, wait)
- **signal/** - Signal handling and delivery
- **exec/** - Program execution
- **internal/** - Internal headers

#### Runtime (`runtime/`)
- **native/** - Native iOS execution
- **wasi/** - WebAssembly (WAMR) execution

#### File System (`fs/`)
- **vfs.c** - Virtual filesystem layer
- **fdtable.c** - File descriptor management
- **iox_path.c** - Path translation

#### Drivers (`drivers/`)
- **tty/** - TTY/PTY support

### ixland-wasm

**Purpose:** WebAssembly contract definitions (future extraction target)

**Components:**
- `ixland-wasm-engine/` - Engine-neutral backend contract
- `ixland-wasm-host/` - Host service contract
- `ixland-wasm-wasi/` - WASI guest ABI policy

## Data Flows

### Process Creation Flow

```
1. App calls iox_fork()
   ↓
2. Kernel: iox_task_alloc() - allocate task struct
   ↓
3. Kernel: iox_alloc_pid() - assign unique PID
   ↓
4. Kernel: Link to parent's children list
   ↓
5. Kernel: pthread_create() - start thread
   ↓
6. Child runs with new PID
```

### Signal Delivery Flow

```
1. Process calls iox_kill(target_pid, sig)
   ↓
2. Kernel: Lookup target in process table
   ↓
3. Kernel: Check permissions
   ↓
4. Kernel: pthread_kill(target->thread, SIGUSR1)
   ↓
5. Target: Signal handler invoked
   ↓
6. Target: Action taken (default/ignore/handler)
```

### File Operation Flow

```
1. Process calls iox_open(path, flags)
   ↓
2. VFS: Translate path (handle cwd, symlinks)
   ↓
3. VFS: Call native open() with translated path
   ↓
4. FD Table: Allocate fd slot
   ↓
5. Return fd to process
```

## Key Data Structures

### iox_task_t (Process)

```c
struct iox_task {
    pid_t pid;              // Process ID
    pid_t ppid;             // Parent PID
    pid_t pgid;             // Process group ID
    pid_t sid;              // Session ID

    atomic_int state;       // Task state (RUNNING, ZOMBIE, etc.)
    pthread_t thread;       // Thread handle

    iox_files_t *files;     // File descriptor table
    iox_fs_t *fs;           // Filesystem context (cwd, root)
    iox_sighand_t *sighand; // Signal handlers

    // Tree links
    struct iox_task *parent;
    struct iox_task *children;
    struct iox_task *next_sibling;
};
```

### Process States

- `IOX_TASK_RUNNING` - Normal execution
- `IOX_TASK_INTERRUPTIBLE` - Sleeping, can be interrupted
- `IOX_TASK_UNINTERRUPTIBLE` - Sleeping, cannot be interrupted
- `IOX_TASK_STOPPED` - Stopped by signal
- `IOX_TASK_ZOMBIE` - Exited, waiting for parent wait()
- `IOX_TASK_DEAD` - Being reaped

### FD Table

```c
typedef struct iox_files {
    struct file *fd[IOX_MAX_FD];  // File pointers
    atomic_int count;              // Reference count
    pthread_mutex_t lock;          // Access lock
} iox_files_t;
```

## Threading Model

Since iOS forbids fork(), iXland uses pthreads:

1. **Each "process" is a pthread**
2. **Thread-local storage** tracks current task
3. **setjmp/longjmp** for vfork() simulation
4. **pthread_kill** for signal delivery

### Thread-Local Current Task

```c
_Thread_local iox_task_t *iox_current = NULL;

iox_task_t *iox_current_task(void) {
    return iox_current;
}

void iox_set_current_task(iox_task_t *task) {
    iox_current = task;
}
```

## Memory Management

### Reference Counting

All shared structures use atomic reference counting:

```c
void iox_task_ref(iox_task_t *task) {
    atomic_fetch_add(&task->refs, 1);
}

void iox_task_unref(iox_task_t *task) {
    if (atomic_fetch_sub(&task->refs, 1) == 1) {
        iox_task_free(task);
    }
}
```

### Allocation Patterns

- **Task structs** - kmalloc-style allocator (or malloc)
- **PID numbers** - Atomic counter with reuse
- **FD table** - Reference counted, copied on fork
- **Signal handlers** - Per-process, inherited on fork

## Build Dependencies

```
ixland-libc-headers (no deps)
    ↓
ixland-libc-core (uses headers)
    ↓
ixland-wasm-contracts (no deps)
    ↓
ixland-system (uses libc-headers, wasm-contracts)
```

## Future Directions

1. **Runtime extraction** - Move runtime/wasi to ixland-wasm
2. **Driver extraction** - TTY drivers may move to separate component
3. **Networking** - Socket layer may get its own boundary
4. **Security** - Capsicum-style capabilities for sandboxing

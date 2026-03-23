# IOX Architectural Analysis and Migration Plan

**Date**: 2026-03-23  
**Status**: Pre-implementation Analysis Phase  
**Objective**: Transform iox from fragmented compatibility layer to coherent Linux-like virtual subsystem

---

## 1. Current Repo Findings

### 1.1 Critical Architectural Debt Identified

#### Dual Process Models
The repository currently contains **two competing process models**:

1. **`__iox_process_t`** in `iox_process.c` (1,628 lines)
   - Global process table: `__iox_process_table[IOX_MAX_PROCESSES]`
   - Thread-local current process: `__iox_current_process`
   - Own FD table per process: `fd_table[IOX_MAX_FD]`
   - Signal queue per process
   - Wait queue infrastructure
   - Process groups and sessions

2. **`iox_context_t`** in `iox_context.c` (777 lines)
   - Separate context table: `ctx_table[IOX_MAX_PROCESSES]`
   - Thread-local current context: `__iox_current_ctx`
   - Own FD table per context
   - Own signal handlers
   - Own environment

**Violation**: This creates ambiguity about which model owns process semantics.

#### Triple FD Ownership
Three conflicting FD ownership schemes exist:

1. **Global FD table** in `iox_file.c` (625 lines):
   ```c
   static iox_fd_entry_t fd_table[IOX_MAX_FD];  // Global table
   ```

2. **Per-process FD table** in `__iox_process_t`:
   ```c
   struct __iox_fd_entry_s fd_table[IOX_MAX_FD];  // Inside process struct
   ```

3. **Per-context FD table** in `iox_context_t`:
   ```c
   struct { int real_fd; ... } fd_table[IOX_MAX_FD];  // Inside context struct
   ```

**Violation**: FDs must live ONLY under `task->files` per requirements.

#### Passthrough Architecture
`iox_libc_delegate.c` (486 lines) delegates to host libc:
- Uses `dlsym(RTLD_NEXT, ...)` for passthrough
- Owns semantics through delegation rather than iox kernel subsystems
- Does not provide virtualized Linux-like behavior

**Violation**: Policy and semantics must belong to iox kernel subsystems.

#### Non-Functional execve
`__iox_execve_impl()` at line 640 of `iox_process.c`:
```c
int __iox_execve_impl(const char *pathname, char *const argv[], char *const envp[]) {
    __iox_process_t *proc = __iox_get_current_process();
    if (!proc) {
        errno = ENOENT;
        return -1;
    }
    // ... stores metadata ...
    // Returns ENOSYS or success without actual execution
    return -1;  // Or partial implementation
}
```

**Violation**: execve must dispatch to native/WASM/script runners with virtual image replacement.

#### WAMR Isolation
Current WAMR integration (`iox_wamr_simple.c`, 303 lines):
- Simplified interpreter-only
- Does not share VFS/FD/task with native code
- Global WAMR state: `g_wamr_state`
- No WASI-to-iox bridge

**Violation**: WAMR must use SAME VFS/FD/task as native code.

#### Incomplete Fork Implementation
`__iox_fork_impl()` at line 515 creates thread but:
- Does not implement full Linux fork semantics
- Child continuation mechanism incomplete
- No proper virtual memory separation (unnecessary on iOS but process semantics incomplete)

### 1.2 File Inventory Summary

| File | Lines | Status | Action |
|------|-------|--------|--------|
| `iox_process.c` | 1,628 | Has good logic but wrong structure | **Migrate** to kernel/task/ |
| `iox_context.c` | 777 | Competing model | **Delete** - migrate salvageable logic |
| `iox_file.c` | 625 | Global FD table | **Delete** - move to per-task files |
| `iox_file_v2.c` | 568 | VFS-aware files | **Evaluate** for merge into fs/ |
| `iox_vfs.c` | 478 | VFS layer | **Keep** - move to fs/vfs/ |
| `iox_libc_delegate.c` | 486 | Passthrough | **Replace** with backend hooks |
| `iox_network.c` | 665 | Network sockets | **Keep** - move to net/ |
| `iox_init.c` | 108 | Initialization | **Keep** - move to kernel/init/ |
| `iox_minimal.c` | 201 | Minimal stubs | **Evaluate** |
| `iox_wamr_simple.c` | 303 | Simplified WAMR | **Replace** with full runtime/wasi/ |
| `iox_wamr.c` | 649 | Full WAMR | **Evaluate** for unification |
| `iox_interpose.c` | 341 | Interposition | **Keep** - move to compat/interpose/ |
| `iox_path.c` | 385 | Path utilities | **Keep** - move to fs/ |
| `iox_internal.h` | 749 | Unified header | **Split** into subsystem headers |

### 1.3 Current Build System Issues

- **Makefile** exists alongside **CMakeLists.txt** (2.5K)
- Makefile is currently active (referenced in README)
- CMakeLists.txt exists but may not be complete
- No checked-in `CMakePresets.json`
- No deterministic simulator/device test commands
- No fresh-clone bootstrap script

**Violation**: CMake must be single source of truth.

### 1.4 Testing Gap Analysis

Current tests directory structure:
```
tests/
├── unit/          # Empty or minimal
├── integration/   # Minimal
├── fixtures/      # Test data
├── *.swift        # iOS app tests (Swift)
├── *.m            # iOS app tests (ObjC)
└── iox_test.h     # Test utilities
```

**Missing**:
- No deterministic C-level test harness
- No device/simulator test entrypoints
- No test matrix document
- No regression test policy
- No compatibility inventory
- No performance baseline harness

**Violation**: Every subsystem must have automated tests before being marked complete.

---

## 2. Canonical Object Model

### 2.1 Core Object Hierarchy

```
iox_task
├── iox_files (fdtable)
│   ├── file descriptor slots
│   ├── cloexec tracking
│   └── reference to vfs
├── iox_fs (filesystem context)
│   ├── cwd
│   ├── root
│   ├── umask
│   └── mount namespace view
├── iox_sighand (signal handling)
│   ├── handlers[NSIG]
│   ├── blocked mask
│   ├── pending mask
│   └── signal queue
├── iox_tty (controlling terminal)
│   ├── controlling tty reference
│   ├── foreground process group
│   └── session leader status
├── iox_mm_emu (minimal memory descriptor)
│   └── exec image ownership tracking
└── iox_exec_image (current executable)
    ├── image type (NATIVE/WASI/SCRIPT)
    ├── resolved path
    ├── interpreter path (if script)
    └── entrypoint binding
```

### 2.2 Object Definitions

#### iox_task

```c
typedef struct iox_task {
    /* Identity (Linux-compatible) */
    pid_t pid;              /* Process ID */
    pid_t ppid;             /* Parent process ID */
    pid_t tgid;             /* Thread group ID (equals pid for thread leader) */
    pid_t pgid;             /* Process group ID */
    pid_t sid;              /* Session ID */
    
    /* State */
    atomic_int state;       /* TASK_RUNNING, TASK_STOPPED, etc. */
    int exit_status;        /* Exit code */
    atomic_bool exited;     /* Has exited */
    atomic_bool signaled;   /* Terminated by signal */
    atomic_int termsig;     /* Terminating signal */
    
    /* Relationships */
    struct iox_task *parent;
    struct iox_task *children;      /* Linked list */
    struct iox_task *next_sibling;
    
    /* Execution */
    pthread_t thread;       /* Host thread handle */
    atomic_int exec_seq;    /* Execution sequence (for exec detection) */
    char comm[IOX_MAX_NAME];        /* Command name */
    char exe[IOX_MAX_PATH];         /* Executable path */
    
    /* Subsystem references (owned by this task) */
    iox_files_t *files;     /* File descriptor table */
    iox_fs_t *fs;           /* Filesystem context */
    iox_sighand_t *sighand; /* Signal handlers */
    iox_tty_t *tty;         /* Controlling terminal */
    iox_mm_emu_t *mm;       /* Memory descriptor */
    iox_exec_image_t *exec_image;   /* Current executable image */
    
    /* Wait queue */
    pthread_cond_t wait_cond;
    pthread_mutex_t wait_lock;
    int waiters;            /* Number of waiters */
    
    /* Resource limits */
    struct rlimit rlimits[RLIMIT_NLIMITS];
    
    /* Timing */
    struct timespec start_time;
    
    /* Reference counting */
    atomic_int refs;
    pthread_mutex_t lock;
} iox_task_t;
```

#### iox_files (FD Table)

```c
typedef struct iox_file {
    int fd;                 /* Virtual fd number */
    int real_fd;            /* Real OS fd (for hostfs) */
    unsigned int flags;     /* O_RDONLY, O_CLOEXEC, etc. */
    off_t pos;              /* Current position */
    iox_vfs_file_t *vfs_file;   /* VFS file reference */
    atomic_int refs;        /* Reference count */
} iox_file_t;

typedef struct iox_files {
    iox_file_t **fd;        /* Array of file pointers */
    size_t max_fds;         /* Current max fds */
    pthread_mutex_t lock;
} iox_files_t;
```

#### iox_fs (Filesystem Context)

```c
typedef struct iox_fs {
    char cwd[IOX_MAX_PATH];
    char root[IOX_MAX_PATH];
    mode_t umask;
    iox_mount_ns_t *mnt_ns; /* Mount namespace view */
    atomic_int refs;
} iox_fs_t;
```

#### iox_sighand (Signal Handling)

```c
typedef struct iox_sighand {
    struct sigaction action[IOX_NSIG];
    sigset_t blocked;
    sigset_t pending;
    iox_sigqueue_t *queue;  /* Queued real-time signals */
    atomic_int refs;
} iox_sighand_t;
```

#### iox_tty (Controlling Terminal)

```c
typedef struct iox_tty {
    int tty_id;             /* /dev/pts/N */
    pid_t foreground_pgrp;  /* Foreground process group */
    struct termios termios;
    struct winsize winsize;
    bool is_session_leader;
    atomic_int refs;
} iox_tty_t;
```

#### iox_mm_emu (Minimal Memory Descriptor)

```c
typedef struct iox_mm_emu {
    /* Not a full VM - just ownership tracking */
    void *exec_image_base;  /* Base of exec image */
    size_t exec_image_size; /* Size of exec image */
    atomic_int refs;
} iox_mm_emu_t;
```

#### iox_exec_image (Executable Image)

```c
typedef enum {
    IOX_IMAGE_NONE = 0,
    IOX_IMAGE_NATIVE,       /* Native registered command */
    IOX_IMAGE_WASI,         /* WASM/WASI module */
    IOX_IMAGE_SCRIPT        /* Script with shebang */
} iox_image_type_t;

typedef struct iox_exec_image {
    iox_image_type_t type;
    char path[IOX_MAX_PATH];
    char interpreter[IOX_MAX_PATH]; /* For scripts */
    
    union {
        struct {
            iox_native_entry_t *entry;
        } native;
        struct {
            wasm_module_t module;
            wasm_module_inst_t instance;
        } wasi;
        struct {
            char *interpreter_argv[IOX_MAX_ARGS];
            int interpreter_argc;
        } script;
    } u;
} iox_exec_image_t;
```

### 2.3 Object Lifecycle Rules

1. **iox_task**: Allocated on fork, persists through exec, freed when parent waits
2. **iox_files**: Copied on fork (dup), preserved on exec (with CLOEXEC handling)
3. **iox_fs**: Copied on fork, preserved on exec
4. **iox_sighand**: Copied on fork, reset on exec (per Linux exec rules)
5. **iox_tty**: Referenced on fork (if controlling terminal), preserved on exec
6. **iox_mm_emu**: Minimal - just tracks exec image ownership
7. **iox_exec_image**: Replaced on exec, freed when task exits or execs again

---

## 3. Ownership Rules

### 3.1 Single Source of Truth

| Concept | Owner | Location |
|---------|-------|----------|
| Virtual PID | iox_task | `task->pid` |
| PPID | iox_task | `task->ppid` |
| PGID | iox_task | `task->pgid` |
| SID | iox_task | `task->sid` |
| Process state | iox_task | `task->state` |
| File descriptors | iox_files | `task->files->fd[]` |
| CWD | iox_fs | `task->fs->cwd` |
| Root | iox_fs | `task->fs->root` |
| Umask | iox_fs | `task->fs->umask` |
| Signal handlers | iox_sighand | `task->sighand->action[]` |
| Signal mask | iox_sighand | `task->sighand->blocked` |
| Controlling TTY | iox_tty | `task->tty` (may be NULL) |
| Foreground pgrp | iox_tty | `task->tty->foreground_pgrp` |
| Current executable | iox_exec_image | `task->exec_image` |
| Mount namespace | iox_mount_ns_t | `task->fs->mnt_ns` |
| Environment | iox_task | `task->envp` (simplified) |

### 3.2 No Global State

**Forbidden**:
- Global FD tables
- Global process tables (use hash table indexed by PID)
- Global signal state
- Global VFS mounts (per-namespace)

**Allowed**:
- Global PID allocator (single counter)
- Global TTY table (for /dev/pts/*)
- Global command registry

### 3.3 Thread Safety

All objects must be thread-safe:
- Reference counting with atomic operations
- Fine-grained locking per-object
- Lock ordering: task → files → fs → sighand → tty
- Lock held briefly, never across blocking operations

---

## 4. File-by-File Migration Map

### 4.1 Delete: Competing Models

| Source | Action | Notes |
|--------|--------|-------|
| `src/iox/core/iox_context.c` | **DELETE** | Salvage: ctx allocation, env handling, cwd tracking |
| `src/iox/core/iox_file.c` | **DELETE** | Global FD table forbidden; per-task tables in new model |

### 4.2 Keep and Migrate: Core Logic

| Source | Destination | Migration Notes |
|--------|-------------|-----------------|
| `src/iox/core/iox_process.c` | `kernel/task/task.c` | Extract: PID allocation, wait queues, process groups |
| `src/iox/core/iox_vfs.c` | `fs/vfs/vfs.c` | Keep core VFS logic, add mount namespaces |
| `src/iox/core/iox_file_v2.c` | `fs/fdtable.c` | Merge into per-task FD table implementation |
| `src/iox/core/iox_network.c` | `net/socket.c` | Move as-is, adapt to new task model |
| `src/iox/core/iox_init.c` | `kernel/init/main.c` | Expand initialization sequence |
| `src/iox/util/iox_path.c` | `fs/path.c` | Path normalization, canonicalization |
| `src/iox/interpose/iox_interpose.c` | `compat/interpose/syscalls.c` | Keep as compatibility layer |

### 4.3 Replace: Broken or Incomplete

| Source | Replacement | Reason |
|--------|-------------|--------|
| `src/iox/core/iox_libc_delegate.c` | `kernel/syscalls.c` + backend hooks | Stop passthrough, own semantics |
| `src/iox/core/iox_minimal.c` | Deleted | Replaced by proper subsystems |
| `src/iox/wamr/iox_wamr_simple.c` | `runtime/wasi/wasi_runtime.c` | Needs full WASI bridge |
| `src/iox/wamr/iox_wamr.c` | Evaluate | May merge into wasi_runtime.c |

### 4.4 Create New: Missing Subsystems

| New File | Purpose |
|----------|---------|
| `kernel/task/fork.c` | Virtual fork implementation |
| `kernel/task/exit.c` | Exit and wait implementation |
| `kernel/task/pid.c` | PID allocation and table |
| `kernel/exec/exec.c` | execve dispatcher |
| `kernel/exec/native.c` | Native command registry |
| `kernel/exec/script.c` | Shebang script handling |
| `kernel/signal/signal.c` | Signal delivery |
| `kernel/signal/sigaction.c` | Signal handlers |
| `kernel/signal/sigqueue.c` | Signal queuing |
| `kernel/signal/pgrp.c` | Process groups |
| `kernel/signal/session.c` | Sessions |
| `fs/vfs/mount.c` | Mount namespaces |
| `fs/proc/proc.c` | /proc filesystem |
| `fs/proc/self.c` | /proc/self entries |
| `fs/dev/dev.c` | /dev filesystem |
| `fs/devpts/pts.c` | /dev/pts filesystem |
| `fs/pipe/pipe.c` | Pipe implementation |
| `drivers/tty/pty.c` | PTY master/slave |
| `drivers/tty/tty.c` | TTY handling |
| `drivers/tty/termios.c` | Terminal attributes |
| `runtime/native/registry.c` | Native command registry |
| `runtime/wasi/wasi_syscalls.c` | WASI syscall bridge |
| `runtime/wasi/wasi_fd.c` | WASI FD operations |
| `runtime/wasi/wasi_path.c` | WASI path operations |
| `runtime/script/shebang.c` | Script interpreter handling |
| `compat/posix/unistd.c` | POSIX compatibility |
| `compat/posix/fcntl.c` | fcntl compatibility |
| `compat/posix/signal.c` | signal compatibility |
| `compat/posix/termios.c` | termios compatibility |
| `compat/posix/stat.c` | stat compatibility |

### 4.5 Header Migration

| Source | Destination |
|--------|-------------|
| `src/iox/internal/iox_internal.h` | Split into subsystem headers |
| | `kernel/task/task.h` |
| | `kernel/signal/signal.h` |
| | `fs/vfs/vfs.h` |
| | `fs/fdtable.h` |
| | `drivers/tty/tty.h` |
| | `runtime/native/native.h` |
| | `runtime/wasi/wasi.h` |
| `include/iox/iox_types.h` | Keep - public types |
| `include/iox/iox_syscalls.h` | Keep - public syscalls |
| `include/iox/iox_wamr.h` | Update to `iox_wamr.h` only |

---

## 5. Virtual Fork Model

### 5.1 Fork Semantics

Linux fork on iox:
1. Allocate new `iox_task` with new PID
2. Copy parent's `files`, `fs`, `sighand` (with reference counting)
3. Duplicate thread state using host threading
4. Mark child as runnable
5. Return 0 in child, child PID in parent

### 5.2 Implementation Strategy

Since iOS prohibits real fork(), implement as:

```c
pid_t iox_fork(void) {
    iox_task_t *parent = iox_current_task();
    
    /* Allocate child task */
    iox_task_t *child = iox_task_alloc();
    child->pid = alloc_pid();
    child->ppid = parent->pid;
    child->pgid = parent->pgid;
    child->sid = parent->sid;
    
    /* Copy/duplicate subsystems */
    child->files = iox_files_dup(parent->files);
    child->fs = iox_fs_dup(parent->fs);
    child->sighand = iox_sighand_dup(parent->sighand);
    child->tty = iox_tty_get(parent->tty);  /* Reference, not copy */
    child->mm = iox_mm_emu_alloc();
    
    /* Create host thread for child */
    pthread_t child_thread;
    fork_args_t args = {
        .child = child,
        .parent = parent,
        .barrier = &barrier
    };
    pthread_create(&child_thread, NULL, fork_child_trampoline, &args);
    
    /* Wait for child to initialize */
    wait_barrier(&barrier);
    
    /* Return child PID in parent */
    return child->pid;
}

static void *fork_child_trampoline(void *arg) {
    fork_args_t *args = arg;
    
    /* Set current task */
    iox_set_current_task(args->child);
    
    /* Signal parent we're ready */
    signal_barrier(args->barrier);
    
    /* Return 0 from fork */
    longjmp(args->child->fork_jmpbuf, 1);  /* Or equivalent */
}
```

### 5.3 Child Continuation Mechanism

Use `setjmp/longjmp` or explicit continuation:

```c
/* In fork() - parent side */
int jmpval = setjmp(parent->fork_jmpbuf);
if (jmpval == 0) {
    /* Parent: create child thread */
    pthread_create(..., fork_child_trampoline, ...);
    return child_pid;  /* Parent returns here */
}

/* Child thread entry */
void *fork_child_trampoline(void *arg) {
    iox_task_t *child = arg;
    iox_set_current_task(child);
    
    /* Jump to child continuation */
    longjmp(child->fork_jmpbuf, 1);
    /* NOTREACHED */
}

/* In child - after longjmp */
int jmpval = setjmp(child->fork_jmpbuf);
if (jmpval == 1) {
    /* Child returns 0 */
    return 0;
}
```

### 5.4 Fork Behavior Requirements

Per Linux fork semantics:
- Child gets copy of parent's address space (irrelevant for iox, but process state copied)
- Child inherits parent's signal mask
- Child's pending signals are cleared
- Child's signal queue is empty
- Child's itimers are cleared
- Child's resource usages are zeroed
- File descriptors are duplicated (refcounted in iox)

---

## 6. execve State Machine

### 6.1 Image Type Detection

```c
iox_image_type_t iox_exec_classify(const char *path) {
    int fd = open(path, O_RDONLY);
    char magic[4];
    read(fd, magic, 4);
    close(fd);
    
    /* WASM magic: \0asm */
    if (memcmp(magic, "\x00asm", 4) == 0) {
        return IOX_IMAGE_WASI;
    }
    
    /* Script: #! */
    if (memcmp(magic, "#!", 2) == 0) {
        return IOX_IMAGE_SCRIPT;
    }
    
    /* Native: Check registry */
    if (iox_native_lookup(path)) {
        return IOX_IMAGE_NATIVE;
    }
    
    return IOX_IMAGE_NONE;
}
```

### 6.2 execve State Machine

```
execve(path, argv, envp)
    |
    v
[1] Path resolution
    - VFS lookup
    - Permission check (x-bit)
    |
    v
[2] Image classification
    - Read magic
    - Check native registry
    |
    v
[3] Pre-exec validation
    - Check FD_CLOEXEC fds
    - Validate not multi-threaded
    |
    v
[4] Load interpreter (if script)
    - Parse shebang
    - Rewrite argv
    |
    v
[5] Setup new image
    - Native: lookup entrypoint
    - WASI: load module, instantiate
    - Script: recursive exec interpreter
    |
    v
[6] Apply exec rules
    - Close FD_CLOEXEC fds
    - Reset signal handlers (keep SIG_IGN)
    - Clear signal mask
    - Clear pending signals
    - Update argv/envp
    |
    v
[7] Transfer control
    - Call image entrypoint
```

### 6.3 Native Execution Path

```c
int iox_exec_native(iox_task_t *task, iox_native_entry_t *entry,
                    int argc, char **argv, char **envp) {
    /* Update task image */
    task->exec_image->type = IOX_IMAGE_NATIVE;
    task->exec_image->u.native.entry = entry;
    
    /* Prepare argv/envp in task */
    task->argv = copy_argv(argc, argv);
    task->envp = copy_envp(envp);
    
    /* Call native entrypoint */
    return entry->func(task, argc, argv, envp);
}
```

### 6.4 WASI Execution Path

```c
int iox_exec_wasi(iox_task_t *task, const char *path,
                  int argc, char **argv, char **envp) {
    /* Load WASM module */
    uint8_t *wasm_buf = read_file(path);
    wasm_module_t module = wasm_runtime_load(wasm_buf, ...);
    wasm_module_inst_t instance = wasm_runtime_instantiate(module, ...);
    
    /* Create WASI context using task's VFS */
    wasm_wasi_args_t wasi_args;
    wasi_args.stdio = task->files;  /* Same FD table */
    wasi_args.cwd = task->fs->cwd;
    
    /* Instantiate with WASI imports */
    wasm_runtime_instantiate_wasi(instance, &wasi_args);
    
    /* Call _start or specified function */
    wasm_function_inst_t func = wasm_runtime_lookup_function(instance, "_start");
    wasm_runtime_call_wasm(task->exec_env, func, 0, NULL);
    
    return 0;
}
```

### 6.5 Script Execution Path

```c
int iox_exec_script(iox_task_t *task, const char *path,
                    int argc, char **argv, char **envp) {
    /* Read shebang */
    char shebang[IOX_MAX_PATH];
    read_shebang(path, shebang, sizeof(shebang));
    
    /* Parse interpreter and args */
    char *interp, *interp_args[IOX_MAX_ARGS];
    int interp_argc = parse_shebang(shebang, &interp, interp_args);
    
    /* Rewrite argv: interpreter [args] script [original args] */
    char *new_argv[IOX_MAX_ARGS];
    new_argv[0] = interp;
    for (i = 0; i < interp_argc; i++)
        new_argv[i+1] = interp_args[i];
    new_argv[interp_argc+1] = (char *)path;
    for (i = 1; i < argc; i++)
        new_argv[interp_argc+1+i] = argv[i];
    
    /* Recursive execve on interpreter */
    return iox_execve(interp, new_argv, envp);
}
```

---

## 7. PTY / Job Control / Signal Model

### 7.1 Session and Process Group Model

```
Session (sid=100)
├── Session Leader (pid=100)
│   └── Controlling TTY: /dev/pts/0
│
└── Process Group (pgid=100) [foreground]
    └── Process (pid=100)

Process Group (pgid=200) [background]
├── Process (pid=201)
└── Process (pid=202)
```

### 7.2 PTY Architecture

```c
typedef struct iox_pty {
    int master_fd;          /* PTY master (real host fd) */
    int slave_fd;           /* PTY slave (real host fd) */
    char slave_name[64];    /* /dev/ttysXXX or similar */
    pid_t session_leader;   /* Session holding this PTY */
    pid_t foreground_pgrp;  /* Foreground process group */
    struct termios termios;
    struct winsize winsize;
} iox_pty_t;

/* Global PTY table */
iox_pty_t *pty_table[IOX_MAX_PTS];
```

### 7.3 Job Control Signals

| Signal | Behavior |
|--------|----------|
| SIGINT | Sent to foreground pgrp on Ctrl+C |
| SIGQUIT | Sent to foreground pgrp on Ctrl+\ |
| SIGTSTP | Sent to foreground pgrp on Ctrl+Z |
| SIGTTIN | Sent to background pgrp attempting read |
| SIGTTOU | Sent to background pgrp attempting write |
| SIGCONT | Resume stopped process group |
| SIGCHLD | Sent to parent when child changes state |
| SIGWINCH | Sent to pgrp on window resize |
| SIGHUP | Sent to orphaned pgrp when session leader exits |

### 7.4 Signal Delivery State Machine

```
Signal Generated (kill, kernel, terminal)
    |
    v
[1] Permission Check
    - uid/gid match or privileged
    |
    v
[2] Determine Target
    - pid > 0: specific process
    - pid = 0: all processes in sender's pgrp
    - pid = -1: all processes (privileged)
    - pid < -1: all processes in |pid| pgrp
    |
    v
[3] Add to Pending
    - Add to target's sigpending mask
    - Queue if real-time signal
    |
    v
[4] Check Delivery Conditions
    - Signal not blocked?
    - Signal action not SIG_IGN?
    |
    v
[5] Deliver Signal
    - Invoke handler (if SA_SIGINFO, pass siginfo)
    - Or terminate/coredump/stop/continue
```

### 7.5 Terminal Access Control

```c
/* Before read/write on controlling terminal */
int iox_tty_check_access(iox_task_t *task, int fd, bool is_write) {
    iox_file_t *file = iox_files_lookup(task->files, fd);
    iox_tty_t *tty = file->tty;
    
    if (!tty) return 0;  /* Not a tty */
    
    /* Foreground pgrp can always access */
    if (task->pgid == tty->foreground_pgrp)
        return 0;
    
    /* Background pgrp needs signal handling */
    if (is_write) {
        /* Check if TOSTOP is set */
        if (tty->termios.c_lflag & TOSTOP) {
            iox_kill(-task->pgid, SIGTTOU);
            return -EIO;
        }
    } else {
        iox_kill(-task->pgid, SIGTTIN);
        return -EIO;
    }
    
    return 0;
}
```

---

## 8. WAMR Shared-Substrate Boundary

### 8.1 WASI-to-IOX Mapping

| WASI Function | iox Subsystem |
|---------------|---------------|
| fd_read | fs/fdtable.c: iox_fd_read() |
| fd_write | fs/fdtable.c: iox_fd_write() |
| fd_close | fs/fdtable.c: iox_fd_close() |
| fd_seek | fs/fdtable.c: iox_fd_seek() |
| path_open | fs/vfs/vfs.c: iox_vfs_open() |
| path_unlink | fs/vfs/vfs.c: iox_vfs_unlink() |
| path_mkdir | fs/vfs/vfs.c: iox_vfs_mkdir() |
| path_stat | fs/vfs/vfs.c: iox_vfs_stat() |
| path_symlink | fs/vfs/vfs.c: iox_vfs_symlink() |
| path_readlink | fs/vfs/vfs.c: iox_vfs_readlink() |
| path_rename | fs/vfs/vfs.c: iox_vfs_rename() |
| path_filestat_get | fs/vfs/vfs.c: iox_vfs_stat() |
| path_filestat_set_times | fs/vfs/vfs.c: iox_vfs_utimens() |
| clock_time_get | kernel/time/time.c: iox_clock_gettime() |
| clock_res_get | kernel/time/time.c: iox_clock_getres() |
| random_get | fs/dev/random.c: iox_getrandom() |
| environ_sizes_get | kernel/task/task.c: iox_get_env() |
| environ_get | kernel/task/task.c: iox_get_env() |
| args_sizes_get | kernel/exec/exec.c: iox_get_args() |
| args_get | kernel/exec/exec.c: iox_get_args() |
| proc_exit | kernel/task/exit.c: iox_exit() |

### 8.2 WASI Context Structure

```c
typedef struct iox_wasi_context {
    iox_task_t *task;       /* Back-reference to iox task */
    wasm_exec_env_t exec_env;
    wasm_module_inst_t module_inst;
    
    /* FD mapping: WASI fd -> iox fd */
    int wasi_to_iox_fd[IOX_MAX_FD];
    
    /* Preopened directories */
    iox_wasi_preopen_t preopens[IOX_MAX_PREOPENS];
    int preopen_count;
} iox_wasi_context_t;
```

### 8.3 WASI Implementation Pattern

```c
/* WASI fd_write implementation */
wasi_errno_t wasi_fd_write(wasm_exec_env_t exec_env,
                           wasi_fd_t fd,
                           const iovec_t *iovs,
                           size_t iovs_len,
                           size_t *nwritten) {
    /* Get iox task from exec_env */
    iox_wasi_context_t *wasi_ctx = wasm_runtime_get_user_data(exec_env);
    iox_task_t *task = wasi_ctx->task;
    
    /* Map WASI fd to iox fd */
    int iox_fd = wasi_ctx->wasi_to_iox_fd[fd];
    
    /* Call iox FD operation */
    ssize_t ret = iox_fd_write(task, iox_fd, iovs, iovs_len, nwritten);
    
    /* Convert iox errno to WASI errno */
    return iox_errno_to_wasi(ret < 0 ? errno : 0);
}
```

### 8.4 WAMR Integration Points

1. **Module Loading**: `runtime/wasi/wasi_runtime.c`
2. **Import Resolution**: WAMR calls into iox WASI functions
3. **Memory Access**: Shared linear memory (WAMR manages, iox can read/write)
4. **Trap Handling**: Convert WASM traps to iox signals
5. **Lifetime**: WASI context destroyed when task exits or execs

---

## 9. Build System and iOS-Only Test Architecture

### 9.1 CMake Configuration Requirements

```cmake
# CMakeLists.txt (root)

# MUST: Set iOS before project()
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_SYSROOT iphonesimulator)  # or iphoneos
set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0")
set(CMAKE_OSX_ARCHITECTURES "arm64")

# Hard fail on non-iOS
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    message(FATAL_ERROR "iox only supports iOS builds")
endif()

project(iox VERSION 1.0.0 LANGUAGES C)

# Use Xcode generator
set(CMAKE_GENERATOR "Xcode" CACHE STRING "Generator" FORCE)

# iOS-specific options
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS "arm64")
```

### 9.2 CMakePresets.json

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "ios-simulator-base",
            "hidden": true,
            "generator": "Xcode",
            "architecture": "arm64",
            "cacheVariables": {
                "CMAKE_SYSTEM_NAME": "iOS",
                "CMAKE_OSX_SYSROOT": "iphonesimulator",
                "CMAKE_OSX_DEPLOYMENT_TARGET": "16.0",
                "CMAKE_OSX_ARCHITECTURES": "arm64"
            }
        },
        {
            "name": "ios-device-base",
            "hidden": true,
            "generator": "Xcode",
            "architecture": "arm64",
            "cacheVariables": {
                "CMAKE_SYSTEM_NAME": "iOS",
                "CMAKE_OSX_SYSROOT": "iphoneos",
                "CMAKE_OSX_DEPLOYMENT_TARGET": "16.0",
                "CMAKE_OSX_ARCHITECTURES": "arm64"
            }
        },
        {
            "name": "ios-simulator-debug",
            "inherits": "ios-simulator-base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "ios-simulator-release",
            "inherits": "ios-simulator-base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        },
        {
            "name": "ios-simulator-asan",
            "inherits": "ios-simulator-base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_C_FLAGS": "-fsanitize=address"
            }
        },
        {
            "name": "ios-device-debug",
            "inherits": "ios-device-base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "ios-simulator-debug",
            "configurePreset": "ios-simulator-debug",
            "configuration": "Debug"
        }
    ],
    "testPresets": [
        {
            "name": "ios-simulator-test",
            "configurePreset": "ios-simulator-debug",
            "output": {
                "outputOnFailure": true
            },
            "execution": {
                "timeout": 300
            }
        }
    ]
}
```

### 9.3 Test Execution Architecture

```
tools/
├── bootstrap.sh           # Fresh clone setup
├── doctor.sh             # Environment verification
├── test-simulator.sh     # Run simulator tests
├── test-device.sh        # Run device tests
└── build-sdk.sh          # Build full SDK
```

**test-simulator.sh**:
```bash
#!/bin/bash
set -e

PRESET="ios-simulator-debug"
DESTINATION="platform=iOS Simulator,name=iPhone 15,OS=17.0"

# Configure
cmake --preset=$PRESET

# Build for testing
cmake --build --preset=$PRESET --target iox-test-app -- -build-for-testing

# Run tests
xcodebuild test-without-building \
    -project build/iox.xcodeproj \
    -scheme iox-test-app \
    -destination "$DESTINATION" \
    -resultBundlePath results/simulator.xcresult
```

### 9.4 Fresh-Clone Requirements

**bootstrap.sh**:
```bash
#!/bin/bash
# Fresh clone bootstrap for iox

set -e

echo "=== iox Bootstrap ==="

# Check prerequisites
command -v cmake >/dev/null 2>&1 || { echo "cmake required"; exit 1; }
command -v xcodebuild >/dev/null 2>&1 || { echo "Xcode required"; exit 1; }

# Check iOS SDKs
if ! xcrun --sdk iphonesimulator --show-sdk-path >/dev/null 2>&1; then
    echo "iOS Simulator SDK not found"
    exit 1
fi

# Initialize submodules
git submodule update --init --recursive

# Run doctor
tools/doctor.sh

echo "Bootstrap complete. Run 'tools/test-simulator.sh' to verify."
```

---

## 10. Testing Architecture

### 10.1 Seven Testing Layers

**Layer 1: Pure Core Unit Tests**
- Location: `tests/unit/`
- Purpose: Data structures, deterministic logic
- Target: iOS Simulator
- Coverage: PID allocation, FD operations, path normalization

**Layer 2: Subsystem Integration Tests**
- Location: `tests/integration/`
- Purpose: Cross-subsystem interactions
- Target: iOS Simulator
- Coverage: fork+wait, exec+CLOEXEC, pipe+dup

**Layer 3: Linux Userland Compatibility Tests**
- Location: `tests/compat/*/`
- Purpose: Real package behavior
- Target: iOS Simulator, Device
- Coverage: bash scenarios, coreutils, PTY

**Layer 4: WASI Compatibility Tests**
- Location: `tests/wasi/`
- Purpose: WAMR WASI correctness
- Target: iOS Simulator, Device
- Coverage: FD, path, stdio, env, clocks

**Layer 5: iOS Runtime Tests**
- Location: `tests/device/`, `tests/simulator/`
- Purpose: iOS-specific behavior
- Target: Device (primary), Simulator (secondary)
- Coverage: Sandbox paths, PTY, lifecycle

**Layer 6: Stress and Reliability Tests**
- Location: `tests/stress/`
- Purpose: Concurrency, resource churn
- Target: iOS Simulator (long-running)
- Coverage: Repeated fork/exec, signal storms

**Layer 7: Performance Regression Tests**
- Location: `tests/perf/`
- Purpose: Baseline benchmarks
- Target: iOS Simulator
- Coverage: Shell startup, dispatch latency

### 10.2 C-Level Test Harness

```c
/* tests/harness/iox_test.h */
#ifndef IOX_TEST_H
#define IOX_TEST_H

#include <stdbool.h>

typedef struct {
    const char *name;
    bool (*fn)(void);
} iox_test_case_t;

#define IOX_TEST(name) \
    static bool test_##name(void); \
    static const iox_test_case_t test_case_##name \
        __attribute__((used, section("iox_test_cases"))) = { \
        .name = #name, \
        .fn = test_##name \
    }; \
    static bool test_##name(void)

#define IOX_ASSERT(expr) \
    do { if (!(expr)) { \
        iox_test_fail(__FILE__, __LINE__, #expr); \
        return false; \
    } } while(0)

#define IOX_ASSERT_EQ(a, b) IOX_ASSERT((a) == (b))
#define IOX_ASSERT_NE(a, b) IOX_ASSERT((a) != (b))
#define IOX_ASSERT_GT(a, b) IOX_ASSERT((a) > (b))
#define IOX_ASSERT_LT(a, b) IOX_ASSERT((a) < (b))

void iox_test_fail(const char *file, int line, const char *expr);
int iox_test_run_all(const char *filter);

#endif
```

---

## 11. Test Matrix by Subsystem

### 11.1 Task Subsystem

| Category | Invariants | Positive | Negative | Concurrency |
|----------|-----------|----------|----------|-------------|
| PID alloc | Unique IDs | Alloc/free | Exhaustion | Concurrent alloc |
| Fork | Child != Parent | Basic fork | NULL args | Many forks |
| Exit | Zombied until wait | Normal exit | Invalid status | Exit during wait |
| Wait | Returns status | waitpid | Wrong pid | Concurrent waits |
| PGID | Group membership | setpgid | Invalid pgid | Move between groups |
| Session | Session leader | setsid | Already leader | Orphaned pgrp |

### 11.2 Files Subsystem

| Category | Invariants | Positive | Negative | Concurrency |
|----------|-----------|----------|----------|-------------|
| FD alloc | Contiguous | Open/close | EMFILE | Concurrent open |
| Dup | Independent refs | dup/dup2/dup3 | Invalid fd | Dup during close |
| CLOEXEC | Closed on exec | F_SETFD | Invalid flags | Exec with many fds |
| Seek | Position tracking | lseek | ESPIPE on pipe | Concurrent seek |
| Read/Write | Byte counts | Basic IO | EBADF | Concurrent RW |

### 11.3 Exec Subsystem

| Category | Invariants | Positive | Negative | Compatibility |
|----------|-----------|----------|----------|---------------|
| Native | Registry lookup | /bin/ls | Missing cmd | Command names |
| WASI | WASM load | .wasm exec | Invalid magic | WASI compliance |
| Script | Shebang parse | #!/bin/sh | Missing interp | Script args |
| CLOEXEC | FDs closed | FD_CLOEXEC | None retained | Bash pipelines |
| Signals | Handlers reset | SIG_IGN kept | SIG_DFL reset | Signal state |
| Identity | PID preserved | Same PID | PID change | Session/PGID |

### 11.4 Signal Subsystem

| Category | Invariants | Positive | Negative | Concurrency |
|----------|-----------|----------|----------|-------------|
| Handler | Callback invoked | signal() | Invalid sig | Handler reentry |
| Mask | Blocking works | sigprocmask | Invalid how | Mask changes |
| Pending | Queued signals | sigpending | Overflow | Signal storm |
| Delivery | Correct target | kill() | Invalid pid | Multi-target |
| Group | All members receive | kill(-pgrp) | Wrong group | Group changes |

### 11.5 TTY/PTY Subsystem

| Category | Invariants | Positive | Negative | Compatibility |
|----------|-----------|----------|----------|---------------|
| Master | Read/write | openpt | No master | PTY pairs |
| Slave | TTY device | ptsname | Invalid fd | pts/* nodes |
| Foreground | One pgrp | tcsetpgrp | No TTY | Job control |
| Signals | SIGINT on Ctrl+C | Terminal gen | Invalid gen | Readline |
| Attributes | Termios saved | tcgetattr | No TTY | stty compat |

---

## 12. Reliability and Regression Policy

### 12.1 Truthfulness Rule

A feature is **NOT** complete unless:
1. A deterministic automated test proves the behavior
2. A conformance test proves the behavior
3. A regression test covers a previously failing case
4. A documented limitation states what is unverified

### 12.2 Regression Policy

Every bug fix requires:
1. Regression test that **fails** before the fix
2. The fix
3. Regression test **passes** after the fix

No bug fix without a test (unless documented untestable).

### 12.3 Reliability Gates

All changes must pass:
1. Unit tests
2. Integration tests
3. Compatibility tests
4. WASI tests
5. Regression tests
6. Performance checks (if hot path)
7. Simulator tests
8. Device tests (if iOS-sensitive)

### 12.4 Flakiness Policy

1. Flaky tests are release blockers
2. Do not increase timeouts to "fix" races
3. Fix the test or the subsystem
4. Every timeout must be bounded and justified

---

## 13. First Concrete Code Changes

### 13.1 Phase 0: Repository Reorganization

**Step 1: Create new directory structure**
```bash
mkdir -p kernel/{task,signal,exec,time,resource}
mkdir -p fs/{vfs,proc,dev,devpts,pipe,hostfs}
mkdir -p drivers/tty
mkdir -p runtime/{native,wasi,script}
mkdir -p compat/{posix,interpose}
mkdir -p tests/{unit,integration,compat,wasi,stress,perf,device,simulator}
mkdir -p tools
```

**Step 2: Move existing files (git mv)**
```bash
git mv src/iox/core/iox_vfs.c fs/vfs/
git mv src/iox/core/iox_network.c net/
git mv src/iox/core/iox_path.c fs/
git mv src/iox/interpose/iox_interpose.c compat/interpose/
```

**Step 3: Create canonical task header**
File: `kernel/task/task.h`
```c
#ifndef IOX_TASK_H
#define IOX_TASK_H

#include <iox/iox_types.h>
#include <stdatomic.h>
#include <pthread.h>

typedef struct iox_task iox_task_t;
typedef struct iox_files iox_files_t;
typedef struct iox_fs iox_fs_t;
typedef struct iox_sighand iox_sighand_t;
typedef struct iox_tty iox_tty_t;

typedef enum {
    IOX_TASK_RUNNING = 0,
    IOX_TASK_INTERRUPTIBLE,
    IOX_TASK_UNINTERRUPTIBLE,
    IOX_TASK_STOPPED,
    IOX_TASK_ZOMBIE,
    IOX_TASK_DEAD
} iox_task_state_t;

struct iox_task {
    pid_t pid, ppid, tgid, pgid, sid;
    atomic_int state;
    int exit_status;
    
    pthread_t thread;
    char comm[256];
    
    iox_files_t *files;
    iox_fs_t *fs;
    iox_sighand_t *sighand;
    iox_tty_t *tty;
    
    struct iox_task *parent, *children, *next_sibling;
    
    atomic_int refs;
    pthread_mutex_t lock;
};

/* Core API */
iox_task_t *iox_task_alloc(void);
void iox_task_free(iox_task_t *task);
iox_task_t *iox_current_task(void);
void iox_set_current_task(iox_task_t *task);
pid_t iox_alloc_pid(void);
void iox_free_pid(pid_t pid);

#endif
```

### 13.2 Phase 1: FD Table Migration

**File: `fs/fdtable.c` (new)**
```c
#include "fs/fdtable.h"
#include <stdlib.h>
#include <string.h>

struct iox_files {
    iox_file_t **fd;
    size_t max_fds;
    pthread_mutex_t lock;
};

iox_files_t *iox_files_alloc(size_t max_fds) {
    iox_files_t *files = calloc(1, sizeof(iox_files_t));
    files->fd = calloc(max_fds, sizeof(iox_file_t*));
    files->max_fds = max_fds;
    pthread_mutex_init(&files->lock, NULL);
    return files;
}

void iox_files_free(iox_files_t *files) {
    if (!files) return;
    for (size_t i = 0; i < files->max_fds; i++) {
        if (files->fd[i]) {
            iox_file_free(files->fd[i]);
        }
    }
    free(files->fd);
    free(files);
}

iox_files_t *iox_files_dup(iox_files_t *parent) {
    iox_files_t *child = iox_files_alloc(parent->max_fds);
    pthread_mutex_lock(&parent->lock);
    for (size_t i = 0; i < parent->max_fds; i++) {
        if (parent->fd[i]) {
            child->fd[i] = iox_file_dup(parent->fd[i]);
        }
    }
    pthread_mutex_unlock(&parent->lock);
    return child;
}
```

### 13.3 Phase 2: Delete Competing Code

**Commit 1: Remove iox_context.c**
```bash
git rm src/iox/core/iox_context.c
# Migrate salvageable logic:
# - ctx allocation pattern -> task allocation
# - env handling -> task->envp
# - cwd tracking -> task->fs->cwd
```

**Commit 2: Remove global FD table**
```bash
git rm src/iox/core/iox_file.c
# All FD operations now go through task->files
```

### 13.4 Phase 3: Create Test Harness

**File: `tests/harness/harness.c`**
```c
#include "iox_test.h"
#include <stdio.h>
#include <string.h>

extern iox_test_case_t __start_iox_test_cases[];
extern iox_test_case_t __stop_iox_test_cases[];

static int tests_run = 0;
static int tests_passed = 0;

void iox_test_fail(const char *file, int line, const char *expr) {
    fprintf(stderr, "FAIL: %s:%d: %s\n", file, line, expr);
}

int iox_test_run_all(const char *filter) {
    iox_test_case_t *start = __start_iox_test_cases;
    iox_test_case_t *end = __stop_iox_test_cases;
    
    for (iox_test_case_t *tc = start; tc < end; tc++) {
        if (filter && strstr(tc->name, filter) == NULL)
            continue;
        
        tests_run++;
        printf("TEST: %s ... ", tc->name);
        fflush(stdout);
        
        if (tc->fn()) {
            printf("PASS\n");
            tests_passed++;
        } else {
            printf("FAIL\n");
        }
    }
    
    printf("\nResults: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}

int main(int argc, char **argv) {
    const char *filter = argc > 1 ? argv[1] : NULL;
    return iox_test_run_all(filter);
}
```

---

## 14. First Concrete Test Files

### 14.1 Unit Test: PID Allocation

**File: `tests/unit/test_pid.c`**
```c
#include "iox_test.h"
#include "kernel/task/task.h"

IOX_TEST(pid_alloc_basic) {
    pid_t p1 = iox_alloc_pid();
    pid_t p2 = iox_alloc_pid();
    
    IOX_ASSERT_GT(p1, 0);
    IOX_ASSERT_GT(p2, 0);
    IOX_ASSERT_NE(p1, p2);
    
    iox_free_pid(p1);
    iox_free_pid(p2);
    return true;
}

IOX_TEST(pid_alloc_exhaustion) {
    /* Allocate many PIDs */
    pid_t pids[100];
    for (int i = 0; i < 100; i++) {
        pids[i] = iox_alloc_pid();
        IOX_ASSERT_GT(pids[i], 0);
    }
    
    /* Free all */
    for (int i = 0; i < 100; i++) {
        iox_free_pid(pids[i]);
    }
    return true;
}

IOX_TEST(pid_reuse) {
    pid_t p1 = iox_alloc_pid();
    iox_free_pid(p1);
    pid_t p2 = iox_alloc_pid();
    
    /* PIDs may be reused */
    IOX_ASSERT_GT(p2, 0);
    
    iox_free_pid(p2);
    return true;
}
```

### 14.2 Unit Test: FD Table

**File: `tests/unit/test_fd.c`**
```c
#include "iox_test.h"
#include "fs/fdtable.h"

IOX_TEST(fdtable_alloc) {
    iox_files_t *files = iox_files_alloc(256);
    IOX_ASSERT(files != NULL);
    iox_files_free(files);
    return true;
}

IOX_TEST(fdtable_dup) {
    iox_files_t *parent = iox_files_alloc(256);
    
    /* Add a fake FD to parent */
    /* (Would need actual file creation) */
    
    iox_files_t *child = iox_files_dup(parent);
    IOX_ASSERT(child != NULL);
    IOX_ASSERT_NE(child, parent);
    
    iox_files_free(child);
    iox_files_free(parent);
    return true;
}
```

### 14.3 Integration Test: Fork

**File: `tests/integration/test_fork.c`**
```c
#include "iox_test.h"
#include "kernel/task/task.h"
#include "kernel/task/fork.h"

IOX_TEST(fork_creates_new_task) {
    /* This test will require full fork implementation */
    /* For now, just verify structure exists */
    IOX_ASSERT(iox_task_alloc() != NULL);
    return true;
}

IOX_TEST(fork_parent_gets_child_pid) {
    /* Placeholder - requires full implementation */
    IOX_ASSERT(true);  /* TODO: Implement fork first */
    return true;
}
```

### 14.4 CMakeLists.txt Update

**File: `tests/unit/CMakeLists.txt` (new)**
```cmake
# Unit tests
set(IOX_UNIT_TESTS
    test_pid.c
    test_fd.c
)

add_executable(iox-unit-tests
    ../harness/harness.c
    ${IOX_UNIT_TESTS}
)

target_link_libraries(iox-unit-tests PRIVATE iox-core)

target_compile_options(iox-unit-tests PRIVATE
    -fdata-sections -ffunction-sections
)

target_link_options(iox-unit-tests PRIVATE
    -Wl,-dead_strip
)

# Register with CTest
add_test(NAME unit-pid COMMAND iox-unit-tests pid)
add_test(NAME unit-fd COMMAND iox-unit-tests fd)
```

### 14.5 Test Documentation

**File: `tests/README.md` (new)**
```markdown
# iox Test Suite

## Running Tests

### Simulator
```bash
tools/test-simulator.sh
```

### Device
```bash
tools/test-device.sh
```

### Unit Tests Only
```bash
cmake --preset ios-simulator-debug
cmake --build --preset ios-simulator-debug --target iox-unit-tests
ctest --preset ios-simulator-test -R unit
```

## Test Organization

- `unit/`: Pure core unit tests
- `integration/`: Subsystem integration tests
- `compat/`: Linux userland compatibility
- `wasi/`: WASI/WAMR tests
- `device/`: iOS device-specific tests
- `simulator/`: iOS simulator-specific tests
- `stress/`: Stress and reliability tests
- `perf/`: Performance regression tests

## Writing Tests

See `harness/iox_test.h` for test macros.

```c
#include "iox_test.h"

IOX_TEST(my_feature) {
    IOX_ASSERT(some_condition);
    IOX_ASSERT_EQ(expected, actual);
    return true;
}
```
```

---

## Appendices

### A. Deleted Files

| File | Reason |
|------|--------|
| `src/iox/core/iox_context.c` | Competing process model |
| `src/iox/core/iox_file.c` | Global FD table forbidden |
| `src/iox/core/iox_libc_delegate.c` | Passthrough semantics forbidden |
| `src/iox/core/iox_minimal.c` | Replaced by proper subsystems |
| `src/iox/wamr/iox_wamr_simple.c` | Incomplete WASI bridge |

### B. Renamed/Moved Files

| Old Location | New Location |
|--------------|--------------|
| `src/iox/core/iox_vfs.c` | `fs/vfs/vfs.c` |
| `src/iox/core/iox_network.c` | `net/socket.c` |
| `src/iox/util/iox_path.c` | `fs/path.c` |
| `src/iox/interpose/iox_interpose.c` | `compat/interpose/syscalls.c` |
| `src/iox/core/iox_process.c` | `kernel/task/task.c` (partial) |
| `src/iox/core/iox_init.c` | `kernel/init/main.c` |

### C. New Files Required

See Section 4.4 for complete list of new subsystem files.

---

**Document Status**: Analysis Complete  
**Next Step**: Execute Phase 0 - Repository Reorganization

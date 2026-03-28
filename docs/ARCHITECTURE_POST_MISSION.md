# iXland Architecture - Post Mission Documentation

**Version**: 1.0.0  
**Date**: 2026-03-28  
**Status**: Implementation Complete

---

## Table of Contents

1. [System Architecture Overview](#system-architecture-overview)
2. [Component Interaction Flows](#component-interaction-flows)
3. [Data Flow Diagrams](#data-flow-diagrams)
4. [Build System Structure](#build-system-structure)
5. [Header Organization](#header-organization)

---

## System Architecture Overview

### High-Level Architecture

```mermaid
flowchart TB
    subgraph App["iOS Application Layer"]
        SwiftUI["SwiftUI Terminal"]
        SessionMgr["Session Manager"]
        CmdDispatch["Command Dispatch"]
    end

    subgraph libc["ixland-libc (Public API)"]
        direction TB
        IoxH["iox/iox.h<br/>Umbrella Header"]
        Types["iox/iox_types.h<br/>Type Definitions"]
        Syscalls["iox/iox_syscalls.h<br/>Syscall Declarations"]
        Pwd["pwd.h<br/>User Database"]
        Grp["grp.h<br/>Group Database"]
    end

    subgraph System["ixland-system (Kernel Implementation)"]
        direction TB

        subgraph Core["Core Subsystems"]
            Init["iox_init.c<br/>Initialization"]
            Identity["iox_identity.c<br/>UID/GID Management"]
            Poll["iox_poll.c<br/>Poll/Select"]
        end

        subgraph Task["Task Subsystem"]
            TaskCore["task.c<br/>Task Lifecycle"]
            Fork["fork.c<br/>Fork/Vfork"]
            Exit["exit.c<br/>Process Exit"]
            Wait["wait.c<br/>Wait Syscalls"]
            Pid["pid.c<br/>PID Management"]
        end

        subgraph Signal["Signal Subsystem"]
            SigCore["signal.c<br/>Signal Handling"]
            SigQueue["Signal Queue"]
        end

        subgraph VFS["VFS Subsystem"]
            VfsCore["vfs.c<br/>Path Translation"]
            Fdtable["fdtable.c<br/>FD Management"]
            Path["iox_path.c<br/>Path Operations"]
        end

        subgraph Exec["Exec Subsystem"]
            ExecCore["exec.c<br/>Program Execution"]
            Image["Image Loading"]
        end

        subgraph WASI["WASI Runtime"]
            WamrAdapter["wasm_adapter.c<br/>WAMR Integration"]
            WamrSimple["iox_wamr_simple.c<br/>Simple WAMR"]
        end
    end

    subgraph Runtime["Runtime Backends"]
        Native["Native Execution"]
        WASIRuntime["WASI Runtime"]
        Script["Script Interpreter"]
    end

    App -->|@_cdecl("iox_*)"| libc
    libc -->|Internal Calls| System
    System -->|Execute| Runtime

    TaskCore -->|uses| Signal
    TaskCore -->|uses| VFS
    ExecCore -->|uses| Task
    WASI -->|uses| VFS

    style libc fill:#ccffcc
    style System fill:#ffcccc
    style Runtime fill:#ccccff
```

### Component Dependencies

```mermaid
flowchart LR
    subgraph Public["Public API"]
        Libc["ixland-libc"]
    end

    subgraph Internal["Internal Implementation"]
        Task["Task Subsystem"]
        Signal["Signal Subsystem"]
        VFS["VFS Subsystem"]
        Exec["Exec Subsystem"]
    end

    subgraph Platform["Platform Layer"]
        BSD["BSD Syscalls"]
        Posix["POSIX Threads"]
    end

    Libc -->|calls| Task
    Libc -->|calls| Signal
    Libc -->|calls| VFS

    Task -->|signals| Signal
    Task -->|files| VFS
    Exec -->|task mgmt| Task
    Exec -->|signals| Signal
    Exec -->|files| VFS

    Task -->|threads| Posix
    Signal -->|signals| BSD
    VFS -->|files| BSD

    style Libc fill:#ccffcc
    style Task fill:#ffcccc
    style Signal fill:#ffcccc
    style VFS fill:#ffcccc
    style Exec fill:#ffcccc
```

---

## Component Interaction Flows

### Process Creation Flow (fork/exec)

```mermaid
sequenceDiagram
    participant App as iOS App
    participant Libc as ixland-libc
    participant Task as Task Subsystem
    participant Pid as PID Manager
    participant Sig as Signal Subsystem
    participant VFS as VFS Layer

    Note over App,VFS: Process Creation Sequence

    App->>Libc: iox_fork()
    Libc->>Task: iox_task_alloc()
    Task->>Pid: iox_alloc_pid()
    Pid-->>Task: pid = 1000

    Task->>Task: Initialize task structure
    Task->>Sig: iox_sighand_alloc()
    Sig-->>Task: sighand

    Task->>VFS: iox_files_alloc()
    VFS-->>Task: files (fdtable)

    Task->>Task: Set parent-child links
    Task-->>Libc: Return child PID
    Libc-->>App: Return 1000

    Note over App,VFS: Process Execution Sequence

    App->>Libc: iox_execve(path, argv, envp)
    Libc->>Task: iox_current_task()

    Task->>VFS: vfs_translate_path(path)
    VFS-->>Task: real_path

    Task->>Task: Load exec image
    Task->>VFS: Replace fdtable (if CLOEXEC)
    Task->>Sig: Reset signal handlers (SIG_DFL)

    Task-->>Libc: Return (enters new image)
```

### Signal Delivery Flow

```mermaid
sequenceDiagram
    participant Sender as Sending Process
    participant Sig as Signal Handler
    recipient as Recipient Task
    Queue as Signal Queue
    Wait as Wait Queue

    Note over Sender,Wait: Signal Delivery Sequence

    Sender->>Sig: iox_kill(pid, SIGINT)
    Sig->>recipient: iox_task_lookup(pid)
    recipient-->>Sig: task

    Sig->>recipient: Check sigprocmask

    alt Signal Blocked
        Sig->>Queue: iox_sigqueue_entry_alloc()
        Queue->>recipient: Add to pending
        Sig-->>Sender: Return 0
    else Signal Not Blocked
        Sig->>recipient: Set signaled = true
        Sig->>recipient: termsig = SIGINT

        alt Process is Waiting
            Sig->>Wait: pthread_cond_broadcast()
        end

        Sig-->>Sender: Return 0
    end

    Note over Sender,Wait: Signal Handling

    recipient->>Sig: Process pending signals
    Sig->>Queue: Dequeue pending
    Sig->>recipient: Get sigaction

    alt SA_SIGINFO
        Sig->>recipient: Call sa_sigaction(sig, info, context)
    else Standard
        Sig->>recipient: Call sa_handler(sig)
    end
```

### Process Wait Flow (waitpid)

```mermaid
sequenceDiagram
    participant Parent as Parent Process
    participant Wait as Wait Subsystem
    participant Child as Child Process
    participant Task as Task Table

    Note over Parent,Task: waitpid(pid, &status, options)

    Parent->>Wait: iox_waitpid(pid, wstatus, options)
    Wait->>Task: Find child processes

    alt WNOHANG set
        Task->>Wait: Check for zombie children

        alt Zombie found
            Task->>Wait: Return exit status
            Wait->>Task: iox_task_free(child)
            Wait-->>Parent: Return child PID
        else No zombies
            Wait-->>Parent: Return 0 (no block)
        end
    else Blocking wait
        loop Until child exits
            Wait->>Task: Check for zombies

            alt No zombies
                Wait->>Wait: pthread_cond_wait()
            end
        end

        Task->>Wait: Child exited
        Wait->>Task: iox_task_free(child)
        Wait-->>Parent: Return child PID
    end

    Note over Parent,Task: Child Exit Sequence

    Child->>Task: iox_exit(status)
    Task->>Task: Set zombie state
    Task->>Task: exit_status = status

    alt Has waiters
        Task->>Wait: pthread_cond_broadcast()
    end

    Task->>Task: If orphaned, reparent to init
```

### File Operation Flow

```mermaid
sequenceDiagram
    participant App as Application
    participant Libc as ixland-libc
    participant VFS as VFS Layer
    participant FDT as FD Table
    participant Kernel as Native Kernel

    Note over App,Kernel: Open File Flow

    App->>Libc: iox_open(path, flags, mode)
    Libc->>VFS: vfs_translate_path(path)
    VFS->>VFS: Check if system path

    alt System Path (/etc, /usr, etc.)
        VFS->>VFS: Prepend prefix
        VFS-->>Libc: /prefix/etc/file
    else Regular Path
        VFS-->>Libc: path (unchanged)
    end

    Libc->>Kernel: open(real_path, flags, mode)
    Kernel-->>Libc: fd = 3

    Libc->>FDT: iox_fdtable_alloc()
    FDT->>FDT: Store fd in task's fdtable
    FDT-->>Libc: ok

    Libc-->>App: Return fd (3)

    Note over App,Kernel: Close File Flow

    App->>Libc: iox_close(fd)
    Libc->>FDT: iox_fdtable_get(fd)
    FDT-->>Libc: fd_entry

    Libc->>Kernel: close(real_fd)
    Kernel-->>Libc: ok

    Libc->>FDT: iox_fdtable_free(fd)
    FDT-->>Libc: ok
    Libc-->>App: Return 0
```

### Vfork Flow (Optimized Fork)

```mermaid
sequenceDiagram
    participant Parent as Parent Process
    participant Task as Task Subsystem
    participant Child as Child Process
    participant Exec as Exec Subsystem

    Note over Parent,Exec: Vfork Sequence

    Parent->>Task: iox_vfork()
    Task->>Task: Create child task
    Task->>Task: Child shares parent's memory
    Task->>Task: Mark vfork_parent
    Task->>Task: Block parent (wait on vfork_lock)

    Task-->>Parent: Return child PID

    Note over Parent,Exec: Child Executes

    Child->>Exec: iox_execve(program, argv, envp)
    Exec->>Task: Load new image
    Exec->>Task: Unblock parent

    Note over Parent,Exec: Parent Resumes

    Task->>Parent: vfork_lock released
    Task->>Task: Child becomes independent
    Parent->>Parent: Continue execution

    alt Child exits without exec
        Child->>Task: iox_exit()
        Task->>Task: Unblock parent
        Task->>Task: Child resources freed
    end
```

---

## Data Flow Diagrams

### Process State Transitions

```mermaid
stateDiagram-v2
    [*] --> ALLOCATED: iox_task_alloc()
    ALLOCATED --> RUNNING: iox_fork() completes
    RUNNING --> INTERRUPTIBLE: Waiting for I/O
    RUNNING --> UNINTERRUPTIBLE: Disk wait (uninterruptible)
    RUNNING --> STOPPED: SIGSTOP received
    STOPPED --> RUNNING: SIGCONT received
    RUNNING --> ZOMBIE: iox_exit() called
    ZOMBIE --> DEAD: wait() collected
    RUNNING --> DEAD: SIGKILL (immediate)
    INTERRUPTIBLE --> RUNNING: I/O complete
    UNINTERRUPTIBLE --> RUNNING: I/O complete

    ALLOCATED: ALLOCATED<br/>(structure ready)
    RUNNING: RUNNING<br/>(active execution)
    INTERRUPTIBLE: INTERRUPTIBLE<br/>(sleep, can wake)
    UNINTERRUPTIBLE: UNINTERRUPTIBLE<br/>(uninterruptible sleep)
    STOPPED: STOPPED<br/>(job control)
    ZOMBIE: ZOMBIE<br/>(awaiting wait)
    DEAD: DEAD<br/>(resources freed)
```

### Signal State Machine

```mermaid
stateDiagram-v2
    [*] --> IDLE: Process running
    IDLE --> PENDING: Signal received (blocked)
    IDLE --> DELIVERING: Signal received (unblocked)
    PENDING --> DELIVERING: sigprocmask unblocks
    DELIVERING --> HANDLING: Signal handler entered
    HANDLING --> IDLE: Handler returns
    HANDLING --> TERMINATED: SIG_DFL (term signal)
    DELIVERING --> TERMINATED: SIG_DFL (term signal)
    DELIVERING --> COREDUMP: SIG_DFL (core signal)
    PENDING --> DISCARDED: Signal ignored

    IDLE: IDLE<br/>(no signals)
    PENDING: PENDING<br/>(queued, blocked)
    DELIVERING: DELIVERING<br/>(being delivered)
    HANDLING: HANDLING<br/>(in handler)
    TERMINATED: TERMINATED<br/>(process killed)
    COREDUMP: COREDUMP<br/>(dump + kill)
    DISCARDED: DISCARDED<br/>(ignored)
```

### Memory Management Flow

```mermaid
flowchart TD
    subgraph TaskMem["Task Memory"]
        TaskStruct["iox_task_t<br/>~2KB"]
        SigHand["iox_sighand_t<br/>~1KB"]
        Files["iox_files_t<br/>~2KB"]
        Fs["iox_fs_t<br/>~512B"]
        MM["iox_mm_emu_t<br/>~256B"]
    end

    subgraph Shared["Shared Resources"]
        PathCache["Path Cache"]
        FdTable["FD Table Pool"]
        SigQueue["Signal Queue Pool"]
    end

    subgraph Runtime["Runtime Memory"]
        Thread["pthread_t<br/>stack"]
        ExecImage["exec_image<br/>program data"]
    end

    TaskStruct -->|references| SigHand
    TaskStruct -->|references| Files
    TaskStruct -->|references| Fs
    TaskStruct -->|references| MM

    Files -->|uses| FdTable
    SigHand -->|uses| SigQueue
    Fs -->|uses| PathCache

    TaskStruct -->|owns| Thread
    MM -->|references| ExecImage

    style TaskStruct fill:#ffcccc
    style Thread fill:#ccccff
```

---

## Build System Structure

### CMake Build Flow

```mermaid
flowchart TB
    subgraph Root["Root CMake"]
        CMakeRoot["CMakeLists.txt<br/>Project: ixland"]
    end

    subgraph Components["Component CMake"]
        LibcC["ixland-libc/CMakeLists.txt"]
        SystemC["ixland-system/CMakeLists.txt"]
        WasmC["ixland-wasm/CMakeLists.txt"]
        ToolchainC["ixland-toolchain/CMakeLists.txt"]
    end

    subgraph Targets["Build Targets"]
        LibcLib["ixland-libc.a<br/>(Public Library)"]
        SystemLib["ixland-system.a<br/>(Kernel Library)"]
        WasmLib["ixland-wasm.a<br/>(WASM Runtime)"]
        Tests["unit_tests<br/>(Test Suite)"]
    end

    CMakeRoot -->|add_subdirectory| LibcC
    CMakeRoot -->|add_subdirectory| SystemC
    CMakeRoot -->|add_subdirectory| WasmC

    LibcC -->|add_library| LibcLib
    SystemC -->|add_library| SystemLib
    SystemC -->|link| LibcLib
    WasmC -->|add_library| WasmLib

    SystemC -->|add_executable| Tests
    Tests -->|link| SystemLib
    Tests -->|link| LibcLib

    style Root fill:#ccffcc
    style Components fill:#ffffcc
    style Targets fill:#ccccff
```

### Include Hierarchy

```mermaid
flowchart BT
    subgraph System["System Headers"]
        SysTypes["<sys/types.h>"]
        SysStat["<sys/stat.h>"]
        Signal["<signal.h>"]
        PThread["<pthread.h>"]
    end

    subgraph Public["Public API"]
        IoxTypes["iox/iox_types.h"]
        IoxSyscalls["iox/iox_syscalls.h"]
        IoxUmbrella["iox/iox.h"]
    end

    subgraph Private["Private Headers"]
        IxlandKernel["ixland_kernel.h"]
        TaskH["task.h"]
        SignalH["iox_signal.h"]
        VfsH["vfs.h"]
    end

    subgraph Source["Source Files"]
        TaskC["task.c"]
        SignalC["signal.c"]
        VfsC["vfs.c"]
    end

    SysTypes -->|includes| IoxTypes
    SysStat -->|includes| IoxTypes
    Signal -->|includes| IoxTypes

    IoxTypes -->|included by| IoxSyscalls
    IoxSyscalls -->|included by| IoxUmbrella

    IoxTypes -->|included by| IxlandKernel
    IxlandKernel -->|includes| TaskH
    IxlandKernel -->|includes| SignalH
    IxlandKernel -->|includes| VfsH

    TaskH -->|included by| TaskC
    SignalH -->|included by| SignalC
    VfsH -->|included by| VfsC

    style Public fill:#ccffcc
    style Private fill:#ffcccc
    style Source fill:#ccccff
```

---

## Header Organization

### Header Dependency Graph

```mermaid
flowchart LR
    subgraph Level0["Level 0: System"]
        Stdint["<stdint.h>"]
        SysTypes["<sys/types.h>"]
        SignalStd["<signal.h>"]
    end

    subgraph Level1["Level 1: Types"]
        IoxTypes["iox/iox_types.h"]
    end

    subgraph Level2["Level 2: Syscalls"]
        IoxSyscalls["iox/iox_syscalls.h"]
    end

    subgraph Level3["Level 3: Umbrella"]
        IoxH["iox/iox.h"]
    end

    subgraph Level4["Level 4: Internal"]
        IxlandKernel["ixland_kernel.h"]
        TaskH["task/task.h"]
        SignalH["signal/iox_signal.h"]
    end

    Stdint -->|provides types| IoxTypes
    SysTypes -->|provides pid_t| IoxTypes
    SignalStd -->|provides sigset_t| IoxTypes

    IoxTypes -->|required by| IoxSyscalls
    IoxTypes -->|required by| IoxH

    IoxSyscalls -->|required by| IxlandKernel
    IoxTypes -->|required by| TaskH
    IoxTypes -->|required by| SignalH
```

### Public API Surface

```mermaid
classDiagram
    class iox_types_h {
        +IOX_VERSION_STRING
        +iox_error_t
        +iox_config_t
        +iox_proc_info_t
        +iox_thread_info_t
        +iox_sys_info_t
        +iox_syscall_t
        +IOX_MAX_PATH
    }

    class iox_syscalls_h {
        +iox_fork()
        +iox_execve()
        +iox_exit()
        +iox_waitpid()
        +iox_kill()
        +iox_sigaction()
        +iox_open()
        +iox_read()
        +iox_write()
        +iox_close()
        +iox_mmap()
        +iox_munmap()
        +iox_init()
        +iox_cleanup()
    }

    class iox_h {
        Convenience umbrella header
        includes both above
    }

    class pwd_h {
        +getpwnam()
        +getpwuid()
        +getpwnam_r()
        +struct passwd
    }

    class grp_h {
        +getgrnam()
        +getgrgid()
        +getgrnam_r()
        +struct group
    }

    iox_h --> iox_types_h
    iox_h --> iox_syscalls_h
    iox_h --> pwd_h
    iox_h --> grp_h
```

### Internal Header Relationships

```mermaid
classDiagram
    class ixland_kernel_h {
        Kernel umbrella header
        Includes all internal
    }

    class task_h {
        iox_task_t
        iox_task_alloc()
        iox_task_free()
        iox_fork()
        iox_waitpid()
    }

    class iox_signal_h {
        iox_sighand_t
        iox_sigaction()
        iox_kill()
        iox_sigprocmask()
    }

    class vfs_h {
        vfs_translate_path()
        vfs_get_prefix()
    }

    class fdtable_h {
        iox_files_t
        iox_fdtable_alloc()
        iox_fdtable_get()
    }

    class exec_h {
        iox_execve()
        iox_exec_image_t
    }

    ixland_kernel_h --> task_h
    ixland_kernel_h --> iox_signal_h
    ixland_kernel_h --> vfs_h
    ixland_kernel_h --> fdtable_h

    task_h --> iox_signal_h : uses sighand
    task_h --> fdtable_h : uses files
    exec_h --> task_h : uses tasks
```

---

## Summary

This architecture documentation captures the complete post-mission state of the iXland kernel system:

1. **System Architecture**: Thread-based process simulation with virtual PIDs
2. **Component Interaction**: Clear flow diagrams for fork/exec, signals, and file operations
3. **Data Flow**: State machines and memory management visualization
4. **Build System**: CMake-based multi-component build
5. **Header Organization**: Clear separation between public and internal APIs

The architecture enables:
- **POSIX Compatibility**: Linux-compatible syscalls on iOS
- **Clean Boundaries**: Public API in ixland-libc, implementation in ixland-system
- **Extensibility**: WASI runtime integration ready
- **Testability**: Comprehensive test coverage with 213 assertions

---

*End of Architecture Documentation*

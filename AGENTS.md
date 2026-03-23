# Agent Guide: iox

## Overview

**iox** is a Linux-like virtual kernel subsystem for iOS, designed for maximum practical Linux userland compatibility within App Store constraints. It is not a compatibility shim—it is a virtual kernel substrate that implements Linux process, filesystem, signal, and terminal semantics.

## Mission Statement

Transform the current project from a fragmented compatibility layer into one coherent Linux-like virtual subsystem for iOS, achieving:

1. Maximum practical Linux userland compatibility (bash, coreutils, etc.)
2. Zero reliance on real host `fork()` or image replacement
3. One shared substrate for native commands and WAMR WASI modules
4. Deterministic structure mirroring Linux subsystem boundaries
5. Deterministic, trustworthy, reproducible build and test behavior

## Platform Policy (Non-negotiable)

- **Platform**: iOS only
- **Minimum deployment**: iOS 16.0+
- **Supported SDKs**: `iphonesimulator` and `iphoneos` only
- **Validation authority**: iOS Simulator and Device tests only
- **macOS builds**: Not authoritative for correctness

**Hard constraint**: Configure must hard-fail if not targeting iOS.

## Architectural Principles

### Single Canonical Execution Object

There is exactly one canonical execution object: **`iox_task`**.

- `iox_context.c` is a competing model—**must be eliminated**
- Global FD table in `iox_file.c`—**must be eliminated**
- All FDs live only under `task->files`

### WAMR as Execution Backend

WAMR is a secondary execution backend, not a separate universe:

- WASI uses same `task->files`, `task->fs`, `task->sighand`
- WAMR does not define independent process, fs, or signal models
- App customers access WAMR only through `iox_wamr_*`
- `deps/wamr/` remains pristine (read-only upstream)

### Virtual Process Model

No reliance on host `fork()` or `execve()`:

- `fork()`: Virtual process creation inside iox task system
- `execve()`: Virtual image replacement (native/WASI/script)
- Parent and child are separate `iox_task` objects
- All semantics implemented by iox, not host kernel

### Native Command Registry

Native commands execute through a registry, not arbitrary loading:

- Pre-registered at build time
- Entry ABI: `int iox_native_main(iox_task_t *task, int argc, char **argv, char **envp)`
- Examples: `/bin/bash`, `/bin/ls`, `/usr/bin/env`

## Core Objects

### iox_task

The single canonical task structure:

```c
typedef struct iox_task {
    pid_t pid, ppid, tgid, pgid, sid;
    atomic_int state;
    int exit_status;
    pthread_t thread;
    
    iox_files_t *files;     /* FD table (per-task) */
    iox_fs_t *fs;           /* Filesystem context */
    iox_sighand_t *sighand; /* Signal handlers */
    iox_tty_t *tty;         /* Controlling terminal */
    iox_exec_image_t *exec_image; /* Current executable */
    
    struct iox_task *parent, *children, *next_sibling;
} iox_task_t;
```

### Ownership Rules

| Concept | Owner | Location |
|---------|-------|----------|
| PID | iox_task | `task->pid` |
| FDs | iox_files | `task->files->fd[]` |
| CWD | iox_fs | `task->fs->cwd` |
| Signal handlers | iox_sighand | `task->sighand->action[]` |
| TTY | iox_tty | `task->tty` |
| Mounts | iox_mount_ns | `task->fs->mnt_ns` |

**No global state** for these concepts.

## Repository Layout

```
iox/
├── kernel/              # Kernel subsystems
│   ├── task/            # Process management
│   ├── signal/          # Signal delivery
│   ├── exec/            # Exec dispatch
│   ├── time/            # Clocks
│   └── resource/        # rlimits
├── fs/                  # Filesystem
│   ├── vfs/             # VFS
│   ├── proc/            # /proc
│   ├── dev/             # /dev
│   ├── devpts/          # /dev/pts
│   └── pipe/            # Pipes
├── drivers/
│   └── tty/             # PTY, termios
├── runtime/             # Execution backends
│   ├── native/          # Native commands
│   ├── wasi/            # WAMR WASI
│   └── script/          # Shebang scripts
├── compat/
│   ├── posix/           # POSIX compat
│   └── interpose/       # Symbol interposition
├── tests/               # Test suite
└── tools/               # Build scripts
```

## Build System (CMake Only)

### Requirements

1. CMake is single source of truth
2. Checked-in `CMakePresets.json`
3. iOS-only builds (hard fail otherwise)
4. Xcode generator
5. Deterministic test commands

### Supported Presets

- `ios-simulator-debug`
- `ios-simulator-release`
- `ios-simulator-asan`
- `ios-device-debug`
- `ios-device-release`

### Fresh Clone Bootstrap

```bash
tools/bootstrap.sh   # Setup
tools/doctor.sh      # Verify
tools/test-simulator.sh  # Test
```

## Testing Doctrine

### Truthfulness Rule

A feature is **NOT** complete without:
1. Deterministic automated test, OR
2. Conformance test, OR
3. Regression test, OR
4. Documented limitation

### Required Test Layers

1. **Unit tests**: Data structures, logic
2. **Integration tests**: Cross-subsystem
3. **Compatibility tests**: Linux userland
4. **WASI tests**: WASI compliance
5. **iOS tests**: Simulator and device
6. **Stress tests**: Concurrency
7. **Performance tests**: Regression

### Test Execution

```bash
# Simulator
tools/test-simulator.sh

# Device
tools/test-device.sh

# CTest (orchestrates xcodebuild)
ctest --preset ios-simulator-test
```

### Platform Testing Policy

- **Simulator**: Structural, integration, most correctness
- **Device**: Final authority, PTY, sandbox, timing
- **macOS**: Not authoritative

## Naming Conventions

### Three-Level Naming

| Level | Pattern | Purpose | Example |
|-------|---------|---------|---------|
| Internal | `__iox_*_impl()` | Implementation | `__iox_fork_impl()` |
| Public | `iox_*()` | Public API | `iox_fork()` |
| Interposed | Standard names | Interposition | `fork()` |

### WAMR Naming

| Layer | Pattern | Purpose |
|-------|---------|---------|
| Public | `iox_wamr_*` | App-facing API |
| Internal | `runtime/wasi/*` | Bridge logic |
| Upstream | `deps/wamr/*` | Read-only (no edits) |

## Agent Instructions

### Always Do

1. Keep `deps/wamr/` pristine—no edits
2. Implement Linux semantics in iox, not passthrough
3. Use `iox_task` as single execution model
4. Put FDs only under `task->files`
5. Test every subsystem with automated tests
6. Require iOS Simulator/Device validation
7. Document customer-facing API limits
8. Commit regression tests with bug fixes
9. Use deterministic test commands
10. Pin simulator/device in test scripts

### Never Do

1. Do not edit `deps/wamr/` (unless explicit upstream patch request)
2. Do not rely on host `fork()` or `execve()`
3. Do not use global FD tables
4. Do not keep dual process models
5. Do not present WAMR as separate universe
6. Do not mark features complete without tests
7. Do not use macOS-only validation
8. Do not rely on manual testing
9. Do not weaken bash compatibility goal
10. Do not leave high-risk code without stress tests
11. Do not split repo yet
12. Do not change build system before semantic unification

## Compatibility Target

Maximum practical Linux userland compatibility:

- **Supported**: bash, coreutils, grep, sed, awk, tar, make, findutils, readline, ncurses
- **Required**: PTY, job control, signals, /proc, /dev, pipes, redirections
- **WASI**: Same substrate as native code

Not "just POSIX"—Linux-oriented userland compatibility like Termux/Cygwin/MSYS2.

## execve Image Types

`execve` resolves exactly one of:

1. **Native registered command**: `/bin/ls` → `iox_native_main`
2. **WASM/WASI module**: `*.wasm` → WAMR runtime
3. **Script with shebang**: `#!/bin/sh` → interpreter dispatch

No other execution paths allowed.

## Fork Model

Virtual process creation:

1. Allocate new `iox_task` with new PID
2. Copy `files`, `fs`, `sighand` (refcounted)
3. Create host thread for child
4. Child runs in thread, returns 0
5. Parent returns child PID

Use `setjmp/longjmp` or equivalent for child continuation.

## Testing Requirements

### Per Subsystem

Every subsystem needs:
- Invariant tests
- Positive scenarios
- Negative scenarios
- Concurrency tests
- Compatibility tests
- Simulator coverage
- Device coverage

### Reliability Gates

All changes must pass:
1. Unit tests
2. Integration tests
3. Compatibility tests
4. WASI tests
5. Regression tests
6. Performance checks
7. Simulator tests
8. Device tests

### Flakiness Policy

- Flaky tests are release blockers
- Fix race conditions, not with longer sleeps
- Every timeout bounded and justified
- Tests deterministic and parallel-safe

## Security Constraints

- No real fork/exec
- No setuid
- No sandbox bypass
- No JIT or dynamic code generation
- No executable writable memory
- Validate all WASI crossings

## Current Status

**Phase**: Pre-implementation analysis complete

**Next**: Phase 0 - Repository reorganization

**See**: `docs/IOX_ARCHITECTURAL_ANALYSIS.md` for complete migration plan

## Documentation

- `README.md`: User-facing overview
- `docs/IOX_ARCHITECTURAL_ANALYSIS.md`: Architecture and migration
- `docs/SYSCALLS.md`: Syscall reference
- `docs/PORTING.md`: Porting guide
- `tests/README.md`: Testing guide

## Last Updated

2026-03-23

**Status**: Architecture defined, ready for Phase 0 implementation

**Primary Rule**: `deps/wamr/` is external, app customers access WAMR only through `iox_wamr_*`, and every subsystem requires executable test evidence.

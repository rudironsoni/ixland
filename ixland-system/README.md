> [!IMPORTANT]
> This component now lives inside the `a-shell-next` monorepo.
> Do not run `git submodule update --init --recursive`.
> Use the monorepo root checkout and see the root `README.md` for current setup and build instructions.
> Historical instructions below may be outdated.

# iox: iOS Linux-Like Virtual Subsystem

<p align="center">
<img src="https://img.shields.io/badge/Platform-iOS%2016.0+-lightgrey.svg" alt="Platform: iOS">
<img src="https://img.shields.io/badge/arch-arm64%20device%20%7C%20arm64%2Fx86__64%20sim-blue.svg" alt="Architecture: iOS only">
</p>

**iox** is a Linux-like virtual kernel subsystem for iOS, designed for maximum practical Linux userland compatibility within App Store constraints. It provides:

- Virtual `fork()`, `execve()`, and `waitpid()` without host process creation
- Unified task model for native commands and WAMR WASI modules
- PTY, job control, sessions, and process groups
- `/proc` and `/dev` virtual filesystems
- Complete file descriptor, signal, and VFS management
- Deterministic, trustworthy, and reproducible behavior

## Status

⚠️ **Work In Progress**: Transforming from compatibility layer to Linux-like subsystem. See `docs/IOX_ARCHITECTURAL_ANALYSIS.md` for the migration plan.

## Platform Policy

- **Platform**: iOS only (iOS 16.0+)
- **Architectures**: arm64 device, arm64/x86_64 simulator
- **Build Target**: iphonesimulator and iphoneos SDKs only
- **Validation Authority**: iOS Simulator and Device tests only
- **macOS builds**: Not authoritative for correctness

## Architecture Overview

iox implements Linux userland semantics through a virtual kernel substrate:

```
┌─────────────────────────────────────────────────────────────┐
│                     iOS App Container                       │
├─────────────────────────────────────────────────────────────┤
│                   Linux Userland Apps                       │
│         (bash, coreutils, grep, sed, awk, etc.)            │
├─────────────────────────────────────────────────────────────┤
│                 POSIX Syscall Interface                     │
│               fork, execve, open, signal, etc.            │
├─────────────────────────────────────────────────────────────┤
│                    iox Virtual Kernel                       │
│  ┌─────────┬─────────┬─────────┬─────────┬─────────┐       │
│  │  Task   │ Signal  │  Exec   │   VFS   │  TTY    │       │
│  │Manager  │Handler  │Dispatch │ Filesys │Driver   │       │
│  └─────────┴─────────┴─────────┴─────────┴─────────┘       │
├─────────────────────────────────────────────────────────────┤
│              Native | WASI | Script Runtimes                │
│   ┌──────────┐   ┌──────────┐   ┌──────────┐              │
│   │  Native  │   │   WASI   │   │  Script  │              │
│   │ Commands │   │  (WAMR)  │   │ Shebang  │              │
│   └──────────┘   └──────────┘   └──────────┘              │
├─────────────────────────────────────────────────────────────┤
│                    iOS Host APIs                            │
└─────────────────────────────────────────────────────────────┘
```

### Key Objects

- **`iox_task`**: Single canonical execution object with PID, PGID, SID
- **`iox_files`**: Per-task file descriptor table
- **`iox_fs`**: Per-task filesystem context (cwd, root, mounts)
- **`iox_sighand`**: Per-task signal handlers
- **`iox_tty`**: Controlling terminal and foreground pgrp
- **`iox_exec_image`**: Currently executing image (native/WASI/script)

## Compatibility Target

Maximum practical Linux userland compatibility, including:

- bash (with job control)
- coreutils (ls, cp, mv, rm, cat, etc.)
- grep, sed, awk
- tar, xz, make
- findutils, readline, ncurses
- WASI workloads via WAMR

Not literal Linux kernel compatibility—rather, Linux-like userland behavior through virtual kernel substrate.

## Build System

**Single Source of Truth**: CMake with `CMakePresets.json`

### Fresh Clone Setup

```bash
# Clone the monorepo (no submodules needed)
git clone https://github.com/rudironsoni/a-shell-next.git
cd a-shell-next/ixland-system

# Bootstrap (checks prerequisites, configures)
tools/bootstrap.sh

# Verify environment
tools/doctor.sh
```

### Build Commands

```bash
# Configure for iOS Simulator
cmake --preset ios-simulator-debug

# Build
cmake --build --preset ios-simulator-debug

# Configure for iOS Device
cmake --preset ios-device-debug

# Build
cmake --build --preset ios-device-debug
```

### Test Commands

```bash
# Simulator tests
tools/test-simulator.sh

# Device tests
tools/test-device.sh

# With CTest (orchestrates xcodebuild)
ctest --preset ios-simulator-test
```

## Repository Layout

```
iox/
├── include/
│   ├── iox/               # Public headers
│   │   ├── iox.h
│   │   ├── iox_syscalls.h
│   │   └── iox_wamr.h
│   └── uapi/linux/        # Linux-compatible UAPI
├── kernel/                # Kernel subsystems
│   ├── task/              # fork, exit, wait, PID
│   ├── signal/            # signal delivery, pgrp, session
│   ├── exec/              # exec dispatch
│   ├── time/              # clocks
│   └── resource/          # rlimits
├── fs/                    # Filesystem
│   ├── vfs/               # VFS and mount namespaces
│   ├── proc/              # /proc filesystem
│   ├── dev/               # /dev filesystem
│   ├── devpts/            # /dev/pts
│   └── pipe/              # pipe implementation
├── drivers/
│   └── tty/               # PTY, termios, TTY
├── runtime/               # Execution backends
│   ├── native/            # Native command registry
│   ├── wasi/              # WAMR WASI bridge
│   └── script/            # Shebang interpreter
├── compat/
│   ├── posix/             # POSIX compatibility
│   └── interpose/         # Symbol interposition
├── tests/                 # Test suite
│   ├── unit/              # Core unit tests
│   ├── integration/       # Subsystem integration
│   ├── compat/            # Linux compatibility
│   ├── wasi/              # WASI tests
│   ├── device/            # Device-specific
│   ├── simulator/         # Simulator-specific
│   ├── stress/            # Stress tests
│   └── perf/              # Performance tests
├── tools/                 # Build and test scripts
└── deps/
    └── wamr/              # WAMR submodule (read-only)
```

## Native Command Registry

Native commands are pre-registered, not dynamically loaded:

```c
// Register a native command
IOX_NATIVE_CMD("/bin/ls", iox_ls_main);
IOX_NATIVE_CMD("/bin/cat", iox_cat_main);

// Entry ABI
int iox_ls_main(iox_task_t *task, int argc, char **argv, char **envp) {
    // Implementation
    return 0;
}
```

Registry is generated at build time from `runtime/native/commands/`.

## WASI/WAMR Integration

WAMR is an execution backend, not a separate universe:

- WASI operations use same `task->files` as native code
- WASI paths resolve through same VFS
- WASI stdio uses same descriptors
- WASI clocks/random through iox kernel abstractions

```c
// Load and run WASM module
iox_wamr_load_module(wasm_buffer, wasm_size);
iox_wamr_call_function("_start", argc, argv);
```

## Testing Doctrine

Every subsystem requires automated tests:

1. **Unit tests**: Data structures, deterministic logic
2. **Integration tests**: Cross-subsystem interactions
3. **Compatibility tests**: Linux userland behavior
4. **WASI tests**: WASI syscall compliance
5. **Device tests**: iOS-specific behavior
6. **Stress tests**: Concurrency and reliability
7. **Performance tests**: Regression baselines

A feature is **not** complete without executable evidence.

### Test Commands

```bash
# Run all tests
tools/test-simulator.sh

# Run specific test category
ctest --preset ios-simulator-test -R unit
ctest --preset ios-simulator-test -R integration

# Run with sanitizer
cmake --preset ios-simulator-asan
ctest --preset ios-simulator-test
```

## Constraints

- No real `fork()` or `execve()` image replacement
- No setuid
- No sandbox bypass
- No JIT or dynamic code generation
- No executable writable memory
- All WASI crossings validated

## Documentation

- `docs/IOX_ARCHITECTURAL_ANALYSIS.md` - Architecture and migration plan
- `AGENTS.md` - Developer guidelines
- `docs/SYSCALLS.md` - Syscall reference
- `docs/PORTING.md` - Porting guide
- `tests/README.md` - Testing guide

## Development Status

- **Phase 0**: Repository reorganization (current)
- **Phase 1**: Core kernel objects (task, files, fs, signal)
- **Phase 2**: Fork/exec implementation
- **Phase 3**: PTY, job control, signals
- **Phase 4**: Native command registry
- **Phase 5**: WASI integration
- **Phase 6**: Test infrastructure
- **Phase 7**: Bash compatibility validation

See `docs/IOX_ARCHITECTURAL_ANALYSIS.md` for detailed implementation order.

## License

[License information]

---

> **Note**: WAMR integration is handled within the monorepo. See the root `README.md` for current setup instructions. This component is now tracked directly in the monorepo.

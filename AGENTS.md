# Agent Guide: libiox

## Overview

**libiox** is the iOS eXtension subsystem - a complete Linux syscall compatibility layer for iOS. Unlike the previous a-shell-kernel, libiox provides clean symbol interposition without macro pollution.

### Key Changes from a-shell-kernel

| Old | New |
|-----|-----|
| `a_shell_fork()` | `__iox_fork_impl()` (internal) |
| | `iox_fork()` (public) |
| `a_shell_system.m` | `src/iox/core/*.c` |
| `libc_replacement.c` | `src/iox/runtime/*.c` |
| `#define fork a_shell_fork` | Strong symbol: `pid_t fork(void) { return iox_fork(); }` |
| `include/linux/` | `include/iox/sys/` |

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Linux Application                        │
│                    (unmodified source)                      │
├─────────────────────────────────────────────────────────────┤
│  #include <unistd.h>                                       │
│  fork(); execve();                                          │
├─────────────────────────────────────────────────────────────┤
│                    libiox.a                                 │
│  Strong symbol: fork() ──► iox_fork()                       │
│  Strong symbol: execve() ──► iox_execve()                   │
│  ...                                                        │
├─────────────────────────────────────────────────────────────┤
│                    iOS Kernel APIs                          │
│  Thread-based process simulation                            │
│  Virtual PIDs                                               │
└─────────────────────────────────────────────────────────────┘
```

## Naming Convention

### Three-Level Naming

| Level | Pattern | Purpose | Example |
|-------|---------|---------|---------|
| **Internal** | `__iox_*_impl()` | Implementation details | `__iox_fork_impl()` |
| **Public** | `iox_*()` | Public C API | `iox_fork()` |
| **Linux** | Standard names | Symbol interposition | `fork()` (via strong symbol) |

### File Naming

| Type | Pattern | Example |
|------|---------|---------|
| Core syscalls | `iox_<category>.c` | `iox_process.c` |
| Interposition | `iox_interpose.c` | All 300+ wrappers |
| Runtime | `iox_<func>.c` | `iox_stdio.c` |
| WAMR | `iox_wamr_*.c` | `iox_wamr_wasi.c` |

## Directory Structure

```
a-shell-kernel/
├── include/iox/              # Public headers
│   ├── iox.h                 # Master umbrella
│   ├── iox_syscalls.h        # Public API declarations
│   ├── iox_wamr.h           # WAMR API
│   └── sys/                  # Linux-compatible
│       ├── unistd.h
│       ├── wait.h
│       └── ...
│
├── src/iox/                  # ALL implementations
│   ├── core/                 # Syscall implementations
│   │   ├── iox_process.c
│   │   ├── iox_file.c
│   │   └── ...
│   ├── interpose/            # Symbol interposition
│   │   └── iox_interpose.c   # 300+ wrappers
│   ├── runtime/              # C runtime
│   │   ├── iox_stdio.c
│   │   ├── iox_env.c
│   │   └── ...
│   ├── wamr/                 # WAMR integration
│   │   ├── iox_wamr_runtime.c
│   │   ├── iox_wamr_wasi.c
│   │   └── ...
│   └── internal/             # Private headers
│       └── iox_internal.h
│
├── lib/                      # Build outputs
│   ├── libiox.a
│   └── libiox.dylib
│
├── bin/                      # Tools
│   └── iox-cc                # Compiler wrapper
│
└── deps/wamr/                # WAMR submodule
```

## Responsibilities

### What libiox DOES

1. **Process Simulation**
   - Virtual PIDs (1024 max processes)
   - `iox_fork()` - Thread-based fork simulation
   - `iox_execve()` - Thread-based execution
   - `iox_waitpid()` - Virtual process management
   - `iox_getpid()`, `iox_getppid()` - Virtual PID tracking

2. **Symbol Interposition**
   - Strong symbols: `fork()`, `execve()`, `open()`, etc.
   - Zero macro pollution
   - Compile-time interception

3. **Signal Handling**
   - Thread-based signal delivery
   - `iox_signal()`, `iox_sigaction()`, `iox_kill()`
   - Signal masking: `iox_sigprocmask()`, `iox_sigpending()`

4. **File I/O**
   - Standard operations: `iox_open()`, `iox_read()`, `iox_write()`, `iox_close()`
   - Directory operations: `iox_mkdir()`, `iox_rmdir()`, `iox_chdir()`, `iox_getcwd()`
   - Metadata: `iox_stat()`, `iox_fstat()`, `iox_lstat()`
   - Links: `iox_link()`, `iox_symlink()`, `iox_readlink()`

5. **Memory Management**
   - `iox_mmap()`, `iox_munmap()`, `iox_mprotect()`
   - iOS-specific restrictions

6. **WAMR Integration**
   - WASI syscall bridge
   - AoT module loading
   - WebAssembly execution

### What libiox DOES NOT DO

- ❌ NOT a terminal emulator (that's a-shell/)
- ❌ NOT a package manager (separate repo: iox-packages)
- ❌ NOT a shell (bash compiled with iox-cc)
- ❌ NOT a build system (use iox-cc or CMake)

## Build System

### Building libiox

```bash
mkdir build && cd build
cmake ..
make

# Create XCFramework
make xcframework
```

### Output

- `build/libiox.a` - Static library
- `build/libiox.dylib` - Dynamic library (optional)
- `build/libiox.xcframework/` - Universal framework
- Headers included: `include/iox/` copied into XCFramework

## How to Add New Syscalls

### Step 1: Declare Public API

```c
// include/iox/iox_syscalls.h

pid_t iox_newsyscall(int arg);
```

### Step 2: Implement

```c
// src/iox/core/iox_<category>.c

int __iox_newsyscall_impl(int arg) {
    // iOS-safe implementation
    return result;
}

pid_t iox_newsyscall(int arg) {
    return __iox_newsyscall_impl(arg);
}
```

### Step 3: Add Interposition

```c
// src/iox/interpose/iox_interpose.c

pid_t newsyscall(int arg) __attribute__((visibility("default"))) {
    return iox_newsyscall(arg);
}
```

### Step 4: Test

```c
// tests/unit/test_<category>.c

void test_newsyscall() {
    pid_t result = newsyscall(42);
    assert(result == expected);
}
```

## Agent Instructions

### When Working on This Layer

1. **Test compilation frequently**
   ```bash
   cd build && make clean && make
   ```

2. **Run test suite**
   ```bash
   cd build && make test
   ```

3. **Verify library creation**
   ```bash
   ls -la build/libiox.a
   nm build/libiox.a | grep " T " | head -20
   ```

4. **Check integration**
   - Verify app builds with updated library
   - Test basic syscalls work

### Common Tasks

**Adding a new syscall:**
1. Add declaration to `include/iox/iox_syscalls.h`
2. Implement in `src/iox/core/iox_<category>.c`
3. Add interposition in `src/iox/interpose/iox_interpose.c`
4. Test compilation
5. Commit: `feat(syscall): add iox_newsyscall`

**Fixing a syscall:**
1. Identify the issue
2. Check implementation in `src/iox/core/iox_<category>.c`
3. Look for iOS-specific workarounds
4. Test the fix
5. Commit: `fix(syscall): iox_<syscall> <description>`

**Adding WAMR support:**
1. Add WASI bridge function in `src/iox/wamr/iox_wamr_wasi.c`
2. Register in native symbol table
3. Test with WASM binary
4. Commit: `feat(wamr): add <syscall> WASI bridge`

## Integration Points

### a-shell/ (App)
- Links with `libiox.a` or `libiox.xcframework`
- Shell binaries compiled with iox-cc
- WASM binaries run via iox-wamr

### iox-packages/ (Package Manager)
- Compiles packages with iox-cc
- Uses `include/iox/` headers
- Links against libiox syscalls

## Constraints

- **iOS 16.0+** minimum deployment target
- **arm64** only (device and simulator)
- **No real fork/exec** - use threads
- **No setuid** - iOS sandbox restrictions
- **File paths** - must use app sandbox paths
- **App Store compliance** - no JIT, no dynamic code generation

## Security Considerations

- Never bypass iOS sandbox
- Always validate paths
- Use thread-local storage for per-process data
- No executable memory (W^X policy)
- WASI syscalls validated before execution

## Documentation

- **Architecture**: `docs/LIBIOX_ARCHITECTURE.md`
- **Syscalls**: `docs/SYSCALLS.md`
- **WAMR**: `docs/WAMR.md`
- **Porting**: `docs/PORTING.md`

---

**Last Updated**: 2025-01-XX  
**Status**: Foundation Complete, Implementation In Progress  
**Next Steps**: Implement core syscalls, integrate WAMR, create packages

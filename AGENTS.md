# Agent Guide: a-shell-kernel

## Overview

**a-shell-kernel** is the syscall simulation layer that provides Linux-compatible system calls on iOS. It allows native Linux tools (bash, coreutils, git, vim, python) to run without modification by intercepting syscalls and redirecting them to iOS-safe implementations.

## Responsibilities

### What This Layer DOES

1. **Process Simulation**
   - Virtual PIDs (1024 max processes)
   - `fork()`, `vfork()` - Returns ENOSYS (iOS forbids real fork)
   - `execve()` - Thread-based execution simulation
   - `waitpid()`, `wait3()`, `wait4()` - Virtual process management
   - `getpid()`, `getppid()`, `getpgrp()` - Virtual PID tracking

2. **Signal Handling**
   - Signal delivery to threads (not real processes)
   - `signal()`, `sigaction()`, `kill()`
   - Signal masking: `sigprocmask()`, `sigpending()`
   - Alarm: `alarm()`, `setitimer()`

3. **File I/O**
   - Standard file operations: `open()`, `read()`, `write()`, `close()`
   - Directory operations: `mkdir()`, `rmdir()`, `chdir()`, `getcwd()`
   - File metadata: `stat()`, `fstat()`, `lstat()`, `chmod()`, `chown()`
   - Links: `link()`, `symlink()`, `readlink()`, `unlink()`

4. **Memory Management**
   - `mmap()`, `munmap()`, `mprotect()`
   - iOS-specific restrictions (no executable stack, etc.)

5. **TTY/Terminal**
   - `tcgetattr()`, `tcsetattr()`
   - `ioctl()` for terminal window size

6. **Time**
   - `nanosleep()`, `usleep()`, `alarm()`
   - `gettimeofday()`, `clock_gettime()`

### What This Layer DOES NOT DO

- ❌ NOT a terminal emulator (that's a-shell/)
- ❌ NOT a package manager (that's a-shell-packages/pkg/)
- ❌ NOT a shell (bash lives in a-Shell.app/bin/)
- ❌ NOT a build system (that's a-shell-packages/scripts/)

## Architecture

```
Linux Tool (bash)
    ↓ calls
a-shell-kernel (syscalls)
    ↓ translates to
iOS APIs (threads, files)
```

## Key Files

| File | Purpose |
|------|---------|
| `include/linux/unistd.h` | Process, file, time syscalls |
| `include/linux/signal.h` | Signal handling |
| `include/linux/fcntl.h` | File operations |
| `include/linux/stat.h` | File metadata |
| `include/linux/sys/wait.h` | Process waiting |
| `a_shell_system.m` | Main implementation |
| `libc_replacement.c` | libc function replacements |

## Build System

### Building the XCFramework

```bash
cd a-shell-kernel/
make clean
make              # Build iOS + Simulator
make xcframework  # Create XCFramework
```

### Output

- `build/a-shell-kernel.xcframework/` - Universal framework
- Headers included: `include/` copied into XCFramework

## How to Add New Syscalls

1. **Declare in header** (`include/linux/<category>.h`)
   ```c
   extern int a_shell_<syscall>(...);
   #define <syscall> a_shell_<syscall>
   ```

2. **Implement** (in `src/syscalls/` or `a_shell_system.m`)
   ```c
   int a_shell_<syscall>(...) {
       // iOS-safe implementation
       return result;
   }
   ```

3. **Test** (`tests/test_<category>.c`)
   ```c
   void test_<syscall>() {
       // Test cases
   }
   ```

## Testing

```bash
cd a-shell-kernel/
make test-compile  # Compile all tests
```

## Agent Instructions

### When Working on This Layer

1. **Test compilation frequently**
   ```bash
   make clean && make
   ```

2. **Run test suite**
   ```bash
   make test-compile
   ```

3. **Verify XCFramework creation**
   ```bash
   ls -la build/a-shell-kernel.xcframework/
   ```

4. **Check kernel integration**
   - Verify app still builds with updated kernel
   - Test basic commands work

### Common Tasks

**Adding a new syscall:**
1. Check if it exists in headers
2. If not, add to appropriate header in `include/linux/`
3. Implement in `src/syscalls/` or `a_shell_system.m`
4. Update umbrella header `include/a_shell_kernel.h`
5. Test compilation
6. Commit with message: "feat(kernel): add <syscall> syscall"

**Fixing a syscall:**
1. Identify the issue
2. Check implementation in `a_shell_system.m`
3. Look for iOS-specific workarounds
4. Test the fix
5. Commit with message: "fix(kernel): <syscall> <description>"

**Updating headers:**
1. Headers are in `include/linux/` (Linux-compatible)
2. Follow Linux header structure
3. Use `#include <...>` for system headers
4. Use `#include "..."` for project headers
5. Keep macros at end of headers

## Integration Points

### a-shell/ (App)
- Uses `a-shell-kernel.xcframework`
- Links against kernel syscalls
- Shell binaries run on top of kernel

### a-shell-packages/ (Build System)
- Compiles packages with kernel headers
- Uses `-I../../a-shell-kernel/include`
- Packages link against kernel syscalls

## Constraints

- **iOS 16.0+** minimum deployment target
- **arm64** only (device and simulator)
- **No fork/exec** - use threads
- **No setuid** - iOS sandbox restrictions
- **File paths** - must use app sandbox paths

## Security Considerations

- Never bypass iOS sandbox
- Always validate paths
- Use thread-local storage for per-process data
- No executable memory (W^X policy)

## Documentation

- **Kernel headers**: `include/linux/*.h`
- **Test suite**: `tests/`
- **Main build**: `Makefile`

---

**Last Updated**: 2026-03-20
**Status**: Kernel build working, XCFramework ready
**Next Steps**: Continue building packages, integrate with app

# libiox: iOS Subsystem for Linux

<p align="center">
<img src="https://img.shields.io/badge/Platform-iOS%2016.0+-lightgrey.svg" alt="Platform: iOS">
<img src="https://img.shields.io/badge/arch-arm64-blue.svg" alt="Architecture: arm64">
<a href="https://github.com/yourusername/a-shell-kernel/actions"><img src="https://github.com/yourusername/a-shell-kernel/workflows/CI/badge.svg" alt="Build Status"/></a>
</p>

**libiox** is a Linux syscall compatibility layer for iOS that enables unmodified Linux binaries to run natively on iOS devices. It provides complete Linux syscall simulation through static symbol interposition and optional WebAssembly (WAMR) support.

## Overview

libiox allows Linux applications (bash, git, vim, python, etc.) to run on iOS without source code modifications:

```bash
# Compile Linux source for iOS
iox-cc program.c -o program_ios

# Run natively at full speed
./program_ios

# Or run prebuilt WASM binaries
iox-wamr program.wasm
```

## Key Features

- **Native Performance**: Linux binaries compiled with `iox-cc` run at 100% native speed
- **Zero Source Changes**: Standard Linux programs compile and run unmodified
- **Complete Syscall Support**: 300+ Linux syscalls simulated
- **WAMR Integration**: Run WebAssembly binaries with WASI support
- **App Store Compliant**: No JIT, no dynamic code generation
- **Thread-Based Processes**: Virtual PIDs without real fork()

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Linux Application                        │
│                    (unmodified source)                      │
├─────────────────────────────────────────────────────────────┤
│  #include <unistd.h>  // Standard Linux headers            │
│  fork(); execve();  // Standard syscalls                   │
├─────────────────────────────────────────────────────────────┤
│                    libiox.a (static library)                │
│  fork() ──► iox_fork() ──► __iox_fork_impl()               │
│  execve() ──► iox_execve() ──► thread creation            │
│  ... 300+ syscalls ...                                     │
├─────────────────────────────────────────────────────────────┤
│                    iOS APIs                                 │
│  Thread-based process simulation                            │
│  Virtual PID management                                       │
│  Signal delivery                                              │
│  File I/O (sandboxed)                                         │
└─────────────────────────────────────────────────────────────┘
```

## Quick Start

### Building libiox

```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/a-shell-kernel.git
cd a-shell-kernel

# Build
mkdir build && cd build
cmake ..
make

# Install
sudo make install
```

### Compiling Linux Programs

```bash
# Use iox compiler wrapper
iox-cc hello.c -o hello

# Or manually
clang -target arm64-apple-ios16.0 \
    -isystem /usr/local/include/iox \
    -L/usr/local/lib -liox \
    hello.c -o hello
```

### Running WASM Binaries

```bash
# Build with WASI SDK
wasi-sdk/bin/clang -o program.wasm program.c

# Convert to AoT (optional, for better performance)
wamrc --target=aarch64 -o program.aot program.wasm

# Run with iox-wamr
iox-wamr program.aot
```

## Syscall Coverage

### Process Management
- fork, vfork, execve, execv, exit
- getpid, getppid, waitpid, wait, wait3, wait4
- system

### File Operations
- open, openat, creat, read, write, close
- lseek, pread, pwrite, dup, dup2, fcntl
- stat, fstat, lstat, mkdir, rmdir
- link, symlink, readlink, unlink
- chmod, chown, access
- chdir, getcwd, chroot

### Signal Handling
- signal, kill, raise, sigaction
- sigprocmask, sigpending, sigsuspend
- alarm, setitimer, pause

### Memory Management
- mmap, munmap, mprotect
- mlock, munlock

### Time
- sleep, usleep, nanosleep
- gettimeofday, clock_gettime, time

### Network (Passthrough)
- socket, bind, connect, listen, accept
- send, sendto, recv, recvfrom
- shutdown, getsockopt, setsockopt

### And 200+ more...

See [docs/SYSCALLS.md](docs/SYSCALLS.md) for complete list.

## Project Structure

```
a-shell-kernel/
├── include/iox/           # Public headers
│   ├── iox.h              # Master header
│   └── sys/               # Linux-compatible headers
├── src/iox/               # Implementation
│   ├── core/              # Syscall implementations
│   ├── interpose/         # Symbol interposition
│   ├── runtime/           # C runtime
│   └── wamr/              # WAMR integration
├── lib/                   # Build outputs
├── bin/                   # Tools (iox-cc, iox-wamr)
├── deps/wamr/             # WAMR submodule
└── tests/                 # Test suite
```

## Integration

### As Static Library

```c
// Your iOS app
dylibimport("libiox.a");

// Now Linux syscalls work
pid_t pid = fork();  // Thread-based simulation
```

### As XCFramework

```bash
make xcframework
# Generates: build/libiox.xcframework/
```

Then in Xcode:
1. Add `libiox.xcframework` to your project
2. Link against it
3. Include `<iox/iox.h>`

## WAMR Integration

libiox includes full WAMR (WebAssembly Micro Runtime) integration for running WASM binaries:

- **WASI Support**: Complete WASI syscall bridge
- **AoT Compilation**: 90-95% native performance
- **App Store Compliant**: No JIT, no runtime code generation

### Building WAMR

```bash
cd deps/wamr/product-mini/platforms/ios
mkdir build && cd build
cmake .. -DWAMR_BUILD_AOT=1 -DWAMR_BUILD_LIBC_WASI=1
make
```

## Package Management

### iox-pkg

Install Linux tools on iOS:

```bash
# Install native-compiled bash
iox-pkg install bash

# Install WASM version of ffmpeg
iox-pkg install ffmpeg

# List installed packages
iox-pkg list

# Search available packages
iox-pkg search editor
```

Packages are hosted on GitHub Releases.

## Compatibility

### Requirements
- iOS 16.0+
- arm64 (device and simulator)
- Xcode 14.0+

### Limitations
- **No real fork()**: Thread-based simulation
- **No setuid**: iOS sandbox restrictions
- **Sandboxed paths**: Must stay within app container
- **No root**: Cannot run privileged commands

### Tested Programs
- ✅ bash
- ✅ coreutils (ls, cp, mv, rm, etc.)
- ✅ git
- ✅ vim
- ✅ python3 (via WASM)
- ✅ ffmpeg (via WASM)

## Documentation

- [Architecture](docs/LIBIOX_ARCHITECTURE.md) - Complete system architecture
- [Syscalls](docs/SYSCALLS.md) - Full syscall reference
- [Porting Guide](docs/PORTING.md) - How to port Linux programs
- [WAMR Setup](docs/WAMR.md) - WebAssembly integration
- [Package Creation](docs/PACKAGES.md) - Creating packages

## Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

### Development Setup

```bash
# Install dependencies
brew install cmake llvm

# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run tests
make test
```

## License

BSD 3-Clause License. See [LICENSE](LICENSE) for details.

## Acknowledgments

- Based on a-Shell and ios_system by Nicolas Holzschuch
- WAMR integration by Bytecode Alliance
- WASI SDK by WebAssembly Community

---

**Version**: 1.0.0  
**Status**: Foundation Complete  
**Last Updated**: 2025-01-XX

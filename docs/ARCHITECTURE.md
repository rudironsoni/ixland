# a-Shell Next Architecture

## Overview

a-Shell Next brings a complete Linux-like environment to iOS by:
1. **Simulating** Unix processes with threads (no real fork on iOS)
2. **Patching** upstream packages via compile-time macros
3. **Providing** both native XCFramework and WASM execution modes

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    a-Shell iOS App                          │
│              (UIKit, Terminal Emulator, Swift)                │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│               a-shell-distro (Swift)                          │
│  - Session management                                         │
│  - Command dispatch                                           │
│  - WAMR WASM integration                                      │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│              a-shell-kernel (C)                             │
│  - ios_fork(), ios_execv(), ios_waitpid()                    │
│  - Virtual process table (1024 processes)                   │
│  - Thread-local I/O redirection                             │
│  - Signal emulation                                           │
└──────────────────────────┬──────────────────────────────────┘
                           │
        ┌──────────────────┴──────────────────┐
        │                                       │
┌───────▼────────┐                  ┌────────▼──────┐
│ Native Tools     │                  │ WASM Runtime  │
│ (XCFrameworks)   │                  │ (WAMR)        │
│                  │                  │               │
│ - apt 3.1.16     │                  │ - vim         │
│ - python 3.12    │                  │ - nodejs      │
│ - bash, zsh      │                  │ - emacs       │
│ - curl, ssh      │                  │ - etc         │
└──────────────────┘                  └───────────────┘
```

## Component Details

### a-shell-kernel

**Purpose**: Low-level syscall simulation for iOS

**Key Files**:
- `a_shell_kernel.h` - Main kernel interface
- `a_shell_system.h` - Syscall macros for package builds
- `a_shell_syscalls.c` - Syscall implementations
- `a_shell_process.c` - Process table management
- `a_shell_error.h` - Error handling and I/O redirection

**Process Model**:
- **1024 virtual processes** tracked in process table
- **Thread-based**: Each "process" is a pthread
- **Parent/child tracking**: via ppid field
- **vfork() semantics**: Allocate PID without creating thread (for exec)
- **Ghost thread prevention**: Cleanup thread removes exited processes

**Syscall Replacement**:
```c
// Via a_shell_system.h (compile-time macros)
#define fork ios_fork       // Returns ENOSYS (no real fork)
#define vfork ios_vfork     // Allocates virtual PID
#define execv ios_execv     // Executes via ios_system()
#define waitpid ios_waitpid // Polls process table
```

### a-shell-distro

**Purpose**: High-level Linux distribution features

**Key Components**:
- Session management (environment, working directory)
- Command dispatch (native vs WASM)
- WAMR integration for WASM packages
- Package registry interface

### a-shell-packages

**Purpose**: Package build system (Termux-inspired)

**Structure**:
```
a-shell-packages/
├── core-packages/       # Native XCFrameworks
│   ├── apt/             # Package manager
│   ├── python/          # Python runtime
│   ├── bash/            # Shells
│   └── ...
├── packages/            # WASM packages
│   ├── vim/
│   ├── nodejs/
│   └── ...
└── scripts/             # Build infrastructure
```

**Build Variables**:
- `ASHELL_PKG_NAME` - Package name
- `ASHELL_PKG_VERSION` - Package version
- `ASHELL_PKG_SRCURL` - Source URL
- `ASHELL_PKG_SHA256` - Source checksum
- `ASHELL_PREFIX` - Install prefix (default: /usr/local)

**Build Steps**:
1. Download and verify source
2. Extract to `$ASHELL_PKG_SRCDIR`
3. Apply patches (with @ASHELL_PREFIX@ substitution)
4. Configure (CMake/Autotools/Meson)
5. Compile with `-include a_shell_system.h`
6. Install to `$ASHELL_PKG_PREFIX`
7. Create XCFramework
8. Codesign

## Execution Flow

### Native Command Execution

```
1. User types: $ ls -la
2. a-shell-distro parses command
3. Checks if native (XCFramework) or WASM
4. For native:
   a. Create new virtual PID via ios_vfork()
   b. Set up thread-local I/O
   c. Execute via ios_system()
   d. Wait via ios_waitpid()
   e. Cleanup
```

### WASM Command Execution

```
1. User types: $ vim file.txt
2. a-shell-distro recognizes as WASM package
3. WAMR loads vim.wasm module
4. WASI syscalls redirected to a-shell-kernel
5. Execute in WASM sandbox
```

## Package Registry

**URL**: `https://packages.a-shell.dev/`

**Structure**:
```
stable/
├── core/               # Native XCFrameworks
│   ├── apt/
│   ├── python/
│   └── ...
└── wasm/               # WASM packages
    ├── vim/
    ├── nodejs/
    └── ...

pip/                    # Python wheel index
└── simple/             # PyPI-compatible
    ├── numpy/
    └── ...
```

**Package Format**:
- Native: `.xcframework` (iOS standard)
- WASM: `.wasm` + `.json` metadata
- Python: `.whl` (iOS-specific architecture tags)

## iOS-Specific Considerations

### No fork() Support

iOS forbids `fork()` system call. We simulate with:
- **vfork()**: Allocate virtual PID without thread
- **pthread**: Each "process" is a thread
- **Process table**: Track parent/child relationships

### Code Signing Required

All binaries must be codesigned:
- Development: Self-signed certificate
- Distribution: Apple Developer certificate
- XCFramework format handles signing

### App Sandbox Restrictions

- No access to `/system`, `/proc`
- Limited file system access
- No raw sockets (without entitlements)
- TTY/PTY support varies by iOS version

### Mach-O vs ELF

- iOS uses Mach-O binaries, not ELF
- XCFrameworks contain multiple architectures
- Dynamic linking different from Linux

## Build Environments

### Primary: macOS + Xcode

**Requirements**:
- macOS 14+ (Sonoma)
- Xcode 15+ with iOS SDK
- Command Line Tools

**Usage**:
```bash
./build.sh bash
./build.sh python
```

### Secondary: Linux + osxcross

**Requirements**:
- Docker
- osxcross toolchain

**Usage**:
```bash
./build.sh --cross-compile libz
./build.sh --cross-compile libssl
```

**Limitations**:
- No iOS Simulator support
- Limited framework support
- Good for libraries, not apps

## Development Workflow

### Adding a New Package

1. Create `a-shell-packages/packages/<name>/build.sh`
2. Add patches in `patches/` directory
3. Test with `./build.sh <name>`
4. Submit PR

### Patch Guidelines

- Use `@ASHELL_PREFIX@` placeholder
- Document changes in patch headers
- Keep patches minimal
- Test on real device

### Testing

**Local Testing**:
```bash
./build.sh <package>
./scripts/install-to-device.sh <package>
```

**CI Testing**:
- GitHub Actions with macOS runners
- Build all packages
- Run unit tests

## Performance Considerations

### Memory Usage

- Process table: ~100KB (1024 entries)
- WAMR runtime: ~200KB
- Thread overhead: ~1MB per thread

### Execution Speed

- Native XCFrameworks: Near-native speed
- WASM (interpreter): ~10x slower
- WASM (AOT): ~2x slower

### Bundle Size

- a-shell-kernel: ~500KB
- Essential packages: ~50MB
- Full distribution: ~200MB

## Security Model

### Sandbox

- App Sandbox restricts file access
- Network restricted to declared domains
- No code generation (JIT limited)

### Package Security

- All packages codesigned
- SHA256 verification
- Trusted registry only

### WASM Security

- WAMR sandbox isolates WASM
- WASI capabilities limited
- No raw system calls

## Future Enhancements

### Phase 2
- [ ] Full TTY/PTY support
- [ ] Process groups and sessions
- [ ] Signal delivery mechanism
- [ ] Shared memory support

### Phase 3
- [ ] JIT compilation for WASM
- [ ] Package caching
- [ ] Parallel builds
- [ ] CI/CD integration

## References

- [Termux Packages](https://github.com/termux/termux-packages)
- [BeeWare Python-Apple-Support](https://github.com/beeware/Python-Apple-support)
- [WAMR Documentation](https://wamr.gitbook.io/)
- [iOS System Extensions](https://developer.apple.com/documentation/systemextensions)

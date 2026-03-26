# a-Shell Next: Master Implementation Plan

**Version**: 1.0  
**Last Updated**: 2026-03-19  
**Status**: In Progress

---

## Executive Summary

a-Shell Next brings a complete Linux-like environment to iOS through:
- **ixland-system**: Thread-based process simulation (no real fork/exec on iOS)
- **a-shell-distro**: Session management and command dispatch  
- **ixland-packages**: Termux-style package build system
- **WAMR**: WebAssembly runtime for downloadable packages
- **Python-Apple-Support**: Native Python with custom pip index for iOS wheels

---

## Architecture

```
a-shell-next/
├── ixland-system/           # Syscall simulation layer (C)
│   ├── a_shell_kernel.h     # Main kernel header
│   ├── a_shell_system.h     # Syscall macros for packages
│   ├── a_shell_syscalls.c   # Syscall implementations
│   └── wamr/                # WAMR WASM runtime
├── a-shell-distro/          # Distribution/session management (Swift)
│   ├── Sources/
│   │   ├── Session/         # Session management
│   │   ├── Commands/        # Command dispatch
│   │   └── WASM/            # WAMR integration
│   └── Package.swift
├── a-shell-packages/        # Package build system
│   ├── core-packages/       # Native XCFrameworks
│   │   ├── apt/             # Package manager (3.1.16)
│   │   ├── python/          # Python 3.12 with custom pip
│   │   ├── bash/
│   │   ├── zsh/
│   │   └── ...
│   ├── packages/            # WASM packages (WAMR)
│   └── scripts/             # Build scripts
└── a-shell/                 # iOS app (Terminal UI)
```

---

## Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| **Thread-based processes** | iOS forbids fork(), simulate with threads |
| **Virtual PIDs** | Track "processes" via thread-local storage |
| **WAMR over wasm3** | Better WASI support, AOT compilation, smaller (~200KB) |
| **Python via BeeWare** | Native iOS Python, custom pip index for wheels |
| **XCFrameworks** | iOS standard, codesign required |
| **No dpkg** | apt handles everything directly |
| **osxcross + macOS** | Budget builds on Linux, release on macOS |

---

## Phase 1: Foundation (Week 1-2)

### 1.1 Archive Old Documentation
**Status**: ✅ Complete  
**Action**: Move old plan docs to `docs/archive/`

### 1.2 Remove Swift apt
**Status**: ✅ Complete  
**Action**: Delete Swift-based apt implementation
```
Deleted:
- ashell-core/Sources/Commands/apt.swift
- ashell-core/Sources/PackageManager/PackageManager.swift
- ashell-core/Sources/PackageManager/ArchiveExtractor.swift
```

### 1.3 Rename ios_system → a-shell-kernel
**Status**: ✅ Complete  
**Action**: Rename directory and core files
```
ios_system/ → a-shell-kernel/
ios_error.h → a_shell_error.h
ios_system.m → a_shell_kernel.m
ios_system/ → a_shell_kernel/
```

### 1.4 Create a_shell_system.h
**Status**: ✅ Complete  
**Location**: `a-shell-kernel/a_shell_system.h`

Provides compile-time macro redirects:
```c
#define fork ios_fork
#define execv ios_execv
#define waitpid ios_waitpid
// ... etc
```

Used via `-include` flag in package builds (not manual inclusion).

### 1.5 Integrate WAMR
**Status**: Migrated to monorepo  
**Note**: The repository structure has changed. WAMR integration is now handled within the `ixland-system` component as part of the monorepo. See root `README.md` for current setup instructions.

**Historical Note**: This section previously described adding WAMR as a submodule. Submodules are no longer used in this project.

---

## Phase 2: Core Systems (Week 3-4)

### 2.1 Extend Process Table
**Priority**: High  
**Target**: Robust virtual process management

```c
#define A_SHELL_MAX_PROCESSES 1024

typedef struct {
    pid_t pid;
    pid_t ppid;              // Parent tracking
    pthread_t thread;
    char name[64];
    int status;
    bool running;
    struct sigaction *sig_handlers[NSIG];
    char **environment;
} a_shell_process_t;
```

**Implement**:
- `ios_vfork()` - allocate PID without thread
- Parent/child tracking
- Ghost thread cleanup
- Signal emulation (setjmp/longjmp)

### 2.2 apt 3.1.16
**Priority**: High  
**Location**: `a-shell-packages/core-packages/apt/`

```bash
ASHELL_PKG_VERSION="3.1.16"
ASHELL_PKG_SRCURL="https://salsa.debian.org/apt-team/apt/-/archive/3.1.16/apt-3.1.16.tar.bz2"
```

**Build Integration**:
```bash
export CFLAGS="$CFLAGS -include $ASHELL_PREFIX/include/a_shell_system.h"
```

Patches use macros automatically - no code modifications needed.

### 2.3 Python-Apple-Support
**Priority**: High  
**Location**: `a-shell-packages/core-packages/python/`

Based on: https://github.com/beeware/Python-Apple-support

**Features**:
- Python 3.12 runtime
- Custom pip index: `pip.a-shell.dev`
- iOS wheel architecture tags:
  - `arm64-apple-ios`
  - `x86_64-apple-ios-simulator`

**User Experience**:
```bash
pip install numpy
# Searches pip.a-shell.dev for iOS-compatible wheels
```

---

## Phase 3: Essential Tools (Week 5-6)

### Native XCFrameworks (core-packages)
- [ ] apt 3.1.16
- [ ] python 3.12
- [ ] bash 5.x
- [ ] zsh 5.x
- [ ] curl
- [ ] libz, libssl, libcurl
- [ ] openssh
- [ ] git

### WASM Packages (packages/)
- [ ] vim
- [ ] nodejs
- [ ] emacs
- [ ] ripgrep
- [ ] fd

---

## Phase 4: TTY & Infrastructure (Week 7-8)

### 4.1 First-Class TTY Support
**File**: `a-shell-kernel/a_shell_tty.c`

```c
int a_shell_opentty(void);
int a_shell_set_raw_mode(int fd, bool raw);
int a_shell_set_window_size(int fd, struct winsize *ws);
int a_shell_set_echo(int fd, bool echo);
```

**Features**:
- PTY support (where available)
- Pipe simulation fallback
- Window resize handling
- Raw/cooked mode switching

### 4.2 Build Infrastructure

**Docker + osxcross** (Budget/Linux):
```bash
./build.sh --cross-compile libz
./build.sh --cross-compile libssl
```

**macOS + Xcode** (Primary/Release):
```bash
./build.sh bash
./build.sh zsh
```

---

## Phase 5: Documentation

### Essential Docs (Create First)
1. `docs/ARCHITECTURE.md` - System overview
2. `docs/AGENT_GUIDE.md` - AI assistant instructions
3. `docs/MANIFEST.md` - AI-readable project state
4. `docs/DEVELOPER_GUIDE.md` - Human developer onboarding

### Technical Docs
5. `docs/a-shell-kernel-process-model.md` - Process simulation
6. `docs/a-shell-packages-coding-guideline.md` - Build conventions
7. `docs/a-shell-common-porting-problems.md` - iOS-specific issues

---

## Build Variables Reference

| Variable | Description | Example |
|----------|-------------|---------|
| `ASHELL_PREFIX` | Install prefix | `/usr/local` |
| `ASHELL_PKG_NAME` | Package name | `bash` |
| `ASHELL_PKG_VERSION` | Package version | `5.2.15` |
| `ASHELL_PKG_SRCURL` | Source URL | `https://...` |
| `ASHELL_PKG_SHA256` | Source checksum | `abc123...` |

---

## Registry Structure

```
https://packages.a-shell.dev/
├── stable/
│   ├── core/              # Native XCFrameworks
│   │   ├── apt/
│   │   ├── python/
│   │   └── ...
│   └── wasm/              # WAMR packages
│       ├── vim/
│       ├── nodejs/
│       └── ...
├── community/             # User-contributed
└── pip/                   # Python wheel index
    ├── simple/
    │   ├── numpy/
    │   └── ...
```

---

## Success Criteria

| Phase | Deliverable | Status |
|-------|-------------|--------|
| 1.1 | Archive old docs | ✅ |
| 1.2 | Remove Swift apt | ✅ |
| 1.3 | Rename to a-shell-kernel | ✅ |
| 1.4 | Create a_shell_system.h | ✅ |
| 1.5 | Integrate WAMR | ⏳ |
| 2.1 | Process table | ⏳ |
| 2.2 | apt 3.1.16 | ⏳ |
| 2.3 | Python + custom pip | ⏳ |
| 3 | Essential tools | ⏳ |
| 4 | TTY support | ⏳ |
| 5 | Documentation | ⏳ |

---

## Working with This Plan

### For AI Agents
1. Check `bd ready` for available work
2. Claim issue: `bd update <id> --claim`
3. Update plan status as you complete tasks
4. Create discovered issues: `bd create "Found X" --deps discovered-from:<parent>`
5. Close completed: `bd close <id> --reason "Done"`

### For Developers
1. Read `docs/DEVELOPER_GUIDE.md` first
2. Check `docs/a-shell-common-porting-problems.md` for iOS quirks
3. Follow `docs/a-shell-packages-coding-guideline.md` for new packages

---

**Next Steps**: Execute Phase 1.5 (WAMR Integration) → Phase 2 (Core Systems)

# Current Architecture (Updated 2026-03-20)

## Overview

a-Shell Next is a three-layer system:

```
┌─────────────────────────────────────────────────────────────┐
│ Layer 3: a-shell (iOS App)                                    │
│ - Terminal UI (WebView)                                       │
│ - Bundle: bash, vim, git, python, coreutils                   │
│ - Session management (Swift)                                  │
│ - PackageManager (WASM downloads)                             │
└──────────────────────────┬────────────────────────────────────┘
                           │
┌──────────────────────────▼────────────────────────────────────┐
│ Layer 2: a-shell-packages (Build System)                      │
│ - Build recipes for core tools                              │
│ - pkg command (C binary) for WASM management                │
│ - GitHub releases for WASM packages                           │
└──────────────────────────┬────────────────────────────────────┘
                           │
┌──────────────────────────▼────────────────────────────────────┐
│ Layer 1: a-shell-kernel (Syscalls)                          │
│ - fork(), exec(), waitpid() simulation                      │
│ - Signal handling                                           │
│ - File I/O redirection                                      │
│ - Process table (virtual PIDs)                              │
└─────────────────────────────────────────────────────────────┘
```

## What Was Decided

### ✅ Included in App Bundle (Core Native Tools)

**Size**: ~30MB

| Tool | Version | Purpose |
|------|---------|---------|
| bash | 5.2.21 | Shell |
| coreutils | 9.5 | ls, cp, mv, cat, grep, etc. (50+ binaries) |
| python | 3.12 | Interpreter |
| git | latest | Version control |
| vim | latest | Editor |
| curl | 8.9.1 | HTTP client |
| ssh | latest | SSH client |
| tar, gzip | latest | Archivers |

**Libraries**:
- libz (compression)
- libssl (crypto)
- libcurl (HTTP)
- ncurses (terminal UI)
- readline (line editing)

### ✅ WASM Packages (User-installed)

**Size**: 500KB-5MB each

Downloaded via `pkg install <name>`:
- node (JavaScript runtime)
- ripgrep (fast grep)
- fd (fast find)
- Other tools

### ❌ Not Included (Changed from Original Plan)

**Removed a-shell-distro layer**:
- We decided the app itself handles session management
- No separate distro component needed
- Bootstrap is built-in, not extracted

**No apt/dpkg**:
- Using custom pkg command instead
- GitHub releases for WASM packages
- Simpler, iOS-friendly

## Directory Structure

```
a-shell-next/
├── a-shell/                      # Layer 3: iOS App
│   ├── a-Shell.xcodeproj/
│   ├── a-Shell/
│   │   ├── ContentViewTerm.swift    # Terminal UI
│   │   ├── SessionManager.swift     # Session management
│   │   └── Package/
│   │       ├── PackageManager.swift # WASM downloads
│   │       └── WasmRuntime.swift    # WAMR integration
│   └── AGENTS.md                  # Agent instructions
│
├── a-shell-packages/             # Layer 2: Build System
│   ├── packages/
│   │   ├── core/                   # Core tools recipes
│   │   │   ├── libz/
│   │   │   ├── libssl/
│   │   │   ├── bash/
│   │   │   └── ...
│   │   └── wasm/                   # WASM recipes
│   │
│   ├── scripts/
│   │   ├── a_shell_package.sh      # Build library
│   │   └── build-core.sh           # Build all core
│   │
│   └── pkg/                        # Package manager
│       └── pkg.c                   # C binary
│
├── a-shell-kernel/               # Layer 1: Syscalls
│   ├── include/
│   │   ├── a_shell_kernel.h        # Main header
│   │   └── linux/                  # Linux headers
│   │
│   ├── src/
│   │   └── syscalls/               # Syscall implementations
│   │
│   ├── Makefile                    # Build kernel
│   └── AGENTS.md                   # Agent instructions
│
└── docs/
    ├── ARCHITECTURE_CURRENT.md     # This file
    └── AGENT_GUIDE.md              # Cross-layer guide
```

## Key Design Decisions

### 1. Built-in vs Downloaded

**Built-in (Native)**:
- Must work offline
- Critical tools (bash, coreutils)
- Updated via App Store
- Size: ~30MB

**Downloaded (WASM)**:
- Optional extras
- User decides what to install
- GitHub releases
- Size: ~500KB-5MB per package

### 2. No Extraction

**Original plan**: Extract bootstrap.tar.gz on first launch
**Current plan**: Everything pre-built in app bundle

**Why**:
- Faster first launch
- Simpler code (no extraction logic)
- All code signed at build time
- App Store compliant

### 3. Package Manager

**Language**: C (for pkg binary)
**Communication**: IPC to Swift PackageManager
**Format**: .pkg.tar.xz with WASM
**Repository**: GitHub releases

### 4. Kernel Integration

**All tools use kernel**:
- Built-in tools link against kernel
- WASM uses WASI which uses kernel
- Unified syscall layer

## Agent Workflows

### Working on Kernel (a-shell-kernel/)

1. Modify syscalls in `src/syscalls/`
2. Update headers in `include/linux/`
3. Test: `make test-compile`
4. Build: `make clean && make`
5. Verify: `ls build/a-shell-kernel.xcframework/`
6. Commit: "feat(kernel): <description>"

### Working on Packages (a-shell-packages/)

1. Create build recipe in `packages/core/<name>/build.sh`
2. Test build: `cd packages/core/<name> && ./build.sh`
3. Verify output in `.build/`
4. Commit: "feat(packages): add <name> <version>"

### Working on App (a-shell/)

1. Edit Swift code in `a-Shell/`
2. Build in Xcode
3. Test on device
4. Verify core tools work
5. Test pkg command
6. Commit: "feat(app): <description>"

## Integration Points

### Kernel → App

```
a-shell-kernel/
├── build/a-shell-kernel.xcframework
│   └── linked by a-Shell.xcodeproj
└── headers included by packages
```

### Packages → App

```
a-shell-packages/
├── packages/core/*/build.sh
│   └── builds binaries
├── .build/bin/*
│   └── copied to app bundle
└── .build/lib/*
    └── linked by app
```

### App → Packages

```
a-shell/
├── bin/pkg (from a-shell-packages)
│   └── manages WASM downloads
└── PackageManager.swift
    └── handles IPC from pkg
```

## Build Order

**Dependencies**:

```
libz ─┬─→ libssl ─┐
      ├─→ ncurses ─┼─→ bash
      │             │
      ├─→ readline ─┘
      │
      ├─→ libcurl ──→ git
      │
      └─→ coreutils

All link against:
a-shell-kernel (syscalls)
```

**Build phases**:

1. Build kernel XCFramework
2. Build libraries (libz, libssl, etc.)
3. Build core tools (bash, coreutils, etc.)
4. Copy to app bundle
5. Compile Swift app
6. Link everything
7. Sign and create IPA

## Constraints

- **iOS 16.0+** minimum
- **arm64** only
- **No JIT** (WebAssembly interpreted)
- **Sandbox**: ~/Documents/, ~/Library/
- **Signed code**: All native code at build time
- **Size**: App < 200MB, core ~30MB

## Success Criteria

**MVP**:
- [ ] All core tools built and working
- [ ] Terminal starts bash
- [ ] Basic commands work (ls, cd, pwd)
- [ ] pkg command downloads WASM
- [ ] IPA generates successfully
- [ ] App installs on device

**Full**:
- [ ] Python works
- [ ] Git works
- [ ] Vim works
- [ ] WASM packages run (node, ripgrep)
- [ ] App Store ready

## Next Steps

1. **Phase 1**: Build core packages
   - libz, libssl, ncurses, readline
   - libcurl, bash, coreutils
   - python, git, vim

2. **Phase 2**: Integrate with app
   - Update Xcode build phases
   - Copy binaries to bundle
   - Test core tools

3. **Phase 3**: Package manager
   - Create pkg binary
   - Create PackageManager.swift
   - Test WASM downloads

4. **Phase 4**: Generate IPA
   - Build release configuration
   - Sign with distribution cert
   - Test on device

5. **Phase 5**: App Store
   - Prepare screenshots
   - Write description
   - Submit for review

---

**Last Updated**: 2026-03-20
**Status**: Architecture defined, ready for implementation
**Key Changes**: Removed a-shell-distro, no bootstrap extraction, everything built-in

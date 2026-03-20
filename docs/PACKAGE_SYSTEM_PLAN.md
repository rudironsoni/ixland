# Package System Implementation Plan

**Date:** 2026-03-20  
**Status:** Approved - Ready for Implementation  
**Based on:** Termux architecture + a-shell kernel

---

## Executive Summary

Three-tier package system for a-Shell:
1. **XCFrameworks** (Built-in, ~30MB) - Core commands shipped with app
2. **.deb Packages** (User-installed) - APT-managed extensible packages
3. **WebAssembly** (Sandboxed) - Security-focused user code

Bootstrap: 50MB initial environment with bash, coreutils, apt, dpkg.

---

## Architecture

### Directory Structure

```
a-shell-next/
├── a-shell-kernel/              # Syscall kernel (separate plan)
│   ├── include/linux/           # Syscall headers
│   ├── include/sys/
│   ├── src/                     # Implementations
│   └── ...
│
├── a-shell-packages/           # NEW: Package build system
│   ├── scripts/
│   │   ├── a_shell_package.sh  # Main build library
│   │   ├── setup-toolchain.sh
│   │   ├── build-package.sh
│   │   └── build-all.sh
│   │
│   ├── packages/
│   │   ├── core/                # Bootstrap (20 packages)
│   │   │   ├── bash/
│   │   │   ├── coreutils/
│   │   │   ├── apt/
│   │   │   ├── dpkg/
│   │   │   ├── libz/
│   │   │   ├── libssl/
│   │   │   └── ...
│   │   │
│   │   ├── extra/               # Essential (100 packages)
│   │   └── extended/            # Everything else (1000+)
│   │
│   ├── repos/                   # Repository hosting
│   └── docker/
│
└── a-shell/                     # iOS app
    └── Resources/
        └── bootstrap.tar.gz     # ~50MB
```

---

## Three-Tier System

### Tier 1: Built-in XCFrameworks (Shipped)

```swift
// Loaded at app startup
commands = [
    "ls": "files.framework",
    "cp": "files.framework", 
    "cat": "text.framework",
    "curl": "curl_ios.framework",
    "ssh": "ssh_cmd.framework",
    // ~50 commands, ~30MB
]
```

- **Performance:** Native (1x)
- **Update:** With app updates
- **Size:** ~30MB

### Tier 2: User-Installed .deb Packages

```bash
# User installs:
$ apt install vim git python3
# Downloads → ~/Library/ashell/usr/
```

- **Performance:** Native (1x)
- **Update:** Via apt
- **Size:** Variable

### Tier 3: WebAssembly (Optional)

```bash
$ wasm ffmpeg.wasm
```

- **Performance:** 2-5x slower
- **Use case:** Security, user-compiled
- **Size:** Small

---

## Bootstrap (~50MB)

### Included in App Bundle

```
bootstrap.tar.gz
├── usr/
│   ├── bin/
│   │   ├── bash              # Shell
│   │   ├── apt               # Package manager
│   │   ├── dpkg              # Package backend
│   │   ├── ls, cp, mv        # Coreutils
│   │   ├── cat, grep, sed    # Text processing
│   │   └── [~30 commands]
│   │
│   ├── lib/
│   │   ├── libz.dylib
│   │   ├── libssl.dylib
│   │   └── [core libraries]
│   │
│   └── etc/apt/sources.list
│
└── var/cache/apt/
```

---

## Build Priorities

### Phase 1: Toolchain (Week 1)
- Docker environment
- Xcode toolchain
- Build scripts

### Phase 2: Core Libraries (Weeks 2-3)
1. libz
2. libssl  
3. libcurl
4. ncurses
5. readline

### Phase 3: Core Tools (Weeks 4-5)
1. bash
2. coreutils
3. tar, gzip
4. grep, sed, awk

### Phase 4: Package Management (Weeks 6-7)
1. dpkg
2. apt
3. Bootstrap creation

### Phase 5: Bootstrap Integration (Week 8)
- 50MB bootstrap.tar.gz
- First-launch extraction
- UI integration

### Phase 6: Essential Packages (Weeks 9-12)
- vim
- git
- python3
- nodejs

**Total to MVP: 12 weeks**

---

## Technical Specifications

### Supported Architectures
- iOS arm64 (device)
- iOS Simulator arm64

### Package Format
- **Format:** .deb (Debian)
- **Manager:** APT
- **Backend:** dpkg

### Environment Variables
```bash
HOME=~/Documents
A_SHELL_PREFIX=~/Library/ashell
PATH=$A_SHELL_PREFIX/usr/bin:[built-in]:/usr/bin:/bin
LD_LIBRARY_PATH=$A_SHELL_PREFIX/usr/lib
CPPFLAGS=-include $A_SHELL_PREFIX/include/linux/unistd.h
```

### Repository Structure
```
packages.a-shell.dev/
├── dists/stable/main/binary-ios-arm64/Packages.gz
├── pool/main/
│   ├── b/bash/bash_5.2.21_ios-arm64.deb
│   └── v/vim/vim_8.2.0_ios-arm64.deb
└── bootstrap/bootstrap-ios-arm64.tar.gz
```

---

## Implementation Notes

### Build Script Template

```bash
#!/bin/bash
# packages/core/bash/build.sh

A_SHELL_PKG_NAME="bash"
A_SHELL_PKG_VERSION="5.2.21"
A_SHELL_PKG_SRCURL="https://..."
A_SHELL_PKG_DEPENDS="libncurses"

a_shell_step_configure() {
    export CC="xcrun -sdk iphoneos clang -arch arm64"
    export CPPFLAGS="-include $A_SHELL_PREFIX/include/linux/unistd.h"
    
    ./configure \
        --prefix=$A_SHELL_PREFIX \
        --host=arm-apple-darwin
}

a_shell_step_make() {
    make -j$(sysctl -n hw.ncpu)
}

a_shell_step_package() {
    make DESTDIR=$A_SHELL_PKG_STAGING install
    dpkg-deb --build $A_SHELL_PKG_STAGING
}
```

### Minimal Patches

Only patch when necessary:
1. **Path substitution:** `/etc` → `A_SHELL_PREFIX/etc`
2. **iOS-specific:** Rare platform guards
3. **NO syscall patches** - kernel handles via macros

---

## Decisions

| Aspect | Decision | Rationale |
|--------|----------|-----------|
| Binary format | Both (XCFramework + .deb) | XCFrameworks for speed, .deb for extensibility |
| Bootstrap size | ~50MB | Balance between size and functionality |
| WASM | Separate | Keep for security/sandboxing |
| Architectures | iOS arm64 + Simulator arm64 | Real devices + testing |
| Build priority | Core packages first | Essential tools before extras |
| Package manager | APT + dpkg | Proven, familiar |
| Format | .deb | Standard, well-documented |

---

## Next Steps

1. Create directory structure
2. Implement `a_shell_package.sh` build library
3. Set up Docker build environment
4. Build core libraries (libz, libssl)
5. Build core tools (bash, coreutils)
6. Build package management (apt, dpkg)
7. Create bootstrap package
8. Test first-launch extraction
9. Build essential packages (vim, git, python)
10. Set up repository hosting

---

## References

- Termux packages: https://github.com/termux/termux-packages
- Debian APT: https://wiki.debian.org/Apt
- iOS XCFrameworks: Apple Developer Documentation
- This plan integrates with kernel plan in docs/MASTER_PLAN.md

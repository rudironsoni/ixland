# Agent Guide: a-shell-packages

## Overview

**a-shell-packages** is the package build system. It compiles open-source Linux tools (bash, vim, git, python, etc.) for iOS and creates WebAssembly packages for user installation.

## Responsibilities

### What This Layer DOES

1. **Build Core Tools**
   - Compile bash, coreutils, vim, git, python
   - Target: iOS arm64, static/dynamic libraries
   - Output: Binaries and libraries for app bundle

2. **Build System Libraries**
   - libz (compression)
   - libssl (crypto)
   - libcurl (HTTP client)
   - ncurses (terminal UI)
   - readline (line editing)

3. **Create WebAssembly Packages**
   - Compile tools to WASM (node, ripgrep, fd, etc.)
   - Host on GitHub releases
   - Managed by pkg command

4. **Package Management**
   - Download packages from GitHub releases
   - Install/remove user packages
   - Track dependencies
   - Verify checksums

### What This Layer DOES NOT DO

- ❌ NOT a syscall layer (that's a-shell-kernel/)
- ❌ NOT a terminal emulator (that's a-shell/)
- ❌ NOT the iOS app (that's a-shell/)
- ❌ NOT execute code (just builds it)

## Architecture

```
a-shell-packages/
├── packages/core/          # Core tools (in app bundle)
│   ├── libz/
│   ├── libssl/
│   ├── ncurses/
│   ├── readline/
│   ├── libcurl/
│   ├── bash/
│   ├── coreutils/
│   ├── python/
│   ├── git/
│   └── vim/
│
├── packages/wasm/          # WASM packages (user-installed)
│   ├── node/
│   ├── ripgrep/
│   └── ...
│
├── scripts/                # Build scripts
│   ├── a_shell_package.sh  # Build library
│   └── build-core.sh       # Build all core packages
│
└── pkg/                    # Package manager
    └── pkg.c               # Compiled to binary
```

## Two-Tier System

### Tier 1: Core Native Tools (Built-in)

**Location**: `packages/core/`

**Included in app bundle**:
- libz, libssl, ncurses, readline (libraries)
- bash (shell)
- coreutils (ls, cp, mv, cat, grep, etc.)
- python (interpreter)
- git (version control)
- vim (editor)
- curl, ssh (network tools)

**Size**: ~25-30MB

**Update**: Via App Store only

### Tier 2: WASM Packages (User-installed)

**Location**: `packages/wasm/`

**Downloaded by user**:
- node (JavaScript runtime)
- ripgrep (fast grep)
- fd (fast find)
- Other tools

**Format**: `.wasm` files

**Size**: ~500KB-5MB each

**Update**: Via `pkg install` command

## Package Format

### Native Package (Core)

**Output**: Binaries + libraries

**Layout**:
```
build/
├── bin/
│   ├── bash
│   ├── ls
│   └── ...
├── lib/
│   ├── libz.dylib
│   ├── libssl.dylib
│   └── ...
└── share/
    └── ...
```

### WASM Package (User)

**File**: `.pkg.tar.xz`

**Contents**:
```
<name>-<version>.pkg.tar.xz
├── PKG-INFO          # JSON metadata
├── bin/
│   └── <name>.wasm   # WebAssembly binary
└── share/
    └── ...           # Data files
```

**PKG-INFO format**:
```json
{
  "name": "node",
  "version": "20.0.0",
  "architecture": "wasm32",
  "dependencies": {},
  "files": ["bin/node.wasm"],
  "checksum": "sha256:..."
}
```

## Build System

### Prerequisites

```bash
# macOS with Xcode
xcode-select --install

# Verify
xcrun --show-sdk-path
```

### Build Targets

The build system supports three targets:

| Target | SDK | Use Case | Output |
|--------|-----|----------|--------|
| `ios` | iphoneos | App Store submission | `.build/ios/` |
| `simulator` | iphonesimulator | Testing on Simulator | `.build/simulator/` |
| `universal` | Both combined | Development (default) | `.build/universal/` |
| `both` | ios + simulator | Build both separately | `.build/ios/` + `.build/simulator/` |

### Building Core Packages

```bash
cd a-shell-packages/

# Build all core packages (default: universal)
./scripts/build-all.sh

# Build for specific target
./scripts/build-all.sh --target ios
./scripts/build-all.sh --target simulator
./scripts/build-all.sh --target universal
./scripts/build-all.sh --target both

# Build individual package
cd packages/core/bash
./build.sh

# Output goes to .build/<target>/
ls .build/universal/staging/usr/local/
ls .build/ios/staging/usr/local/
ls .build/simulator/staging/usr/local/
```

### Build Order (Dependencies)

**Wave 1**: libz
**Wave 2**: libssl (needs libz)
**Wave 3**: ncurses, readline (parallel, needs libz)
**Wave 4**: libcurl, bash, coreutils (parallel)
**Wave 5**: python, git, vim (parallel)

### Build Script Template

```bash
#!/bin/bash
# packages/core/<name>/build.sh

A_SHELL_PKG_NAME="<name>"
A_SHELL_PKG_VERSION="<version>"
A_SHELL_PKG_SRCURL="<url>"
A_SHELL_PKG_SHA256="<checksum>"
A_SHELL_PKG_DEPENDS="<space-separated deps>"

a_shell_pkg_configure() {
    # Target is automatically set via A_SHELL_BUILD_TARGET
    # SDK and flags are configured by the build system
    
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        || a_shell_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || a_shell_error "Build failed"
}

a_shell_pkg_install() {
    make DESTDIR="$A_SHELL_PKG_STAGING" install \
        || a_shell_error "Install failed"
}
```

### Universal Binary Creation

For `universal` target, the build system:
1. Builds for iOS device
2. Builds for iOS Simulator
3. Combines with `lipo -create` into fat binary

```bash
# The build system handles this automatically
./scripts/build-package.sh libz --target universal

# Verify universal binary
lipo -info .build/universal/staging/usr/local/lib/libz.a
# Output: Architectures in the fat file: libz.a are: arm64 (for device), arm64 (for simulator)
```

## Package Manager

### pkg Command

**Location**: `pkg/pkg.c` → compiled to `bin/pkg`

**Usage**:
```bash
pkg search <name>        # Search repository
pkg install <name>       # Download & install WASM package
pkg remove <name>        # Uninstall package
pkg list                 # Show installed
pkg upgrade              # Update all packages
```

### Implementation

**Language**: C (small, fast)

**Communication**: IPC to Swift PackageManager in app

**Process**:
1. User types `pkg install node`
2. Shell forks, execs `bin/pkg`
3. pkg sends message to app via XPC
4. App downloads node.wasm from GitHub
5. Extracts to ~/Documents/.a-shell/wasm/bin/
6. Records metadata in ~/Documents/.a-shell/pkg/installed/

## Repository

**GitHub Releases**:
```
https://github.com/rudironsoni/a-shell-packages/releases
├── index.json              # Package catalog
└── v1.0.0/
    ├── node-20.0.0.wasm
    └── ...
```

**index.json**:
```json
{
  "schema_version": "1.0",
  "packages": {
    "node": {
      "latest": "20.0.0",
      "versions": {
        "20.0.0": {
          "url": "https://github.com/.../node-20.0.0.wasm",
          "size": 5242880,
          "checksum": "sha256:..."
        }
      }
    }
  }
}
```

## Agent Instructions

### When Building Packages

1. **Always use cross-compilation**
   ```bash
   export CC="xcrun -sdk iphoneos clang -arch arm64"
   ```

2. **Include kernel headers**
   ```bash
   export CPPFLAGS="-I../../a-shell-kernel/include"
   ```

3. **Static libraries preferred**
   ```bash
   ./configure --disable-shared --enable-static
   ```

4. **Test compilation**
   ```bash
   make -j$(sysctl -n hw.ncpu)
   ```

5. **Test on iOS Simulator**
   ```bash
   # Build with universal target (includes simulator)
   ./scripts/build-package.sh libz --target universal
   
   # Run tests
   ./ios-test-framework/run-tests.sh libz
   ```

### Common Tasks

**Creating a new package:**
1. Create `packages/core/<name>/` directory
2. Write `build.sh` script
3. Download source, verify checksum
4. Add dependencies to A_SHELL_PKG_DEPENDS
5. Configure with iOS flags
6. Build: `./scripts/build-package.sh <name> --target universal`
7. Test: `./ios-test-framework/run-tests.sh <name>`
8. Commit with message: "feat(packages): add <name> <version>"

**Building a package:**
```bash
cd a-shell-packages/

# Build for development (universal - works on device + simulator)
./scripts/build-package.sh libz
./scripts/build-package.sh libz --target universal

# Build for production (device only, smaller)
./scripts/build-package.sh libz --target ios

# Build for testing (simulator only, faster)
./scripts/build-package.sh libz --target simulator

# Build both ios + simulator separately
./scripts/build-package.sh libz --target both

# Check output
ls .build/universal/staging/usr/local/lib/
ls .build/ios/staging/usr/local/lib/
ls .build/simulator/staging/usr/local/lib/
```

**Testing packages:**
```bash
# Auto-detect and run tests (prefers universal build)
./ios-test-framework/run-tests.sh libz

# Build test app only
./ios-test-framework/build-test.sh libz
```

**Troubleshooting:**
- Check configure.log for errors
- Verify dependencies built first
- Ensure iOS SDK path correct: `xcrun --show-sdk-path`
- Check architecture: `lipo -info .build/universal/staging/usr/local/lib/lib*.a`
- Check for hardcoded /etc or /usr paths
- Missing simulator build? Use `--target universal` or `--target simulator`
- Tests failing to link? Ensure library supports simulator architecture

## Integration Points

### a-shell/ (App)
- Uses built binaries from core packages
- Packages linked during Xcode build
- pkg binary manages WASM downloads

### a-shell-kernel/ (Kernel)
- Provides syscall headers
- Packages link against kernel syscalls
- No kernel modifications needed for packages

## Constraints

- **iOS 16.0+** minimum deployment target
- **arm64** only
- **No fork/exec** - use threads
- **Sandbox paths only** - ~/Documents/, ~/Library/
- **Static libraries preferred** - easier distribution
- **GitHub releases** - for WASM distribution

## Security

- Verify all downloads with SHA256
- Check package signatures (if implemented)
- Sandboxed WASM execution
- No native code execution from user space

## Documentation

- **Build scripts**: `packages/core/*/build.sh`
- **Build library**: `scripts/a_shell_package.sh`
- **Package builder**: `scripts/build-package.sh --target ios|simulator|universal|both`
- **Package manager**: `pkg/pkg.c`
- **Test framework**: `ios-test-framework/README.md`
- **This guide**: `AGENTS.md`

### Testing

**iOS Simulator Testing:**
```bash
# Build universal (includes simulator support)
./scripts/build-package.sh libz --target universal

# Run tests on iOS Simulator
./ios-test-framework/run-tests.sh libz
```

**Test Framework Features:**
- Auto-detects universal or simulator builds
- Builds test app for iOS Simulator
- Runs tests and reports pass/fail
- Works with all three build targets

---

**Last Updated**: 2026-03-21
**Status**: Multi-target build system complete (ios, simulator, universal)
**Next Steps**: Build and validate all packages with iOS Simulator testing

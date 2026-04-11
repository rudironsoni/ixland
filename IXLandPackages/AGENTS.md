# Agent Guide: IXLandPackages

## Overview

**IXLandPackages** is the iOS package build system. It compiles open-source Linux tools (bash, vim, git, python, etc.) for iOS native execution using the IXLandSystem syscall layer.

## Key Principle: No Debian Infrastructure

**This is NOT a Debian system.** There is no:
- ❌ dpkg
- ❌ apt
- ❌ .deb packages
- ❌ DEBIAN/ directories
- ❌ Debian package management

**Instead:** We build native iOS binaries that link against IXLandSystem, producing static libraries and executables for direct inclusion in the iOS app bundle.

## Quick Start

```bash
# Build the reference libz package
cd IXLandPackages
./scripts/build-package.sh libz

# List available packages
ls packages/core/

# Clean build artifacts
./scripts/build-package.sh libz --clean

# Build all packages
./scripts/build-all.sh --target universal
```

## Responsibilities

### What This Layer DOES

1. **Build Core iOS Tools**
   - Compile bash, coreutils, vim, git, python for iOS arm64
   - Target: iOS device and Simulator (universal binaries)
   - Output: Static libraries and binaries in staging directories
   - Link against IXLandSystem XCFramework

2. **Build System Libraries**
   - libz (compression)
   - libssl (crypto)
   - libcurl (HTTP client)
   - ncurses (terminal UI)
   - readline (line editing)

3. **Create WebAssembly Packages** (separate system)
   - WASM packages use different build process
   - See `packages/wasm/` for WASM-specific builds

### What This Layer DOES NOT DO

- ❌ NOT create .deb packages (we're iOS, not Debian)
- ❌ NOT use dpkg or apt (removed from all packages)
- ❌ NOT a syscall layer (that's IXLandSystem/)
- ❌ NOT a terminal emulator (that's IXLand/)
- ❌ NOT the iOS app (that's IXLand/)

## Architecture

```
IXLandPackages/
├── packages/core/          # Core iOS tools (in app bundle)
│   ├── libz/              # Compression library
│   ├── libssl/            # Cryptography
│   ├── ncurses/           # Terminal handling
│   ├── readline/          # Line editing
│   ├── libcurl/           # HTTP client
│   ├── bash/              # Shell
│   ├── coreutils/         # ls, cp, mv, cat, etc.
│   ├── python/            # Python interpreter
│   ├── git/               # Version control
│   └── vim/               # Editor
│
├── packages/wasm/          # WASM packages (separate build system)
│   └── ...                # See WASM documentation
│
├── scripts/                # Build scripts
│   ├── ixland_package.sh  # Main build library
│   ├── build-package.sh   # Single package builder
│   └── build-all.sh       # Build all packages
│
└── ios-test-framework/     # iOS Simulator testing
    ├── run-tests.sh
    └── build-test.sh
```

## Build System

### Output Format

**Native packages produce:**
```
.build/<target>/staging/usr/local/
├── bin/
│   ├── bash              # iOS executable
│   ├── ls
│   └── ...
├── lib/
│   ├── libz.a            # Static library
│   ├── libssl.a
│   └── ...
├── include/
│   ├── zlib.h
│   └── ...
└── share/
    └── terminfo/         # Data files
```

**No .deb, no dpkg, no Debian.** Just raw binaries in staging directories.

### Build Targets

| Target | SDK | Output | Use Case |
|--------|-----|--------|----------|
| `ios` | iphoneos | `.build/ios/` | App Store submission |
| `simulator` | iphonesimulator | `.build/simulator/` | Testing |
| `universal` | Both | `.build/universal/` | Development (default) |
| `both` | Separate | `.build/ios/` + `.build/simulator/` | CI/CD |

### Prerequisites

```bash
# macOS with Xcode (only requirement)
xcode-select --install

# Verify
xcrun --show-sdk-path
```

**No dpkg-dev, no apt, no Debian tools required.**

### Building Packages

```bash
cd IXLandPackages/

# Build single package (universal is default)
./scripts/build-package.sh libz

# Build for specific targets
./scripts/build-package.sh libz --target ios
./scripts/build-package.sh libz --target simulator
./scripts/build-package.sh libz --target universal
./scripts/build-package.sh libz --target both

# Build all packages
./scripts/build-all.sh --target universal

# Output is in staging directories
ls .build/universal/staging/usr/local/bin/
ls .build/universal/staging/usr/local/lib/
```

## Package Patching Guide

### Why Patches Are Needed

Linux packages assume:
- Standard Linux syscalls (fork, exec, wait) - **Handled by kernel**
- Standard paths (/etc, /usr, /var) - **Need patches**
- Standard functions (getentropy) - **Need patches for missing iOS functions**

### What Kernel Handles (NO PATCHES NEEDED)

**The IXLandSystem provides syscall wrappers via compile-time macros:**

```c
// In your code:
pid_t pid = fork();

// Kernel header automatically converts to:
pid_t pid = ixland_fork();
```

**No patches needed for:**
- fork, exec, wait, waitpid
- open, read, write, close
- signal handling
- File operations
- Standard POSIX functions

**Just include kernel headers:**
```bash
export CPPFLAGS="-I../../IXLandSystem/include"
```

### What Needs Patches

**Three categories require patches:**

#### 1. Missing iOS Functions
Functions iOS doesn't provide, need alternatives:

| Function | iOS Replacement | Package Example |
|----------|----------------|-----------------|
| `getentropy()` | `arc4random()` | bash-minimal |
| `gethostbyname_r()` | `getaddrinfo()` | libcurl (if used) |
| `clock_settime()` | Not available | coreutils (configure disables) |

**Patch pattern:**
```c
#if defined(__APPLE__) && defined(__MACH__)
  /* iOS: use arc4random instead of getentropy */
  ret = arc4random();
#else
  ret = getentropy(&buf, sizeof(buf));
#endif
```

#### 2. Hardcoded Paths
Unix paths that need sandbox locations:

| Hardcoded Path | iOS Location | Patch Type |
|----------------|--------------|------------|
| `/etc/` | `@IXLAND_PREFIX@/etc/` | configure-time substitution |
| `/tmp/` | `$TMPDIR` or app temp | Environment variable |
| `/var/` | `~/Library/Caches/` | Runtime detection |
| `/usr/share/` | `@IXLAND_PREFIX@/share/` | configure-time substitution |

**Patch pattern (configure.ac):**
```autoconf
- sysconfdir=/etc
+ sysconfdir=@IXLAND_PREFIX@/etc
```

#### 3. Platform Detection
Code that checks `__linux__` but not `__APPLE__`:

**Patch pattern:**
```c
- #ifdef __linux__
+ #if defined(__linux__) || defined(__APPLE__)
    /* Linux-specific code */
  #endif
```

### Patch Discovery Process

When adding a new package, follow this workflow:

**Step 1: Create initial build.sh**
```bash
IXLAND_PKG_NAME="mypackage"
IXLAND_PKG_VERSION="1.0.0"
IXLAND_PKG_SRCURL="https://..."
IXLAND_PKG_DEPENDS="libz"

ixland_pkg_configure() {
    ./configure \
        --prefix="$IXLAND_PREFIX" \
        --host="arm-apple-darwin" \
        || ixland_error "Configure failed"
}

ixland_pkg_make() {
    make -j$(sysctl -n hw.ncpu)
}

ixland_pkg_install() {
    make DESTDIR="$IXLAND_PKG_STAGING" install
}
```

**Step 2: Attempt build**
```bash
./scripts/build-package.sh mypackage --target universal 2>&1 | tee build.log
```

**Step 3: Analyze errors**

**Error types and solutions:**

| Error | Category | Solution |
|-------|----------|----------|
| `implicit declaration of function 'getentropy'` | Missing Function | Patch to use arc4random() |
| `error: '/etc' is not writable` | Hardcoded Path | Patch path to use @IXLAND_PREFIX@ |
| `undefined reference to fork` | Include Missing | Check CPPFLAGS includes kernel headers |
| `__linux__ not defined` | Platform Detection | Add __APPLE__ to #ifdef |

**Step 4: Create patch**

See `PATCH_TEMPLATE.md` for patch format and examples.

**Step 5: Test**
```bash
# Clean and rebuild
rm -rf .build/
./scripts/build-package.sh mypackage --target universal

# Run tests if available
./ios-test-framework/run-tests.sh mypackage
```

### Patch Organization

```
packages/core/<package>/
├── build.sh
├── patches/
│   ├── 01-ios-functions.patch    # Missing function replacements
│   ├── 02-ios-paths.patch        # Path substitutions
│   ├── 03-ios-platform.patch     # Platform detection
│   └── README.md                 # Patch documentation
```

**Naming convention:**
- `XX-ios-<category>.patch` - XX is order (01, 02, 03...)
- Apply in numerical order
- Categories: functions, paths, platform, config

### Removing Debian Dependencies

**If a package has Debian dependencies (dpkg, apt, deb), you MUST:**

1. **Remove configure options:**
   ```bash
   # BAD - Debian-specific
   ./configure --with-dpkg ...

   # GOOD - iOS-compatible
   ./configure --without-dpkg --without-apt ...
   ```

2. **Create patches to remove Debian code:**
   - Comment out dpkg feature checks
   - Remove apt dependency initialization
   - Replace deb-specific paths with @IXLAND_PREFIX@

3. **Use cache variables to skip detection:**
   ```bash
   export ac_cv_path_DPKG=/bin/false
   export ac_cv_path_APTGET=/bin/false
   export ac_cv_func_dpkg_parse=no
   ```

**Example - Removing dpkg from a package:**

```patch
--- a/configure.ac
+++ b/configure.ac
@@ -50,7 +50,10 @@
 AC_CHECK_FUNCS([getentropy])

 # Check for dpkg (DEBIAN-SPECIFIC - REMOVE FOR iOS)
-AC_PATH_PROG([DPKG], [dpkg], [no])
+# iOS: No dpkg, skip this check
+# AC_PATH_PROG([DPKG], [dpkg], [no])
+DPKG=no
+
 if test "x$DPKG" != "xno"; then
   AC_DEFINE([HAVE_DPKG], [1], [Define if dpkg is available])
 fi
```

### Build Order (Dependencies)

**Wave 1 - Foundation (no deps):**
- libz

**Wave 2 - Crypto (needs libz):**
- libssl

**Wave 3 - Terminal (needs libz):**
- ncurses
- readline

**Wave 4 - Tools (parallel):**
- libcurl
- bash
- coreutils

**Wave 5 - High-level (parallel):**
- python
- git
- vim

### Build Script Template

```bash
#!/bin/bash
# packages/core/<name>/build.sh

IXLAND_PKG_NAME="<name>"
IXLAND_PKG_VERSION="<version>"
IXLAND_PKG_SRCURL="<url>"
IXLAND_PKG_SHA256="<checksum>"
IXLAND_PKG_DEPENDS="libz"

ixland_pkg_configure() {
    # iOS-specific cache variables
    export ac_cv_func_<missing_func>=no

    # Remove any Debian dependencies
    export ac_cv_path_DPKG=/bin/false
    export ac_cv_path_APTGET=/bin/false

    ./configure \
        --prefix="$IXLAND_PREFIX" \
        --host="arm-apple-darwin" \
        --disable-shared \
        --enable-static \
        --without-dpkg \
        --without-apt \
        || ixland_error "Configure failed"
}

ixland_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || ixland_error "Build failed"
}

ixland_pkg_install() {
    make DESTDIR="$IXLAND_PKG_STAGING" install \
        || ixland_error "Install failed"
}
```

## Testing

### iOS Simulator Testing

```bash
# Build universal (includes simulator)
./scripts/build-package.sh libz --target universal

# Run tests on iOS Simulator
./ios-test-framework/run-tests.sh libz
```

### Manual Testing

```bash
# Build for simulator
./scripts/build-package.sh libz --target simulator

# Check architecture
lipo -info .build/simulator/staging/usr/local/lib/libz.a
# Expected: arm64 (for simulator)

# Check it links
file .build/simulator/staging/usr/local/lib/libz.a
# Expected: archive with arm64 objects
```

## Common Tasks

**Creating a new package:**
1. Create `packages/core/<name>/` directory
2. Write `build.sh` following template
3. **Remove any Debian dependencies**
4. Download source, verify checksum
5. Add dependencies to IXLAND_PKG_DEPENDS
6. Attempt build, identify errors
7. Create patches in `patches/` directory
8. Test with `./scripts/build-package.sh <name> --target universal`
9. Run tests: `./ios-test-framework/run-tests.sh <name>`
10. Commit with message: "feat(packages): add <name> <version>"

**Adding patches to existing package:**
1. Identify the error (function missing, path issue, etc.)
2. See `PATCH_TEMPLATE.md` for patch format
3. Create patch: `packages/core/<name>/patches/XX-ios-<category>.patch`
4. Document in patches/README.md
5. Test rebuild

**Removing Debian code from package:**
1. Search for dpkg, apt, deb references in source
2. Create patch to disable/remove Debian-specific code
3. Use cache variables to skip detection
4. Test that package builds without Debian tools

## Integration Points

### IXLandSystem/ (Kernel)
- Provides syscall headers with automatic redirection
- Include with: `CPPFLAGS="-I../../IXLandSystem/include"`
- No patches needed for standard syscalls
- Kernel macros handle: fork, exec, wait, signals, file I/O

### IXLand/ (App)
- Uses built binaries from staging directories
- Links against kernel XCFramework
- Packages linked during Xcode build
- No .deb installation step needed

## Constraints

- **iOS 16.0+** minimum deployment target
- **arm64** only (device and simulator)
- **No fork/exec** - kernel provides thread-based simulation
- **Sandbox paths only** - ~/Documents/, ~/Library/
- **Static libraries preferred** - easier distribution
- **NO Debian infrastructure** - no dpkg, apt, .deb

## Security

- Verify all downloads with SHA256
- Sandboxed execution (iOS sandbox)
- Static linking preferred
- No executable stack (W^X policy)

## Documentation

- **Build scripts**: `packages/core/*/build.sh`
- **Build library**: `scripts/ixland_package.sh`
- **Package builder**: `scripts/build-package.sh`
- **Test framework**: `ios-test-framework/`
- **Patch template**: `PATCH_TEMPLATE.md`
- **This guide**: `AGENTS.md`

## Files to Check

**Before committing a new package, verify:**
- [ ] No dpkg/apt references in build.sh
- [ ] No DEBIAN directory creation
- [ ] CPPFLAGS includes kernel headers (if using syscalls)
- [ ] Patches documented in patches/README.md
- [ ] Builds successfully with `--target universal`
- [ ] Tests pass (if test framework exists for package)

---

**Last Updated**: 2026-04-11
**Status**: iOS-native build system (no Debian)
**Next Steps**: Create missing package build.sh files, add patches as needed

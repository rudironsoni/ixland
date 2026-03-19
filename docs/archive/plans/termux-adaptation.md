# Termux Mechanisms Applied to ashell-packages

**Analysis Date**: 2026-03-19
**Purpose**: Adapt Termux's proven build system patterns for iOS

---

## Overview

Termux has a sophisticated build system with ~2000+ packages. We adapt their patterns for iOS while accounting for:
- iOS uses Xcode toolchain instead of Android NDK
- iOS produces XCFrameworks instead of ELF binaries
- iOS has stricter sandboxing (no fork/exec)
- iOS uses Mach-O instead of ELF

---

## 1. Directory Structure Comparison

### Termux Structure
```
termux-packages/
├── packages/                    # All packages (no root-packages distinction)
│   ├── apt/
│   │   ├── build.sh
│   │   └── *.patch
│   ├── coreutils/
│   └── ...
├── scripts/
│   ├── build/                   # Build step implementations
│   │   ├── termux_step_setup_variables.sh
│   │   ├── termux_step_extract_package.sh
│   │   ├── termux_step_patch_package.sh
│   │   └── ...
│   ├── utils/                   # Utility functions
│   │   ├── termux/package/
│   │   └── docker/
│   └── setup-*.sh               # Setup scripts
├── build-package.sh             # Main entry point
└── repo.json                    # Repository metadata
```

### Proposed ashell-packages Structure (Termux-Inspired)
```
ashell-packages/
├── root-packages/               # Core distro packages (iOS-specific distinction)
│   ├── apt/
│   │   ├── build.sh
│   │   └── patches/
│   ├── coreutils/
│   └── ...
├── packages/                    # User packages
│   ├── vim/
│   └── ...
├── scripts/
│   ├── build/                   # Build step implementations (NEW)
│   │   ├── ashell_step_setup_variables.sh
│   │   ├── ashell_step_extract_package.sh
│   │   ├── ashell_step_patch_package.sh
│   │   ├── ashell_step_configure.sh
│   │   ├── ashell_step_make.sh
│   │   ├── ashell_step_make_install.sh
│   │   ├── ashell_step_create_xcframework.sh
│   │   └── ashell_step_generate_plist.sh
│   ├── utils/                   # Utility functions (NEW)
│   │   ├── ashell/package.sh    # Package metadata handling
│   │   └── ios/toolchain.sh     # iOS toolchain setup
│   └── setup/                   # Setup scripts (NEW)
│       ├── setup-ios-toolchain.sh
│       └── setup-xcode.sh
├── ashell_package.sh            # Main library (expanded from current)
├── build.sh                     # Main entry point
└── repo.json                    # Repository metadata (NEW)
```

---

## 2. Variable Naming Convention

### Termux Pattern
```bash
TERMUX_PKG_NAME              # Package name
TERMUX_PKG_VERSION           # Version
TERMUX_PKG_SRCURL            # Source URL
TERMUX_PKG_SHA256            # Checksum
TERMUX_PKG_DEPENDS           # Runtime deps
TERMUX_PKG_BUILD_DEPENDS     # Build deps
TERMUX_PKG_EXTRA_CONFIGURE_ARGS
TERMUX_PKG_BUILD_IN_SRC      # Build in source dir
TERMUX_PKG_CONFFILES         # Config files
TERMUX_PKG_ESSENTIAL         # Cannot be removed
```

### ashell-packages Adaptation
```bash
ASHELL_PKG_NAME              # Package name
ASHELL_PKG_VERSION           # Version
ASHELL_PKG_SRCURL            # Source URL
ASHELL_PKG_SHA256            # Checksum
ASHELL_PKG_DEPENDS           # Runtime deps
ASHELL_PKG_BUILD_DEPENDS     # Build deps
ASHELL_PKG_EXTRA_CONFIGURE_ARGS
ASHELL_PKG_BUILD_IN_SRC      # Build in source dir
ASHELL_PKG_CONFFILES         # Config files (iOS: stored in Documents/)
ASHELL_PKG_ESSENTIAL         # Part of root-packages, cannot uninstall

# iOS-specific additions:
ASHELL_PKG_COMMANDS          # Commands provided (for plist generation)
ASHELL_PKG_FRAMEWORKS        # Frameworks to bundle
ASHELL_PKG_MIN_IOS_VERSION   # Minimum iOS version (e.g., 16.0)
ASHELL_PKG_ARCHS             # Architectures (arm64, x86_64_sim)
```

---

## 3. Build Lifecycle (Termux Step Functions)

### Termux Steps
```bash
termux_step_setup_variables()     # Set up all variables
termux_step_handle_buildarch()    # Handle architecture
termux_step_setup_toolchain()     # Set up compiler toolchain
termux_step_extract_package()     # Download and extract
termux_step_patch_package()       # Apply patches
termux_step_pre_configure()       # Pre-configure hooks
termux_step_configure()           # Run cmake/configure
termux_step_post_configure()      # Post-configure hooks
termux_step_pre_make()            # Pre-make hooks
termux_step_make()                # Build
termux_step_post_make()           # Post-make hooks
termux_step_pre_install()         # Pre-install hooks
termux_step_install()             # Install to staging
termux_step_post_install()        # Post-install hooks
termux_step_install_license()     # Install license
termux_step_massage()             # Clean up before packaging
termux_step_create_debian_package() # Create .deb
```

### ashell-packages Adaptation
```bash
ashell_step_setup_variables()      # Set up all variables (iOS-specific)
ashell_step_setup_toolchain()      # Set up Xcode toolchain
ashell_step_extract_package()      # Download and extract (same)
ashell_step_patch_package()        # Apply patches (same)
ashell_step_pre_configure()        # Pre-configure hooks (same)
ashell_step_configure()            # Run cmake/configure (same)
ashell_step_post_configure()       # Post-configure hooks (same)
ashell_step_pre_make()             # Pre-make hooks (same)
ashell_step_make()                 # Build (same)
ashell_step_post_make()            # Post-make hooks (same)
ashell_step_pre_install()          # Pre-install hooks (same)
ashell_step_make_install()         # Install to staging (same)
ashell_step_post_make_install()    # Post-install hooks (iOS-specific)

# iOS-specific additions:
ashell_step_create_xcframework()   # Create XCFramework
ashell_step_codesign()             # Sign frameworks
ashell_step_generate_plist()       # Generate commands.plist
ashell_step_create_pkg()           # Create installable package
```

---

## 4. Key Build Script Patterns

### Termux apt/build.sh
```bash
TERMUX_PKG_HOMEPAGE=https://packages.debian.org/apt
TERMUX_PKG_DESCRIPTION="Front-end for the dpkg package manager"
TERMUX_PKG_LICENSE="GPL-2.0"
TERMUX_PKG_MAINTAINER="@termux"
TERMUX_PKG_VERSION="2.8.1"
TERMUX_PKG_REVISION=2
TERMUX_PKG_SRCURL=https://salsa.debian.org/apt-team/apt/-/archive/${TERMUX_PKG_VERSION}/apt-${TERMUX_PKG_VERSION}.tar.bz2
TERMUX_PKG_SHA256=87ca18392c10822a133b738118505f7d04e0b31ba1122bf5d32911311cb2dc7e
TERMUX_PKG_DEPENDS="coreutils, dpkg, findutils, gpgv, grep, libandroid-glob, ..."
TERMUX_PKG_BUILD_DEPENDS="docbook-xsl,libdb"
TERMUX_PKG_ESSENTIAL=true
TERMUX_PKG_CONFFILES="etc/apt/sources.list"

TERMUX_PKG_EXTRA_CONFIGURE_ARGS="
-DCMAKE_POLICY_VERSION_MINIMUM=3.5
-DPERL_EXECUTABLE=$(command -v perl)
-DCACHE_DIR=${TERMUX_CACHE_DIR}/apt
-DCOMMON_ARCH=$TERMUX_ARCH
"

termux_step_pre_configure() {
    # Certain packages are not safe to build on device
    if $TERMUX_ON_DEVICE_BUILD; then
        termux_error_exit "Package '$TERMUX_PKG_NAME' is not safe for on-device builds."
    fi

    # Fix i686 builds
    CXXFLAGS+=" -Wno-c++11-narrowing"
    # Fix glob() on Android 7
    LDFLAGS+=" -Wl,--no-as-needed -landroid-glob"
}

termux_step_post_make_install() {
    # Configure apt sources
    {
        echo "# The main termux repository"
        echo "deb https://packages-cf.termux.dev/apt/termux-main/ stable main"
    } > $TERMUX_PREFIX/etc/apt/sources.list
}
```

### Adapted ashell-packages apt/build.sh
```bash
ASHELL_PKG_HOMEPAGE=https://packages.debian.org/apt
ASHELL_PKG_DESCRIPTION="Front-end for the dpkg package manager"
ASHELL_PKG_LICENSE="GPL-2.0"
ASHELL_PKG_MAINTAINER="@rudironsoni"
ASHELL_PKG_VERSION="3.1.16"
ASHELL_PKG_SRCURL=https://salsa.debian.org/apt-team/apt/-/archive/${ASHELL_PKG_VERSION}/apt-${ASHELL_PKG_VERSION}.tar.bz2
ASHELL_PKG_SHA256=<to_be_computed>
ASHELL_PKG_DEPENDS="dpkg, coreutils"
ASHELL_PKG_BUILD_DEPENDS="cmake, ninja"
ASHELL_PKG_ESSENTIAL=true
ASHELL_PKG_MIN_IOS_VERSION="16.0"
ASHELL_PKG_ARCHS="arm64,x86_64_sim"

ASHELL_PKG_COMMANDS=(
    "apt:apt_main::no"
    "apt-cache:apt_cache_main::no"
    "apt-get:apt_get_main::no"
)

ASHELL_PKG_EXTRA_CONFIGURE_ARGS="
-DCMAKE_POLICY_VERSION_MINIMUM=3.5
-DPERL_EXECUTABLE=$(command -v perl)
-DCOMMON_ARCH=arm64
-DDPKG_DATADIR=@ASHELL_PREFIX@/share/dpkg
-DUSE_NLS=OFF
-DWITH_DOC=OFF
-DWITH_DOC_MANPAGES=ON
-DCMAKE_INSTALL_LIBEXECDIR=lib
"

ashell_step_pre_configure() {
    # iOS-specific: Set up cross-compilation
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export CFLAGS="$ASHELL_CFLAGS"
    export CXXFLAGS="$ASHELL_CXXFLAGS"
    export LDFLAGS="$ASHELL_LDFLAGS"

    # Apply iOS patches
    local patch_dir="$ASHELL_PKG_BUILDER_DIR/patches"
    for patch in "$patch_dir"/*.patch; do
        if [[ -f "$patch" ]]; then
            ashell_info "Applying patch: $(basename $patch)"
            patch -p1 < "$patch" || ashell_error "Failed to apply patch"
        fi
    done
}

ashell_step_post_make_install() {
    # Configure apt sources for a-Shell
    mkdir -p "$ASHELL_PKG_STAGINGDIR@ASHELL_PREFIX@/etc/apt"
    {
        echo "# a-Shell package repository"
        echo "deb [trusted=yes] https://packages.ashell.dev/apt/stable/ ios main"
    } > "$ASHELL_PKG_STAGINGDIR@ASHELL_PREFIX@/etc/apt/sources.list"
}
```

---

## 5. Main Build Script (build-package.sh Pattern)

### Termux's Approach
```bash
#!/bin/bash

# Setting the TMPDIR variable
: "${TMPDIR:=/tmp}"
export TMPDIR

set -euo pipefail

cd "$(realpath "$(dirname "$0")")"
TERMUX_SCRIPTDIR=$(pwd)
export TERMUX_SCRIPTDIR

# Source all the utility functions
source "$TERMUX_SCRIPTDIR/scripts/utils/termux/package/termux_package.sh"
source "$TERMUX_SCRIPTDIR/scripts/build/termux_error_exit.sh"
source "$TERMUX_SCRIPTDIR/scripts/build/termux_download.sh"
# ... many more sources

# Main build logic
termux_step_setup_variables
termux_step_handle_buildarch
termux_step_setup_toolchain
termux_step_extract_package
termux_step_patch_package
termux_step_pre_configure
termux_step_configure
termux_step_post_configure
# ... etc
```

### ashell-packages Adaptation
```bash
#!/bin/bash
# ashell-packages/build.sh

set -euo pipefail

cd "$(realpath "$(dirname "$0")")"
ASHELL_SCRIPTDIR=$(pwd)
export ASHELL_SCRIPTDIR

# Source all utility functions
source "$ASHELL_SCRIPTDIR/scripts/utils/ashell/package.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_error_exit.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_download.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_setup_variables.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_setup_toolchain.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_extract_package.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_patch_package.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_configure.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_make.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_make_install.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_create_xcframework.sh"
source "$ASHELL_SCRIPTDIR/scripts/build/ashell_step_generate_plist.sh"

# Main entry point
show_usage() {
    echo "Usage: $0 <package> [command]"
    echo "Commands:"
    echo "  build     - Build the package (default)"
    echo "  clean     - Clean build artifacts"
    echo "  install   - Install to device (requires connection)"
    echo "  package   - Create distributable package"
    echo "  list      - List available packages"
}

main() {
    local package="${1:-}"
    local command="${2:-build}"

    if [[ -z "$package" ]]; then
        show_usage
        exit 1
    fi

    # Find package directory (search root-packages first, then packages)
    if [[ -d "$ASHELL_SCRIPTDIR/root-packages/$package" ]]; then
        ASHELL_PKG_BUILDER_DIR="$ASHELL_SCRIPTDIR/root-packages/$package"
    elif [[ -d "$ASHELL_SCRIPTDIR/packages/$package" ]]; then
        ASHELL_PKG_BUILDER_DIR="$ASHELL_SCRIPTDIR/packages/$package"
    else
        ashell_error_exit "Package '$package' not found"
    fi

    # Source package build.sh
    source "$ASHELL_PKG_BUILDER_DIR/build.sh"

    # Execute build steps
    case "$command" in
        build)
            ashell_step_setup_variables
            ashell_step_setup_toolchain
            ashell_step_extract_package
            ashell_step_patch_package
            ashell_step_pre_configure
            ashell_step_configure
            ashell_step_post_configure
            ashell_step_pre_make
            ashell_step_make
            ashell_step_post_make
            ashell_step_pre_install
            ashell_step_make_install
            ashell_step_post_make_install
            ashell_step_create_xcframework
            ashell_step_generate_plist
            ashell_info "Build complete: $ASHELL_PKG_NAME"
            ;;
        clean)
            ashell_step_setup_variables
            rm -rf "$ASHELL_PKG_BUILDDIR"
            rm -rf "$ASHELL_PKG_STAGINGDIR"
            ashell_info "Cleaned: $ASHELL_PKG_NAME"
            ;;
        *)
            show_usage
            exit 1
            ;;
    esac
}

main "$@"
```

---

## 6. Toolchain Setup

### Termux's NDK Setup
```bash
termux_step_setup_toolchain() {
    # Android NDK-based toolchain
    export CC="$TERMUX_STANDALONE_TOOLCHAIN/bin/$TERMUX_HOST_PLATFORM-clang"
    export CXX="$TERMUX_STANDALONE_TOOLCHAIN/bin/$TERMUX_HOST_PLATFORM-clang++"
    export AR="$TERMUX_STANDALONE_TOOLCHAIN/bin/llvm-ar"
    # ... more tools

    # Flags
    export CFLAGS="..."
    export CXXFLAGS="..."
    export LDFLAGS="..."
}
```

### ashell-packages Xcode Setup
```bash
ashell_step_setup_toolchain() {
    # Xcode-based iOS toolchain
    export ASHELL_DEVELOPER_DIR="${ASHELL_DEVELOPER_DIR:-$(xcode-select -p)}"
    export ASHELL_SDK="${ASHELL_SDK:-iphoneos}"
    export ASHELL_DEPLOYMENT_TARGET="${ASHELL_DEPLOYMENT_TARGET:-16.0}"

    # Toolchain
    export ASHELL_CC="$ASHELL_DEVELOPER_DIR/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
    export ASHELL_CXX="$ASHELL_DEVELOPER_DIR/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
    export ASHELL_AR="$ASHELL_DEVELOPER_DIR/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar"
    export ASHELL_RANLIB="$ASHELL_DEVELOPER_DIR/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib"
    export ASHELL_LD="$ASHELL_DEVELOPER_DIR/Toolchains/XcodeDefault.xctoolchain/usr/bin/ld"

    # Simulator vs Device
    if [[ "$ASHELL_SDK" == "iphonesimulator" ]]; then
        export ASHELL_ARCH="${ASHELL_ARCH:-arm64}"  # or x86_64 for Intel sim
        export ASHELL_SYSROOT="$ASHELL_DEVELOPER_DIR/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk"
    else
        export ASHELL_ARCH="${ASHELL_ARCH:-arm64}"
        export ASHELL_SYSROOT="$ASHELL_DEVELOPER_DIR/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk"
    fi

    # Flags
    export ASHELL_CFLAGS="-arch $ASHELL_ARCH -isysroot $ASHELL_SYSROOT -mios-version-min=$ASHELL_DEPLOYMENT_TARGET -fembed-bitcode-marker"
    export ASHELL_CXXFLAGS="$ASHELL_CFLAGS -stdlib=libc++"
    export ASHELL_LDFLAGS="-arch $ASHELL_ARCH -isysroot $ASHELL_SYSROOT -mios-version-min=$ASHELL_DEPLOYMENT_TARGET"

    # Pass to autotools/cmake
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export CFLAGS="$ASHELL_CFLAGS"
    export CXXFLAGS="$ASHELL_CXXFLAGS"
    export LDFLAGS="$ASHELL_LDFLAGS"
}
```

---

## 7. Patch Application

### Termux Pattern
```bash
termux_step_patch_package() {
    cd "$TERMUX_PKG_SRCDIR"

    # Auto-patch: Find all .patch files and apply
    local patch_file patch_filename
    for patch_file in "$TERMUX_PKG_BUILDER_DIR"/*.patch; do
        if [[ -f "$patch_file" ]]; then
            patch_filename=$(basename "$patch_file")
            echo "Applying patch: $patch_filename"
            patch -p1 -i "$patch_file" || termux_error_exit "Failed to apply patch: $patch_filename"
        fi
    done
}
```

### ashell-packages Adaptation
```bash
ashell_step_patch_package() {
    cd "$ASHELL_PKG_SRCDIR"

    local patch_dir="$ASHELL_PKG_BUILDER_DIR/patches"
    if [[ ! -d "$patch_dir" ]]; then
        ashell_info "No patches directory found, skipping"
        return 0
    fi

    # Apply patches in order (01-*.patch, 02-*.patch, etc.)
    local patch_file patch_filename
    for patch_file in "$patch_dir"/*.patch; do
        if [[ -f "$patch_file" ]]; then
            patch_filename=$(basename "$patch_file")
            ashell_info "Applying patch: $patch_filename"

            # Substitute @ASHELL_PREFIX@ before applying
            local temp_patch=$(mktemp)
            sed "s|@ASHELL_PREFIX@|$ASHELL_PREFIX|g" "$patch_file" > "$temp_patch"

            if patch -p1 -i "$temp_patch"; then
                rm "$temp_patch"
            else
                rm "$temp_patch"
                ashell_error_exit "Failed to apply patch: $patch_filename"
            fi
        fi
    done
}
```

---

## 8. Key Differences Summary

| Aspect | Termux | ashell-packages |
|--------|--------|-----------------|
| Target Platform | Android (Bionic libc) | iOS (libSystem) |
| Binary Format | ELF | Mach-O |
| Toolchain | Android NDK | Xcode |
| Package Format | .deb | .deb (AltStore) / .wasm (App Store) |
| Output Format | Native binaries | XCFrameworks |
| Distribution | Downloaded packages | Pre-bundled + WASM |
| Root/Regular Distinction | No distinction | root-packages/ vs packages/ |
| Fork/Exec | Supported (Android) | Not supported (iOS) |
| Shebang Fix | termux-exec (LD_PRELOAD) | ashell-fix-shebang (build-time) |

---

## 9. Files to Create

### Scripts (new directory structure)
```
ashell-packages/scripts/
├── build/
│   ├── ashell_step_setup_variables.sh
│   ├── ashell_step_setup_toolchain.sh
│   ├── ashell_step_extract_package.sh
│   ├── ashell_step_patch_package.sh
│   ├── ashell_step_configure.sh
│   ├── ashell_step_make.sh
│   ├── ashell_step_make_install.sh
│   ├── ashell_step_create_xcframework.sh
│   ├── ashell_step_generate_plist.sh
│   ├── ashell_step_codesign.sh
│   ├── ashell_download.sh
│   └── ashell_error_exit.sh
├── utils/
│   └── ashell/
│       └── package.sh
└── setup/
    ├── setup-ios-toolchain.sh
    └── setup-xcode.sh
```

### Package Updates
```
ashell-packages/root-packages/apt/build.sh  (update to 3.1.16)
ashell-packages/root-packages/apt/patches/  (maintain patches)
```

---

## 10. Implementation Priority

### Phase 1: Script Infrastructure
1. Create `scripts/build/` directory structure
2. Implement `ashell_step_setup_variables.sh`
3. Implement `ashell_step_setup_toolchain.sh`
4. Implement `ashell_step_extract_package.sh`
5. Implement `ashell_step_patch_package.sh`

### Phase 2: Build Steps
6. Implement `ashell_step_configure.sh`
7. Implement `ashell_step_make.sh`
8. Implement `ashell_step_make_install.sh`

### Phase 3: iOS-Specific
9. Implement `ashell_step_create_xcframework.sh`
10. Implement `ashell_step_generate_plist.sh`
11. Implement `ashell_step_codesign.sh`

### Phase 4: Integration
12. Update `build.sh` to use new structure
13. Update `ashell_package.sh` as entry point
14. Test with `hello` package
15. Update `apt` package

---

## References

- **Termux packages repo**: https://github.com/termux/termux-packages
- **Termux build system**: https://github.com/termux/termux-packages/tree/master/scripts/build
- **Termux apt package**: https://github.com/termux/termux-packages/tree/master/packages/apt

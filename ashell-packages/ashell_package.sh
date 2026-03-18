#!/bin/bash
# ashell_package.sh - Build system library for a-Shell packages
# Termux-inspired package building system for iOS
#
# Usage: source this file in package build.sh scripts
# Example: source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"

set -e
set -o pipefail

# =============================================================================
# CONFIGURATION
# =============================================================================

# Build directories
ASHELL_PKG_BUILDER_DIR="${ASHELL_PKG_BUILDER_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)}"
ASHELL_PKG_TOPDIR="${ASHELL_PKG_TOPDIR:-$ASHELL_PKG_BUILDER_DIR}"
ASHELL_PKG_BUILDDIR="${ASHELL_PKG_BUILDDIR:-$ASHELL_PKG_TOPDIR/.build}"

# Installation paths (iOS-specific)
ASHELL_PREFIX="${ASHELL_PREFIX:-\$HOME/Library/ashell}"
ASHELL_CONFIG="${ASHELL_CONFIG:-\$HOME/Documents/.ashell}"

# Cross-compilation target
ASHELL_HOST_PLATFORM="${ASHELL_HOST_PLATFORM:-$(uname -m)-apple-darwin}"
ASHELL_TARGET_PLATFORM="${ASHELL_TARGET_PLATFORM:-arm64-apple-ios16.0}"

# Compiler settings
ASHELL_CC="${ASHELL_CC:-clang}"
ASHELL_CXX="${ASHELL_CXX:-clang++}"
ASHELL_AR="${ASHELL_AR:-ar}"
ASHELL_RANLIB="${ASHELL_RANLIB:-ranlib}"
ASHELL_STRIP="${ASHELL_STRIP:-strip}"

# SDK paths (computed on first use, not at load)
ASHELL_DEPLOYMENT_TARGET="${ASHELL_DEPLOYMENT_TARGET:-16.0}"

# Lazily compute SDK path to avoid errors when just listing packages
ashell_get_sdk_path() {
    if [[ -z "$ASHELL_SDK_PATH" ]]; then
        if command -v xcrun &> /dev/null; then
            ASHELL_SDK_PATH=$(xcrun --sdk iphoneos --show-sdk-path 2>/dev/null || echo "")
        fi
        if [[ -z "$ASHELL_SDK_PATH" ]]; then
            ashell_warning "iOS SDK not found. Set ASHELL_SDK_PATH manually."
            ASHELL_SDK_PATH="/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk"
        fi
    fi
    echo "$ASHELL_SDK_PATH"
}

# Build flags
ASHELL_CFLAGS="${ASHELL_CFLAGS:-}"
ASHELL_CXXFLAGS="${ASHELL_CXXFLAGS:-}"
ASHELL_LDFLAGS="${ASHELL_LDFLAGS:-}"
ASHELL_CPPFLAGS="${ASHELL_CPPFLAGS:-}"

# Package metadata (to be set by package build.sh)
ASHELL_PKG_NAME="${ASHELL_PKG_NAME:-}"
ASHELL_PKG_VERSION="${ASHELL_PKG_VERSION:-}"
ASHELL_PKG_SRCURL="${ASHELL_PKG_SRCURL:-}"
ASHELL_PKG_SHA256="${ASHELL_PKG_SHA256:-}"
ASHELL_PKG_DEPENDS="${ASHELL_PKG_DEPENDS:-}"
ASHELL_PKG_BUILD_DEPENDS="${ASHELL_PKG_BUILD_DEPENDS:-}"
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="${ASHELL_PKG_EXTRA_CONFIGURE_ARGS:-}"
ASHELL_PKG_EXTRA_MAKE_ARGS="${ASHELL_PKG_EXTRA_MAKE_ARGS:-}"
ASHELL_PKG_COMMANDS="${ASHELL_PKG_COMMANDS:-}"

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

# Log message with timestamp
ashell_log() {
    local level="$1"
    local message="$2"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] [$level] $message"
}

# Log info message
ashell_info() {
    ashell_log "INFO" "$1"
}

# Log warning message
ashell_warning() {
    ashell_log "WARNING" "$1" >&2
}

# Log error message and exit
ashell_error() {
    ashell_log "ERROR" "$1" >&2
    exit 1
}

# Create directory if it doesn't exist
ashell_mkdir_p() {
    mkdir -p "$1" || ashell_error "Failed to create directory: $1"
}

# Verify SHA256 checksum
ashell_verify_checksum() {
    local file="$1"
    local expected_hash="$2"

    if [[ -z "$expected_hash" ]]; then
        ashell_warning "No checksum provided, skipping verification"
        return 0
    fi

    local actual_hash=$(shasum -a 256 "$file" | cut -d' ' -f1)
    if [[ "$actual_hash" != "$expected_hash" ]]; then
        ashell_error "Checksum mismatch for $file\nExpected: $expected_hash\nActual:   $actual_hash"
    fi

    ashell_info "Checksum verified for $(basename "$file")"
}

# Download file with curl
ashell_download() {
    local url="$1"
    local output="$2"

    ashell_info "Downloading $url"
    curl -L -o "$output" "$url" --fail --progress-bar || \
        ashell_error "Failed to download $url"
}

# Substitute @ASHELL_PREFIX@ in files
ashell_substitute_prefix() {
    local dir="$1"
    local prefix="${ASHELL_PREFIX//\$/\\$}"

    if [[ -d "$dir" ]]; then
        find "$dir" -type f \( -name "*.patch" -o -name "*.in" -o -name "*.ac" \) -exec \
            sed -i '' "s|@ASHELL_PREFIX@|$prefix|g" {} \;
    fi
}

# Get package source directory
ashell_pkg_srcdir() {
    echo "$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/src"
}

# Get package build directory
ashell_pkg_builddir() {
    echo "$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/build"
}

# Get package staging directory
ashell_pkg_stagingdir() {
    echo "$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/staging"
}

# =============================================================================
# BUILD STEP FUNCTIONS
# =============================================================================

# Download and extract source package
ashell_step_extract_package() {
    ashell_info "Extracting package: $ASHELL_PKG_NAME"

    local srcdir=$(ashell_pkg_srcdir)
    ashell_mkdir_p "$srcdir"

    # Check if source URL is provided
    if [[ -z "$ASHELL_PKG_SRCURL" ]]; then
        ashell_warning "No source URL provided, assuming local source"
        return 0
    fi

    local filename=$(basename "$ASHELL_PKG_SRCURL")
    local download_path="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$filename"

    # Download if not cached
    if [[ ! -f "$download_path" ]]; then
        ashell_download "$ASHELL_PKG_SRCURL" "$download_path"
    fi

    # Verify checksum
    ashell_verify_checksum "$download_path" "$ASHELL_PKG_SHA256"

    # Extract based on file extension
    ashell_info "Extracting $filename"
    case "$filename" in
        *.tar.gz|*.tgz)
            tar -xzf "$download_path" -C "$srcdir" --strip-components=1
            ;;
        *.tar.bz2|*.tbz2)
            tar -xjf "$download_path" -C "$srcdir" --strip-components=1
            ;;
        *.tar.xz|*.txz)
            tar -xJf "$download_path" -C "$srcdir" --strip-components=1
            ;;
        *.zip)
            unzip -q "$download_path" -d "$srcdir"
            ;;
        *)
            ashell_error "Unknown archive format: $filename"
            ;;
    esac

    ashell_info "Package extracted to $srcdir"
}

# Apply patches from patches/ directory
ashell_step_patch_package() {
    ashell_info "Patching package: $ASHELL_PKG_NAME"

    local srcdir=$(ashell_pkg_srcdir)
    local patchdir="$ASHELL_PKG_BUILDER_DIR/$ASHELL_PKG_NAME/patches"

    if [[ ! -d "$patchdir" ]]; then
        ashell_info "No patches directory found"
        return 0
    fi

    # Apply patches in order (01-*.patch, 02-*.patch, etc.)
    for patch in "$patchdir"/[0-9][0-9]-*.patch; do
        if [[ -f "$patch" ]]; then
            ashell_info "Applying patch: $(basename "$patch")"
            (cd "$srcdir" && patch -p1 -i "$patch") || \
                ashell_error "Failed to apply patch: $(basename "$patch")"
        fi
    done
}

# Pre-configure hook (override in package build.sh)
ashell_step_pre_configure() {
    :
}

# Configure the package
ashell_step_configure() {
    ashell_info "Configuring package: $ASHELL_PKG_NAME"

    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)
    ashell_mkdir_p "$builddir"

    # Ensure SDK path is set
    local sdk_path=$(ashell_get_sdk_path)

    # Set up iOS cross-compilation flags
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export AR="$ASHELL_AR"
    export RANLIB="$ASHELL_RANLIB"
    export STRIP="$ASHELL_STRIP"

    export CFLAGS="-arch arm64 -isysroot $sdk_path -mios-version-min=$ASHELL_DEPLOYMENT_TARGET -fembed-bitcode $ASHELL_CFLAGS"
    export CXXFLAGS="-arch arm64 -isysroot $sdk_path -mios-version-min=$ASHELL_DEPLOYMENT_TARGET -fembed-bitcode $ASHELL_CXXFLAGS"
    export LDFLAGS="-arch arm64 -isysroot $sdk_path -mios-version-min=$ASHELL_DEPLOYMENT_TARGET $ASHELL_LDFLAGS"
    export CPPFLAGS="-arch arm64 -isysroot $sdk_path $ASHELL_CPPFLAGS"

    # Export target settings
    export ac_cv_func_malloc_0_nonnull=yes
    export ac_cv_func_realloc_0_nonnull=yes

    # Configure with prefix
    local prefix="${ASHELL_PREFIX//\$/\\$}"
    local configure_args="--prefix=$prefix --host=$ASHELL_TARGET_PLATFORM $ASHELL_PKG_EXTRA_CONFIGURE_ARGS"

    if [[ -f "$srcdir/configure" ]]; then
        (cd "$builddir" && "$srcdir/configure" $configure_args) || \
            ashell_error "Configure failed"
    elif [[ -f "$srcdir/CMakeLists.txt" ]]; then
        (cd "$builddir" && cmake "$srcdir" \
            -DCMAKE_INSTALL_PREFIX="$prefix" \
            -DCMAKE_SYSTEM_NAME=iOS \
            -DCMAKE_OSX_ARCHITECTURES=arm64 \
            -DCMAKE_OSX_DEPLOYMENT_TARGET=$ASHELL_DEPLOYMENT_TARGET \
            -DCMAKE_C_FLAGS="$CFLAGS" \
            -DCMAKE_CXX_FLAGS="$CXXFLAGS" \
            $ASHELL_PKG_EXTRA_CONFIGURE_ARGS) || \
            ashell_error "CMake configuration failed"
    else
        ashell_warning "No configure script or CMakeLists.txt found"
    fi
}

# Post-configure hook (override in package build.sh)
ashell_step_post_configure() {
    :
}

# Pre-make hook (override in package build.sh)
ashell_step_pre_make() {
    :
}

# Build the package
ashell_step_make() {
    ashell_info "Building package: $ASHELL_PKG_NAME"

    local builddir=$(ashell_pkg_builddir)

    # Get CPU count (macOS uses sysctl, Linux uses nproc)
    local cpu_count
    if command -v sysctl &> /dev/null; then
        cpu_count=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
    elif command -v nproc &> /dev/null; then
        cpu_count=$(nproc 2>/dev/null || echo 4)
    else
        cpu_count=4
    fi

    local make_args="-j$cpu_count $ASHELL_PKG_EXTRA_MAKE_ARGS"

    (cd "$builddir" && make $make_args) || \
        ashell_error "Build failed"
}

# Post-make hook (override in package build.sh)
ashell_step_post_make() {
    :
}

# Install to staging directory
ashell_step_make_install() {
    ashell_info "Installing package: $ASHELL_PKG_NAME"

    local builddir=$(ashell_pkg_builddir)
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$stagingdir"

    (cd "$builddir" && make DESTDIR="$stagingdir" install) || \
        ashell_error "Installation failed"
}

# Post-install hook (override in package build.sh)
ashell_step_post_make_install() {
    :
}

# =============================================================================
# iOS-SPECIFIC STEPS
# =============================================================================

# Create XCFramework from built binaries
ashell_step_create_xcframework() {
    ashell_info "Creating XCFramework for: $ASHELL_PKG_NAME"

    local stagingdir=$(ashell_pkg_stagingdir)
    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"

    ashell_mkdir_p "$framework_dir"

    # Collect libraries and binaries
    local libs=()
    if [[ -d "$stagingdir" ]]; then
        while IFS= read -r -d '' lib; do
            libs+=("$lib")
        done < <(find "$stagingdir" -name "*.a" -print0 2>/dev/null || true)
    fi

    # If no static libraries, look for object files
    if [[ ${#libs[@]} -eq 0 ]]; then
        ashell_warning "No static libraries found, checking for binaries"
    fi

    # Create framework structure
    ashell_mkdir_p "$framework_dir/Headers"
    ashell_mkdir_p "$framework_DIR/Modules"

    # Copy headers if present
    if [[ -d "$stagingdir/include" ]]; then
        cp -R "$stagingdir/include/"* "$framework_dir/Headers/" 2>/dev/null || true
    fi

    # Generate module map
    cat > "$framework_dir/Modules/module.modulemap" <<EOF
framework module $ASHELL_PKG_NAME {
    umbrella header "$ASHELL_PKG_NAME.h"
    export *
    module * { export * }
}
EOF

    # Create umbrella header
    cat > "$framework_dir/Headers/$ASHELL_PKG_NAME.h" <<EOF
#import <Foundation/Foundation.h>

//! Project version number for $ASHELL_PKG_NAME.
FOUNDATION_EXPORT double ${ASHELL_PKG_NAME}_versionNumber;

//! Project version string for $ASHELL_PKG_NAME.
FOUNDATION_EXPORT const unsigned char ${ASHELL_PKG_NAME}_versionString[];

// In this header, you should import all the public headers of your framework.
EOF

    # Include individual headers
    for header in "$framework_dir/Headers/"*.h; do
        if [[ -f "$header" && "$(basename "$header")" != "$ASHELL_PKG_NAME.h" ]]; then
            echo "#import \"$(basename "$header")\"" >> "$framework_dir/Headers/$ASHELL_PKG_NAME.h"
        fi
    done

    # Copy binaries
    local bindir="$stagingdir${ASHELL_PREFIX//\$/}/bin"
    if [[ -d "$bindir" ]]; then
        for binary in "$bindir"/*; do
            if [[ -f "$binary" && -x "$binary" ]]; then
                cp "$binary" "$framework_dir/" || true
            fi
        done
    fi

    ashell_info "Framework created at $framework_dir"
}

# Generate commands.plist for command registration
ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for: $ASHELL_PKG_NAME"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    # Parse ASHELL_PKG_COMMANDS and generate plist
    # Format: "command:entry_point:auth_type:type"
    # Example: "hello:hello_main::no"

    cat > "$plist_path" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
EOF

    IFS=' ' read -ra commands <<< "$ASHELL_PKG_COMMANDS"
    for cmd_spec in "${commands[@]}"; do
        IFS=':' read -ra parts <<< "$cmd_spec"
        local cmd="${parts[0]}"
        local entry="${parts[1]}"
        local auth="${parts[2]}"
        local type="${parts[3]}"

        cat >> "$plist_path" <<EOF
    <key>$cmd</key>
    <array>
        <string>$ASHELL_PKG_NAME.framework/$ASHELL_PKG_NAME</string>
        <string>${entry:-${cmd}_main}</string>
        <string>${auth:-}</string>
        <string>${type:-file}</string>
    </array>
EOF
    done

    cat >> "$plist_path" <<'EOF'
</dict>
</plist>
EOF

    ashell_info "Commands plist generated at $plist_path"
}

# Code sign the framework
ashell_step_codesign() {
    ashell_info "Code signing: $ASHELL_PKG_NAME"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"

    if [[ ! -d "$framework_dir" ]]; then
        ashell_warning "Framework not found, skipping code signing"
        return 0
    fi

    # Sign with ad-hoc signature for CI/testing
    # In production, use proper signing identity
    codesign --force --sign "-" --timestamp=none "$framework_dir" 2>/dev/null || \
        ashell_warning "Code signing failed (may need proper identity)"
}

# =============================================================================
# MAIN BUILD SEQUENCE
# =============================================================================

# Run the complete build sequence
ashell_build_package() {
    ashell_info "Starting build for: $ASHELL_PKG_NAME v$ASHELL_PKG_VERSION"

    # Validate required variables
    [[ -z "$ASHELL_PKG_NAME" ]] && ashell_error "ASHELL_PKG_NAME not set"
    [[ -z "$ASHELL_PKG_VERSION" ]] && ashell_error "ASHELL_PKG_VERSION not set"

    # Execute build steps
    ashell_step_extract_package
    ashell_step_patch_package
    ashell_step_pre_configure
    ashell_step_configure
    ashell_step_post_configure
    ashell_step_pre_make
    ashell_step_make
    ashell_step_post_make
    ashell_step_make_install
    ashell_step_post_make_install
    ashell_step_create_xcframework
    ashell_step_generate_plist
    ashell_step_codesign

    ashell_info "Build completed successfully: $ASHELL_PKG_NAME"
}

# Export functions for use in package scripts (bash only)
if [ -n "$BASH_VERSION" ]; then
    export -f ashell_log ashell_info ashell_warning ashell_error
    export -f ashell_mkdir_p ashell_verify_checksum ashell_download ashell_substitute_prefix
    export -f ashell_pkg_srcdir ashell_pkg_builddir ashell_pkg_stagingdir
    export -f ashell_step_extract_package ashell_step_patch_package
    export -f ashell_step_pre_configure ashell_step_configure ashell_step_post_configure
    export -f ashell_step_pre_make ashell_step_make ashell_step_post_make
    export -f ashell_step_make_install ashell_step_post_make_install
    export -f ashell_step_create_xcframework ashell_step_generate_plist ashell_step_codesign
    export -f ashell_build_package
fi

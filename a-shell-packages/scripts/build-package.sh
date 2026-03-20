#!/bin/bash
# build-package.sh - Build a single package
# Usage: ./build-package.sh <package-name>

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PKG_NAME="$1"

if [ -z "$PKG_NAME" ]; then
    echo "Usage: $0 <package-name>"
    echo "Example: $0 libz"
    exit 1
fi

# Source build library
source "$SCRIPT_DIR/a_shell_package.sh"

# Setup
a_shell_setup_directories
a_shell_setup_toolchain

# Find package build script
PKG_DIR="$SCRIPT_DIR/../packages/core/$PKG_NAME"
if [ ! -f "$PKG_DIR/build.sh" ]; then
    a_shell_error "Package $PKG_NAME not found at $PKG_DIR/build.sh"
fi

a_shell_info "Building package: $PKG_NAME"

    # Source package build script
    # Unset CPPFLAGS to avoid injecting kernel headers that break builds
    unset CPPFLAGS
    source "$PKG_DIR/build.sh"

# Download and extract
a_shell_step "Downloading $A_SHELL_PKG_NAME..."
mkdir -p "$A_SHELL_PKG_TMPDIR"
cd "$A_SHELL_PKG_TMPDIR"

if [ ! -f "$(basename $A_SHELL_PKG_SRCURL)" ]; then
    curl -L -o "$(basename $A_SHELL_PKG_SRCURL)" "$A_SHELL_PKG_SRCURL"
fi

# Verify checksum
if [ -n "$A_SHELL_PKG_SHA256" ]; then
    echo "$A_SHELL_PKG_SHA256  $(basename $A_SHELL_PKG_SRCURL)" | shasum -a 256 -c - || a_shell_error "Checksum verification failed"
fi

# Extract
if [ ! -d "$A_SHELL_PKG_NAME-$A_SHELL_PKG_VERSION" ] && [ ! -d "$(basename $A_SHELL_PKG_SRCURL .tar.gz)" ] && [ ! -d "$(basename $A_SHELL_PKG_SRCURL .tar.xz)" ]; then
    a_shell_step "Extracting..."
    tar -xzf "$(basename $A_SHELL_PKG_SRCURL)" 2>/dev/null || tar -xJf "$(basename $A_SHELL_PKG_SRCURL)" 2>/dev/null || a_shell_error "Failed to extract"
fi

# Find extracted directory
EXTRACT_DIR="$A_SHELL_PKG_NAME-$A_SHELL_PKG_VERSION"
if [ ! -d "$EXTRACT_DIR" ]; then
    # Try without the package name prefix (e.g., zlib instead of libz)
    EXTRACT_DIR="$(basename $A_SHELL_PKG_SRCURL .tar.gz)"
    if [ ! -d "$EXTRACT_DIR" ]; then
        EXTRACT_DIR="$(basename $A_SHELL_PKG_SRCURL .tar.xz)"
    fi
fi

if [ ! -d "$EXTRACT_DIR" ]; then
    a_shell_error "Could not find extracted directory"
fi

cd "$EXTRACT_DIR"

# Configure
a_shell_step "Configuring..."
a_shell_pkg_configure

# Build
a_shell_step "Building..."
a_shell_pkg_make

# Install
a_shell_step "Installing..."
a_shell_pkg_install

a_shell_info "Build complete: $PKG_NAME"
a_shell_info "Output: $A_SHELL_PKG_STAGING"

# Show what was built
ls -la "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/lib/" 2>/dev/null | head -20 || true
ls -la "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/bin/" 2>/dev/null | head -20 || true

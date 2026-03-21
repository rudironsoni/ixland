#!/bin/bash
# build-all.sh - Build all core packages for a-Shell
# Builds: libz, bash, coreutils, ncurses, readline, libcurl, libssl, git, vim, python

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="$SCRIPT_DIR/../packages/core"
BUILD_DIR="$SCRIPT_DIR/../.build"

echo "=========================================="
echo "a-Shell Core Packages Build"
echo "=========================================="
echo ""

# Track which packages built successfully
BUILT_PACKAGES=()
FAILED_PACKAGES=()

build_package() {
    local pkg=$1
    local desc=$2
    
    echo ""
    echo "=========================================="
    echo "Building: $pkg"
    echo "Description: $desc"
    echo "=========================================="
    
    if [ -f "$PACKAGES_DIR/$pkg/build.sh" ]; then
        if "$SCRIPT_DIR/build-package.sh" "$pkg"; then
            BUILT_PACKAGES+=("$pkg")
            echo "✓ $pkg built successfully"
        else
            FAILED_PACKAGES+=("$pkg")
            echo "✗ $pkg failed to build"
            # Continue with next package
        fi
    else
        echo "✗ $pkg: No build.sh found"
        FAILED_PACKAGES+=("$pkg")
    fi
}

# Wave 1: Foundation (no deps)
echo ""
echo "Wave 1: Foundation packages"
echo "---------------------------"
build_package "libz" "zlib compression library"

# Wave 2: Core shell (minimal deps)
echo ""
echo "Wave 2: Core shell"
echo "-------------------"
build_package "bash-minimal" "GNU Bourne Again Shell (minimal)"

# Wave 3: Essential utilities
echo ""
echo "Wave 3: Essential utilities"
echo "-------------------------"
build_package "coreutils" "GNU Core Utilities (ls, cp, mv, cat, etc.)"

# Summary
echo ""
echo "=========================================="
echo "Build Summary"
echo "=========================================="
echo ""

if [ ${#BUILT_PACKAGES[@]} -gt 0 ]; then
    echo "✓ Successfully built (${#BUILT_PACKAGES[@]}):"
    for pkg in "${BUILT_PACKAGES[@]}"; do
        echo "  - $pkg"
    done
fi

echo ""

if [ ${#FAILED_PACKAGES[@]} -gt 0 ]; then
    echo "✗ Failed to build (${#FAILED_PACKAGES[@]}):"
    for pkg in "${FAILED_PACKAGES[@]}"; do
        echo "  - $pkg"
    done
    echo ""
    echo "Check build logs for details"
    exit 1
else
    echo ""
    echo "=========================================="
    echo "All packages built successfully!"
    echo "=========================================="
    echo ""
    echo "Output location: $BUILD_DIR/staging/usr/local/"
    echo "  Binaries: $BUILD_DIR/staging/usr/local/bin/"
    echo "  Libraries: $BUILD_DIR/staging/usr/local/lib/"
    echo ""
    
    # Count built binaries
    if [ -d "$BUILD_DIR/staging/usr/local/bin" ]; then
        BIN_COUNT=$(ls "$BUILD_DIR/staging/usr/local/bin/" 2>/dev/null | wc -l)
        echo "Total binaries: $BIN_COUNT"
        ls "$BUILD_DIR/staging/usr/local/bin/" | head -20
    fi
fi

#!/bin/bash
# build-package.sh - Build a single package with target selection
# Usage: ./build-package.sh <package-name> [--target ios|simulator|universal|both]
#
# Targets:
#   ios         - iOS device only (arm64, for App Store)
#   simulator   - iOS Simulator only (arm64, for testing)
#   universal   - Fat binary with both architectures (default, for development)
#   both        - Build ios and simulator separately (no combining)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PKG_NAME=""
TARGET="universal"  # Default to universal for development

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --target|-t)
            TARGET="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 <package-name> [--target ios|simulator|universal|both]"
            echo ""
            echo "Targets:"
            echo "  ios         - iOS device only (arm64, for App Store submission)"
            echo "  simulator   - iOS Simulator only (arm64, for testing)"
            echo "  universal   - Fat binary with both architectures (default, for development)"
            echo "  both        - Build ios and simulator separately (no combining)"
            echo ""
            echo "Examples:"
            echo "  $0 libz                          # Build universal (default)"
            echo "  $0 libz --target ios             # Device only"
            echo "  $0 libz --target simulator       # Simulator only"
            echo "  $0 libz --target universal       # Combined fat binary"
            echo "  $0 libz --target both            # Both ios + simulator separate"
            exit 0
            ;;
        -*)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
        *)
            if [ -z "$PKG_NAME" ]; then
                PKG_NAME="$1"
            else
                echo "Unexpected argument: $1"
                exit 1
            fi
            shift
            ;;
    esac
done

if [ -z "$PKG_NAME" ]; then
    echo "Usage: $0 <package-name> [--target ios|simulator|universal|both]"
    echo "Example: $0 libz --target universal"
    exit 1
fi

# Validate target
case "$TARGET" in
    ios|simulator|universal|both)
        ;;
    *)
        echo "Error: Invalid target '$TARGET'"
        echo "Valid targets: ios, simulator, universal, both"
        exit 1
        ;;
esac

# Source build library
source "$SCRIPT_DIR/a_shell_package.sh"

# Function to build a single target
build_single_target() {
    local target="$1"
    
    echo ""
    echo "=========================================="
    echo "Building $PKG_NAME for target: $target"
    echo "=========================================="
    
    # Set target
    export A_SHELL_BUILD_TARGET="$target"
    
    # Find package build script
    PKG_DIR="$SCRIPT_DIR/../packages/core/$PKG_NAME"
    if [ ! -f "$PKG_DIR/build.sh" ]; then
        a_shell_error "Package $PKG_NAME not found at $PKG_DIR/build.sh"
    fi
    
    # Set paths based on target
    a_shell_set_target_paths "$target"
    
    # Setup
    a_shell_setup_directories
    a_shell_setup_toolchain
    
    # Source package build script
    # Unset CPPFLAGS to avoid injecting kernel headers that break builds
    unset CPPFLAGS
    source "$PKG_DIR/build.sh"
    
    a_shell_info "Building package: $PKG_NAME (target: $target)"
    
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
    
    # Apply patches
    a_shell_apply_patches
    
    # Configure
    a_shell_step "Configuring..."
    a_shell_pkg_configure
    
    # Build
    a_shell_step "Building..."
    a_shell_pkg_make
    
    # Install
    a_shell_step "Installing..."
    a_shell_pkg_install
    
    # Package (only for non-simulator)
    if [ "$target" != "simulator" ]; then
        a_shell_step_package
    fi
    
    a_shell_info "Build complete: $PKG_NAME (target: $target)"
    a_shell_info "Output: $A_SHELL_PKG_STAGING"
}

# Main build logic based on target
case "$TARGET" in
    ios|simulator)
        # Single target build
        build_single_target "$TARGET"
        
        echo ""
        echo "=========================================="
        echo "Build Summary"
        echo "=========================================="
        echo "Package: $PKG_NAME"
        echo "Target: $TARGET"
        echo "Output: .build/$TARGET/"
        echo ""
        
        # Show what was built
        if [ -d ".build/$TARGET/staging/usr/local/lib" ]; then
            echo "Libraries:"
            ls -la ".build/$TARGET/staging/usr/local/lib/" 2>/dev/null | grep -E "^-" | head -10 || true
        fi
        if [ -d ".build/$TARGET/staging/usr/local/bin" ]; then
            echo ""
            echo "Binaries:"
            ls -la ".build/$TARGET/staging/usr/local/bin/" 2>/dev/null | head -10 || true
        fi
        ;;
        
    universal)
        # Build ios + simulator, then combine
        echo "Building universal binary (ios + simulator)..."
        
        # Build iOS first
        build_single_target "ios"
        
        # Build simulator
        build_single_target "simulator"
        
        # Create universal binary
        export A_SHELL_BUILD_TARGET="universal"
        a_shell_set_target_paths "universal"
        a_shell_create_universal_binary
        
        echo ""
        echo "=========================================="
        echo "Universal Build Complete"
        echo "=========================================="
        echo "Package: $PKG_NAME"
        echo "Output: .build/universal/"
        echo ""
        echo "Architecture support:"
        if [ -f ".build/universal/staging/usr/local/lib/lib${PKG_NAME}.a" ]; then
            lipo -info ".build/universal/staging/usr/local/lib/lib${PKG_NAME}.a" 2>/dev/null || true
        fi
        echo ""
        echo "Components:"
        echo "  iOS:      .build/ios/"
        echo "  Simulator:.build/simulator/"
        echo "  Universal:.build/universal/"
        ;;
        
    both)
        # Build ios and simulator separately (no combining)
        echo "Building both ios and simulator targets..."
        
        # Build iOS
        build_single_target "ios"
        
        # Build simulator
        build_single_target "simulator"
        
        echo ""
        echo "=========================================="
        echo "Both Builds Complete"
        echo "=========================================="
        echo "Package: $PKG_NAME"
        echo ""
        echo "Outputs:"
        echo "  iOS:      .build/ios/"
        echo "  Simulator:.build/simulator/"
        echo ""
        echo "Note: Use --target universal to create combined fat binaries"
        ;;
esac

echo ""
echo "=========================================="
echo "Build finished successfully!"
echo "=========================================="

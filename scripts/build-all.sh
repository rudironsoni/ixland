#!/bin/bash
# build-all.sh - Build all core packages for a-Shell
# Supports multiple targets: ios, simulator, universal, both
#
# Usage: ./build-all.sh [--target ios|simulator|universal|both]
#
# Targets:
#   ios         - iOS device only (for App Store submission)
#   simulator   - iOS Simulator only (for testing)
#   universal   - Fat binaries (default, for development)
#   both        - Build ios and simulator separately

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="$SCRIPT_DIR/../packages/core"
TARGET="universal"  # Default to universal for development

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --target|-t)
            TARGET="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [--target ios|simulator|universal|both]"
            echo ""
            echo "Build all core packages for a-Shell"
            echo ""
            echo "Targets:"
            echo "  ios         - iOS device only (arm64, for App Store submission)"
            echo "  simulator   - iOS Simulator only (arm64, for testing)"
            echo "  universal   - Fat binaries with both architectures (default, for development)"
            echo "  both        - Build ios and simulator separately (no combining)"
            echo ""
            echo "Examples:"
            echo "  $0                          # Build universal (default)"
            echo "  $0 --target ios             # Device only"
            echo "  $0 --target simulator       # Simulator only"
            echo "  $0 --target universal       # Combined fat binaries"
            echo "  $0 --target both            # Both ios + simulator separate"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

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

# Track which packages built successfully
BUILT_PACKAGES=()
FAILED_PACKAGES=()

build_package() {
    local pkg=$1
    local desc=$2
    local target=$3
    
    echo ""
    echo "=========================================="
    echo "Building: $pkg"
    echo "Description: $desc"
    echo "Target: $target"
    echo "=========================================="
    
    if [ -f "$PACKAGES_DIR/$pkg/build.sh" ]; then
        if "$SCRIPT_DIR/build-package.sh" "$pkg" --target "$target"; then
            BUILT_PACKAGES+=("$pkg ($target)")
            echo "✓ $pkg built successfully ($target)"
        else
            FAILED_PACKAGES+=("$pkg ($target)")
            echo "✗ $pkg failed to build ($target)"
            # Continue with next package
        fi
    else
        echo "✗ $pkg: No build.sh found"
        FAILED_PACKAGES+=("$pkg ($target)")
    fi
}

echo "=========================================="
echo "a-Shell Core Packages Build"
echo "=========================================="
echo "Target: $TARGET"
echo ""

# Build based on target
case "$TARGET" in
    ios|simulator)
        # Single target - build all packages
        echo ""
        echo "Wave 1: Foundation packages"
        echo "---------------------------"
        build_package "libz" "zlib compression library" "$TARGET"
        
        echo ""
        echo "Wave 2: Core shell"
        echo "-------------------"
        build_package "bash-minimal" "GNU Bourne Again Shell (minimal)" "$TARGET"
        
        echo ""
        echo "Wave 3: Essential utilities"
        echo "-------------------------"
        build_package "coreutils" "GNU Core Utilities (ls, cp, mv, cat, etc.)" "$TARGET"
        ;;
        
    universal)
        # Universal - each package builds both ios + simulator, then combines
        echo ""
        echo "Wave 1: Foundation packages"
        echo "---------------------------"
        build_package "libz" "zlib compression library" "universal"
        
        echo ""
        echo "Wave 2: Core shell"
        echo "-------------------"
        build_package "bash-minimal" "GNU Bourne Again Shell (minimal)" "universal"
        
        echo ""
        echo "Wave 3: Essential utilities"
        echo "-------------------------"
        build_package "coreutils" "GNU Core Utilities (ls, cp, mv, cat, etc.)" "universal"
        ;;
        
    both)
        # Both - build ios, then build simulator for each package
        
        # Build all packages for iOS first
        echo ""
        echo "=========================================="
        echo "Phase 1: Building for iOS Device"
        echo "=========================================="
        
        echo ""
        echo "Wave 1: Foundation packages"
        echo "---------------------------"
        build_package "libz" "zlib compression library" "ios"
        
        echo ""
        echo "Wave 2: Core shell"
        echo "-------------------"
        build_package "bash-minimal" "GNU Bourne Again Shell (minimal)" "ios"
        
        echo ""
        echo "Wave 3: Essential utilities"
        echo "-------------------------"
        build_package "coreutils" "GNU Core Utilities (ls, cp, mv, cat, etc.)" "ios"
        
        # Then build all packages for Simulator
        echo ""
        echo "=========================================="
        echo "Phase 2: Building for iOS Simulator"
        echo "=========================================="
        
        echo ""
        echo "Wave 1: Foundation packages"
        echo "---------------------------"
        build_package "libz" "zlib compression library" "simulator"
        
        echo ""
        echo "Wave 2: Core shell"
        echo "-------------------"
        build_package "bash-minimal" "GNU Bourne Again Shell (minimal)" "simulator"
        
        echo ""
        echo "Wave 3: Essential utilities"
        echo "-------------------------"
        build_package "coreutils" "GNU Core Utilities (ls, cp, mv, cat, etc.)" "simulator"
        ;;
esac

# Summary
echo ""
echo "=========================================="
echo "Build Summary"
echo "=========================================="
echo "Target: $TARGET"
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
    
    # Show output locations based on target
    case "$TARGET" in
        ios)
            echo "Output location: .build/ios/staging/usr/local/"
            echo "  Binaries: .build/ios/staging/usr/local/bin/"
            echo "  Libraries: .build/ios/staging/usr/local/lib/"
            ;;
        simulator)
            echo "Output location: .build/simulator/staging/usr/local/"
            echo "  Binaries: .build/simulator/staging/usr/local/bin/"
            echo "  Libraries: .build/simulator/staging/usr/local/lib/"
            ;;
        universal)
            echo "Output locations:"
            echo "  Universal: .build/universal/staging/usr/local/"
            echo "  iOS:       .build/ios/staging/usr/local/"
            echo "  Simulator: .build/simulator/staging/usr/local/"
            ;;
        both)
            echo "Output locations:"
            echo "  iOS:       .build/ios/staging/usr/local/"
            echo "  Simulator: .build/simulator/staging/usr/local/"
            echo ""
            echo "Note: Use --target universal to create combined fat binaries"
            ;;
    esac
    echo ""
    
    # Show universal binary info if applicable
    if [ "$TARGET" = "universal" ]; then
        echo "Universal binary architecture info:"
        for lib in .build/universal/staging/usr/local/lib/*.a; do
            if [ -f "$lib" ]; then
                echo "  $(basename $lib):"
                lipo -info "$lib" 2>/dev/null | sed 's/^/    /' || true
            fi
        done
    fi
fi

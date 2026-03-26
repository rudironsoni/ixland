#!/bin/bash
# Build ixland-system for iOS
# Usage: ./scripts/build-kernel.sh [arm64|simulator|all]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

# Default to all architectures
ARCH=${1:-all}

echo "=========================================="
echo "Building ixland-system"
echo "Architecture: $ARCH"
echo "=========================================="

# Check for Xcode
if ! command -v xcodebuild &> /dev/null; then
    echo "ERROR: Xcode not found. Please install Xcode."
    exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"

cd "$PROJECT_ROOT/ixland-system"

# Build for iOS device (arm64)
if [[ "$ARCH" == "arm64" || "$ARCH" == "all" ]]; then
    echo ""
    echo "Building for iOS arm64..."
    
    xcodebuild \
        -project a_shell_system.xcodeproj \
        -scheme a_shell_system \
        -destination 'generic/platform=iOS' \
        -configuration Release \
        -derivedDataPath "$BUILD_DIR/DerivedData/ios" \
        clean build \
        || echo "iOS arm64 build may require proper signing setup"
fi

# Build for iOS Simulator (x86_64)
if [[ "$ARCH" == "simulator" || "$ARCH" == "all" ]]; then
    echo ""
    echo "Building for iOS Simulator x86_64..."
    
    xcodebuild \
        -project a_shell_system.xcodeproj \
        -scheme a_shell_system \
        -destination 'generic/platform=iOS Simulator' \
        -configuration Release \
        -derivedDataPath "$BUILD_DIR/DerivedData/simulator" \
        clean build \
        || echo "Simulator build may require proper signing setup"
fi

# Create XCFramework
if [[ "$ARCH" == "all" ]]; then
    echo ""
    echo "Creating XCFramework..."
    
    IOS_FRAMEWORK="$BUILD_DIR/DerivedData/ios/Build/Products/Release-iphoneos/a_shell_system.framework"
    SIM_FRAMEWORK="$BUILD_DIR/DerivedData/simulator/Build/Products/Release-iphonesimulator/a_shell_system.framework"
    
    if [[ -d "$IOS_FRAMEWORK" && -d "$SIM_FRAMEWORK" ]]; then
        xcodebuild -create-xcframework \
            -framework "$IOS_FRAMEWORK" \
            -framework "$SIM_FRAMEWORK" \
            -output "$BUILD_DIR/ixland-system.xcframework"
        echo "XCFramework created at: $BUILD_DIR/ixland-system.xcframework"
    else
        echo "WARNING: Frameworks not found. Build may have failed."
        echo "iOS Framework: $IOS_FRAMEWORK"
        echo "Simulator Framework: $SIM_FRAMEWORK"
    fi
fi

echo ""
echo "=========================================="
echo "Build complete"
echo "=========================================="

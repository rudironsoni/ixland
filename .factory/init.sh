#!/bin/bash
# Factory init script for iXland
# Sets up the build environment for workers

set -e

echo "=== iXland Factory Init ==="

# Check prerequisites
echo "Checking prerequisites..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Please install CMake 3.20+"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -1 | awk '{print $3}')
echo "  CMake: $CMAKE_VERSION"

# Check for Xcode/clang
if ! command -v clang &> /dev/null; then
    echo "ERROR: clang not found. Please install Xcode"
    exit 1
fi

CLANG_VERSION=$(clang --version | head -1)
echo "  Clang: $CLANG_VERSION"

# Check for iOS SDK
if ! xcrun --sdk iphonesimulator --show-sdk-path &> /dev/null; then
    echo "WARNING: iOS Simulator SDK not found"
else
    SIM_SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)
    echo "  iOS Simulator SDK: $SIM_SDK"
fi

# Verify scripts exist
echo "Checking scripts..."
if [ -f "./scripts/lint.sh" ]; then
    echo "  lint.sh: OK"
else
    echo "  lint.sh: NOT FOUND"
fi

if [ -f "./scripts/format.sh" ]; then
    echo "  format.sh: OK"
else
    echo "  format.sh: NOT FOUND"
fi

# Optional: Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo ""
    echo "Build directory doesn't exist. Run 'factory build configure' to set up."
fi

echo ""
echo "=== Init Complete ==="

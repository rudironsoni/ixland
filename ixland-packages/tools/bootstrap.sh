#!/bin/bash
# iox Bootstrap Script
# Fresh clone setup

set -e

echo "=== iox Bootstrap ==="
echo ""

# Check prerequisites
echo "Checking prerequisites..."

if ! command -v cmake &>/dev/null; then
    echo "ERROR: cmake is required but not installed"
    exit 1
fi

if ! command -v xcodebuild &>/dev/null; then
    echo "ERROR: Xcode command line tools are required"
    exit 1
fi

# Check iOS SDKs
if ! xcrun --sdk iphonesimulator --show-sdk-path &>/dev/null; then
    echo "ERROR: iOS Simulator SDK not found"
    exit 1
fi

echo "✓ Prerequisites OK"
echo ""

# Initialize submodules
echo "Initializing submodules..."
if [ -d .git ]; then
    git submodule update --init --recursive || true
fi
echo "✓ Submodules initialized"
echo ""

# Run doctor
echo "Running doctor..."
if [ -f tools/doctor.sh ]; then
    bash tools/doctor.sh
fi

echo ""
echo "Bootstrap complete!"
echo ""
echo "Next steps:"
echo "  cmake --preset ios-simulator-debug"
echo "  cmake --build --preset ios-simulator-debug"
echo "  ctest --preset ios-simulator-test"

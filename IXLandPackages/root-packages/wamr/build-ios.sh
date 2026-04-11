#!/bin/bash
# WAMR iOS Build Script
# Creates XCFramework for iOS device and simulator

set -e

WAMR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/wamr"
BUILD_DIR="${WAMR_DIR}/build-ios"
OUTPUT_DIR="${WAMR_DIR}/.dist"

echo "========================================"
echo "WAMR iOS Build"
echo "========================================"

# Check if WAMR submodule exists
if [ ! -d "${WAMR_DIR}/core" ]; then
    echo "Error: WAMR submodule not found at ${WAMR_DIR}"
    echo "Please run: git submodule update --init a-shell-kernel/wamr"
    exit 1
fi

# Clean previous builds
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
mkdir -p "${OUTPUT_DIR}"

# Build for iOS Device (arm64)
echo ""
echo "Building WAMR for iOS Device (arm64)..."
mkdir -p "${BUILD_DIR}/ios-arm64"
cd "${BUILD_DIR}/ios-arm64"

cmake "${WAMR_DIR}" \
    -DWAMR_BUILD_PLATFORM=ios \
    -DWAMR_BUILD_AOT=1 \
    -DWAMR_BUILD_INTERP=1 \
    -DWAMR_BUILD_FAST_INTERP=1 \
    -DWAMR_BUILD_LIBC_WASI=1 \
    -DWAMR_BUILD_LIBC_BUILTIN=1 \
    -DWAMR_BUILD_MULTI_MODULE=1 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${BUILD_DIR}/ios-arm64/install"

make -j$(sysctl -n hw.ncpu)
make install

echo "✓ iOS Device build complete"

# Build for iOS Simulator (arm64)
echo ""
echo "Building WAMR for iOS Simulator (arm64)..."
mkdir -p "${BUILD_DIR}/simulator-arm64"
cd "${BUILD_DIR}/simulator-arm64"

cmake "${WAMR_DIR}" \
    -DWAMR_BUILD_PLATFORM=ios \
    -DWAMR_BUILD_AOT=1 \
    -DWAMR_BUILD_INTERP=1 \
    -DWAMR_BUILD_FAST_INTERP=1 \
    -DWAMR_BUILD_LIBC_WASI=1 \
    -DWAMR_BUILD_LIBC_BUILTIN=1 \
    -DWAMR_BUILD_MULTI_MODULE=1 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=$(xcrun -sdk iphonesimulator --show-sdk-path) \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${BUILD_DIR}/simulator-arm64/install"

make -j$(sysctl -n hw.ncpu)
make install

echo "✓ Simulator arm64 build complete"

# Build for iOS Simulator (x86_64)
echo ""
echo "Building WAMR for iOS Simulator (x86_64)..."
mkdir -p "${BUILD_DIR}/simulator-x86_64"
cd "${BUILD_DIR}/simulator-x86_64"

cmake "${WAMR_DIR}" \
    -DWAMR_BUILD_PLATFORM=ios \
    -DWAMR_BUILD_AOT=1 \
    -DWAMR_BUILD_INTERP=1 \
    -DWAMR_BUILD_FAST_INTERP=1 \
    -DWAMR_BUILD_LIBC_WASI=1 \
    -DWAMR_BUILD_LIBC_BUILTIN=1 \
    -DWAMR_BUILD_MULTI_MODULE=1 \
    -DCMAKE_OSX_ARCHITECTURES=x86_64 \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=$(xcrun -sdk iphonesimulator --show-sdk-path) \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${BUILD_DIR}/simulator-x86_64/install"

make -j$(sysctl -n hw.ncpu)
make install

echo "✓ Simulator x86_64 build complete"

# Create XCFramework
echo ""
echo "Creating XCFramework..."
cd "${WAMR_DIR}"

# Create universal library for simulator
lipo -create \
    "${BUILD_DIR}/simulator-arm64/libvmlib.a" \
    "${BUILD_DIR}/simulator-x86_64/libvmlib.a" \
    -output "${BUILD_DIR}/simulator-universal/libvmlib.a"

# Create XCFramework
xcodebuild -create-xcframework \
    -library "${BUILD_DIR}/ios-arm64/libvmlib.a" \
    -headers "${WAMR_DIR}/core/iwasm/include" \
    -library "${BUILD_DIR}/simulator-universal/libvmlib.a" \
    -headers "${WAMR_DIR}/core/iwasm/include" \
    -output "${OUTPUT_DIR}/wamr.xcframework"

echo ""
echo "========================================"
echo "WAMR XCFramework created successfully!"
echo "========================================"
echo "Location: ${OUTPUT_DIR}/wamr.xcframework"
echo ""
echo "Platforms:"
echo "  - iOS arm64 (device)"
echo "  - iOS arm64 + x86_64 (simulator)"

#!/bin/bash
#
# Build WAMR for iOS Device and Simulator
# Creates libwamr.a for linking with libiox

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "Building WAMR for iOS"
echo "========================================"

# Detect SDK paths
IOS_SDK=$(xcrun --sdk iphoneos --show-sdk-path)
SIM_SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)

if [ -z "$IOS_SDK" ] || [ -z "$SIM_SDK" ]; then
    echo -e "${RED}ERROR: iOS SDKs not found. Install Xcode.${NC}"
    exit 1
fi

echo "iOS SDK: $IOS_SDK"
echo "Sim SDK: $SIM_SDK"

# Directories
WAMR_DIR="deps/wamr"
BUILD_DIR="build/wamr"
INSTALL_DIR="lib/wamr"

# Clean previous builds
rm -rf "$BUILD_DIR" "$INSTALL_DIR"
mkdir -p "$BUILD_DIR/device" "$BUILD_DIR/simulator" "$INSTALL_DIR"

# Ensure we build static libraries, not shared
CMAKE_SHARED="-DBUILD_SHARED_LIBS=OFF"

# Architecture detection
ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ]; then
    SIM_ARCH="arm64"
else
    SIM_ARCH="x86_64"
fi

echo ""
echo "Building for iOS Simulator ($SIM_ARCH)..."
cd "$BUILD_DIR/simulator"
cmake ../../../$WAMR_DIR/product-mini/platforms/ios \
    -DIOS_PLATFORM=SIMULATOR \
    -DCMAKE_OSX_SYSROOT="$SIM_SDK" \
    -DCMAKE_OSX_ARCHITECTURES="$SIM_ARCH" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DWAMR_BUILD_PLATFORM="ios" \
    -DWAMR_BUILD_TARGET="$SIM_ARCH" \
    -DWAMR_BUILD_INTERP=1 \
    -DWAMR_BUILD_FAST_INTERP=0 \
    -DWAMR_BUILD_AOT=0 \
    -DWAMR_BUILD_JIT=0 \
    -DWAMR_BUILD_LIBC_WASI=1 \
    -DWAMR_BUILD_LIBC_UVWASI=0 \
    -DWAMR_BUILD_LIB_PTHREAD=0 \
    -DWAMR_BUILD_MULTI_MODULE=0 \
    -DWAMR_BUILD_MINI_LOADER=0 \
    -DWAMR_BUILD_SIMD=0 \
    -DWAMR_BUILD_DEBUG_INTERP=0 \
    -DWAMR_BUILD_DEBUG_AOT=0 \
    -DWAMR_BUILD_MEMORY_PROFILING=0 \
    2>&1 | tail -20

make -j$(sysctl -n hw.ncpu) 2>&1 | tail -10
cd ../../..

echo -e "${GREEN}✓ Simulator build complete${NC}"

echo ""
echo "Building for iOS Device (arm64)..."
cd "$BUILD_DIR/device"
cmake ../../../$WAMR_DIR/product-mini/platforms/ios \
    -DIOS_PLATFORM=OS \
    -DCMAKE_OSX_SYSROOT="$IOS_SDK" \
    -DCMAKE_OSX_ARCHITECTURES="arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DWAMR_BUILD_PLATFORM="ios" \
    -DWAMR_BUILD_TARGET="AARCH64" \
    -DWAMR_BUILD_INTERP=1 \
    -DWAMR_BUILD_FAST_INTERP=0 \
    -DWAMR_BUILD_AOT=0 \
    -DWAMR_BUILD_JIT=0 \
    -DWAMR_BUILD_LIBC_WASI=1 \
    -DWAMR_BUILD_LIBC_UVWASI=0 \
    -DWAMR_BUILD_LIB_PTHREAD=0 \
    -DWAMR_BUILD_MULTI_MODULE=0 \
    -DWAMR_BUILD_MINI_LOADER=0 \
    -DWAMR_BUILD_SIMD=0 \
    -DWAMR_BUILD_DEBUG_INTERP=0 \
    -DWAMR_BUILD_DEBUG_AOT=0 \
    -DWAMR_BUILD_MEMORY_PROFILING=0 \
    2>&1 | tail -20

make -j$(sysctl -n hw.ncpu) 2>&1 | tail -10
cd ../../..

echo -e "${GREEN}✓ Device build complete${NC}"

echo ""
echo "Creating universal binary..."
mkdir -p "$INSTALL_DIR"

# Create fat library
lipo -create \
    "$BUILD_DIR/simulator/iwasm/libiwasm.a" \
    "$BUILD_DIR/device/iwasm/libiwasm.a" \
    -output "$INSTALL_DIR/libwamr.a"

echo -e "${GREEN}✓ Created libwamr.a${NC}"

# Copy headers
mkdir -p "$INSTALL_DIR/include"
cp "$WAMR_DIR/core/iwasm/include/wasm_export.h" "$INSTALL_DIR/include/"
cp "$WAMR_DIR/core/iwasm/include/libc_wasi.h" "$INSTALL_DIR/include/"

echo ""
echo "========================================"
echo -e "${GREEN}WAMR build complete!${NC}"
echo "========================================"
echo "Library: $INSTALL_DIR/libwamr.a"
echo "Headers: $INSTALL_DIR/include/"
echo ""

# Show architectures
lipo -info "$INSTALL_DIR/libwamr.a"

# Show size
ls -lh "$INSTALL_DIR/libwamr.a"

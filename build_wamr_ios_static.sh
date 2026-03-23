#!/bin/bash
# Build WAMR as static library for iOS
# Creates libiwasm.a for both device and simulator

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building WAMR static libraries for iOS...${NC}"

# Detect iOS SDKs
IOS_SDK=$(xcrun --sdk iphoneos --show-sdk-path)
SIM_SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)

if [ -z "$IOS_SDK" ] || [ -z "$SIM_SDK" ]; then
    echo -e "${RED}Error: iOS SDKs not found. Install Xcode.${NC}"
    exit 1
fi

echo "iOS SDK: $IOS_SDK"
echo "Sim SDK: $SIM_SDK"

# Architecture
ARCH="arm64"

echo ""
echo -e "${YELLOW}=== Building for iOS Device (${ARCH}) ===${NC}"

# Create build directory
mkdir -p build/wamr-device
cd build/wamr-device

# Configure with CMake for static library
# Note: Remove -fembed-bitcode as it conflicts with -ffunction-sections
cmake ../../deps/wamr/product-mini/platforms/ios \
    -DCMAKE_OSX_ARCHITECTURES=${ARCH} \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
    -DCMAKE_OSX_SYSROOT=${IOS_SDK} \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DWAMR_BUILD_AOT=0 \
    -DWAMR_BUILD_JIT=0 \
    -DWAMR_BUILD_FAST_JIT=0 \
    -DWAMR_BUILD_LIBC_WASI=1 \
    -DWAMR_BUILD_LIBC_BUILTIN=1 \
    -DWAMR_BUILD_LIB_PTHREAD=1 \
    -DWAMR_BUILD_SIMD=1

# Build
make -j$(sysctl -n hw.ncpu)

# Rename to static library if needed
if [ -f "distribution/wasm/lib/libiwasm.dylib" ]; then
    echo -e "${YELLOW}Converting to static library...${NC}"
    # Extract object files and recreate as static
    mkdir -p tmp_extract
    cd tmp_extract
    ar -x ../distribution/wasm/lib/libiwasm.dylib 2>/dev/null || \
        otool -l ../distribution/wasm/lib/libiwasm.dylib | grep -q "MH_OBJECT"
    
    # If it's actually a static library disguised as dylib
    if [ $? -eq 0 ]; then
        # Find all .o files
        find . -name "*.o" -exec cp {} . \; 2>/dev/null || true
        ar rcs ../libiwasm-device.a *.o 2>/dev/null || true
    fi
    cd ..
fi

# Copy headers
mkdir -p ../../include/wamr
cp -r distribution/wasm/include/* ../../include/wamr/ 2>/dev/null || true

echo -e "${GREEN}iOS Device build complete${NC}"

cd ../..

# Build for Simulator
echo ""
echo -e "${YELLOW}=== Building for iOS Simulator (${ARCH}) ===${NC}"

mkdir -p build/wamr-simulator
cd build/wamr-simulator

# Configure for simulator - note: remove -mno-thumb if present
cmake ../../deps/wamr/product-mini/platforms/ios \
    -DCMAKE_OSX_ARCHITECTURES=${ARCH} \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
    -DCMAKE_OSX_SYSROOT=${SIM_SDK} \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DWAMR_BUILD_AOT=0 \
    -DWAMR_BUILD_JIT=0 \
    -DWAMR_BUILD_FAST_JIT=0 \
    -DWAMR_BUILD_LIBC_WASI=1 \
    -DWAMR_BUILD_LIBC_BUILTIN=1 \
    -DWAMR_BUILD_LIB_PTHREAD=1 \
    -DWAMR_BUILD_SIMD=1

# Build
make -j$(sysctl -n hw.ncpu) 2>&1 | grep -v "argument unused during compilation" || true

echo -e "${GREEN}iOS Simulator build complete${NC}"

cd ../..

echo ""
echo -e "${GREEN}=== Build Summary ===${NC}"
echo "Device library: build/wamr-device/"
echo "Simulator library: build/wamr-simulator/"
echo "Headers: include/wamr/"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "1. Check the output directories for libiwasm files"
echo "2. If .dylib files were created, they need to be converted to .a"
echo "3. Link against libiox when building your iOS app"

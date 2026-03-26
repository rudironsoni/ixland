#!/bin/bash
# Build upstream WAMR out-of-tree for iOS.
#
# Policy:
# - deps/wamr is treated as read-only upstream source
# - no local edits or fork assumptions
# - iOS toolchain/config is owned in this repository only

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE_DIR="$ROOT_DIR/deps/wamr/product-mini/platforms/ios"
DEVICE_BUILD_DIR="$ROOT_DIR/build/wamr-device"
SIM_BUILD_DIR="$ROOT_DIR/build/wamr-simulator"

MODE="${1:-all}"

IOS_SDK="$(xcrun --sdk iphoneos --show-sdk-path)"
SIM_SDK="$(xcrun --sdk iphonesimulator --show-sdk-path)"
UNAME_M="$(uname -m)"

if [[ "$UNAME_M" == "arm64" ]]; then
    SIM_ARCH="arm64"
else
    SIM_ARCH="x86_64"
fi

COMMON_ARGS=(
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_SHARED_LIBS=OFF
    -DWAMR_BUILD_AOT=0
    -DWAMR_BUILD_JIT=0
    -DWAMR_BUILD_FAST_JIT=0
    -DWAMR_BUILD_LIBC_WASI=1
    -DWAMR_BUILD_LIBC_BUILTIN=1
    -DWAMR_BUILD_LIB_PTHREAD=1
    -DWAMR_BUILD_SIMD=1
)

build_target() {
    local build_dir="$1"
    local sysroot="$2"
    local arch="$3"
    local variant="$4"

    mkdir -p "$build_dir"

    cmake -S "$SOURCE_DIR" -B "$build_dir" \
        -DCMAKE_OSX_ARCHITECTURES="$arch" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
        -DCMAKE_OSX_SYSROOT="$sysroot" \
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$build_dir" \
        "${COMMON_ARGS[@]}"

    cmake --build "$build_dir" --parallel "$(sysctl -n hw.ncpu)"

    if [[ ! -f "$build_dir/libiwasm.a" ]]; then
        echo "ERROR: Expected static library not found for $variant at $build_dir/libiwasm.a" >&2
        exit 1
    fi

    echo "Built $variant: $build_dir/libiwasm.a"
}

case "$MODE" in
    simulator)
        # On Apple Silicon Macs, iOS Simulator uses arm64 natively
        # No need for x86_64 slice since WAMR doesn't support it anyway
        build_target "$SIM_BUILD_DIR" "$SIM_SDK" "arm64" "iOS Simulator"
        ;;
    device)
        build_target "$DEVICE_BUILD_DIR" "$IOS_SDK" "arm64" "iOS Device"
        ;;
    all)
        build_target "$DEVICE_BUILD_DIR" "$IOS_SDK" "arm64" "iOS Device"
        build_target "$SIM_BUILD_DIR" "$SIM_SDK" "$SIM_ARCH" "iOS Simulator"
        ;;
    *)
        echo "Usage: $0 [device|simulator|all]" >&2
        exit 1
        ;;
esac

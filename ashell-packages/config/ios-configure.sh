#!/bin/bash
# iOS Cross-Compilation Configuration Script
# Usage: source ios-configure.sh [arch]
#
# Sets up environment variables for building with autotools/make
# for iOS targets (arm64 device, x86_64 simulator)
#
# Source: /home/rrj/src/github/rudironsoni/a-shell-next/ashell-packages/config/ios-configure.sh

set -e

# =============================================================================
# DETECT ENVIRONMENT
# =============================================================================

# Detect if running on macOS with Xcode
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "ERROR: iOS cross-compilation requires macOS with Xcode." >&2
    echo "This script cannot run on Linux." >&2
    return 1 2>/dev/null || exit 1
fi

if ! command -v xcrun &> /dev/null; then
    echo "ERROR: xcrun not found. Install Xcode from the App Store." >&2
    return 1 2>/dev/null || exit 1
fi

# =============================================================================
# ARCHITECTURE SELECTION
# =============================================================================

# Default to arm64 device
ASHELL_IOS_ARCH="${1:-arm64}"

case "$ASHELL_IOS_ARCH" in
    arm64)
        ASHELL_IOS_SDK="iphoneos"
        ASHELL_IOS_TRIPLE="arm64-apple-ios"
        ;;
    x86_64)
        ASHELL_IOS_SDK="iphonesimulator"
        ASHELL_IOS_TRIPLE="x86_64-apple-ios"
        ;;
    arm64_sim)
        ASHELL_IOS_SDK="iphonesimulator"
        ASHELL_IOS_TRIPLE="arm64-apple-ios"
        ;;
    *)
        echo "ERROR: Unknown architecture: $ASHELL_IOS_ARCH" >&2
        echo "Supported: arm64, x86_64, arm64_sim" >&2
        return 1 2>/dev/null || exit 1
        ;;
esac

# =============================================================================
# SDK DETECTION
# =============================================================================

echo "Configuring for iOS $ASHELL_IOS_ARCH..."

# Get SDK path
ASHELL_IOS_SDK_PATH=$(xcrun --sdk "$ASHELL_IOS_SDK" --show-sdk-path 2>/dev/null)
if [[ -z "$ASHELL_IOS_SDK_PATH" ]]; then
    echo "ERROR: iOS SDK not found for $ASHELL_IOS_SDK" >&2
    return 1 2>/dev/null || exit 1
fi

echo "  SDK: $ASHELL_IOS_SDK_PATH"

# Get deployment target (default to 16.0)
ASHELL_IOS_DEPLOYMENT_TARGET="${ASHELL_IOS_DEPLOYMENT_TARGET:-16.0}"

# =============================================================================
# COMPILER DETECTION
# =============================================================================

ASHELL_CC=$(xcrun --sdk "$ASHELL_IOS_SDK" --find clang 2>/dev/null)
ASHELL_CXX=$(xcrun --sdk "$ASHELL_IOS_SDK" --find clang++ 2>/dev/null)
ASHELL_AR=$(xcrun --sdk "$ASHELL_IOS_SDK" --find ar 2>/dev/null)
ASHELL_RANLIB=$(xcrun --sdk "$ASHELL_IOS_SDK" --find ranlib 2>/dev/null)
ASHELL_STRIP=$(xcrun --sdk "$ASHELL_IOS_SDK" --find strip 2>/dev/null)

if [[ -z "$ASHELL_CC" ]]; then
    echo "ERROR: Could not find clang for $ASHELL_IOS_SDK" >&2
    return 1 2>/dev/null || exit 1
fi

echo "  CC: $ASHELL_CC"
echo "  CXX: $ASHELL_CXX"

# =============================================================================
# SET ENVIRONMENT
# =============================================================================

export CC="$ASHELL_CC"
export CXX="$ASHELL_CXX"
export AR="$ASHELL_AR"
export RANLIB="$ASHELL_RANLIB"
export STRIP="$ASHELL_STRIP"

export CFLAGS="-arch $ASHELL_IOS_ARCH -isysroot $ASHELL_IOS_SDK_PATH -mios-version-min=$ASHELL_IOS_DEPLOYMENT_TARGET -fembed-bitcode"
export CXXFLAGS="-arch $ASHELL_IOS_ARCH -isysroot $ASHELL_IOS_SDK_PATH -mios-version-min=$ASHELL_IOS_DEPLOYMENT_TARGET -fembed-bitcode"
export LDFLAGS="-arch $ASHELL_IOS_ARCH -isysroot $ASHELL_IOS_SDK_PATH -mios-version-min=$ASHELL_IOS_DEPLOYMENT_TARGET"
export CPPFLAGS="-arch $ASHELL_IOS_ARCH -isysroot $ASHELL_IOS_SDK_PATH"

# =============================================================================
# AUTOTOOLS HINTS
# =============================================================================

# Tell autotools we're cross-compiling
export ac_cv_func_malloc_0_nonnull=yes
export ac_cv_func_realloc_0_nonnull=yes
export ac_cv_func_memcmp_working=yes

# Disable features not available on iOS
export ac_cv_func_fork=no
export ac_cv_func_vfork=no

# =============================================================================
# CONFIGURE OPTIONS
# =============================================================================

# Common configure options for iOS
ASHELL_IOS_CONFIGURE_OPTS=""
ASHELL_IOS_CONFIGURE_OPTS="$ASHELL_IOS_CONFIGURE_OPTS --host=$ASHELL_IOS_TRIPLE"
ASHELL_IOS_CONFIGURE_OPTS="$ASHELL_IOS_CONFIGURE_OPTS --enable-static"
ASHELL_IOS_CONFIGURE_OPTS="$ASHELL_IOS_CONFIGURE_OPTS --disable-shared"
ASHELL_IOS_CONFIGURE_OPTS="$ASHELL_IOS_CONFIGURE_OPTS --disable-nls"

export ASHELL_IOS_CONFIGURE_OPTS

# =============================================================================
# SUMMARY
# =============================================================================

echo ""
echo "iOS cross-compilation environment configured:"
echo "  Architecture: $ASHELL_IOS_ARCH"
echo "  SDK: $ASHELL_IOS_SDK"
echo "  Deployment Target: $ASHELL_IOS_DEPLOYMENT_TARGET"
echo "  Triple: $ASHELL_IOS_TRIPLE"
echo ""
echo "Environment variables set:"
echo "  CC, CXX, AR, RANLIB, STRIP"
echo "  CFLAGS, CXXFLAGS, LDFLAGS, CPPFLAGS"
echo ""
echo "Configure with:"
echo "  ./configure \$ASHELL_IOS_CONFIGURE_OPTS --prefix=YOUR_PREFIX"
echo ""

# =============================================================================
# USAGE EXAMPLE
# =============================================================================

: '
# Example build script usage:

source /path/to/ios-configure.sh arm64

./configure \
    --prefix=/your/install/path \
    $ASHELL_IOS_CONFIGURE_OPTS \
    --disable-some-ios-incompatible-feature

make -j$(sysctl -n hw.ncpu)
make install
'

#!/bin/bash
# a_shell_package.sh - Build library for a-Shell packages
# Termux-style build system for iOS

set -e
set -o pipefail

# =============================================================================
# CONFIGURATION
# =============================================================================

A_SHELL_PKG_VERSION="1.0.0"
A_SHELL_PKG_BUILDER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Default paths (can be overridden)
A_SHELL_PREFIX="${A_SHELL_PREFIX:-$HOME/Library/ashell}"
A_SHELL_CONFIG="${A_SHELL_CONFIG:-$HOME/Documents/.ashell}"
A_SHELL_TMPDIR="${A_SHELL_TMPDIR:-$A_SHELL_CONFIG/tmp}"
A_SHELL_PKG_TMPDIR="${A_SHELL_PKG_TMPDIR:-$A_SHELL_TMPDIR/build}"
A_SHELL_PKG_STAGING="${A_SHELL_PKG_STAGING:-$A_SHELL_PKG_TMPDIR/staging}"

# iOS SDK settings
IOS_SDK="${IOS_SDK:-iphoneos}"
IOS_ARCH="${IOS_ARCH:-arm64}"
IOS_VERSION="${IOS_VERSION:-16.0}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

a_shell_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

a_shell_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

a_shell_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
    exit 1
}

a_shell_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# =============================================================================
# SETUP FUNCTIONS
# =============================================================================

a_shell_setup_directories() {
    mkdir -p "$A_SHELL_PREFIX"/{bin,lib,include,etc,share,var}
    mkdir -p "$A_SHELL_CONFIG"/{etc,var/log,tmp}
    mkdir -p "$A_SHELL_PKG_TMPDIR"
    mkdir -p "$A_SHELL_PKG_STAGING"
}

a_shell_setup_toolchain() {
    a_shell_step "Setting up iOS toolchain..."
    
    # Detect Xcode path
    if [ -z "$XCODE_PATH" ]; then
        XCODE_PATH=$(xcode-select -p 2>/dev/null || echo "/Applications/Xcode.app/Contents/Developer")
    fi
    
    # Get SDK path
    export SDK_PATH=$(xcrun -sdk $IOS_SDK --show-sdk-path)
    
    # Set compiler with sysroot
    export CC="xcrun -sdk $IOS_SDK clang -arch $IOS_ARCH"
    export CXX="xcrun -sdk $IOS_SDK clang++ -arch $IOS_ARCH"
    export AR="xcrun -sdk $IOS_SDK ar"
    export RANLIB="xcrun -sdk $IOS_SDK ranlib"
    export LD="xcrun -sdk $IOS_SDK ld"
    
    # Compiler flags with proper sysroot
    export CFLAGS="-arch $IOS_ARCH -mios-version-min=$IOS_VERSION -isysroot $SDK_PATH -fembed-bitcode -O2"
    export CXXFLAGS="$CFLAGS"
    export LDFLAGS="-arch $IOS_ARCH -mios-version-min=$IOS_VERSION -isysroot $SDK_PATH"
    
    # Include paths (only if directories exist)
    if [ -d "$A_SHELL_PREFIX/include" ]; then
        export CFLAGS="$CFLAGS -I$A_SHELL_PREFIX/include"
        export CPPFLAGS="-I$A_SHELL_PREFIX/include $CPPFLAGS"
    fi
    
    if [ -d "$A_SHELL_PREFIX/lib" ]; then
        export LDFLAGS="$LDFLAGS -L$A_SHELL_PREFIX/lib"
    fi
    
    # Standard Unix paths
    export PREFIX="$A_SHELL_PREFIX"
    export PKG_CONFIG_PATH="$A_SHELL_PREFIX/lib/pkgconfig:$A_SHELL_PREFIX/share/pkgconfig"
    
    a_shell_info "Toolchain: $CC"
    a_shell_info "SDK: $SDK_PATH"
    a_shell_info "Prefix: $A_SHELL_PREFIX"
}

a_shell_setup_cache_variables() {
    # Cache variables for cross-compilation
    # Only disable what iOS truly forbids
    export ac_cv_func_mount=no
    export ac_cv_func_umount=no
    export ac_cv_func_reboot=no
    export ac_cv_func_mknod=no
    
    # Let configure know fork/etc. work (kernel provides)
    export ac_cv_func_fork=yes
    export ac_cv_func_vfork=yes
    export ac_cv_func_execve=yes
    export ac_cv_func_waitpid=yes
}

# =============================================================================
# DOWNLOAD FUNCTIONS
# =============================================================================

a_shell_download_and_extract() {
    a_shell_step "Downloading $A_SHELL_PKG_NAME $A_SHELL_PKG_VERSION..."
    
    local src_url="${A_SHELL_PKG_SRCURL}"
    local src_file="$(basename "$src_url")"
    local src_dir="$A_SHELL_PKG_TMPDIR/$A_SHELL_PKG_NAME-$A_SHELL_PKG_VERSION"
    
    mkdir -p "$A_SHELL_PKG_TMPDIR"
    
    if [ ! -f "$A_SHELL_PKG_TMPDIR/$src_file" ]; then
        a_shell_info "Downloading from $src_url"
        curl -L -o "$A_SHELL_PKG_TMPDIR/$src_file" "$src_url" || a_shell_error "Download failed"
    fi
    
    # Verify checksum if provided
    if [ -n "$A_SHELL_PKG_SHA256" ] && [ "$A_SHELL_PKG_SHA256" != "SKIP_CHECKSUM" ]; then
        local actual_checksum=$(shasum -a 256 "$A_SHELL_PKG_TMPDIR/$src_file" | cut -d' ' -f1)
        if [ "$actual_checksum" != "$A_SHELL_PKG_SHA256" ]; then
            a_shell_error "Checksum mismatch! Expected $A_SHELL_PKG_SHA256, got $actual_checksum"
        fi
        a_shell_info "Checksum verified"
    fi
    
    # Extract
    a_shell_info "Extracting..."
    cd "$A_SHELL_PKG_TMPDIR"
    case "$src_file" in
        *.tar.gz|*.tgz)
            tar -xzf "$src_file"
            ;;
        *.tar.bz2)
            tar -xjf "$src_file"
            ;;
        *.tar.xz)
            tar -xJf "$src_file"
            ;;
        *.zip)
            unzip -q "$src_file"
            ;;
        *)
            a_shell_error "Unknown archive format: $src_file"
            ;;
    esac
    
    # Verify extraction
    if [ ! -d "$src_dir" ]; then
        a_shell_error "Expected directory $src_dir not found after extraction"
    fi
    
    export A_SHELL_PKG_SRCDIR="$src_dir"
    a_shell_info "Source ready: $A_SHELL_PKG_SRCDIR"
}

# =============================================================================
# PATCH FUNCTIONS
# =============================================================================

a_shell_apply_patches() {
    a_shell_step "Applying patches..."
    
    local patches_dir="$A_SHELL_PKG_BUILDER_DIR/packages/core/$A_SHELL_PKG_NAME/patches"
    
    if [ ! -d "$patches_dir" ]; then
        a_shell_info "No patches to apply"
        return 0
    fi
    
    local patch_count=0
    for patch in "$patches_dir"/*.patch; do
        if [ -f "$patch" ]; then
            a_shell_info "Applying $(basename "$patch")"
            
            # Substitute placeholders
            sed -e "s%\@A_SHELL_PREFIX\@%${A_SHELL_PREFIX}%g" \
                -e "s%\@A_SHELL_CONFIG\@%${A_SHELL_CONFIG}%g" \
                "$patch" | patch --silent -p1 || a_shell_error "Patch failed: $(basename "$patch")"
            
            ((patch_count++))
        fi
    done
    
    a_shell_info "$patch_count patches applied"
}

# =============================================================================
# BUILD STEP FUNCTIONS
# =============================================================================

a_shell_step_pre_configure() {
    a_shell_setup_directories
    a_shell_setup_toolchain
    a_shell_setup_cache_variables
    
    # Package-specific pre-configure (override in build.sh)
    if type a_shell_pkg_pre_configure &>/dev/null; then
        a_shell_pkg_pre_configure
    fi
}

a_shell_step_configure() {
    a_shell_step "Configuring $A_SHELL_PKG_NAME..."
    cd "$A_SHELL_PKG_SRCDIR"
    
    if type a_shell_pkg_configure &>/dev/null; then
        a_shell_pkg_configure
    else
        # Default configure
        ./configure \
            --prefix="$A_SHELL_PREFIX" \
            --host="arm-apple-darwin" \
            --disable-nls \
            --disable-rpath \
            || a_shell_error "Configure failed"
    fi
}

a_shell_step_make() {
    a_shell_step "Building $A_SHELL_PKG_NAME..."
    cd "$A_SHELL_PKG_SRCDIR"
    
    if type a_shell_pkg_make &>/dev/null; then
        a_shell_pkg_make
    else
        make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4) \
            || a_shell_error "Build failed"
    fi
}

a_shell_step_install() {
    a_shell_step "Installing $A_SHELL_PKG_NAME..."
    cd "$A_SHELL_PKG_SRCDIR"
    
    # Clean staging
    rm -rf "$A_SHELL_PKG_STAGING"
    mkdir -p "$A_SHELL_PKG_STAGING"
    mkdir -p "$A_SHELL_PKG_STAGING/DEBIAN"
    
    if type a_shell_pkg_install &>/dev/null; then
        a_shell_pkg_install
    else
        make install DESTDIR="$A_SHELL_PKG_STAGING" \
            || a_shell_error "Install failed"
    fi
}

a_shell_step_package() {
    a_shell_step "Creating package..."
    
    # Create control file
    cat > "$A_SHELL_PKG_STAGING/DEBIAN/control" <<EOF
Package: $A_SHELL_PKG_NAME
Version: $A_SHELL_PKG_VERSION
Architecture: ios-arm64
Maintainer: a-Shell Team
Depends: ${A_SHELL_PKG_DEPENDS:-}
Section: ${A_SHELL_PKG_SECTION:-base}
Priority: ${A_SHELL_PKG_PRIORITY:-optional}
Homepage: ${A_SHELL_PKG_HOMEPAGE:-}
Description: ${A_SHELL_PKG_DESCRIPTION:-$A_SHELL_PKG_NAME package}
EOF
    
    # Build .deb
    local deb_file="${A_SHELL_PKG_NAME}_${A_SHELL_PKG_VERSION}_ios-arm64.deb"
    dpkg-deb --build "$A_SHELL_PKG_STAGING" "$A_SHELL_PKG_BUILDER_DIR/repos/pool/$deb_file" \
        || a_shell_error "Package creation failed"
    
    a_shell_info "Package created: $deb_file"
}

# =============================================================================
# MAIN BUILD FUNCTION
# =============================================================================

a_shell_build_package() {
    a_shell_step "Building $A_SHELL_PKG_NAME $A_SHELL_PKG_VERSION"
    
    # Load package configuration
    if [ -f "$A_SHELL_PKG_BUILDER_DIR/packages/core/$A_SHELL_PKG_NAME/build.sh" ]; then
        source "$A_SHELL_PKG_BUILDER_DIR/packages/core/$A_SHELL_PKG_NAME/build.sh"
    elif [ -f "$A_SHELL_PKG_BUILDER_DIR/packages/extra/$A_SHELL_PKG_NAME/build.sh" ]; then
        source "$A_SHELL_PKG_BUILDER_DIR/packages/extra/$A_SHELL_PKG_NAME/build.sh"
    else
        a_shell_error "No build.sh found for $A_SHELL_PKG_NAME"
    fi
    
    # Validate required variables
    [ -n "$A_SHELL_PKG_NAME" ] || a_shell_error "A_SHELL_PKG_NAME not set"
    [ -n "$A_SHELL_PKG_VERSION" ] || a_shell_error "A_SHELL_PKG_VERSION not set"
    [ -n "$A_SHELL_PKG_SRCURL" ] || a_shell_error "A_SHELL_PKG_SRCURL not set"
    
    # Run build steps
    a_shell_step_pre_configure
    a_shell_download_and_extract
    a_shell_apply_patches
    a_shell_step_configure
    a_shell_step_make
    a_shell_step_install
    a_shell_step_package
    
    a_shell_info "Build complete: $A_SHELL_PKG_NAME $A_SHELL_PKG_VERSION"
}

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

a_shell_download_package() {
    local pkg_name="$1"
    local version="$2"
    
    a_shell_step "Downloading $pkg_name $version..."
    
    # Download from repo
    local deb_url="https://packages.a-shell.dev/pool/main/${pkg_name:0:1}/$pkg_name/${pkg_name}_${version}_ios-arm64.deb"
    local target="$A_SHELL_PREFIX"
    
    curl -L "$deb_url" -o "$A_SHELL_TMPDIR/$pkg_name.deb" || a_shell_error "Download failed"
    
    # Extract
    dpkg-deb -x "$A_SHELL_TMPDIR/$pkg_name.deb" "$target" || a_shell_error "Extraction failed"
    
    a_shell_info "$pkg_name installed"
}

a_shell_update_packages() {
    a_shell_step "Updating package lists..."
    
    curl -L "https://packages.a-shell.dev/dists/stable/main/binary-ios-arm64/Packages.gz" \
        -o "$A_SHELL_CONFIG/var/cache/apt/Packages.gz" \
        || a_shell_error "Failed to update package list"
    
    gunzip -c "$A_SHELL_CONFIG/var/cache/apt/Packages.gz" > "$A_SHELL_CONFIG/var/cache/apt/Packages"
    
    a_shell_info "Package list updated"
}

# Run if executed directly
if [ "${BASH_SOURCE[0]}" = "${0}" ]; then
    if [ -z "$1" ]; then
        echo "Usage: $0 <package_name>"
        exit 1
    fi
    
    A_SHELL_PKG_NAME="$1"
    a_shell_build_package
fi

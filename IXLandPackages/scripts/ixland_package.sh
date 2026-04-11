#!/bin/bash
# ixland_package.sh - Build library for a-Shell packages
# Termux-style build system for iOS with multi-target support
# Supports: ios, simulator, universal, both

set -e
set -o pipefail

# =============================================================================
# CONFIGURATION
# =============================================================================

A_SHELL_PKG_VERSION="1.0.0"
A_SHELL_PKG_BUILDER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Build target: ios, simulator, universal, both
# Default: universal (for development)
A_SHELL_BUILD_TARGET="${A_SHELL_BUILD_TARGET:-universal}"

# Target-specific paths
# Paths are set based on target in ixland_set_target_paths()
A_SHELL_PREFIX="/usr/local"  # Target prefix on iOS
A_SHELL_PKG_BUILD_DIR=""
A_SHELL_PKG_TMPDIR=""
A_SHELL_PKG_STAGING=""

# iOS SDK settings (set per target)
IOS_SDK=""
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

ixland_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

ixland_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

ixland_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
    exit 1
}

ixland_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# =============================================================================
# TARGET CONFIGURATION
# =============================================================================

ixland_validate_target() {
    case "$A_SHELL_BUILD_TARGET" in
        ios|simulator|universal|both)
            return 0
            ;;
        *)
            ixland_error "Invalid target: $A_SHELL_BUILD_TARGET"
            echo "Valid targets: ios, simulator, universal, both" >&2
            exit 1
            ;;
    esac
}

ixland_set_target_paths() {
    local target="$1"
    
    case "$target" in
        ios)
            A_SHELL_PKG_BUILD_DIR="${A_SHELL_PKG_BUILDER_DIR}/.build/ios"
            IOS_SDK="iphoneos"
            ;;
        simulator)
            A_SHELL_PKG_BUILD_DIR="${A_SHELL_PKG_BUILDER_DIR}/.build/simulator"
            IOS_SDK="iphonesimulator"
            ;;
        universal)
            A_SHELL_PKG_BUILD_DIR="${A_SHELL_PKG_BUILDER_DIR}/.build/universal"
            # Universal uses ios settings by default, combines later
            IOS_SDK="iphoneos"
            ;;
        both)
            # Both is handled separately - shouldn't reach here
            ixland_error "'both' target should be handled by build-package.sh"
            ;;
    esac
    
    # Set derived paths
    A_SHELL_PKG_TMPDIR="${A_SHELL_PKG_BUILD_DIR}/tmp"
    A_SHELL_PKG_STAGING="${A_SHELL_PKG_BUILD_DIR}/staging"
}

ixland_get_sdk_path() {
    xcrun -sdk "$IOS_SDK" --show-sdk-path
}

# =============================================================================
# SETUP FUNCTIONS
# =============================================================================

ixland_setup_directories() {
    # Only create build directories, not target directories
    # Target directories will be created during install
    mkdir -p "$A_SHELL_PKG_TMPDIR"
    mkdir -p "$A_SHELL_PKG_STAGING"
}

ixland_setup_toolchain() {
    ixland_step "Setting up iOS toolchain for target: $A_SHELL_BUILD_TARGET"
    
    # Detect Xcode path
    if [ -z "$XCODE_PATH" ]; then
        XCODE_PATH=$(xcode-select -p 2>/dev/null || echo "/Applications/Xcode.app/Contents/Developer")
    fi
    
    # Get SDK path
    export SDK_PATH=$(ixland_get_sdk_path)
    
    # Determine minimum version flag based on SDK
    local min_version_flag="-mios-version-min"
    if [ "$IOS_SDK" = "iphonesimulator" ]; then
        min_version_flag="-mios-simulator-version-min"
    fi
    
    # Set compiler with sysroot
    export CC="xcrun -sdk $IOS_SDK clang -arch $IOS_ARCH"
    export CXX="xcrun -sdk $IOS_SDK clang++ -arch $IOS_ARCH"
    export AR="xcrun -sdk $IOS_SDK ar"
    export RANLIB="xcrun -sdk $IOS_SDK ranlib"
    export LD="xcrun -sdk $IOS_SDK ld"
    
    # Compiler flags with proper sysroot
    export CFLAGS="-arch $IOS_ARCH ${min_version_flag}=$IOS_VERSION -isysroot $SDK_PATH -fembed-bitcode -O2"
    export CXXFLAGS="$CFLAGS"
    export LDFLAGS="-arch $IOS_ARCH ${min_version_flag}=$IOS_VERSION -isysroot $SDK_PATH"
    
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
    
    ixland_info "Target: $A_SHELL_BUILD_TARGET"
    ixland_info "Toolchain: $CC"
    ixland_info "SDK: $SDK_PATH"
    ixland_info "Prefix: $A_SHELL_PREFIX"
}

ixland_setup_cache_variables() {
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
# UNIVERSAL BINARY CREATION
# =============================================================================

ixland_create_universal_binary() {
    ixland_step "Creating universal binary..."
    
    local pkg_name="$A_SHELL_PKG_NAME"
    local ios_build_dir="${A_SHELL_PKG_BUILDER_DIR}/.build/ios"
    local simulator_build_dir="${A_SHELL_PKG_BUILDER_DIR}/.build/simulator"
    local universal_build_dir="${A_SHELL_PKG_BUILDER_DIR}/.build/universal"
    
    # Check that both builds exist
    if [ ! -d "$ios_build_dir/staging" ]; then
        ixland_error "iOS build not found. Build ios target first."
    fi
    
    if [ ! -d "$simulator_build_dir/staging" ]; then
        ixland_error "Simulator build not found. Build simulator target first."
    fi
    
    # Create universal directories
    mkdir -p "$universal_build_dir/staging"
    
    # Combine static libraries using lipo
    ixland_info "Combining libraries with lipo..."
    
    # Find all .a files in ios build
    find "$ios_build_dir/staging" -name "*.a" -type f | while read ios_lib; do
        # Get relative path
        local rel_path="${ios_lib#$ios_build_dir/staging/}"
        local sim_lib="$simulator_build_dir/staging/$rel_path"
        local universal_lib="$universal_build_dir/staging/$rel_path"
        
        # Create directory for universal library
        mkdir -p "$(dirname "$universal_lib")"
        
        if [ -f "$sim_lib" ]; then
            # Combine with lipo
            ixland_info "Creating universal: $rel_path"
            lipo -create "$ios_lib" "$sim_lib" -output "$universal_lib" || \
                ixland_warn "Failed to create universal library: $rel_path"
        else
            # Copy iOS version if simulator version missing
            ixland_warn "Simulator version missing for $rel_path, using iOS version"
            cp "$ios_lib" "$universal_lib"
        fi
    done
    
    # Copy headers from iOS build (same for both)
    if [ -d "$ios_build_dir/staging/$A_SHELL_PREFIX/include" ]; then
        cp -r "$ios_build_dir/staging/$A_SHELL_PREFIX/include" "$universal_build_dir/staging/$A_SHELL_PREFIX/"
    fi
    
    # Copy binaries (prefer iOS, but check both)
    if [ -d "$ios_build_dir/staging/$A_SHELL_PREFIX/bin" ]; then
        mkdir -p "$universal_build_dir/staging/$A_SHELL_PREFIX/bin"
        find "$ios_build_dir/staging/$A_SHELL_PREFIX/bin" -type f | while read ios_bin; do
            local bin_name=$(basename "$ios_bin")
            local sim_bin="$simulator_build_dir/staging/$A_SHELL_PREFIX/bin/$bin_name"
            local universal_bin="$universal_build_dir/staging/$A_SHELL_PREFIX/bin/$bin_name"
            
            if [ -f "$sim_bin" ]; then
                # Combine binary
                lipo -create "$ios_bin" "$sim_bin" -output "$universal_bin" || \
                    cp "$ios_bin" "$universal_bin"
            else
                cp "$ios_bin" "$universal_bin"
            fi
        done
    fi
    
    # Copy other files (pkgconfig, share, etc.)
    if [ -d "$ios_build_dir/staging/$A_SHELL_PREFIX/lib/pkgconfig" ]; then
        mkdir -p "$universal_build_dir/staging/$A_SHELL_PREFIX/lib/pkgconfig"
        cp -r "$ios_build_dir/staging/$A_SHELL_PREFIX/lib/pkgconfig/"* "$universal_build_dir/staging/$A_SHELL_PREFIX/lib/pkgconfig/" 2>/dev/null || true
    fi
    
    ixland_info "Universal binary created in $universal_build_dir/staging/"
}

# =============================================================================
# DOWNLOAD FUNCTIONS
# =============================================================================

ixland_download_and_extract() {
    ixland_step "Downloading $A_SHELL_PKG_NAME $A_SHELL_PKG_VERSION..."
    
    local src_url="${A_SHELL_PKG_SRCURL}"
    local src_file="$(basename "$src_url")"
    local src_dir="$A_SHELL_PKG_TMPDIR/$A_SHELL_PKG_NAME-$A_SHELL_PKG_VERSION"
    
    mkdir -p "$A_SHELL_PKG_TMPDIR"
    
    if [ ! -f "$A_SHELL_PKG_TMPDIR/$src_file" ]; then
        ixland_info "Downloading from $src_url"
        curl -L -o "$A_SHELL_PKG_TMPDIR/$src_file" "$src_url" || ixland_error "Download failed"
    fi
    
    # Verify checksum if provided
    if [ -n "$A_SHELL_PKG_SHA256" ] && [ "$A_SHELL_PKG_SHA256" != "SKIP_CHECKSUM" ]; then
        local actual_checksum=$(shasum -a 256 "$A_SHELL_PKG_TMPDIR/$src_file" | cut -d' ' -f1)
        if [ "$actual_checksum" != "$A_SHELL_PKG_SHA256" ]; then
            ixland_error "Checksum mismatch! Expected $A_SHELL_PKG_SHA256, got $actual_checksum"
        fi
        ixland_info "Checksum verified"
    fi
    
    # Extract
    ixland_info "Extracting..."
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
            ixland_error "Unknown archive format: $src_file"
            ;;
    esac
    
    # Verify extraction
    if [ ! -d "$src_dir" ]; then
        ixland_error "Expected directory $src_dir not found after extraction"
    fi
    
    export A_SHELL_PKG_SRCDIR="$src_dir"
    ixland_info "Source ready: $A_SHELL_PKG_SRCDIR"
}

# =============================================================================
# PATCH FUNCTIONS
# =============================================================================

ixland_apply_patches() {
    ixland_step "Applying patches..."
    
    local patches_dir="$A_SHELL_PKG_BUILDER_DIR/packages/core/$A_SHELL_PKG_NAME/patches"
    
    if [ ! -d "$patches_dir" ]; then
        ixland_info "No patches to apply"
        return 0
    fi
    
    local patch_count=0
    for patch in "$patches_dir"/*.patch; do
        if [ -f "$patch" ]; then
            ixland_info "Applying $(basename "$patch")"
            
            # Substitute placeholders
            sed -e "s%\@A_SHELL_PREFIX\@%${A_SHELL_PREFIX}%g" \
                -e "s%\@A_SHELL_CONFIG\@%${A_SHELL_CONFIG}%g" \
                "$patch" | patch --silent -p1 || ixland_error "Patch failed: $(basename "$patch")"
            
            ((patch_count++))
        fi
    done
    
    ixland_info "$patch_count patches applied"
}

# =============================================================================
# BUILD STEP FUNCTIONS
# =============================================================================

ixland_step_pre_configure() {
    ixland_setup_directories
    ixland_setup_toolchain
    ixland_setup_cache_variables
    
    # Package-specific pre-configure (override in build.sh)
    if type ixland_pkg_pre_configure &>/dev/null; then
        ixland_pkg_pre_configure
    fi
}

ixland_step_configure() {
    ixland_step "Configuring $A_SHELL_PKG_NAME..."
    cd "$A_SHELL_PKG_SRCDIR"
    
    if type ixland_pkg_configure &>/dev/null; then
        ixland_pkg_configure
    else
        # Default configure
        ./configure \
            --prefix="$A_SHELL_PREFIX" \
            --host="arm-apple-darwin" \
            --disable-nls \
            --disable-rpath \
            || ixland_error "Configure failed"
    fi
}

ixland_step_make() {
    ixland_step "Building $A_SHELL_PKG_NAME..."
    cd "$A_SHELL_PKG_SRCDIR"
    
    if type ixland_pkg_make &>/dev/null; then
        ixland_pkg_make
    else
        make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4) \
            || ixland_error "Build failed"
    fi
}

ixland_step_install() {
    ixland_step "Installing $A_SHELL_PKG_NAME..."
    cd "$A_SHELL_PKG_SRCDIR"
    
    # Clean staging
    rm -rf "$A_SHELL_PKG_STAGING"
    mkdir -p "$A_SHELL_PKG_STAGING"
    mkdir -p "$A_SHELL_PKG_STAGING/DEBIAN"
    
    if type ixland_pkg_install &>/dev/null; then
        ixland_pkg_install
    else
        make install DESTDIR="$A_SHELL_PKG_STAGING" \
            || ixland_error "Install failed"
    fi
}

# =============================================================================
# MAIN BUILD FUNCTION
# =============================================================================

ixland_build_package() {
    # Validate target
    ixland_validate_target
    
    # Set paths based on target
    ixland_set_target_paths "$A_SHELL_BUILD_TARGET"
    
    ixland_step "Building $A_SHELL_PKG_NAME $A_SHELL_PKG_VERSION (target: $A_SHELL_BUILD_TARGET)"
    
    # Load package configuration
    if [ -f "$A_SHELL_PKG_BUILDER_DIR/packages/core/$A_SHELL_PKG_NAME/build.sh" ]; then
        source "$A_SHELL_PKG_BUILDER_DIR/packages/core/$A_SHELL_PKG_NAME/build.sh"
    elif [ -f "$A_SHELL_PKG_BUILDER_DIR/packages/extra/$A_SHELL_PKG_NAME/build.sh" ]; then
        source "$A_SHELL_PKG_BUILDER_DIR/packages/extra/$A_SHELL_PKG_NAME/build.sh"
    else
        ixland_error "No build.sh found for $A_SHELL_PKG_NAME"
    fi
    
    # Validate required variables
    [ -n "$A_SHELL_PKG_NAME" ] || ixland_error "A_SHELL_PKG_NAME not set"
    [ -n "$A_SHELL_PKG_VERSION" ] || ixland_error "A_SHELL_PKG_VERSION not set"
    [ -n "$A_SHELL_PKG_SRCURL" ] || ixland_error "A_SHELL_PKG_SRCURL not set"
    
    # Run build steps
    ixland_step_pre_configure
    ixland_download_and_extract
    ixland_apply_patches
    ixland_step_configure
    ixland_step_make
    ixland_step_install
    
    # Build complete - output is in staging directory
    ixland_info "Build complete: $A_SHELL_PKG_NAME $A_SHELL_PKG_VERSION (target: $A_SHELL_BUILD_TARGET)"
    ixland_info "Output: $A_SHELL_PKG_STAGING"
}

# Run if executed directly
if [ "${BASH_SOURCE[0]}" = "${0}" ]; then
    if [ -z "$1" ]; then
        echo "Usage: $0 <package_name>"
        exit 1
    fi
    
    A_SHELL_PKG_NAME="$1"
    ixland_build_package
fi

#!/bin/bash
# build.sh - Build script for zsh package

set -e

# Source the ashell package library
ASHELL_PKG_BUILDER_DIR="${ASHELL_PKG_BUILDER_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"

# Package metadata
ASHELL_PKG_NAME="zsh"
ASHELL_PKG_VERSION="5.9"
ASHELL_PKG_SRCURL="https://www.zsh.org/pub/zsh-5.9.tar.xz"
ASHELL_PKG_SHA256="9b8d183ec3d1c69a9639055f49e5d6cf93068c7f8fc5d31c0ad6c8e9c131456a"
ASHELL_PKG_DEPENDS="libncurses"
ASHELL_PKG_BUILD_DEPENDS=""

# Package commands provided
ASHELL_PKG_COMMANDS="zsh:bin/zsh"

# Build configuration
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--enable-multibyte --enable-zsh-mime --enable-zsh-secure-free --enable-zsh-pcre"

# Setup build
ashell_setup_build() {
    ashell_info "Setting up zsh build..."
    
    # Download and extract source
    ashell_download_and_extract "$ASHELL_PKG_SRCURL" "$ASHELL_PKG_SHA256"
    
    # Apply patches
    ashell_apply_patches
    
    # Create build directory
    ashell_mkdir_p "$ASHELL_PKG_BUILDDIR"
    
    ashell_info "Build setup complete"
}

# Configure build
ashell_configure() {
    ashell_info "Configuring zsh..."
    
    cd "$ASHELL_PKG_BUILDDIR"
    
    # Export iOS-specific settings
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export CFLAGS="$ASHELL_CFLAGS -I$(ashell_get_sdk_path)/usr/include -I$ASHELL_PREFIX/include"
    export CXXFLAGS="$ASHELL_CXXFLAGS -I$(ashell_get_sdk_path)/usr/include -I$ASHELL_PREFIX/include"
    export LDFLAGS="$ASHELL_LDFLAGS -L$(ashell_get_sdk_path)/usr/lib -L$ASHELL_PREFIX/lib"
    export CPPFLAGS="$ASHELL_CPPFLAGS -I$(ashell_get_sdk_path)/usr/include -I$ASHELL_PREFIX/include"
    
    # Configure with iOS cross-compilation
    "../$ASHELL_PKG_NAME-$ASHELL_PKG_VERSION/configure" \
        --host="$ASHELL_HOST_PLATFORM" \
        --target="$ASHELL_TARGET_PLATFORM" \
        --prefix="$ASHELL_PREFIX" \
        --disable-nls \
        --disable-rpath \
        $ASHELL_PKG_EXTRA_CONFIGURE_ARGS \
        || ashell_error "Configuration failed"
    
    ashell_info "Configuration complete"
}

# Build package
ashell_build() {
    ashell_info "Building zsh..."
    
    cd "$ASHELL_PKG_BUILDDIR"
    
    make -j$(sysctl -n hw.ncpu) || ashell_error "Build failed"
    
    ashell_info "Build complete"
}

# Install package
ashell_install() {
    ashell_info "Installing zsh..."
    
    cd "$ASHELL_PKG_BUILDDIR"
    
    make install DESTDIR="$ASHELL_PKG_DESTDIR" || ashell_error "Install failed"
    
    ashell_info "Install complete"
}

# Package into XCFramework (if applicable)
ashell_package() {
    ashell_info "Packaging zsh..."
    
    # zsh is a binary command, not a library
    # Just verify the binaries exist
    if [ -f "$ASHELL_PKG_DESTDIR/$ASHELL_PREFIX/bin/zsh" ]; then
        ashell_info "zsh binary found"
    else
        ashell_error "zsh binary not found"
    fi
    
    ashell_info "Packaging complete"
}

# Run main build steps
ashell_setup_build
ashell_configure
ashell_build
ashell_install
ashell_package

ashell_info "zsh build completed successfully!"

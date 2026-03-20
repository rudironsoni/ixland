#!/bin/bash
# build.sh - Build script for bash package

set -e

# Source the ashell package library
ASHELL_PKG_BUILDER_DIR="${ASHELL_PKG_BUILDER_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"

# Package metadata
ASHELL_PKG_NAME="bash"
ASHELL_PKG_VERSION="5.2.21"
ASHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/bash/bash-5.2.21.tar.gz"
ASHELL_PKG_SHA256="c8e31bdc59b69a0c411b06e8f7ff86f1f19493a2e77330e593971ce143d6d84a"
ASHELL_PKG_DEPENDS="libncurses"
ASHELL_PKG_BUILD_DEPENDS=""

# Package commands provided
ASHELL_PKG_COMMANDS="bash:bin/bash sh:bin/sh"

# Build configuration
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--with-curses --enable-readline --with-installed-readline=no"

# Setup build
ashell_setup_build() {
    ashell_info "Setting up bash build..."
    
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
    ashell_info "Configuring bash..."
    
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
    ashell_info "Building bash..."
    
    cd "$ASHELL_PKG_BUILDDIR"
    
    make -j$(sysctl -n hw.ncpu) || ashell_error "Build failed"
    
    ashell_info "Build complete"
}

# Install package
ashell_install() {
    ashell_info "Installing bash..."
    
    cd "$ASHELL_PKG_BUILDDIR"
    
    make install DESTDIR="$ASHELL_PKG_DESTDIR" || ashell_error "Install failed"
    
    ashell_info "Install complete"
}

# Package into XCFramework (if applicable)
ashell_package() {
    ashell_info "Packaging bash..."
    
    # bash is a binary command, not a library
    # Just verify the binaries exist
    if [ -f "$ASHELL_PKG_DESTDIR/$ASHELL_PREFIX/bin/bash" ]; then
        ashell_info "bash binary found"
    else
        ashell_error "bash binary not found"
    fi
    
    ashell_info "Packaging complete"
}

# Run main build steps
ashell_setup_build
ashell_configure
ashell_build
ashell_install
ashell_package

ashell_info "bash build completed successfully!"

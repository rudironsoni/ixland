#!/bin/bash
# build.sh - Build script for curl package

set -e

# Source the ashell package library
ASHELL_PKG_BUILDER_DIR="${ASHELL_PKG_BUILDER_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"

# Package metadata
ASHELL_PKG_NAME="curl"
ASHELL_PKG_VERSION="8.5.0"
ASHELL_PKG_SRCURL="https://curl.se/download/curl-8.5.0.tar.gz"
ASHELL_PKG_SHA256="42ab8db764407e235d9c23797e9ba9d7787b4fe6125dc4ca02c474723ec4b4a5"
ASHELL_PKG_DEPENDS="libz libssl"
ASHELL_PKG_BUILD_DEPENDS=""

# Package commands provided
ASHELL_PKG_COMMANDS="curl:bin/curl"

# Build configuration
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--with-openssl --with-zlib --enable-ipv6 --enable-threaded-resolver"

# Setup build
ashell_setup_build() {
    ashell_info "Setting up curl build..."
    
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
    ashell_info "Configuring curl..."
    
    cd "$ASHELL_PKG_BUILDDIR"
    
    # Export iOS-specific settings
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export CFLAGS="$ASHELL_CFLAGS -I$(ashell_get_sdk_path)/usr/include -I$ASHELL_PREFIX/include"
    export CXXFLAGS="$ASHELL_CXXFLAGS -I$(ashell_get_sdk_path)/usr/include -I$ASHELL_PREFIX/include"
    export LDFLAGS="$ASHELL_LDFLAGS -L$(ashell_get_sdk_path)/usr/lib -L$ASHELL_PREFIX/lib"
    export CPPFLAGS="$ASHELL_CPPFLAGS -I$(ashell_get_sdk_path)/usr/include -I$ASHELL_PREFIX/include"
    export PKG_CONFIG_PATH="$ASHELL_PREFIX/lib/pkgconfig:$(ashell_get_sdk_path)/usr/lib/pkgconfig"
    
    # Configure with iOS cross-compilation
    "../$ASHELL_PKG_NAME-$ASHELL_PKG_VERSION/configure" \
        --host="$ASHELL_HOST_PLATFORM" \
        --target="$ASHELL_TARGET_PLATFORM" \
        --prefix="$ASHELL_PREFIX" \
        --disable-nls \
        --disable-rpath \
        --disable-ldap \
        --disable-ldaps \
        $ASHELL_PKG_EXTRA_CONFIGURE_ARGS \
        || ashell_error "Configuration failed"
    
    ashell_info "Configuration complete"
}

# Build package
ashell_build() {
    ashell_info "Building curl..."
    
    cd "$ASHELL_PKG_BUILDDIR"
    
    make -j$(sysctl -n hw.ncpu) || ashell_error "Build failed"
    
    ashell_info "Build complete"
}

# Install package
ashell_install() {
    ashell_info "Installing curl..."
    
    cd "$ASHELL_PKG_BUILDDIR"
    
    make install DESTDIR="$ASHELL_PKG_DESTDIR" || ashell_error "Install failed"
    
    ashell_info "Install complete"
}

# Package into XCFramework (if applicable)
ashell_package() {
    ashell_info "Packaging curl..."
    
    # curl is a binary command, not a library
    # Just verify the binaries exist
    if [ -f "$ASHELL_PKG_DESTDIR/$ASHELL_PREFIX/bin/curl" ]; then
        ashell_info "curl binary found"
    else
        ashell_error "curl binary not found"
    fi
    
    ashell_info "Packaging complete"
}

# Run main build steps
ashell_setup_build
ashell_configure
ashell_build
ashell_install
ashell_package

ashell_info "curl build completed successfully!"

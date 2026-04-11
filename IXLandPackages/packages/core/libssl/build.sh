#!/bin/bash
# build.sh - Build recipe for OpenSSL (libssl)

A_SHELL_PKG_NAME="libssl"
A_SHELL_PKG_VERSION="3.3.1"
A_SHELL_PKG_SRCURL="https://www.openssl.org/source/openssl-3.3.1.tar.gz"
A_SHELL_PKG_SHA256=""  # Will fill in after first download attempt
A_SHELL_PKG_DEPENDS="libz"
A_SHELL_PKG_DESCRIPTION="OpenSSL cryptographic library"

a_shell_pkg_configure() {
    # OpenSSL uses Configure script (capital C)
    # Use ios64-cross target with proper sysroot
    export CC="xcrun -sdk $IOS_SDK clang -arch $IOS_ARCH"
    export CFLAGS="-arch $IOS_ARCH -mios-version-min=$IOS_VERSION -isysroot $SDK_PATH -fembed-bitcode -O2"
    export LDFLAGS="-arch $IOS_ARCH -mios-version-min=$IOS_VERSION -isysroot $SDK_PATH"
    
    ./Configure ios64-cross \
        --prefix="$A_SHELL_PREFIX" \
        --openssldir="$A_SHELL_PREFIX/etc/ssl" \
        --libdir=lib \
        no-shared \
        no-tests \
        no-apps \
        || ixland_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || ixland_error "Build failed"
}

a_shell_pkg_install() {
    make DESTDIR="$A_SHELL_PKG_STAGING" install_sw || ixland_error "Install failed"
    
    # Create pkg-config files
    mkdir -p "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/lib/pkgconfig"
    
    cat > "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/lib/pkgconfig/openssl.pc" <<EOF
prefix=$A_SHELL_PREFIX
exec_prefix=\${prefix}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: OpenSSL
Description: Secure Sockets Layer and cryptography libraries and tools
Version: $A_SHELL_PKG_VERSION
Libs: -L\${libdir} -lssl -lcrypto
Cflags: -I\${includedir}
EOF
}

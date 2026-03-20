#!/bin/bash
# build.sh - Build recipe for libz (zlib)

A_SHELL_PKG_NAME="libz"
A_SHELL_PKG_VERSION="1.3.2"
A_SHELL_PKG_SRCURL="https://github.com/madler/zlib/releases/download/v1.3.2/zlib-1.3.2.tar.gz"
A_SHELL_PKG_SHA256="bb329a0a2cd0274d05519d61c667c062e06990d72e125ee2dfa8de64f0119d16"
A_SHELL_PKG_DEPENDS=""
A_SHELL_PKG_DESCRIPTION="zlib compression library"

a_shell_pkg_configure() {
    export CC="xcrun -sdk $IOS_SDK clang -arch $IOS_ARCH"
    export CFLAGS="-arch $IOS_ARCH -mios-version-min=$IOS_VERSION -fembed-bitcode -O3"
    export LDFLAGS="-arch $IOS_ARCH -mios-version-min=$IOS_VERSION"
    
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --static \
        --libdir="$A_SHELL_PREFIX/lib" \
        --includedir="$A_SHELL_PREFIX/include" \
        || a_shell_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) libz.a || a_shell_error "Build failed"
}

a_shell_pkg_install() {
    # Manual install (only static library and headers)
    mkdir -p "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/lib"
    mkdir -p "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/include"
    
    cp libz.a "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/lib/"
    cp zlib.h zconf.h "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/include/"
    
    # Create pkg-config file
    mkdir -p "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/lib/pkgconfig"
    cat > "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/lib/pkgconfig/zlib.pc" <<EOF
prefix=$A_SHELL_PREFIX
exec_prefix=\${prefix}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: zlib
Description: zlib compression library
Version: $A_SHELL_PKG_VERSION
Libs: -L\${libdir} -lz
Cflags: -I\${includedir}
EOF
}

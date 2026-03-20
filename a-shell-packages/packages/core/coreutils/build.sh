#!/bin/bash
# build.sh - Build recipe for coreutils

A_SHELL_PKG_NAME="coreutils"
A_SHELL_PKG_VERSION="9.4"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/coreutils/coreutils-9.4.tar.xz"
A_SHELL_PKG_SHA256="ea5fde12ed1e365c6970ef2c853a7a9e3088b73ce5567e99e7d0c36e6a7e0e72"
A_SHELL_PKG_DEPENDS="libz"
A_SHELL_PKG_DESCRIPTION="GNU core utilities (ls, cp, mv, cat, etc.)"

a_shell_pkg_configure() {
    # Disable features not needed or problematic on iOS
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        --disable-nls \
        --disable-rpath \
        --disable-acl \
        --disable-xattr \
        --disable-libcap \
        --without-selinux \
        --with-openssl=no \
        --with-gnutls=no \
        gl_cv_func_working_mkstemp=yes \
        || a_shell_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || a_shell_error "Build failed"
}

a_shell_pkg_install() {
    make install DESTDIR="$A_SHELL_PKG_STAGING" || a_shell_error "Install failed"
    
    # Ensure critical symlinks exist
    ln -sf ln "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/bin/link" 2>/dev/null || true
}

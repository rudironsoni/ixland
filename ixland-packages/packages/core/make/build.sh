#!/bin/bash
# build.sh - Build recipe for GNU make

A_SHELL_PKG_NAME="make"
A_SHELL_PKG_VERSION="4.4.1"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz"
A_SHELL_PKG_SHA256="dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3"
A_SHELL_PKG_DEPENDS=""
A_SHELL_PKG_DESCRIPTION="GNU Make build tool"

a_shell_pkg_configure() {
    # iOS-specific settings
    export ac_cv_func_getloadavg=no
    export ac_cv_func_clock_gettime=no
    
    # Disable job server (uses fork)
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        --disable-nls \
        --without-guile \
        --without-dmalloc \
        make_cv_job_server=no \
        || ixland_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || ixland_error "Build failed"
}

a_shell_pkg_install() {
    make DESTDIR="$A_SHELL_PKG_STAGING" install || ixland_error "Install failed"
}

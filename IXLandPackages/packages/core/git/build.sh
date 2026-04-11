#!/bin/bash
# build.sh - Build recipe for Git

A_SHELL_PKG_NAME="git"
A_SHELL_PKG_VERSION="2.44.0"
A_SHELL_PKG_SRCURL="https://github.com/git/git/archive/refs/tags/v2.44.0.tar.gz"
A_SHELL_PKG_SHA256=""
A_SHELL_PKG_DEPENDS="libz libssl libcurl"
A_SHELL_PKG_DESCRIPTION="Git version control system"

a_shell_pkg_configure() {
    # iOS-specific settings - disable features that don't work on iOS
    export ac_cv_fread_reads_directories=no
    export ac_cv_snprintf_returns_bogus=no
    export ac_cv_prog_CURL_CONFIG=/bin/false
    
    # Disable functionality that requires fork/exec or specific iOS features
    export NO_SVN_TESTS=1
    export NO_DARWIN_PORTS=1
    export NO_FINK=1
    export NO_APPLE_COMMON_CRYPTO=1
    
    make configure
    
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        --with-zlib="$A_SHELL_PREFIX" \
        --with-openssl="$A_SHELL_PREFIX" \
        --without-tcltk \
        --without-python \
        --without-perl \
        --without-curl \
        --disable-pthreads \
        --disable-silent-rules \
        ac_cv_c_c99_format=yes \
        || ixland_error "Configure failed"
}

a_shell_pkg_make() {
    # Build with reduced functionality for iOS
    make -j$(sysctl -n hw.ncpu) V=1 \
        NO_TCLTK=1 \
        NO_PYTHON=1 \
        NO_PERL=1 \
        NO_CURL=1 \
        NO_SVN_TESTS=1 \
        || ixland_error "Build failed"
}

a_shell_pkg_install() {
    make DESTDIR="$A_SHELL_PKG_STAGING" install \
        NO_TCLTK=1 \
        NO_PYTHON=1 \
        NO_PERL=1 \
        NO_CURL=1 \
        || ixland_error "Install failed"
}

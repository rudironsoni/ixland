#!/bin/bash
# build.sh - Build recipe for ncurses

A_SHELL_PKG_NAME="ncurses"
A_SHELL_PKG_VERSION="6.5"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/ncurses/ncurses-6.5.tar.gz"
A_SHELL_PKG_SHA256=""
A_SHELL_PKG_DEPENDS="libz"
A_SHELL_PKG_DESCRIPTION="NCurses terminal handling library"

a_shell_pkg_configure() {
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host=arm-apple-darwin \
        --without-shared \
        --without-cxx \
        --without-cxx-binding \
        --without-ada \
        --without-tests \
        --disable-db-install \
        --without-manpages \
        --without-progs \
        --with-termlib \
        --with-ticlib \
        --with-default-terminfo-dir="$A_SHELL_PREFIX/share/terminfo" \
        || a_shell_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || a_shell_error "Build failed"
}

a_shell_pkg_install() {
    make DESTDIR="$A_SHELL_PKG_STAGING" install || a_shell_error "Install failed"
}

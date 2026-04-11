#!/bin/bash
# build.sh - Build recipe for GNU readline

A_SHELL_PKG_NAME="readline"
A_SHELL_PKG_VERSION="8.2"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/readline/readline-8.2.tar.gz"
A_SHELL_PKG_SHA256="3feb7171f16a84ee82ca18a36d7b9be109a52c04f492a053331d7d1095007c35"
A_SHELL_PKG_DEPENDS="ncurses"
A_SHELL_PKG_DESCRIPTION="GNU readline library"

a_shell_pkg_configure() {
    # iOS-specific settings
    export ac_cv_func_getentropy=no
    export ac_cv_lib_ncursesw=yes
    export ac_cv_lib_ncurses=yes
    
    # Configure with iOS paths
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        --disable-shared \
        --enable-static \
        --with-curses \
        --without-purify \
        bash_cv_termcap_lib=libncurses \
        || ixland_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || ixland_error "Build failed"
}

a_shell_pkg_install() {
    make DESTDIR="$A_SHELL_PKG_STAGING" install || ixland_error "Install failed"
}

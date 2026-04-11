#!/bin/bash
# build.sh - Build recipe for Vim

A_SHELL_PKG_NAME="vim"
A_SHELL_PKG_VERSION="9.1.0"
A_SHELL_PKG_SRCURL="https://github.com/vim/vim/archive/refs/tags/v9.1.0.tar.gz"
A_SHELL_PKG_SHA256=""
A_SHELL_PKG_DEPENDS="ncurses libz"
A_SHELL_PKG_DESCRIPTION="Vim text editor"

a_shell_pkg_configure() {
    cd src
    
    # iOS-specific settings
    export ac_cv_func_getentropy=no
    export ac_cv_func_clock_gettime=no
    export ac_cv_func_getxattr=no
    export ac_cv_func_setxattr=no
    export ac_cv_func_removexattr=no
    export ac_cv_func_listxattr=no
    
    # Configure with minimal features for iOS
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        --with-features=normal \
        --disable-gui \
        --disable-netbeans \
        --disable-cscope \
        --disable-xsmp \
        --disable-selinux \
        --disable-pythoninterp \
        --disable-python3interp \
        --disable-rubyinterp \
        --disable-luainterp \
        --disable-perlinterp \
        --disable-tclinterp \
        --enable-multibyte \
        --with-tlib=ncurses \
        --without-x \
        || ixland_error "Configure failed"
}

a_shell_pkg_make() {
    cd src
    make -j$(sysctl -n hw.ncpu) VIMRCLOC="$A_SHELL_PREFIX/etc" \
        || ixland_error "Build failed"
}

a_shell_pkg_install() {
    cd src
    make DESTDIR="$A_SHELL_PKG_STAGING" install \
        || ixland_error "Install failed"
}

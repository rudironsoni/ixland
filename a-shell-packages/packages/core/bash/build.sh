#!/bin/bash
# build.sh - Build recipe for bash

A_SHELL_PKG_NAME="bash"
A_SHELL_PKG_VERSION="5.2.21"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/bash/bash-5.2.21.tar.gz"
A_SHELL_PKG_SHA256="c8e31bdc59b69a0c411b06e8f7ff86f1f19493a2e77330e593971ce143d6d84a"
A_SHELL_PKG_DEPENDS="libncurses, libreadline"
A_SHELL_PKG_DESCRIPTION="GNU Bourne Again SHell"

a_shell_pkg_configure() {
    # Cache variables for cross-compilation
    export ac_cv_sizeof_size_t=8
    export ac_cv_sizeof_char_p=8
    export bash_cv_getcwd_malloc=yes
    export bash_cv_job_control_missing=present
    export bash_cv_sys_named_pipes=present
    export bash_cv_func_sigsetjmp=present
    
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        --without-bash-malloc \
        --with-installed-readline=no \
        --with-libz \
        --enable-multibyte \
        --disable-nls \
        --disable-rpath \
        || a_shell_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || a_shell_error "Build failed"
}

a_shell_pkg_install() {
    make install DESTDIR="$A_SHELL_PKG_STAGING" || a_shell_error "Install failed"
    
    # Create sh symlink
    ln -sf bash "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/bin/sh"
}

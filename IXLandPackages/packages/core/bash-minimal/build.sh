#!/bin/bash
# build.sh - Minimal bash build (no readline, no ncurses)

A_SHELL_PKG_NAME="bash"
A_SHELL_PKG_VERSION="5.2.21"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/bash/bash-5.2.21.tar.gz"
A_SHELL_PKG_SHA256="c8e31bdc59b69aaffc5b36509905ba3e5cbb12747091d27b4b977f078560d5b8"
A_SHELL_PKG_DEPENDS=""
A_SHELL_PKG_DESCRIPTION="GNU Bourne Again SHell (minimal)"

a_shell_pkg_configure() {
    # Cache variables for cross-compilation
    export ac_cv_sizeof_size_t=8
    export ac_cv_sizeof_char_p=8
    export ac_cv_func_setvbuf_reversed=no
    export bash_cv_getcwd_malloc=yes
    export bash_cv_job_control_missing=present
    export bash_cv_sys_named_pipes=present
    export bash_cv_func_sigsetjmp=present
    export bash_cv_printf_a_format=yes
    
    # iOS doesn't have getentropy
    export ac_cv_func_getentropy=no
    
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        --without-bash-malloc \
        --without-libintl-prefix \
        --without-libiconv-prefix \
        --without-libz \
        --disable-readline \
        --disable-history \
        --disable-nls \
        --disable-rpath \
        --disable-net-redirections \
        --disable-separate-helpfiles \
        --disable-progcomp \
        --disable-directory-stack \
        --disable-prompt-string-decoding \
        --disable-select \
        --disable-mem-scramble \
        --disable-debugger \
        --disable-help-builtin \
        || ixland_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || ixland_error "Build failed"
}

a_shell_pkg_install() {
    make install DESTDIR="$A_SHELL_PKG_STAGING" || ixland_error "Install failed"
    
    # Create sh symlink
    ln -sf bash "$A_SHELL_PKG_STAGING$A_SHELL_PREFIX/bin/sh"
}

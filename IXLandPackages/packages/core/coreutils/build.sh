#!/bin/bash
# build.sh - Build recipe for GNU coreutils

A_SHELL_PKG_NAME="coreutils"
A_SHELL_PKG_VERSION="9.5"
A_SHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/coreutils/coreutils-9.5.tar.gz"
A_SHELL_PKG_SHA256=""
A_SHELL_PKG_DEPENDS=""
A_SHELL_PKG_DESCRIPTION="GNU Core Utilities (ls, cp, mv, cat, etc.)"

a_shell_pkg_configure() {
    # Cache variables for cross-compilation
    export ac_cv_sizeof_long=8
    export ac_cv_sizeof_size_t=8
    export ac_cv_sizeof_ssize_t=8
    export ac_cv_sizeof_off_t=8
    export ac_cv_sizeof_long_long=8
    export ac_cv_sizeof_dev_t=4
    export ac_cv_sizeof_ino_t=8
    export ac_cv_func_getcwd_null=yes
    export ac_cv_func_getgroups_works=yes
    export ac_cv_func_memcmp_working=yes
    export ac_cv_func_strtod_works=yes
    export ac_cv_func_chown_works=yes
    export ac_cv_func_fchmodat_works=yes
    export ac_cv_func_fchownat_works=yes
    export ac_cv_func_lchmod_works=no
    export ac_cv_func_stat_empty_string_bug=no
    export ac_cv_func_working_mktime=yes
    export ac_cv_func_fnmatch_works=yes
    export ac_cv_func_fnmatch_gnu=yes
    export gl_cv_func_gettimeofday_clobber=no
    export gl_cv_func_tzset_clobber=no
    
    # iOS doesn't have clock_settime
    export ac_cv_func_clock_settime=no
    export gl_cv_func_clock_settime=no
    
    # Disable programs that require root or special permissions
    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin" \
        --disable-nls \
        --disable-rpath \
        --disable-libcap \
        --disable-xattr \
        --disable-libsmack \
        --without-selinux \
        --without-gmp \
        --enable-install-program="arch,hostname" \
        || ixland_error "Configure failed"
}

a_shell_pkg_make() {
    make -j$(sysctl -n hw.ncpu) || ixland_error "Build failed"
}

a_shell_pkg_install() {
    make install DESTDIR="$A_SHELL_PKG_STAGING" || ixland_error "Install failed"
}

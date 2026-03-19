#!/bin/bash
# ashell-packages/root-packages/dpkg/build.sh
# Debian package management system for iOS

ASHELL_PKG_NAME="dpkg"
ASHELL_PKG_VERSION="1.22.6"
ASHELL_PKG_DESCRIPTION="Debian package management system"
ASHELL_PKG_HOMEPAGE="https://wiki.debian.org/dpkg"

ASHELL_PKG_SRCURL="https://deb.debian.org/debian/pool/main/d/dpkg/dpkg_${ASHELL_PKG_VERSION}.tar.xz"
ASHELL_PKG_SHA256="a9afa6864a0449afd9c7fa11f47b63aaa9e7695beaacac73b872b69a8dd062ce"

ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""

ASHELL_PKG_COMMANDS=(
    "dpkg:dpkg_main::no"
    "dpkg-deb:dpkg_deb_main::no"
    "dpkg-query:dpkg_query_main::no"
    "dpkg-split:dpkg_split_main::no"
)

ASHELL_PKG_EXTRA_CONFIGURE_ARGS="
--prefix=@ASHELL_PREFIX@
--sysconfdir=@ASHELL_CONFIG@/etc
--localstatedir=@ASHELL_PREFIX@/var
--disable-shared
--enable-static
--disable-nls
--disable-dselect
--disable-start-stop-daemon
--disable-update-alternatives
--without-libselinux
--without-selinux
--without-admindir
--without-logdir
--without-libdpkg-perl
"

# Pre-configure for iOS
ashell_step_pre_configure() {
    ashell_info "Configuring dpkg for iOS..."

    # Set up iOS cross-compilation
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export AR="$ASHELL_AR"
    export RANLIB="$ASHELL_RANLIB"
    export CFLAGS="$ASHELL_CFLAGS"
    export CXXFLAGS="$ASHELL_CXXFLAGS"
    export LDFLAGS="$ASHELL_LDFLAGS"

    # Apply patches
    local patch_dir="$ASHELL_PKG_BUILDER_DIR/patches"
    if [[ -d "$patch_dir" ]]; then
        for patch in "$patch_dir"/*.patch; do
            if [[ -f "$patch" ]]; then
                ashell_info "Applying patch: $(basename $patch)"
                patch -p1 < "$patch" || ashell_error "Failed to apply patch"
            fi
        done
    fi
}

# Post-install setup
ashell_step_post_make_install() {
    ashell_info "Setting up dpkg directories..."

    # Create required directories
    mkdir -p "$ASHELL_PKG_PREFIX/var/lib/dpkg"
    mkdir -p "$ASHELL_PKG_PREFIX/var/log"
    mkdir -p "$ASHELL_PKG_PREFIX/etc/dpkg"

    # Create status file
    touch "$ASHELL_PKG_PREFIX/var/lib/dpkg/status"

    # Create available file
    touch "$ASHELL_PKG_PREFIX/var/lib/dpkg/available"

    # Create diversions file
    touch "$ASHELL_PKG_PREFIX/var/lib/dpkg/diversions"

    # Copy architecture info
    mkdir -p "$ASHELL_PKG_PREFIX/share/dpkg"
    echo "ios-arm64" > "$ASHELL_PKG_PREFIX/share/dpkg/architecture"

    ashell_info "dpkg setup complete"
}

source "$ASHELL_PKG_BUILDER_DIR/../../ashell_package.sh"

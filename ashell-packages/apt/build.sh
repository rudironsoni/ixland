#!/bin/bash
# ashell-packages/apt/build.sh
# Build recipe for Debian apt package manager
# Adapted from Termux: https://github.com/termux/termux-packages/tree/master/packages/apt

ASHELL_PKG_NAME="apt"
ASHELL_PKG_VERSION="2.8.1"
ASHELL_PKG_REVISION="1"
ASHELL_PKG_DESCRIPTION="Front-end for the dpkg package manager"
ASHELL_PKG_LICENSE="GPL-2.0"
ASHELL_PKG_HOMEPAGE="https://packages.debian.org/apt"
ASHELL_PKG_MAINTAINER="a-Shell Team"

# Source tarball from Debian Salsa
ASHELL_PKG_SRCURL="https://salsa.debian.org/apt-team/apt/-/archive/${ASHELL_PKG_VERSION}/apt-${ASHELL_PKG_VERSION}.tar.bz2"
ASHELL_PKG_SHA256="87ca18392c10822a133b738118505f7d04e0b31ba1122bf5d32911311cb2dc7e"

# Runtime dependencies (a-Shell equivalents)
ASHELL_PKG_DEPENDS="dpkg, libgcrypt, libgnutls, lz4, xz-utils, zlib, zstd, xxhash"
ASHELL_PKG_BUILD_DEPENDS="cmake, ninja, perl"

# Commands provided by this package
ASHELL_PKG_COMMANDS=(
    "apt:apt_main:apt:file"
    "apt-cache:apt_cache_main:apt-cache:file"
    "apt-config:apt_config_main:apt-config:file"
    "apt-get:apt_get_main:apt-get:file"
    "apt-mark:apt_mark_main:apt-mark:file"
)

# Configuration files that should be preserved on upgrade
ASHELL_PKG_CONFFILES="
etc/apt/sources.list
etc/apt/apt.conf.d/99-ashell
"

# Files to remove after install (not applicable on iOS)
ASHELL_PKG_RM_AFTER_INSTALL="
bin/apt-cdrom
bin/apt-extracttemplates
bin/apt-sortpkgs
share/man/man1/apt-extracttemplates.1
share/man/man1/apt-sortpkgs.1
share/man/man8/apt-cdrom.8
"

# Extra CMake configure arguments
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="
-DCMAKE_POLICY_VERSION_MINIMUM=3.5
-DPERL_EXECUTABLE=$(command -v perl 2>/dev/null || echo /usr/bin/perl)
-DCMAKE_INSTALL_FULL_LOCALSTATEDIR=@ASHELL_PREFIX@
-DCACHE_DIR=@ASHELL_PREFIX@/var/cache/apt
-DCOMMON_ARCH=@ASHELL_TARGET_ARCH@
-DDPKG_DATADIR=@ASHELL_PREFIX@/share/dpkg
-DUSE_NLS=OFF
-DWITH_DOC=OFF
-DWITH_DOC_MANPAGES=ON
-DCMAKE_INSTALL_LIBEXECDIR=lib
-DDOCBOOK_XSL=/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl
"

# Pre-configure step
ashell_step_pre_configure() {
    ashell_log_info "Configuring apt for iOS..."

    # Substitute @ASHELL_PREFIX@ with actual path in patches
    substitute_prefix "$ASHELL_PKG_BUILDER_DIR/patches"

    # iOS-specific compile flags
    export CXXFLAGS="$CXXFLAGS -Wno-c++11-narrowing -DASHELL_BUILD"
    export LDFLAGS="$LDFLAGS -framework Foundation -framework Security"

    # Create initial directories
    mkdir -p "$ASHELL_PKG_BUILD_DIR/etc/apt/apt.conf.d"
    mkdir -p "$ASHELL_PKG_BUILD_DIR/var/cache/apt/archives/partial"
    mkdir -p "$ASHELL_PKG_BUILD_DIR/var/lib/apt/lists/partial"
    mkdir -p "$ASHELL_PKG_BUILD_DIR/var/log/apt"
}

# Post-install step
ashell_step_post_make_install() {
    ashell_log_info "Setting up a-Shell apt configuration..."

    # Create a-Shell specific sources.list
    cat > "$ASHELL_PKG_STAGING_DIR/etc/apt/sources.list" << 'EOF'
# a-Shell package repository
# deb https://packages.ashell.dev/apt/stable main
EOF

    # Create a-Shell specific apt.conf
    cat > "$ASHELL_PKG_STAGING_DIR/etc/apt/apt.conf.d/99-ashell" << EOF
// a-Shell specific apt configuration
APT::Get::Assume-Yes "true";
APT::Get::Fix-Broken "true";
APT::Acquire::Retries "3";
APT::Install-Recommends "false";
APT::Install-Suggests "false";

// iOS-specific paths
Dir::State "$ASHELL_PREFIX/var/lib/apt";
Dir::Cache "$ASHELL_PREFIX/var/cache/apt";
Dir::Log "$ASHELL_PREFIX/var/log/apt";
Dir::Etc "$ASHELL_PREFIX/etc/apt";

// Architecture
APT::Architecture "$ASHELL_TARGET_ARCH";

// Disable periodic update checks (a-Shell manages this)
APT::Periodic::Update-Package-Lists "0";
APT::Periodic::Download-Upgradeable-Packages "0";
APT::Periodic::AutocleanInterval "0";
EOF

    # Set proper permissions for directories
    chmod 755 "$ASHELL_PKG_STAGING_DIR/var/cache/apt/archives" 2>/dev/null || true
    chmod 755 "$ASHELL_PKG_STAGING_DIR/var/lib/apt/lists" 2>/dev/null || true

    ashell_log_success "apt configured for a-Shell"
}

# Load the build system library
source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"

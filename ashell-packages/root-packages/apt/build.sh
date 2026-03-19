#!/bin/bash
# a-Shell apt package build recipe
# Builds apt from Debian upstream (3.1.16) with iOS-specific patches
# Following Termux's approach: https://github.com/termux/termux-packages/tree/master/packages/apt

ASHELL_PKG_NAME="apt"
ASHELL_PKG_VERSION="3.1.16"
ASHELL_PKG_REVISION="1"
ASHELL_PKG_SRCURL="https://salsa.debian.org/apt-team/apt/-/archive/${ASHELL_PKG_VERSION}/apt-${ASHELL_PKG_VERSION}.tar.bz2"
ASHELL_PKG_SHA256="SKIP_CHECKSUM"
ASHELL_PKG_DESCRIPTION="Front-end for the dpkg package manager"
ASHELL_PKG_HOMEPAGE="https://packages.debian.org/apt"

# Runtime dependencies
ASHELL_PKG_DEPENDS="coreutils, libz, libssl, libcurl"

# Build dependencies
ASHELL_PKG_BUILD_DEPENDS="cmake, ninja"

# Commands provided by this package
ASHELL_PKG_COMMANDS=(
    "apt:apt_main::no"
    "apt-get:apt_get_main::no"
    "apt-cache:apt_cache_main::no"
    "apt-config:apt_config_main::no"
)

# Extra configure flags for iOS cross-compilation
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="
-DCMAKE_BUILD_TYPE=Release
-DCMAKE_SYSTEM_NAME=iOS
-DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
-DCMAKE_INSTALL_PREFIX=@ASHELL_PREFIX@
-DCACHE_DIR=@ASHELL_PREFIX@/var/cache/apt
-DLOG_DIR=@ASHELL_PREFIX@/var/log/apt
-DSTATE_DIR=@ASHELL_PREFIX@/var/lib/apt
-DDPKG_DATADIR=@ASHELL_PREFIX@/share/dpkg
-DUSE_NLS=OFF
-DWITH_DOC=OFF
-DWITH_DOC_MANPAGES=OFF
"

# Pre-configure hook - apply iOS-specific patches
ashell_step_pre_configure() {
    ashell_log_info "Applying iOS-specific patches to apt..."

    # Reject on-device builds (too resource intensive)
    if [[ "$ASHELL_ON_DEVICE_BUILD" == "true" ]]; then
        ashell_error_exit "Package 'apt' is not safe for on-device builds."
    fi

    # Apply patches in order
    local patch_dir="$ASHELL_PKG_BUILDER_DIR/patches"
    if [[ -d "$patch_dir" ]]; then
        for patch in "$patch_dir"/*.patch; do
            if [[ -f "$patch" ]]; then
                ashell_log_info "Applying patch: $(basename $patch)"
                # Substitute @ASHELL_PREFIX@ in patches
                sed -e "s%@ASHELL_PREFIX@%${ASHELL_PREFIX}%g" \
                    -e "s%@ASHELL_CACHE_DIR@%${ASHELL_CACHE_DIR}%g" \
                    "$patch" | patch --silent -p1 || {
                    ashell_log_error "Failed to apply patch: $(basename $patch)"
                    return 1
                }
            fi
        done
    fi

    # Fix iOS-specific build issues
    export CXXFLAGS="${CXXFLAGS} -Wno-c++11-narrowing -std=c++17"

    # Disable features not available on iOS
    ashell_log_info "Configuring apt for iOS sandbox..."
}

# Post-install hook - setup apt directories and config
ashell_step_post_make_install() {
    ashell_log_info "Setting up apt configuration..."

    # Create required directories
    mkdir -p "$ASHELL_PKG_PREFIX/var/cache/apt/archives"
    mkdir -p "$ASHELL_PKG_PREFIX/var/lib/apt/lists"
    mkdir -p "$ASHELL_PKG_PREFIX/var/log/apt"
    mkdir -p "$ASHELL_PKG_PREFIX/etc/apt/apt.conf.d"
    mkdir -p "$ASHELL_PKG_PREFIX/etc/apt/sources.list.d"

    # Create default sources.list
    cat > "$ASHELL_PKG_PREFIX/etc/apt/sources.list" << 'EOF'
# a-Shell package repository
deb https://packages.ashell.dev/stable/main ios main
deb https://packages.ashell.dev/stable/python ios main
EOF

    # Create apt.conf for iOS-specific settings
    cat > "$ASHELL_PKG_PREFIX/etc/apt/apt.conf" << EOF
// a-Shell apt configuration
APT::Architecture "ios-arm64";
APT::Get::AllowUnauthenticated "true";
Dir::Cache "${ASHELL_PREFIX}/var/cache/apt";
Dir::State "${ASHELL_PREFIX}/var/lib/apt";
Dir::Log "${ASHELL_PREFIX}/var/log/apt";
Dir::Etc "${ASHELL_PREFIX}/etc/apt";

// iOS-specific: Disable features that require root/special permissions
APT::Get::Clean "always";
Acquire::AllowInsecureRepositories "true";
EOF

    # Remove components not applicable to iOS
    rm -f "$ASHELL_PKG_PREFIX/bin/apt-cdrom"  # No CD-ROM on iOS
    rm -f "$ASHELL_PKG_PREFIX/bin/apt-extracttemplates"
    rm -f "$ASHELL_PKG_PREFIX/bin/apt-sortpkgs"

    ashell_log_success "apt configuration complete"
}

# Load the build system library
source "${ASHELL_PKG_BUILDER_DIR}/../../ashell_package.sh"

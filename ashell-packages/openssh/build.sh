#!/bin/bash
# ashell-packages/openssh/build.sh - SSH client package for iOS
# Builds OpenSSH with libssh2 for a-Shell

# =============================================================================
# PACKAGE METADATA
# =============================================================================

ASHELL_PKG_NAME="openssh"
ASHELL_PKG_VERSION="9.5p1"
ASHELL_PKG_DESCRIPTION="OpenSSH client for a-Shell"
ASHELL_PKG_HOMEPAGE="https://www.openssh.com/"

# OpenSSH source
ASHELL_PKG_SRCURL="https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="f52f3f41d429aa9918e38cf200af225ccdd8e66f052da4418707165173f5e5c0"

# libssh2 source (dependency)
LIBSSH2_VERSION="1.11.0"
LIBSSH2_URL="https://libssh2.org/download/libssh2-${LIBSSH2_VERSION}.tar.gz"

# Command registration
ASHELL_PKG_COMMANDS=(
    "ssh:ssh_main::no"
    "scp:scp_main::no"
    "sftp:sftp_main::no"
    "ssh-keygen:ssh_keygen_main::no"
    "ssh-keyscan:ssh_keyscan_main::no"
)

# Dependencies
ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS="openssl"

# =============================================================================
# LOAD BUILD SYSTEM
# =============================================================================

ASHELL_PKG_BUILDER_DIR="${ASHELL_PKG_BUILDER_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"

# =============================================================================
# OVERRIDE BUILD STEPS
# =============================================================================

# Override extract to also get libssh2
ashell_step_extract_package() {
    ashell_info "Extracting OpenSSH and libssh2 sources"

    # Download and extract OpenSSH
    ashell_download "$ASHELL_PKG_SRCURL" "$ASHELL_PKG_SHA256"
    ashell_extract "$ASHELL_PKG_CACHEDIR/$(basename "$ASHELL_PKG_SRCURL")" "$ASHELL_PKG_SRCDIR"

    # Download and extract libssh2
    ashell_download "$LIBSSH2_URL" ""
    local libssh2_dir="$ASHELL_PKG_SRCDIR/libssh2-$LIBSSH2_VERSION"
    ashell_extract "$ASHELL_PKG_CACHEDIR/$(basename "$LIBSSH2_URL")" "$libssh2_dir"

    # Move libssh2 to expected location
    mv "$libssh2_dir"/* "$ASHELL_PKG_SRCDIR/../libssh2-source/" 2>/dev/null || true

    ashell_info "Sources extracted"
}

# Pre-configure: Set up build environment
ashell_step_pre_configure() {
    ashell_info "Setting up SSH build environment"

    local srcdir=$(ashell_pkg_srcdir)

    # Set up iOS SDK paths
    export SDKROOT=$(ashell_get_sdk_path)
    export CC="$(xcrun --sdk iphoneos --find clang)"
    export CXX="$(xcrun --sdk iphoneos --find clang++)"
    export AR="$(xcrun --sdk iphoneos --find ar)"
    export RANLIB="$(xcrun --sdk iphoneos --find ranlib)"

    # iOS-specific flags
    export CFLAGS="-arch arm64 -isysroot $SDKROOT -mios-version-min=$ASHELL_DEPLOYMENT_TARGET -fembed-bitcode -D__APPLE_USE_RFC_3542"
    export CXXFLAGS="$CFLAGS"
    export LDFLAGS="-arch arm64 -isysroot $SDKROOT -mios-version-min=$ASHELL_DEPLOYMENT_TARGET"

    # OpenSSL paths (from dependency)
    local openssl_prefix="$ASHELL_PREFIX"
    export CPPFLAGS="-I$openssl_prefix/include"
    export LDFLAGS="$LDFLAGS -L$openssl_prefix/lib"

    ashell_info "Build environment configured"
}

# Configure with iOS-specific options
ashell_step_configure() {
    ashell_info "Configuring OpenSSH for iOS"

    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)

    ashell_mkdir_p "$builddir"

    cd "$srcdir"

    # Configure OpenSSH with iOS-friendly options
    # Disable features that don't work on iOS
    ./configure \
        --host=arm-apple-darwin \
        --prefix="@ASHELL_PREFIX@" \
        --sysconfdir="@ASHELL_CONFIG@/etc/ssh" \
        --with-libssl-prefix="$ASHELL_PREFIX" \
        --with-zlib="$SDKROOT/usr" \
        --without-openssl-header-check \
        --disable-strip \
        --disable-etc-default-login \
        --disable-lastlog \
        --disable-utmp \
        --disable-utmpx \
        --disable-wtmp \
        --disable-wtmpx \
        --without-stackprotect \
        --without-hardening \
        --without-rpath \
        --without-pam \
        --without-selinux \
        --without-kerberos5 \
        --without-shadow \
        --without-libedit \
        --without-skey \
        --without-security-key-builtin \
        --without-ldns \
        --without-libfido2 \
        --without-smartcard \
        --without-omapbsd \
        --without-bsd-auth \
        --with-ssl-engine \
        || ashell_error "Configure failed"

    # Substitute PREFIX in generated files
    ashell_substitute_prefix "$srcdir"

    ashell_info "Configuration complete"
}

# Build OpenSSH
ashell_step_make() {
    ashell_info "Building OpenSSH"

    local srcdir=$(ashell_pkg_srcdir)
    local cpu_count

    if command -v nproc &> /dev/null; then
        cpu_count=$(nproc 2>/dev/null || echo 4)
    else
        cpu_count=4
    fi

    cd "$srcdir"

    # Build only the client tools (not the server)
    make -j$cpu_count \
        ssh scp sftp ssh-keygen ssh-keyscan \
        || ashell_error "Build failed"

    ashell_info "Build complete"
}

# Install binaries
ashell_step_make_install() {
    ashell_info "Installing SSH tools"

    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$stagingdir/bin"
    ashell_mkdir_p "$stagingdir/etc/ssh"
    ashell_mkdir_p "$stagingdir/libexec"

    # Install binaries
    local binaries=("ssh" "scp" "sftp" "ssh-keygen" "ssh-keyscan")
    for bin in "${binaries[@]}"; do
        if [[ -f "$srcdir/$bin" ]]; then
            cp "$srcdir/$bin" "$stagingdir/bin/"
            chmod +x "$stagingdir/bin/$bin"
        fi
    done

    # Create default SSH config
    cat > "$stagingdir/etc/ssh/ssh_config" <<'EOF'
# Default SSH config for a-Shell
# User config can be placed in ~/.ssh/config

Host *
    # Use iOS Keychain for host key verification
    UserKnownHostsFile ~/.ssh/known_hosts
    StrictHostKeyChecking ask
    HashKnownHosts yes

    # Connection settings
    ServerAliveInterval 60
    ServerAliveCountMax 3

    # Compression for slower connections
    Compression yes
EOF

    # Create sshd_config (minimal, server not typically used)
    cat > "$stagingdir/etc/ssh/sshd_config" <<'EOF'
# SSH server config - not typically used on iOS
# Server functionality is disabled in a-Shell builds
Port 22
EOF

    ashell_info "Installation complete"
}

# Create XCFramework structure
ashell_step_create_xcframework() {
    ashell_info "Creating SSH framework structure"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir/bin"
    ashell_mkdir_p "$framework_dir/etc/ssh"

    # Copy binaries
    for bin in ssh scp sftp ssh-keygen ssh-keyscan; do
        if [[ -f "$stagingdir/bin/$bin" ]]; then
            cp "$stagingdir/bin/$bin" "$framework_dir/bin/"
        fi
    done

    # Copy config
    cp -R "$stagingdir/etc/ssh" "$framework_dir/etc/"

    # Create umbrella binary
    cat > "$framework_dir/$ASHELL_PKG_NAME" <<'EOF'
#!/bin/sh
# SSH tools wrapper for ios_system
# Actual binaries are in bin/ subdirectory

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Set up environment
export SSH_BIN_DIR="$SCRIPT_DIR/bin"
export SSH_CONFIG_DIR="$SCRIPT_DIR/etc/ssh"

# Delegate to actual binary
COMMAND="$(basename "$0")"
exec "$SSH_BIN_DIR/$COMMAND" "$@"
EOF
    chmod +x "$framework_dir/$ASHELL_PKG_NAME"

    # Create Info.plist
    cat > "$framework_dir/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>$ASHELL_PKG_NAME</string>
    <key>CFBundleIdentifier</key>
    <string>com.rudironsoni.ashell.$ASHELL_PKG_NAME</string>
    <key>CFBundleName</key>
    <string>$ASHELL_PKG_NAME</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>$ASHELL_PKG_VERSION</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>MinimumOSVersion</key>
    <string>16.0</string>
</dict>
</plist>
EOF

    ashell_info "Framework created"
}

# Generate commands plist
ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for OpenSSH"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    cat > "$plist_path" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>ssh</key>
    <array>
        <string>openssh.framework/bin/ssh</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>scp</key>
    <array>
        <string>openssh.framework/bin/scp</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>sftp</key>
    <array>
        <string>openssh.framework/bin/sftp</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>ssh-keygen</key>
    <array>
        <string>openssh.framework/bin/ssh-keygen</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>ssh-keyscan</key>
    <array>
        <string>openssh.framework/bin/ssh-keyscan</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
</dict>
</plist>
EOF

    ashell_info "Commands plist generated"
}

# Post-install: Create .ssh directory setup helper
ashell_step_post_make_install() {
    ashell_info "Setting up SSH directories"

    local stagingdir=$(ashell_pkg_stagingdir)

    # Create .ssh directory structure helper
    cat > "$stagingdir/bin/ssh-setup" <<'EOF'
#!/bin/sh
# Setup SSH directories for a-Shell

SSH_DIR="$HOME/.ssh"
mkdir -p "$SSH_DIR"
chmod 700 "$SSH_DIR"

# Create config if it doesn't exist
if [ ! -f "$SSH_DIR/config" ]; then
    cat > "$SSH_DIR/config" <<'SSHEOF'
# SSH client configuration
# Place your host-specific settings here

# Example:
# Host myserver
#     HostName example.com
#     User myusername
#     IdentityFile ~/.ssh/id_ed25519
SSHEOF
    chmod 600 "$SSH_DIR/config"
fi

# Create known_hosts if it doesn't exist
touch "$SSH_DIR/known_hosts"
chmod 600 "$SSH_DIR/known_hosts"

echo "SSH directory structure created at $SSH_DIR"
echo ""
echo "Generate a key with: ssh-keygen -t ed25519"
EOF
    chmod +x "$stagingdir/bin/ssh-setup"

    ashell_info "SSH setup complete"
}

# =============================================================================
# RUN BUILD
# =============================================================================

ashell_build_package

#!/bin/bash
# ashell-packages/root-packages/libssl/build.sh
# OpenSSL library for iOS

ASHELL_PKG_NAME="libssl"
ASHELL_PKG_VERSION="3.2.1"
ASHELL_PKG_DESCRIPTION="OpenSSL library"
ASHELL_PKG_HOMEPAGE="https://www.openssl.org/"

ASHELL_PKG_SRCURL="https://www.openssl.org/source/openssl-${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="83c7329fe52c850677d75e5d0b0ca245309b97e8ecbcfdc1dfdc4ab9fac35b39"

ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""

# No commands - library only
ASHELL_PKG_COMMANDS=""

# Pre-configure for iOS
ashell_step_pre_configure() {
    ashell_info "Configuring OpenSSL for iOS..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # Set up iOS cross-compilation
    export CC="$ASHELL_CC"
    export CFLAGS="$ASHELL_CFLAGS -fembed-bitcode"
    export LDFLAGS="$ASHELL_LDFLAGS"

    # Configure for iOS (arm64)
    # OpenSSL uses a custom Configure script
    ./Configure \
        ios64-cross \
        --prefix="@ASHELL_PREFIX@" \
        --openssldir="@ASHELL_CONFIG@/etc/ssl" \
        no-shared \
        no-tests \
        no-ssl3 \
        no-ssl3-method \
        no-zlib \
        || ashell_error "OpenSSL configure failed"

    ashell_info "OpenSSL configured"
}

# Override configure step
ashell_step_configure() {
    : # No-op, done in pre_configure
}

# Build
ashell_step_make() {
    ashell_info "Building OpenSSL..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    make -j$(nproc 2>/dev/null || echo 4) || ashell_error "OpenSSL build failed"

    ashell_info "OpenSSL build complete"
}

# Install
ashell_step_make_install() {
    ashell_info "Installing OpenSSL..."

    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)

    cd "$srcdir"

    # Install to staging
    make DESTDIR="$stagingdir" install_sw || ashell_error "OpenSSL install failed"

    ashell_info "OpenSSL installed"
}

# Create XCFramework
ashell_step_create_xcframework() {
    ashell_info "Creating OpenSSL XCFramework..."

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir"

    # Copy libraries
    cp "$stagingdir@ASHELL_PREFIX@/lib/libssl.a" "$framework_dir/libssl.a"
    cp "$stagingdir@ASHELL_PREFIX@/lib/libcrypto.a" "$framework_dir/libcrypto.a"

    # Copy headers
    ashell_mkdir_p "$framework_dir/Headers"
    cp -R "$stagingdir@ASHELL_PREFIX@/include/openssl" "$framework_dir/Headers/"

    # Create module map
    cat > "$framework_dir/Headers/module.modulemap" <<'EOF'
module libssl {
    umbrella header "openssl/ssl.h"
    export *
}

module libcrypto {
    umbrella header "openssl/crypto.h"
    export *
}
EOF

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

    ashell_info "OpenSSL XCFramework created"
}

source "$ASHELL_PKG_BUILDER_DIR/../../ashell_package.sh"

#!/bin/bash
# ashell-packages/root-packages/libcurl/build.sh
# cURL library for iOS

ASHELL_PKG_NAME="libcurl"
ASHELL_PKG_VERSION="8.6.0"
ASHELL_PKG_DESCRIPTION="cURL library for HTTP/HTTPS transfers"
ASHELL_PKG_HOMEPAGE="https://curl.se/"

ASHELL_PKG_SRCURL="https://curl.se/download/curl-${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="9c6db808160015f30f3c656c4dec1257e12e58fb7c1b91a8a96c5c4fa7a9a0a3"

ASHELL_PKG_DEPENDS="libssl"
ASHELL_PKG_BUILD_DEPENDS=""

# No commands - library only
ASHELL_PKG_COMMANDS=""

ASHELL_PKG_EXTRA_CONFIGURE_ARGS="
--prefix=@ASHELL_PREFIX@
--disable-shared
--enable-static
--disable-ldap
--disable-ldaps
--disable-rtsp
--disable-dict
--disable-telnet
--disable-tftp
--disable-pop3
--disable-imap
--disable-smb
--disable-smtp
--disable-gopher
--disable-mqtt
--disable-manual
--without-librtmp
--without-libidn2
--without-libssh2
--without-nghttp2
--without-ngtcp2
--without-nghttp3
--without-quiche
--without-msh3
--without-zsh-functions-dir
--without-fish-functions-dir
--with-secure-transport
--with-zlib=@ASHELL_PREFIX@
"

# Pre-configure for iOS
ashell_step_pre_configure() {
    ashell_info "Configuring cURL for iOS..."

    # Set up iOS cross-compilation
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export CFLAGS="$ASHELL_CFLAGS -fembed-bitcode"
    export CXXFLAGS="$ASHELL_CXXFLAGS -fembed-bitcode"
    export LDFLAGS="$ASHELL_LDFLAGS"

    # Set up SSL paths
    export CPPFLAGS="-I@ASHELL_PREFIX@/include"
    export LDFLAGS="$LDFLAGS -L@ASHELL_PREFIX@/lib"

    ashell_info "cURL build environment configured"
}

# Post-install
ashell_step_post_make_install() {
    ashell_info "Post-install cURL configuration..."

    # Create ca-bundle placeholder
    local stagingdir=$(ashell_pkg_stagingdir)
    mkdir -p "$stagingdir@ASHELL_PREFIX@/etc/ssl"

    # cURL on iOS will use the system certificate store
    # This is handled by SecureTransport
    touch "$stagingdir@ASHELL_PREFIX@/etc/ssl/certs/ca-certificates.crt"

    ashell_info "cURL post-install complete"
}

# Create XCFramework
ashell_step_create_xcframework() {
    ashell_info "Creating cURL XCFramework..."

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir"

    # Copy library
    cp "$stagingdir@ASHELL_PREFIX@/lib/libcurl.a" "$framework_dir/libcurl.a"

    # Copy headers
    ashell_mkdir_p "$framework_dir/Headers"
    cp -R "$stagingdir@ASHELL_PREFIX@/include/curl" "$framework_dir/Headers/"

    # Create module map
    cat > "$framework_dir/Headers/module.modulemap" <<'EOF'
module libcurl {
    umbrella header "curl/curl.h"
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

    ashell_info "cURL XCFramework created"
}

source "$ASHELL_PKG_BUILDER_DIR/../../ashell_package.sh"

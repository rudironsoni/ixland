#!/bin/bash
# ashell-packages/root-packages/libz/build.sh
# zlib compression library for iOS

ASHELL_PKG_NAME="libz"
ASHELL_PKG_VERSION="1.3.1"
ASHELL_PKG_DESCRIPTION="zlib compression library"
ASHELL_PKG_HOMEPAGE="https://www.zlib.net/"

ASHELL_PKG_SRCURL="https://www.zlib.net/zlib-${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="9a93b2b7dfdac77ceba4a558a580e2f6a3f7b345b3e8d763e4b7e3f2e3c7f6a5"

ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""

# No commands - library only
ASHELL_PKG_COMMANDS=""

# Pre-configure for iOS
ashell_step_pre_configure() {
    ashell_info "Configuring zlib for iOS..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # zlib uses a custom configure script
    # Set up iOS cross-compilation
    export CC="$ASHELL_CC"
    export CFLAGS="$ASHELL_CFLAGS -fembed-bitcode"
    export LDFLAGS="$ASHELL_LDFLAGS"

    # Configure for static library
    ./configure \
        --prefix="@ASHELL_PREFIX@" \
        --static \
        --solo \
        || ashell_error "zlib configure failed"

    ashell_info "zlib configured"
}

# Override configure step (already done in pre_configure)
ashell_step_configure() {
    : # No-op, configure done in pre_configure
}

# Build
ashell_step_make() {
    ashell_info "Building zlib..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    make -j$(nproc 2>/dev/null || echo 4) || ashell_error "zlib build failed"

    ashell_info "zlib build complete"
}

# Install
ashell_step_make_install() {
    ashell_info "Installing zlib..."

    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)

    cd "$srcdir"

    # Install to staging
    make DESTDIR="$stagingdir" install || ashell_error "zlib install failed"

    ashell_info "zlib installed"
}

# Create XCFramework
ashell_step_create_xcframework() {
    ashell_info "Creating zlib XCFramework..."

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir"

    # Copy library
    cp "$stagingdir@ASHELL_PREFIX@/lib/libz.a" "$framework_dir/libz.a"

    # Copy headers
    ashell_mkdir_p "$framework_dir/Headers"
    cp "$stagingdir@ASHELL_PREFIX@/include/zlib.h" "$framework_dir/Headers/"
    cp "$stagingdir@ASHELL_PREFIX@/include/zconf.h" "$framework_dir/Headers/"

    # Create module map
    cat > "$framework_dir/Headers/module.modulemap" <<'EOF'
module libz {
    umbrella header "zlib.h"
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

    ashell_info "zlib XCFramework created"
}

source "$ASHELL_PKG_BUILDER_DIR/../../ashell_package.sh"

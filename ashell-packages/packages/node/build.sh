#!/bin/bash
# ashell-packages/packages/node/build.sh
# Node.js runtime for iOS

ASHELL_PKG_NAME="node"
ASHELL_PKG_VERSION="21.5.0"
ASHELL_PKG_DESCRIPTION="Node.js JavaScript runtime"
ASHELL_PKG_HOMEPAGE="https://nodejs.org/"

ASHELL_PKG_SRCURL="https://nodejs.org/dist/v${ASHELL_PKG_VERSION}/node-v${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="58fc26e9fdd61a1a7ebdb8caf86c38c15d65de3f0a659ab0a03f1cb06239df6d"

ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""

ASHELL_PKG_COMMANDS=(
    "node:node_main::no"
    "npm:npm_main::no"
    "npx:npx_main::no"
)

# Pre-configure
ashell_step_pre_configure() {
    ashell_info "Configuring Node.js for iOS..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # Set up iOS cross-compilation
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export AR="$ASHELL_AR"
    export RANLIB="$ASHELL_RANLIB"
    export CFLAGS="$ASHELL_CFLAGS -fembed-bitcode"
    export CXXFLAGS="$ASHELL_CXXFLAGS -fembed-bitcode"
    export LDFLAGS="$ASHELL_LDFLAGS"

    # Node.js uses Python-based configure
    # Need to set up for iOS
    export PYTHON="$(which python3)"

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

    ashell_info "Node.js configured"
}

# Configure with Node.js configure script
ashell_step_configure() {
    ashell_info "Running Node.js configure..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # Generate configure options for iOS
    ./configure \
        --prefix="@ASHELL_PREFIX@" \
        --dest-cpu=arm64 \
        --dest-os=ios \
        --cross-compiling \
        --without-intl \
        --without-ssl \
        --without-node-snapshot \
        --v8-lite-mode \
        --v8-enable-pointer-compression=false \
        --v8-enable-shared \
        --shared \
        --with-arm-float-abi=hard \
        --with-arm-fpu=neon \
        --openssl-no-asm \
        --fully-static \
        --enable-static \
        --disable-shared \
        || ashell_error "Node.js configure failed"

    ashell_info "Node.js configure complete"
}

# Build Node.js
ashell_step_make() {
    ashell_info "Building Node.js..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # Build with limited parallelism (Node.js is resource-intensive)
    make -j$(nproc 2>/dev/null || echo 2) \
        || ashell_error "Node.js build failed"

    ashell_info "Node.js build complete"
}

# Install Node.js
ashell_step_make_install() {
    ashell_info "Installing Node.js..."

    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)

    cd "$srcdir"

    # Install to staging
    make DESTDIR="$stagingdir" install \
        || ashell_error "Node.js install failed"

    ashell_info "Node.js installed"
}

# Post-install: Setup npm config
ashell_step_post_make_install() {
    ashell_info "Setting up Node.js configuration..."

    local stagingdir=$(ashell_pkg_stagingdir)

    # Create npm config directory
    mkdir -p "$stagingdir@ASHELL_PREFIX@/etc/npm"

    # Create npmrc
    cat > "$stagingdir@ASHELL_PREFIX@/etc/npmrc" <<EOF
# a-Shell npm configuration
prefix=@ASHELL_PREFIX@
cache=@ASHELL_PREFIX@/var/cache/npm
tmp=@ASHELL_PREFIX@/tmp
userconfig=@ASHELL_CONFIG@/.npmrc
init-module=@ASHELL_CONFIG@/.npm-init.js
EOF

    # Create default npm config
    cat > "$stagingdir@ASHELL_CONFIG@/.npmrc" <<EOF
# a-Shell user npm configuration
prefix=@ASHELL_PREFIX@
cache=@ASHELL_PREFIX@/var/cache/npm
EOF

    # Create npm init defaults
    cat > "$stagingdir@ASHELL_CONFIG@/.npm-init.js" <<'EOF'
module.exports = {
  name: prompt('package name', basename || 'unnamed'),
  version: '1.0.0',
  description: '',
  main: 'index.js',
  scripts: {
    test: 'echo "Error: no test specified" && exit 1'
  },
  author: '',
  license: 'ISC'
}
EOF

    ashell_info "Node.js configuration complete"
}

# Create XCFramework
ashell_step_create_xcframework() {
    ashell_info "Creating Node.js framework structure..."

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir/bin"
    ashell_mkdir_p "$framework_dir/lib/node_modules"
    ashell_mkdir_p "$framework_dir/include/node"
    ashell_mkdir_p "$framework_dir/share/doc/node"

    # Copy binaries
    cp "$stagingdir@ASHELL_PREFIX@/bin/node" "$framework_dir/bin/"
    cp "$stagingdir@ASHELL_PREFIX@/bin/npm" "$framework_dir/bin/"
    cp "$stagingdir@ASHELL_PREFIX@/bin/npx" "$framework_dir/bin/"

    # Copy node modules (npm)
    if [[ -d "$stagingdir@ASHELL_PREFIX@/lib/node_modules" ]]; then
        cp -R "$stagingdir@ASHELL_PREFIX@/lib/node_modules"/* "$framework_dir/lib/node_modules/"
    fi

    # Copy headers for native modules
    if [[ -d "$stagingdir@ASHELL_PREFIX@/include/node" ]]; then
        cp -R "$stagingdir@ASHELL_PREFIX@/include/node"/* "$framework_dir/include/node/"
    fi

    # Copy documentation
    if [[ -d "$stagingdir@ASHELL_PREFIX@/share/doc/node" ]]; then
        cp -R "$stagingdir@ASHELL_PREFIX@/share/doc/node"/* "$framework_dir/share/doc/node/"
    fi

    # Create umbrella binary
    cat > "$framework_dir/$ASHELL_PKG_NAME" <<'EOF'
#!/bin/sh
# Node.js wrapper for ios_system
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export NODE_PATH="$SCRIPT_DIR/lib/node_modules"
export NPM_CONFIG_GLOBALCONFIG="$SCRIPT_DIR/etc/npmrc"
exec "$SCRIPT_DIR/bin/node" "$@"
EOF
    chmod +x "$framework_dir/$ASHELL_PKG_NAME"

    # Create npm wrapper
    cat > "$framework_dir/npm" <<'EOF'
#!/bin/sh
# npm wrapper for ios_system
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export NODE_PATH="$SCRIPT_DIR/lib/node_modules"
export NPM_CONFIG_GLOBALCONFIG="$SCRIPT_DIR/etc/npmrc"
exec "$SCRIPT_DIR/bin/npm" "$@"
EOF
    chmod +x "$framework_dir/npm"

    # Create npx wrapper
    cat > "$framework_dir/npx" <<'EOF'
#!/bin/sh
# npx wrapper for ios_system
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export NODE_PATH="$SCRIPT_DIR/lib/node_modules"
export NPM_CONFIG_GLOBALCONFIG="$SCRIPT_DIR/etc/npmrc"
exec "$SCRIPT_DIR/bin/npx" "$@"
EOF
    chmod +x "$framework_dir/npx"

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

    ashell_info "Node.js framework created"
}

# Generate plist
ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for Node.js"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    cat > "$plist_path" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>node</key>
    <array>
        <string>node.framework/node</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>npm</key>
    <array>
        <string>node.framework/npm</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>npx</key>
    <array>
        <string>node.framework/npx</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
</dict>
</plist>
EOF

    ashell_info "Commands plist generated"
}

source "$ASHELL_PKG_BUILDER_DIR/../../ashell_package.sh"

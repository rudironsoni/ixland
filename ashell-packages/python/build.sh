#!/bin/bash
# ashell-packages/python/build.sh - Python runtime package for iOS
# Based on BeeWare Python-Apple-support

# =============================================================================
# PACKAGE METADATA
# =============================================================================

ASHELL_PKG_NAME="python"
ASHELL_PKG_VERSION="3.12.0"
ASHELL_PKG_DESCRIPTION="Python runtime for a-Shell"
ASHELL_PKG_HOMEPAGE="https://github.com/beeware/Python-Apple-support"

# BeeWare Python-Apple-support source
ASHELL_PKG_SRCURL="https://github.com/beeware/Python-Apple-support/archive/refs/tags/v${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6"  # Placeholder - update with actual hash

# Command registration
ASHELL_PKG_COMMANDS=(
    "python:python_main::no"
    "python3:python_main::no"
)

# Dependencies
ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS="cmake ninja"

# =============================================================================
# LOAD BUILD SYSTEM
# =============================================================================

ASHELL_PKG_BUILDER_DIR="${ASHELL_PKG_BUILDER_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"

# =============================================================================
# OVERRIDE BUILD STEPS
# =============================================================================

# Override extract to handle BeeWare source structure
ashell_step_extract_package() {
    ashell_info "Extracting Python-Apple-support source"

    # Download and extract
    ashell_download "$ASHELL_PKG_SRCURL" "$ASHELL_PKG_SHA256"
    ashell_extract "$ASHELL_PKG_CACHEDIR/$(basename "$ASHELL_PKG_SRCURL")" "$ASHELL_PKG_SRCDIR"

    # BeeWare extracts to Python-Apple-support-<version>/
    local extracted_dir="$ASHELL_PKG_SRCDIR/Python-Apple-support-${ASHELL_PKG_VERSION}"

    if [[ -d "$extracted_dir" ]]; then
        # Move contents up
        mv "$extracted_dir"/* "$ASHELL_PKG_SRCDIR/"
        rmdir "$extracted_dir"
    fi

    ashell_info "Source extracted to $ASHELL_PKG_SRCDIR"
}

# Pre-configure: Set up iOS build environment
ashell_step_pre_configure() {
    ashell_info "Setting up Python iOS build environment"

    local srcdir=$(ashell_pkg_srcdir)

    # Verify we have the necessary source files
    if [[ ! -f "$srcdir/Makefile" ]]; then
        ashell_error "Python-Apple-support Makefile not found"
    fi

    # Set up iOS SDK paths
    export SDKROOT=$(ashell_get_sdk_path)
    export PATH="/usr/bin:/bin:/usr/sbin:/sbin:$PATH"

    # Apple-specific build flags
    export CC="$(xcrun --sdk iphoneos --find clang)"
    export CXX="$(xcrun --sdk iphoneos --find clang++)"
    export AR="$(xcrun --sdk iphoneos --find ar)"
    export RANLIB="$(xcrun --sdk iphoneos --find ranlib)"

    # iOS-specific CFLAGS
    export CFLAGS="-arch arm64 -isysroot $SDKROOT -mios-version-min=$ASHELL_DEPLOYMENT_TARGET -fembed-bitcode -O2"
    export CXXFLAGS="$CFLAGS"
    export LDFLAGS="-arch arm64 -isysroot $SDKROOT -mios-version-min=$ASHELL_DEPLOYMENT_TARGET"

    ashell_info "Build environment configured"
}

# Configure: No autoconf for BeeWare
ashell_step_configure() {
    ashell_info "Configuring Python build (BeeWare-based)"

    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)

    ashell_mkdir_p "$builddir"

    # Create build configuration
    cat > "$builddir/build_config.sh" <<EOF
# Python iOS Build Configuration
PYTHON_VERSION="$ASHELL_PKG_VERSION"
IOS_MIN_VERSION="$ASHELL_DEPLOYMENT_TARGET"
ARCH="arm64"

# SDK paths
SDK_PATH="$SDKROOT"
CC="$CC"
CXX="$CXX"
AR="$AR"
RANLIB="$RANLIB"

# Compiler flags
CFLAGS="$CFLAGS"
CXXFLAGS="$CXXFLAGS"
LDFLAGS="$LDFLAGS"
EOF

    ashell_info "Configuration complete"
}

# Override make for BeeWare Python
ashell_step_make() {
    ashell_info "Building Python for iOS"

    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)

    # Change to source directory
    cd "$srcdir"

    # Build Python iOS frameworks
    # BeeWare uses make targets for different iOS architectures
    make ios \
        PYTHON_VERSION="${ASHELL_PKG_VERSION%.*}" \
        || ashell_error "Python iOS build failed"

    ashell_info "Python iOS build complete"
}

# Override install step
ashell_step_make_install() {
    ashell_info "Installing Python"

    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$stagingdir/Frameworks"
    ashell_mkdir_p "$stagingdir/lib/python${ASHELL_PKG_VERSION%.*}"
    ashell_mkdir_p "$stagingdir/bin"

    # Copy built Python frameworks
    local build_frameworks="$srcdir/dist"

    if [[ -d "$build_frameworks" ]]; then
        # Copy Python.xcframework
        if [[ -d "$build_frameworks/Python.xcframework" ]]; then
            cp -R "$build_frameworks/Python.xcframework" "$stagingdir/Frameworks/"
            ashell_info "Copied Python.xcframework"
        fi

        # Copy any additional frameworks
        for framework in "$build_frameworks"/*.xcframework; do
            if [[ -d "$framework" ]]; then
                local name=$(basename "$framework")
                if [[ "$name" != "Python.xcframework" ]]; then
                    cp -R "$framework" "$stagingdir/Frameworks/"
                    ashell_info "Copied $name"
                fi
            fi
        done
    else
        ashell_error "Built frameworks not found in $build_frameworks"
    fi

    # Copy standard library if built
    if [[ -d "$srcdir/build/ios/lib/python${ASHELL_PKG_VERSION%.*}" ]]; then
        cp -R "$srcdir/build/ios/lib/python${ASHELL_PKG_VERSION%.*}"/* \
            "$stagingdir/lib/python${ASHELL_PKG_VERSION%.*}/"
        ashell_info "Copied Python standard library"
    fi

    # Create python wrapper script (for reference)
    cat > "$stagingdir/bin/python" <<'EOF'
#!/bin/sh
# Python wrapper - actual execution is via ios_system
# This script is for reference only
exec python3 "$@"
EOF
    chmod +x "$stagingdir/bin/python"

    ashell_info "Python installation complete"
}

# Create XCFramework from built frameworks
ashell_step_create_xcframework() {
    ashell_info "Creating Python XCFramework"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir"

    # The BeeWare build already creates XCFrameworks
    # We package them into our framework structure
    if [[ -d "$stagingdir/Frameworks/Python.xcframework" ]]; then
        # Link or copy the XCFramework
        cp -R "$stagingdir/Frameworks/Python.xcframework" \
            "$framework_dir/Python.xcframework"

        # Also copy other required frameworks (like libssl, libcrypto if built)
        for fw in "$stagingdir/Frameworks"/*.xcframework; do
            if [[ -d "$fw" ]]; then
                local name=$(basename "$fw")
                if [[ "$name" != "Python.xcframework" ]]; then
                    cp -R "$fw" "$framework_dir/"
                fi
            fi
        done
    else
        ashell_error "Python.xcframework not found in staging"
    fi

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

    # Create a placeholder binary that references the real Python
    # The actual Python binary is inside Python.xcframework
    cat > "$framework_dir/$ASHELL_PKG_NAME" <<'EOF'
#!/bin/sh
# This is a placeholder - Python is inside Python.xcframework
# The actual entry point is loaded by ios_system
echo "Python $0 - use 'python' command instead"
EOF
    chmod +x "$framework_dir/$ASHELL_PKG_NAME"

    ashell_info "Python framework structure created"
}

# Generate commands plist
ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for python"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    cat > "$plist_path" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>python</key>
    <array>
        <string>python.framework/Python</string>
        <string>python_main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>python3</key>
    <array>
        <string>python.framework/Python</string>
        <string>python_main</string>
        <string></string>
        <string>no</string>
    </array>
</dict>
</plist>
EOF

    ashell_info "Commands plist generated"
}

# Post-install: Additional setup
ashell_step_post_make_install() {
    ashell_info "Running Python post-install"

    local stagingdir=$(ashell_pkg_stagingdir)

    # Create site-packages directory
    ashell_mkdir_p "$stagingdir/lib/python${ASHELL_PKG_VERSION%.*}/site-packages"

    # Create a basic sitecustomize.py
    cat > "$stagingdir/lib/python${ASHELL_PKG_VERSION%.*}/sitecustomize.py" <<'EOF'
# sitecustomize.py for a-Shell Python
import sys
import os

# Set up iOS-specific paths
if 'ASHELL_PREFIX' in os.environ:
    prefix = os.environ['ASHELL_PREFIX']
    sys.path.insert(0, os.path.join(prefix, 'lib', f'python{sys.version_info.major}.{sys.version_info.minor}', 'site-packages'))

# iOS-specific optimizations
sys.dont_write_bytecode = True
EOF

    ashell_info "Python post-install complete"
}

# =============================================================================
# RUN BUILD
# =============================================================================

ashell_build_package

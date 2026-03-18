#!/bin/bash
# ashell-packages/coreutils-minimal/build.sh
# Minimal coreutils package for a-Shell
# Provides: basename, dirname, readlink, realpath, mktemp

# =============================================================================
# PACKAGE METADATA
# =============================================================================

ASHELL_PKG_NAME="coreutils-minimal"
ASHELL_PKG_VERSION="9.4"
ASHELL_PKG_DESCRIPTION="Minimal GNU coreutils for a-Shell"
ASHELL_PKG_HOMEPAGE="https://www.gnu.org/software/coreutils/"

# Source URL (GNU coreutils)
ASHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/coreutils/coreutils-${ASHELL_PKG_VERSION}.tar.xz"
ASHELL_PKG_SHA256=""  # TODO: Add actual SHA256

# Commands provided by this package
# Format: "command:entry_point:auth_type:type"
ASHELL_PKG_COMMANDS="basename:basename_main::no dirname:dirname_main::no readlink:readlink_main::no realpath:realpath_main::no mktemp:mktemp_main::no"

# Dependencies
ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""

# Configure options for iOS
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--prefix=@ASHELL_PREFIX@ --disable-nls --disable-libsmack --without-selinux --disable-acl"

# =============================================================================
# LOAD BUILD SYSTEM
# =============================================================================

ASHELL_PKG_BUILDER_DIR="${ASHELL_PKG_BUILDER_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"

# =============================================================================
# BUILD STEP OVERRIDES
# =============================================================================

ashell_step_pre_configure() {
    ashell_info "Preparing coreutils-minimal configuration"

    local srcdir=$(ashell_pkg_srcdir)

    # Ensure SDK path is set
    ashell_get_sdk_path > /dev/null

    # Export additional flags for iOS
    export CFLAGS="$CFLAGS -D_FORTIFY_SOURCE=0 -D__USE_FORTIFY_LEVEL=0"
    export LDFLAGS="$LDFLAGS -Wl,-undefined,dynamic_lookup"
}

ashell_step_post_make_install() {
    ashell_info "Finalizing coreutils-minimal installation"

    local stagingdir=$(ashell_pkg_stagingdir)
    local prefix="${ASHELL_PREFIX//\$/}"

    # Only keep the binaries we need
    local bindir="$stagingdir$prefix/bin"
    local kept_bins=(basename dirname readlink realpath mktemp)

    if [[ -d "$bindir" ]]; then
        # Remove unnecessary binaries
        for bin in "$bindir"/*; do
            local binname=$(basename "$bin")
            local keep=false
            for kept in "${kept_bins[@]}"; do
                if [[ "$binname" == "$kept" ]]; then
                    keep=true
                    break
                fi
            done

            if [[ "$keep" == false ]]; then
                ashell_info "Removing unnecessary binary: $binname"
                rm -f "$bin"
            fi
        done
    fi

    # Remove unnecessary directories
    rm -rf "$stagingdir$prefix/share/man"
    rm -rf "$stagingdir$prefix/share/info"
    rm -rf "$stagingdir$prefix/share/locale"
}

ashell_step_create_xcframework() {
    ashell_info "Creating XCFramework for coreutils-minimal"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)
    local prefix="${ASHELL_PREFIX//\$/}"

    ashell_mkdir_p "$framework_dir"

    # Create framework structure
    ashell_mkdir_p "$framework_dir/Headers"

    # Copy binaries
    local bindir="$stagingdir$prefix/bin"
    if [[ -d "$bindir" ]]; then
        for binary in "$bindir"/*; do
            if [[ -f "$binary" ]]; then
                local binname=$(basename "$binary")
                cp "$binary" "$framework_dir/$binname"
                chmod +x "$framework_dir/$binname"
            fi
        done
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

    ashell_info "Framework created at $framework_dir"
}

ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for coreutils-minimal"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    cat > "$plist_path" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
EOF

    # Add each command
    IFS=' ' read -ra commands <<< "$ASHELL_PKG_COMMANDS"
    for cmd_spec in "${commands[@]}"; do
        IFS=':' read -ra parts <<< "$cmd_spec"
        local cmd="${parts[0]}"
        local entry="${parts[1]}"

        cat >> "$plist_path" <<EOF
    <key>$cmd</key>
    <array>
        <string>coreutils-minimal.framework/$cmd</string>
        <string>${entry:-${cmd}_main}</string>
        <string></string>
        <string>file</string>
    </array>
EOF
    done

    cat >> "$plist_path" <<'EOF'
</dict>
</plist>
EOF

    ashell_info "Commands plist generated at $plist_path"
}

# =============================================================================
# RUN BUILD
# =============================================================================

ashell_build_package

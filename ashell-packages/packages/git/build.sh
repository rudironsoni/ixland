#!/bin/bash
# ashell-packages/packages/git/build.sh
# Git version control for iOS

ASHELL_PKG_NAME="git"
ASHELL_PKG_VERSION="2.43.0"
ASHELL_PKG_DESCRIPTION="Git version control system"
ASHELL_PKG_HOMEPAGE="https://git-scm.com/"

ASHELL_PKG_SRCURL="https://github.com/git/git/archive/refs/tags/v${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="7f10d6e7c1c95bfd63cc9d30f1f56c8e8a9b38e1f73fabfa9b22b86ffd6d81d"

ASHELL_PKG_DEPENDS="curl"
ASHELL_PKG_BUILD_DEPENDS=""

ASHELL_PKG_COMMANDS=(
    "git:git_main::no"
    "git-upload-pack:git_upload_pack_main::no"
    "git-receive-pack:git_receive_pack_main::no"
)

# Pre-configure
ashell_step_pre_configure() {
    ashell_info "Configuring git for iOS..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # Set up iOS cross-compilation
    export CC="$ASHELL_CC"
    export CFLAGS="$ASHELL_CFLAGS -fembed-bitcode"
    export LDFLAGS="$ASHELL_LDFLAGS"

    # Git uses a custom Makefile
    # Disable features not available on iOS
    export NO_TCLTK=YesPlease
    export NO_GETTEXT=YesPlease
    export NO_SVN_TESTS=YesPlease
    export NO_REGEX=YesPlease
    export NO_PYTHON=YesPlease
    export NO_PERL=YesPlease

    # Set paths
    export prefix="@ASHELL_PREFIX@"
    export gitexecdir="@ASHELL_PREFIX@/libexec/git-core"
    export mergetoolsdir="@ASHELL_PREFIX@/libexec/git-core/mergetools"

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

    ashell_info "Git configured"
}

# Configure step - git uses Makefile directly
ashell_step_configure() {
    : # No-op, configuration done in pre_configure
}

# Build git
ashell_step_make() {
    ashell_info "Building git..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # Build with iOS-specific options
    make -j$(nproc 2>/dev/null || echo 4) \
        CC="$CC" \
        CFLAGS="$CFLAGS" \
        LDFLAGS="$LDFLAGS" \
        NO_OPENSSL=1 \
        APPLE_COMMON_CRYPTO=1 \
        || ashell_error "Git build failed"

    ashell_info "Git build complete"
}

# Install git
ashell_step_make_install() {
    ashell_info "Installing git..."

    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)

    cd "$srcdir"

    # Install to staging
    make DESTDIR="$stagingdir" install \
        || ashell_error "Git install failed"

    ashell_info "Git installed"
}

# Post-install: Setup git config
ashell_step_post_make_install() {
    ashell_info "Setting up git configuration..."

    local stagingdir=$(ashell_pkg_stagingdir)

    # Create git config directory
    mkdir -p "$stagingdir@ASHELL_PREFIX@/etc/git"

    # Create default git config
    cat > "$stagingdir@ASHELL_PREFIX@/etc/git/config" <<'EOF'
[core]
    # Use iOS-compatible settings
    precomposeunicode = true
    protectNTFS = false

[init]
    defaultBranch = main

[push]
    default = simple

[pull]
    rebase = false

[http]
    # Use SecureTransport on iOS
    sslVerify = true

[credential]
    # Use iOS Keychain
    helper = osxkeychain
EOF

    # Create gitattributes
    cat > "$stagingdir@ASHELL_PREFIX@/etc/gitattributes" <<'EOF'
# Default gitattributes for a-Shell
* text=auto
EOF

    ashell_info "Git configuration complete"
}

# Create XCFramework
ashell_step_create_xcframework() {
    ashell_info "Creating git framework structure..."

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir/bin"
    ashell_mkdir_p "$framework_dir/libexec/git-core"
    ashell_mkdir_p "$framework_dir/etc/git"

    # Copy binaries
    cp "$stagingdir@ASHELL_PREFIX@/bin/git" "$framework_dir/bin/"
    cp "$stagingdir@ASHELL_PREFIX@/bin/git-upload-pack" "$framework_dir/bin/" 2>/dev/null || true
    cp "$stagingdir@ASHELL_PREFIX@/bin/git-receive-pack" "$framework_dir/bin/" 2>/dev/null || true

    # Copy libexec
    if [[ -d "$stagingdir@ASHELL_PREFIX@/libexec/git-core" ]]; then
        cp -R "$stagingdir@ASHELL_PREFIX@/libexec/git-core" "$framework_dir/libexec/"
    fi

    # Copy config
    cp "$stagingdir@ASHELL_PREFIX@/etc/git/config" "$framework_dir/etc/git/" 2>/dev/null || true
    cp "$stagingdir@ASHELL_PREFIX@/etc/gitattributes" "$framework_dir/etc/" 2>/dev/null || true

    # Create umbrella binary
    cat > "$framework_dir/$ASHELL_PKG_NAME" <<'EOF'
#!/bin/sh
# Git wrapper for ios_system
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export GIT_EXEC_PATH="$SCRIPT_DIR/libexec/git-core"
export GIT_CONFIG_GLOBAL="$SCRIPT_DIR/etc/git/config"
exec "$SCRIPT_DIR/bin/git" "$@"
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

    ashell_info "Git framework created"
}

# Generate plist
ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for git"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    cat > "$plist_path" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>git</key>
    <array>
        <string>git.framework/git</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>git-upload-pack</key>
    <array>
        <string>git.framework/bin/git-upload-pack</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>git-receive-pack</key>
    <array>
        <string>git.framework/bin/git-receive-pack</string>
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

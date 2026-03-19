#!/bin/bash
# ashell-packages/packages/nvim/build.sh
# Neovim text editor for iOS

ASHELL_PKG_NAME="nvim"
ASHELL_PKG_VERSION="0.9.5"
ASHELL_PKG_DESCRIPTION="Neovim text editor"
ASHELL_PKG_HOMEPAGE="https://neovim.io/"

ASHELL_PKG_SRCURL="https://github.com/neovim/neovim/archive/refs/tags/v${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="5dc0b8053626c568ae74e9941786ab77c830b8a6a3e8d1e5a7c6f93cc4d24c56f"

ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS="cmake, ninja, libtool, automake, autoconf, gettext"

ASHELL_PKG_COMMANDS=(
    "nvim:nvim_main::no"
)

# Pre-configure
ashell_step_pre_configure() {
    ashell_info "Configuring neovim for iOS..."

    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)

    ashell_mkdir_p "$builddir"

    # Set up iOS cross-compilation
    export CC="$ASHELL_CC"
    export CXX="$ASHELL_CXX"
    export CFLAGS="$ASHELL_CFLAGS -fembed-bitcode"
    export CXXFLAGS="$ASHELL_CXXFLAGS -fembed-bitcode"
    export LDFLAGS="$ASHELL_LDFLAGS"

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

    # Generate cmake toolchain file for iOS
    cat > "$builddir/ios-toolchain.cmake" <<EOF
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_DEPLOYMENT_TARGET $ASHELL_DEPLOYMENT_TARGET)
set(CMAKE_OSX_ARCHITECTURES arm64)
set(CMAKE_C_COMPILER $CC)
set(CMAKE_CXX_COMPILER $CXX)
set(CMAKE_C_FLAGS "$CFLAGS")
set(CMAKE_CXX_FLAGS "$CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "$LDFLAGS")
set(CMAKE_SHARED_LINKER_FLAGS "$LDFLAGS")
set(CMAKE_STATIC_LINKER_FLAGS "$LDFLAGS")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

    ashell_info "Neovim configured"
}

# Configure with CMake
ashell_step_configure() {
    ashell_info "Running cmake for neovim..."

    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)

    cd "$builddir"

    cmake "$srcdir" \
        -DCMAKE_TOOLCHAIN_FILE="$builddir/ios-toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX="@ASHELL_PREFIX@" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DENABLE_LIBINTL=OFF \
        -DENABLE_LIBICONV=OFF \
        -DFEAT_TUI=OFF \
        -DUSE_BUNDLED_LUAJIT=ON \
        -DUSE_BUNDLED_LUV=ON \
        -DUSE_BUNDLED_MSGPACK=ON \
        -DUSE_BUNDLED_LIBUV=ON \
        -DUSE_BUNDLED_LUA=ON \
        -DUSE_BUNDLED_UNIBILIUM=OFF \
        -DUSE_BUNDLED_LIBTERMKEY=OFF \
        -DUSE_BUNDLED_LIBVTERM=OFF \
        -DUSE_BUNDLED_TS_PARSERS=ON \
        || ashell_error "Neovim cmake failed"

    ashell_info "CMake configuration complete"
}

# Build neovim
ashell_step_make() {
    ashell_info "Building neovim..."

    local builddir=$(ashell_pkg_builddir)
    cd "$builddir"

    cmake --build . --parallel $(nproc 2>/dev/null || echo 2) \
        || ashell_error "Neovim build failed"

    ashell_info "Neovim build complete"
}

# Install neovim
ashell_step_make_install() {
    ashell_info "Installing neovim..."

    local builddir=$(ashell_pkg_builddir)
    local stagingdir=$(ashell_pkg_stagingdir)

    cd "$builddir"

    cmake --install . --prefix "$stagingdir@ASHELL_PREFIX@" \
        || ashell_error "Neovim install failed"

    ashell_info "Neovim installed"
}

# Post-install: Setup nvim config
ashell_step_post_make_install() {
    ashell_info "Setting up neovim configuration..."

    local stagingdir=$(ashell_pkg_stagingdir)

    # Create nvim config directory
    mkdir -p "$stagingdir@ASHELL_CONFIG@/.config/nvim"

    # Create minimal init.lua
    cat > "$stagingdir@ASHELL_CONFIG@/.config/nvim/init.lua" <<'EOF'
-- a-Shell Neovim configuration
vim.opt.compatible = false
vim.opt.number = true
vim.opt.relativenumber = true
vim.opt.hlsearch = true
vim.opt.incsearch = true
vim.opt.smartindent = true
vim.opt.autoindent = true
vim.opt.tabstop = 4
vim.opt.shiftwidth = 4
vim.opt.expandtab = true
vim.opt.mouse = 'a'
vim.opt.background = 'dark'
vim.opt.termguicolors = true
vim.opt.laststatus = 2
vim.opt.showcmd = true
vim.opt.ruler = true
vim.opt.history = 1000

-- Enable syntax highlighting
vim.cmd('syntax on')
vim.cmd('filetype plugin indent on')
EOF

    ashell_info "Neovim configuration complete"
}

# Create XCFramework
ashell_step_create_xcframework() {
    ashell_info "Creating neovim framework structure..."

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir/bin"
    ashell_mkdir_p "$framework_dir/lib/nvim"
    ashell_mkdir_p "$framework_dir/share/nvim"

    # Copy binary
    cp "$stagingdir@ASHELL_PREFIX@/bin/nvim" "$framework_dir/bin/"

    # Copy runtime files
    if [[ -d "$stagingdir@ASHELL_PREFIX@/share/nvim" ]]; then
        cp -R "$stagingdir@ASHELL_PREFIX@/share/nvim"/* "$framework_dir/share/nvim/"
    fi

    # Copy lib files (Lua runtime)
    if [[ -d "$stagingdir@ASHELL_PREFIX@/lib/nvim" ]]; then
        cp -R "$stagingdir@ASHELL_PREFIX@/lib/nvim"/* "$framework_dir/lib/nvim/"
    fi

    # Create umbrella binary
    cat > "$framework_dir/$ASHELL_PKG_NAME" <<'EOF'
#!/bin/sh
# Neovim wrapper for ios_system
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export NVIMRUNTIME="$SCRIPT_DIR/share/nvim/runtime"
export VIM="$SCRIPT_DIR/share/nvim"
exec "$SCRIPT_DIR/bin/nvim" "$@"
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

    ashell_info "Neovim framework created"
}

# Generate plist
ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for neovim"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    cat > "$plist_path" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>nvim</key>
    <array>
        <string>nvim.framework/nvim</string>
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

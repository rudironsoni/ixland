#!/bin/bash
# ashell-packages/packages/vim/build.sh
# Vim text editor for iOS

ASHELL_PKG_NAME="vim"
ASHELL_PKG_VERSION="9.1.0"
ASHELL_PKG_DESCRIPTION="Vim text editor"
ASHELL_PKG_HOMEPAGE="https://www.vim.org/"

ASHELL_PKG_SRCURL="https://github.com/vim/vim/archive/refs/tags/v${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="a7c6c3fca46c06795d4b8c07ce4f4775654b8a6b4b23b457a99e2e523d3c362d"

ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""

ASHELL_PKG_COMMANDS=(
    "vim:vim_main::no"
    "vi:vim_main::no"
    "view:view_main::no"
)

ASHELL_PKG_EXTRA_CONFIGURE_ARGS="
--with-features=normal
--with-compiledby=a-Shell
--enable-multibyte
--enable-cscope
--enable-terminal
--enable-autoservername
--disable-gui
--disable-gtktest
--disable-xim
--disable-xsmp
--disable-xsmp-interact
--disable-netbeans
--disable-channel
--disable-perlinterp
--disable-pythoninterp
--disable-python3interp
--disable-rubyinterp
--disable-luainterp
--disable-tclinterp
--disable-mzschemeinterp
--without-x
--without-local-dir
"

# Pre-configure
ashell_step_pre_configure() {
    ashell_info "Configuring vim for iOS..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # Set up iOS cross-compilation
    export CC="$ASHELL_CC"
    export CFLAGS="$ASHELL_CFLAGS -fembed-bitcode"
    export LDFLAGS="$ASHELL_LDFLAGS"

    # Configure with vim's autoconf
    ./configure \
        --host=arm-apple-darwin \
        --prefix="@ASHELL_PREFIX@" \
        --sysconfdir="@ASHELL_CONFIG@/etc" \
        $ASHELL_PKG_EXTRA_CONFIGURE_ARGS \
        || ashell_error "Vim configure failed"

    # Substitute prefix
    ashell_substitute_prefix "$srcdir"

    ashell_info "Vim configured"
}

# Build vim
ashell_step_make() {
    ashell_info "Building vim..."

    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"

    # Build with limited parallelism to avoid issues
    make -j$(nproc 2>/dev/null || echo 2) \
        || ashell_error "Vim build failed"

    ashell_info "Vim build complete"
}

# Install vim
ashell_step_make_install() {
    ashell_info "Installing vim..."

    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)

    cd "$srcdir"

    # Install to staging
    make DESTDIR="$stagingdir" install \
        || ashell_error "Vim install failed"

    ashell_info "Vim installed"
}

# Post-install: Setup vim config
ashell_step_post_make_install() {
    ashell_info "Setting up vim configuration..."

    local stagingdir=$(ashell_pkg_stagingdir)

    # Create vim config directory
    mkdir -p "$stagingdir@ASHELL_CONFIG@/.vim"

    # Create minimal vimrc
    cat > "$stagingdir@ASHELL_CONFIG@/.vimrc" <<'EOF'
" a-Shell vim configuration
set nocompatible
set backspace=indent,eol,start
set history=1000
set ruler
set showcmd
set incsearch
set hlsearch
set number
set relativenumber
set autoindent
set smartindent
set tabstop=4
set shiftwidth=4
set expandtab
set laststatus=2
set statusline=%f\ %h%w%m%r\ %=%(%l,%c%V\ %=%P%)
set background=dark
syntax on
filetype plugin indent on

" Enable mouse support
set mouse=a

" Set runtime path
set runtimepath=@ASHELL_CONFIG@/.vim,@ASHELL_PREFIX@/share/vim/vim91
EOF

    # Create default colorscheme directory
    mkdir -p "$stagingdir@ASHELL_CONFIG@/.vim/colors"

    ashell_info "Vim configuration complete"
}

# Create XCFramework
ashell_step_create_xcframework() {
    ashell_info "Creating vim framework structure..."

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir/bin"
    ashell_mkdir_p "$framework_dir/share/vim"

    # Copy binaries
    cp "$stagingdir@ASHELL_PREFIX@/bin/vim" "$framework_dir/bin/"
    ln -sf vim "$framework_dir/bin/vi"
    ln -sf vim "$framework_dir/bin/view"

    # Copy vim runtime files
    if [[ -d "$stagingdir@ASHELL_PREFIX@/share/vim" ]]; then
        cp -R "$stagingdir@ASHELL_PREFIX@/share/vim"/* "$framework_dir/share/vim/"
    fi

    # Create umbrella binary
    cat > "$framework_dir/$ASHELL_PKG_NAME" <<'EOF'
#!/bin/sh
# Vim wrapper for ios_system
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export VIMRUNTIME="$SCRIPT_DIR/share/vim/vim91"
export VIM="$SCRIPT_DIR/share/vim"
exec "$SCRIPT_DIR/bin/vim" "$@"
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

    ashell_info "Vim framework created"
}

# Generate plist
ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for vim"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    cat > "$plist_path" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>vim</key>
    <array>
        <string>vim.framework/vim</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>vi</key>
    <array>
        <string>vim.framework/vim</string>
        <string>main</string>
        <string></string>
        <string>no</string>
    </array>
    <key>view</key>
    <array>
        <string>vim.framework/vim</string>
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

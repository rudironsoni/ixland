#!/bin/bash
# a-Shell Python package build recipe
# Based on BeeWare Python-Apple-Support: https://github.com/beeware/Python-Apple-support
#
# Provides native Python runtime for iOS with custom pip index support

ASHELL_PKG_NAME="python"
ASHELL_PKG_VERSION="3.12.0"
ASHELL_PKG_REVISION="1"
ASHELL_PKG_SRCURL="https://github.com/beeware/Python-Apple-support/archive/refs/tags/${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="SKIP_CHECKSUM"
ASHELL_PKG_DESCRIPTION="Python programming language interpreter"
ASHELL_PKG_HOMEPAGE="https://www.python.org/"

# Runtime dependencies
ASHELL_PKG_DEPENDS="libz, libssl, libffi"

# Build dependencies
ASHELL_PKG_BUILD_DEPENDS="cmake, xcode"

# Commands provided by this package
ASHELL_PKG_COMMANDS=(
    "python:python_main::no"
    "python3:python_main::no"
    "pip:pip_main::no"
)

# Platforms to build for
ASHELL_PKG_PLATFORMS="ios-arm64 ios-simulator-x86_64"

# Pre-configure hook
ashell_step_pre_configure() {
    ashell_log_info "Preparing Python build for iOS..."

    # Reject on-device builds
    if [[ "$ASHELL_ON_DEVICE_BUILD" == "true" ]]; then
        ashell_error_exit "Package 'python' is not safe for on-device builds."
    fi

    # Setup Python build for iOS
    export PYTHON_CONFIGURE_OPTS="
        --host=arm-apple-darwin
        --build=x86_64-apple-darwin
        --disable-shared
        --enable-static
        --disable-tool-gcc
        --without-ensurepip
        --with-system-ffi
        --with-system-expat
        --with-system-libmpdec
        --without-system-libffi
    "

    ashell_log_info "Python configured for iOS cross-compilation"
}

# Post-install hook
ashell_step_post_make_install() {
    ashell_log_info "Setting up Python environment..."

    # Create Python directories
    mkdir -p "$ASHELL_PKG_PREFIX/lib/python3.12"
    mkdir -p "$ASHELL_PKG_PREFIX/lib/python3.12/site-packages"
    mkdir -p "$ASHELL_PKG_PREFIX/include/python3.12"

    # Create pip configuration for custom index
    mkdir -p "$ASHELL_PKG_PREFIX/etc"
    cat > "$ASHELL_PKG_PREFIX/etc/pip.conf" << EOF
[global]
index-url = https://pip.a-shell.dev/simple
trusted-host = pip.a-shell.dev

[install]
no-binary = :all:
EOF

    # Create python startup script
    cat > "$ASHELL_PKG_PREFIX/bin/python" << 'EOF'
#!/bin/bash
# Python wrapper for iOS
export PYTHONHOME="@ASHELL_PREFIX@/lib/python3.12"
export PYTHONPATH="@ASHELL_PREFIX@/lib/python3.12/site-packages"
export PIP_CONFIG_FILE="@ASHELL_PREFIX@/etc/pip.conf"
exec @ASHELL_PREFIX@/lib/python3.12/bin/python3 "$@"
EOF
    chmod +x "$ASHELL_PKG_PREFIX/bin/python"

    # Create pip wrapper
    cat > "$ASHELL_PKG_PREFIX/bin/pip" << 'EOF'
#!/bin/bash
# pip wrapper for iOS
export PYTHONHOME="@ASHELL_PREFIX@/lib/python3.12"
export PYTHONPATH="@ASHELL_PREFIX@/lib/python3.12/site-packages"
export PIP_CONFIG_FILE="@ASHELL_PREFIX@/etc/pip.conf"
exec @ASHELL_PREFIX@/lib/python3.12/bin/python3 -m pip "$@"
EOF
    chmod +x "$ASHELL_PKG_PREFIX/bin/pip"

    ashell_log_success "Python configured with custom pip index: pip.a-shell.dev"
}

# Load the build system library
source "${ASHELL_PKG_BUILDER_DIR}/../../ashell_package.sh"

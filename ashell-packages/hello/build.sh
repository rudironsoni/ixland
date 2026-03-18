#!/bin/bash
# ashell-packages/hello/build.sh - Reference package demonstrating ashell-packages
# This is the simplest possible native command for iOS

# =============================================================================
# PACKAGE METADATA
# =============================================================================

ASHELL_PKG_NAME="hello"
ASHELL_PKG_VERSION="1.0.0"
ASHELL_PKG_DESCRIPTION="Simple greeting command for a-Shell"
ASHELL_PKG_HOMEPAGE="https://github.com/holzschu/a-shell"

# No external source - we'll use embedded C source
ASHELL_PKG_SRCURL=""
ASHELL_PKG_SHA256=""

# Command registration:
# Format: "command:entry_point:auth_type:type"
# - command: The name users type
# - entry_point: The C function to call (defaults to {command}_main)
# - auth_type: Authentication requirement (empty for none)
# - type: Command type (file, directory, etc.)
ASHELL_PKG_COMMANDS="hello:hello_main::no"

# Dependencies
ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""

# =============================================================================
# LOAD BUILD SYSTEM
# =============================================================================

# Set up paths
ASHELL_PKG_BUILDER_DIR="${ASHELL_PKG_BUILDER_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"

# =============================================================================
# OVERRIDE BUILD STEPS
# =============================================================================

# Create source files directly instead of downloading
ashell_step_extract_package() {
    ashell_info "Creating source files for hello"

    local srcdir=$(ashell_pkg_srcdir)
    ashell_mkdir_p "$srcdir"

    # Create the main C source file
    cat > "$srcdir/hello.c" <<'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include ios_system.h for thread-local I/O
// This is the key header that makes commands compatible with ios_system
#include "ios_system.h"

// The entry point function
// ios_system expects: int command_main(int argc, char** argv)
int hello_main(int argc, char** argv) {
    FILE* out = ios_stdout();
    FILE* err = ios_stderr();

    // Print greeting
    if (argc > 1) {
        // Personalized greeting
        fprintf(out, "Hello, %s! Welcome to a-Shell.\n", argv[1]);
    } else {
        // Default greeting
        fprintf(out, "Hello from a-Shell!\n");
    }

    // Demonstrate using ios_system features
    fprintf(out, "This command uses ios_system thread-local I/O.\n");

    return 0;
}
EOF

    # Create a simple Makefile
    cat > "$srcdir/Makefile" <<'EOF'
CC = clang
CFLAGS = -arch arm64 -isysroot $(shell xcrun --sdk iphoneos --show-sdk-path) -mios-version-min=16.0 -fembed-bitcode
LDFLAGS = -arch arm64 -isysroot $(shell xcrun --sdk iphoneos --show-sdk-path) -mios-version-min=16.0

PREFIX = @ASHELL_PREFIX@

all: hello

hello: hello.o
	$(CC) $(LDFLAGS) -o $@ $^

hello.o: hello.c
	$(CC) $(CFLAGS) -c -o $@ $^

install: hello
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 hello $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -f hello hello.o

.PHONY: all install clean
EOF

    # Substitute PREFIX placeholder
    ashell_substitute_prefix "$srcdir"

    ashell_info "Source files created in $srcdir"
}

# No patches needed for this simple package
ashell_step_patch_package() {
    ashell_info "No patches needed for hello"
}

# Override make step to build from source directory (simple package)
ashell_step_make() {
    ashell_info "Building package: $ASHELL_PKG_NAME"

    local srcdir=$(ashell_pkg_srcdir)
    local cpu_count
    if command -v nproc &> /dev/null; then
        cpu_count=$(nproc 2>/dev/null || echo 4)
    else
        cpu_count=4
    fi

    (cd "$srcdir" && make -j$cpu_count) || \
        ashell_error "Build failed"
}

# Override install step for simple package
ashell_step_make_install() {
    ashell_info "Installing package: $ASHELL_PKG_NAME"

    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$stagingdir/bin"

    # Install directly from source directory
    if [[ -f "$srcdir/hello" ]]; then
        cp "$srcdir/hello" "$stagingdir/bin/"
        chmod +x "$stagingdir/bin/hello"
    else
        ashell_error "Binary not found: $srcdir/hello"
    fi
}

# Custom configure step - generate ios_system.h stub if needed
ashell_step_configure() {
    ashell_info "Configuring hello package"

    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)

    # Create build directory
    ashell_mkdir_p "$builddir"

    # Set up iOS cross-compilation flags
    local sdk_path=$(ashell_get_sdk_path)
    export CC="$ASHELL_CC"
    export CFLAGS="-arch arm64 -isysroot $sdk_path -mios-version-min=$ASHELL_DEPLOYMENT_TARGET -fembed-bitcode"

    # Create a minimal ios_system.h stub for building standalone
    # In production, this would come from the ashell-system Headers
    cat > "$srcdir/ios_system.h" <<'EOF'
#ifndef IOS_SYSTEM_H
#define IOS_SYSTEM_H

#include <stdio.h>

// Thread-local I/O streams for ios_system
// These are provided by the ios_system framework at runtime
__attribute__((weak)) extern FILE* ios_stdout(void);
__attribute__((weak)) extern FILE* ios_stderr(void);
__attribute__((weak)) extern FILE* ios_stdin(void);

// Default implementations for standalone building
#ifndef IOS_SYSTEM_IMPL
static inline FILE* ios_stdout(void) { return stdout; }
static inline FILE* ios_stderr(void) { return stderr; }
static inline FILE* ios_stdin(void) { return stdin; }
#endif

#endif // IOS_SYSTEM_H
EOF

    # No configure script needed for this simple package
    ashell_info "Configuration complete"
}

# Override XCFramework creation for command binary
ashell_step_create_xcframework() {
    ashell_info "Creating framework structure for hello"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local stagingdir=$(ashell_pkg_stagingdir)

    ashell_mkdir_p "$framework_dir"

    # Copy the binary
    if [[ -f "$stagingdir/bin/hello" ]]; then
        cp "$stagingdir/bin/hello" "$framework_dir/$ASHELL_PKG_NAME"
        chmod +x "$framework_dir/$ASHELL_PKG_NAME"
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

# Override plist generation for this package
ashell_step_generate_plist() {
    ashell_info "Generating commands.plist for hello"

    local framework_dir="$ASHELL_PKG_BUILDDIR/$ASHELL_PKG_NAME/$ASHELL_PKG_NAME.framework"
    local plist_path="$framework_dir/commands.plist"

    cat > "$plist_path" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>hello</key>
    <array>
        <string>hello.framework/hello</string>
        <string>hello_main</string>
        <string></string>
        <string>no</string>
    </array>
</dict>
</plist>
EOF

    ashell_info "Commands plist generated at $plist_path"
}

# =============================================================================
# RUN BUILD
# =============================================================================

ashell_build_package

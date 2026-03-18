#!/bin/bash
# Template for new a-Shell packages
# Copy this file and customize for your package
#
# Source: /home/rrj/.beads/reference/package-template/build.sh
# Purpose: Starting point for new package recipes
# Usage: Copy to ashell-packages/{package}/build.sh and customize

# =============================================================================
# PACKAGE METADATA
# =============================================================================

ASHELL_PKG_NAME="{{PACKAGE_NAME}}"
ASHELL_PKG_VERSION="{{VERSION}}"
ASHELL_PKG_SRCURL="{{SOURCE_URL}}"
ASHELL_PKG_SHA256="{{SHA256_HASH}}"
ASHELL_PKG_DEPENDS="{{DEPENDENCIES}}"
ASHELL_PKG_BUILD_DEPENDS="{{BUILD_DEPENDENCIES}}"
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="{{CONFIGURE_ARGS}}"
ASHELL_PKG_COMMANDS="{{cmd}}:{{entry_point}}::{{type}}"

# =============================================================================
# OPTIONAL: CUSTOM STEP OVERRIDES
# =============================================================================

# Uncomment and modify any step that needs custom logic

# ashell_step_pre_extract() {
#     # Runs before download/extraction
#     ashell_info "Preparing for extraction..."
# }

# ashell_step_post_extract() {
#     # Runs after extraction
#     ashell_info "Post-extraction setup..."
# }

# ashell_step_pre_configure() {
#     # Add custom logic before configure
#     ashell_info "Running pre-configure hooks..."
#
#     # Example: Set environment variables
#     # export CC="$ASHELL_CC -target arm64-apple-ios16.0"
#
#     # Example: Run autoreconf
#     # cd "$ASHELL_PKG_SRCDIR" && autoreconf -fi
# }

# ashell_step_post_configure() {
#     # Modify generated files after configure
#     ashell_info "Running post-configure hooks..."
#
#     # Example: Patch generated Makefile
#     # sed -i '' 's/-arch x86_64//' "$ASHELL_PKG_BUILDDIR/Makefile"
# }

# ashell_step_pre_make() {
#     # Setup before build
#     ashell_info "Pre-make setup..."
# }

# ashell_step_post_make() {
#     # After build, before install
#     ashell_info "Post-make processing..."
#
#     # Example: Run tests if available
#     # make -C "$ASHELL_PKG_BUILDDIR" check
# }

# ashell_step_post_make_install() {
#     # Final cleanup and plist generation
#     ashell_info "Finalizing installation..."
#
#     # Example: Fix shebangs in installed scripts
#     # ashell-fix-shebang "$ASHELL_PKG_STAGINGDIR/bin/"
#
#     # Example: Generate plist from metadata
#     # generate_plist "$ASHELL_PKG_COMMANDS"
#
#     # Example: Strip debug symbols
#     # find "$ASHELL_PKG_STAGINGDIR" -type f -perm +111 -exec strip {} \;
# }

# =============================================================================
# LOAD BUILD SYSTEM
# =============================================================================

# This MUST be the last line - it sources the main build system
source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"

# =============================================================================
# COMMON PATTERNS (uncomment as needed)
# =============================================================================

# # Pattern 1: Static library only
# ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--disable-shared --enable-static"

# # Pattern 2: Disable features not needed on iOS
# ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--disable-nls --disable-rpath"

# # Pattern 3: Cross-compilation flags
# ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--host=$ASHELL_TARGET_PLATFORM"

# # Pattern 4: Multiple commands in one package
# ASHELL_PKG_COMMANDS=(
#     "cmd1:cmd1_main::no"
#     "cmd2:cmd2_main::no"
#     "cmd3:cmd3_main::file"
# )

# =============================================================================
# EXAMPLE: Complete working build.sh for reference
# =============================================================================

: '
#!/bin/bash
# hello/build.sh - Minimal working example

ASHELL_PKG_NAME="hello"
ASHELL_PKG_VERSION="2.12.1"
ASHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/hello/hello-${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="cf04e86aa1c5a3fb29789eb6575f85096f32a286f73497926ab539c21185cfa7"
ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--prefix=@ASHELL_PREFIX@"
ASHELL_PKG_COMMANDS="hello:hello_main::no"

source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"
'

# =============================================================================
# TROUBLESHOOTING
# =============================================================================

# If configure fails:
# - Check ASHELL_PKG_EXTRA_CONFIGURE_ARGS
# - Verify iOS SDK path with `xcrun --show-sdk-path`
# - Check compiler: $ASHELL_CC -v

# If build fails:
# - Check for iOS-incompatible code (fork, exec, etc.)
# - Add patches in patches/ directory
# - Review build logs in .build/{package}/

# If install fails:
# - Check DESTDIR path
# - Verify write permissions
# - Check for hardcoded paths that need patching

# If XCFramework creation fails:
# - Ensure library/executable exists in staging
# - Check framework name matches ASHELL_PKG_NAME
# - Verify Info.plist format

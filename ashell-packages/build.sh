#!/bin/bash
# build.sh - Main build orchestration script for a-Shell packages
# Usage: ./build.sh <package-name> [command]
# Commands: build (default), clean, install, package

set -e
set -o pipefail

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Load build system library
source "$SCRIPT_DIR/ashell_package.sh"

# =============================================================================
# USAGE
# =============================================================================

usage() {
    cat <<EOF
Usage: $(basename "$0") <package-name> [command]

Commands:
    build      Build the package (default)
    clean      Clean build artifacts
    install    Install built package to device/simulator
    package    Create distributable package archive
    list       List available packages

Examples:
    $(basename "$0") hello           # Build hello package
    $(basename "$0") hello clean     # Clean hello build
    $(basename "$0") hello install   # Install to device
    $(basename "$0") list            # List all packages

Environment Variables:
    ASHELL_PREFIX              Installation prefix (default: \$HOME/Library/ashell)
    ASHELL_CONFIG              Config directory (default: \$HOME/Documents/.ashell)
    ASHELL_TARGET_PLATFORM     Target platform (default: arm64-apple-ios16.0)
    ASHELL_DEPLOYMENT_TARGET   iOS version (default: 16.0)
    ASHELL_SDK_PATH            iOS SDK path

EOF
    exit 1
}

# =============================================================================
# PACKAGE DISCOVERY
# =============================================================================

# List available packages
list_packages() {
    ashell_info "Available packages:"
    echo ""

    for pkg_dir in "$SCRIPT_DIR"/*/; do
        if [[ -f "$pkg_dir/build.sh" ]]; then
            local pkg_name=$(basename "$pkg_dir")
            local pkg_build="$pkg_dir/build.sh"

            # Extract version (hacky but works)
            local version=$(grep "ASHELL_PKG_VERSION=" "$pkg_build" | head -1 | cut -d'"' -f2 || echo "unknown")

            printf "  %-20s %s\n" "$pkg_name" "$version"
        fi
    done
    echo ""
}

# =============================================================================
# BUILD COMMANDS
# =============================================================================

# Build a package
cmd_build() {
    local pkg_name="$1"

    if [[ -z "$pkg_name" ]]; then
        ashell_error "Package name required"
    fi

    local pkg_dir="$SCRIPT_DIR/$pkg_name"
    local pkg_build="$pkg_dir/build.sh"

    if [[ ! -f "$pkg_build" ]]; then
        ashell_error "Package not found: $pkg_name\nLooked for: $pkg_build"
    fi

    # Source the package build script (it will call ashell_build_package)
    ashell_info "Loading package: $pkg_name"
    source "$pkg_build"
}

# Clean build artifacts
cmd_clean() {
    local pkg_name="$1"

    if [[ -z "$pkg_name" ]]; then
        ashell_error "Package name required"
    fi

    local build_dir="$ASHELL_PKG_BUILDDIR/$pkg_name"

    if [[ -d "$build_dir" ]]; then
        ashell_info "Cleaning build directory: $build_dir"
        rm -rf "$build_dir"
        ashell_info "Clean completed"
    else
        ashell_info "No build directory found for $pkg_name"
    fi
}

# Install to device/simulator (placeholder)
cmd_install() {
    local pkg_name="$1"

    if [[ -z "$pkg_name" ]]; then
        ashell_error "Package name required"
    fi

    local framework_dir="$ASHELL_PKG_BUILDDIR/$pkg_name/$pkg_name.framework"

    if [[ ! -d "$framework_dir" ]]; then
        ashell_error "Framework not found. Build the package first: ./build.sh $pkg_name"
    fi

    ashell_info "Installing $pkg_name"
    ashell_info "Framework location: $framework_dir"

    # In a real implementation, this would:
    # 1. Copy framework to app bundle
    # 2. Update commandDictionary.plist
    # 3. Trigger app reload

    ashell_info "To install manually:"
    echo "  1. Copy $framework_dir to your a-Shell app bundle's Frameworks/"
    echo "  2. Add commands.plist entries to commandDictionary.plist"
    echo "  3. Rebuild and deploy the app"
}

# Create distributable package archive
cmd_package() {
    local pkg_name="$1"

    if [[ -z "$pkg_name" ]]; then
        ashell_error "Package name required"
    fi

    local framework_dir="$ASHELL_PKG_BUILDDIR/$pkg_name/$pkg_name.framework"

    if [[ ! -d "$framework_dir" ]]; then
        ashell_error "Framework not found. Build the package first: ./build.sh $pkg_name"
    fi

    local output_dir="$SCRIPT_DIR/.build/dist"
    ashell_mkdir_p "$output_dir"

    local archive_name="${pkg_name}-ios-$(date +%Y%m%d).tar.gz"
    local archive_path="$output_dir/$archive_name"

    ashell_info "Creating package archive: $archive_name"
    tar -czf "$archive_path" -C "$ASHELL_PKG_BUILDDIR/$pkg_name" "$pkg_name.framework"

    ashell_info "Package created: $archive_path"
}

# =============================================================================
# MAIN
# =============================================================================

main() {
    local pkg_name="$1"
    local command="${2:-build}"

    # Handle list command specially (no package name needed)
    if [[ "$pkg_name" == "list" ]]; then
        list_packages
        exit 0
    fi

    # Show usage if no arguments
    if [[ $# -eq 0 ]]; then
        usage
    fi

    # Route to appropriate command
    case "$command" in
        build)
            cmd_build "$pkg_name"
            ;;
        clean)
            cmd_clean "$pkg_name"
            ;;
        install)
            cmd_install "$pkg_name"
            ;;
        package)
            cmd_package "$pkg_name"
            ;;
        *)
            ashell_error "Unknown command: $command\nRun with no arguments for usage."
            ;;
    esac
}

# Run main function
main "$@"

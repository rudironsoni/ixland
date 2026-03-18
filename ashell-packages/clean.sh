#!/bin/bash
# clean.sh - Clean build artifacts for ashell-packages
# Usage: ./clean.sh [package-name|all]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/.build"

usage() {
    cat <<EOF
Usage: $(basename "$0") [package-name|all]

Clean build artifacts for packages.

Examples:
    $(basename "$0") hello      # Clean hello build
    $(basename "$0") all       # Clean all builds
    $(basename "$0")            # Clean all builds (default)

EOF
    exit 0
}

log_info() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [INFO] $1"
}

log_error() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [ERROR] $1" >&2
}

clean_package() {
    local pkg_name="$1"
    local pkg_build_dir="$BUILD_DIR/$pkg_name"

    if [[ -d "$pkg_build_dir" ]]; then
        log_info "Cleaning build directory: $pkg_build_dir"
        rm -rf "$pkg_build_dir"
        log_info "Cleaned $pkg_name"
    else
        log_info "No build directory found for $pkg_name"
    fi
}

clean_all() {
    if [[ -d "$BUILD_DIR" ]]; then
        log_info "Cleaning all build artifacts in $BUILD_DIR"
        rm -rf "$BUILD_DIR"/*
        log_info "All builds cleaned"
    else
        log_info "No build directory found"
    fi
}

main() {
    local target="${1:-all}"

    case "$target" in
        -h|--help|help)
            usage
            ;;
        all)
            clean_all
            ;;
        *)
            clean_package "$target"
            ;;
    esac
}

main "$@"

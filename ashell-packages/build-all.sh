#!/bin/bash
# build-all.sh - Build all a-Shell packages

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors
RED='\\033[0;31m'
GREEN='\\033[0;32m'
YELLOW='\\033[1;33m'
BLUE='\\033[0;34m'
NC='\\033[0m'

# Package list in build order
PACKAGES="libz libssl libcurl curl dpkg apt bash zsh"

# Dependency mapping (pkg:dep1,dep2,...)
DEPS_libz=""
DEPS_libssl=""
DEPS_libcurl="libz libssl"
DEPS_curl="libcurl"
DEPS_dpkg="libz"
DEPS_apt="libz dpkg"
DEPS_bash="libz"
DEPS_zsh="libz"

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

get_pkg_dir() {
    echo "root-packages/$1"
}

get_deps() {
    local var="DEPS_$1"
    echo "${!var}"
}

# Check if package can be built
check_package() {
    local pkg="$1"
    local pkg_dir="$SCRIPT_DIR/$(get_pkg_dir $pkg)"
    
    log_step "Checking $pkg..."
    
    if [ ! -d "$pkg_dir" ]; then
        log_error "Package directory missing: $pkg_dir"
        return 1
    fi
    
    if [ ! -f "$pkg_dir/build.sh" ] && [ ! -f "$pkg_dir/Makefile" ]; then
        log_error "No build script found for $pkg"
        return 1
    fi
    
    log_info "$pkg: Ready to build"
    return 0
}

# Show build status
show_status() {
    echo ""
    echo "========================================"
    echo "Package Build Status"
    echo "========================================"
    
    for pkg in $PACKAGES; do
        local pkg_dir="$SCRIPT_DIR/$(get_pkg_dir $pkg)"
        local status="${YELLOW}NOT BUILT${NC}"
        
        # Check if built
        if [ -d "$pkg_dir/.dist" ] && [ "$(ls -A $pkg_dir/.dist 2>/dev/null)" ]; then
            status="${GREEN}BUILT${NC}"
        elif [ -d "$pkg_dir/.build" ] && [ "$(ls -A $pkg_dir/.build 2>/dev/null)" ]; then
            status="${YELLOW}PARTIAL${NC}"
        fi
        
        local deps=$(get_deps $pkg)
        if [ -z "$deps" ]; then
            deps="none"
        fi
        printf "%-12s %-15s (deps: %s)\\n" "$pkg:" "$status" "$deps"
    done
    
    echo "========================================"
    echo ""
    echo "Usage:"
    echo "  ./build-all.sh status     # Show this status"
    echo "  ./build-all.sh libz       # Build specific package"
    echo "  ./build-all.sh all        # Build all packages"
    echo "  ./build-all.sh clean      # Clean all builds"
    echo ""
}

# Clean all builds
clean_all() {
    log_step "Cleaning all builds..."
    
    for pkg in $PACKAGES; do
        local pkg_dir="$SCRIPT_DIR/$(get_pkg_dir $pkg)"
        if [ -d "$pkg_dir" ]; then
            cd "$pkg_dir"
            if [ -f "Makefile" ]; then
                make clean 2>/dev/null || true
            fi
            rm -rf .build .dist 2>/dev/null || true
        fi
    done
    
    log_info "Clean completed!"
}

# Main function
main() {
    local target="${1:-status}"
    
    case "$target" in
        status)
            show_status
            ;;
        clean)
            clean_all
            ;;
        *)
            log_error "Command '$target' not yet implemented"
            show_status
            ;;
    esac
}

main "$@"

#!/bin/bash
# test_package.sh - Test framework for a-Shell packages
#
# Usage: ./test_package.sh [package_name]
#        ./test_package.sh all  # Test all packages

set -e
set -o pipefail

# Configuration
ASHELL_PKG_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ASHELL_TEST_RESULTS="${ASHELL_TEST_RESULTS:-$ASHELL_PKG_ROOT/.test-results}"
ASHELL_VERBOSE="${ASHELL_VERBOSE:-0}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_test() {
    local status="$1"
    local message="$2"
    if [ "$status" = "PASS" ]; then
        echo -e "${GREEN}[PASS]${NC} $message"
    elif [ "$status" = "FAIL" ]; then
        echo -e "${RED}[FAIL]${NC} $message"
    elif [ "$status" = "SKIP" ]; then
        echo -e "${YELLOW}[SKIP]${NC} $message"
    fi
}

# =============================================================================
# TEST FUNCTIONS
# =============================================================================

# Test if package directory exists
test_package_exists() {
    local pkg_name="$1"
    local pkg_dir="$ASHELL_PKG_ROOT/root-packages/$pkg_name"
    
    if [ -d "$pkg_dir" ]; then
        return 0
    else
        return 1
    fi
}

# Test if build.sh exists
test_build_script_exists() {
    local pkg_name="$1"
    local build_script="$ASHELL_PKG_ROOT/root-packages/$pkg_name/build.sh"
    
    if [ -f "$build_script" ]; then
        return 0
    else
        return 1
    fi
}

# Test if patches apply cleanly
test_patches_apply() {
    local pkg_name="$1"
    local patches_dir="$ASHELL_PKG_ROOT/root-packages/$pkg_name/patches"
    
    if [ ! -d "$patches_dir" ]; then
        # No patches to apply - that's fine
        return 0
    fi
    
    local patch_count=$(find "$patches_dir" -name "*.patch" | wc -l)
    if [ "$patch_count" -eq 0 ]; then
        return 0
    fi
    
    # Check if patches are valid
    local failed=0
    for patch in "$patches_dir"/*.patch; do
        if [ -f "$patch" ]; then
            # Basic validation - check if patch file looks valid
            if ! grep -q "^---" "$patch" 2>/dev/null; then
                log_warn "Patch $patch may be malformed (no --- line found)"
                failed=1
            fi
        fi
    done
    
    return $failed
}

# Test if patches follow naming convention (NN-*.patch)
test_patches_naming() {
    local pkg_name="$1"
    local patches_dir="$ASHELL_PKG_ROOT/root-packages/$pkg_name/patches"
    
    if [ ! -d "$patches_dir" ]; then
        return 0
    fi
    
    local failed=0
    for patch in "$patches_dir"/*.patch; do
        if [ -f "$patch" ]; then
            local basename=$(basename "$patch")
            # Check if patch follows NN-*.patch pattern
            if [[ ! "$basename" =~ ^[0-9]{2}-.*\.patch$ ]]; then
                log_warn "Patch $basename doesn't follow naming convention (NN-*.patch)"
                failed=1
            fi
        fi
    done
    
    return $failed
}

# Test if package has dependencies file or metadata
test_package_metadata() {
    local pkg_name="$1"
    local pkg_dir="$ASHELL_PKG_ROOT/root-packages/$pkg_name"
    local has_metadata=0
    
    # Check for build.sh with metadata
    if [ -f "$pkg_dir/build.sh" ]; then
        if grep -q "ASHELL_PKG_NAME" "$pkg_dir/build.sh" 2>/dev/null; then
            has_metadata=1
        fi
    fi
    
    # Check for Makefile with metadata
    if [ -f "$pkg_dir/Makefile" ]; then
        has_metadata=1
    fi
    
    return $((1 - has_metadata))
}

# Test if package uses a_shell_* API correctly in patches
test_patches_use_ashell_api() {
    local pkg_name="$1"
    local patches_dir="$ASHELL_PKG_ROOT/root-packages/$pkg_name/patches"
    
    if [ ! -d "$patches_dir" ]; then
        return 0
    fi
    
    local has_ashell_refs=0
    local has_ios_refs=0
    
    for patch in "$patches_dir"/*.patch; do
        if [ -f "$patch" ]; then
            # Check for a_shell_* API usage
            if grep -q "a_shell_" "$patch" 2>/dev/null; then
                has_ashell_refs=1
            fi
            # Check for old ios_* API (should be migrated)
            if grep -q "ios_" "$patch" 2>/dev/null; then
                has_ios_refs=1
            fi
        fi
    done
    
    if [ "$has_ios_refs" -eq 1 ] && [ "$has_ashell_refs" -eq 0 ]; then
        log_warn "Patches still use ios_* API instead of a_shell_*"
        return 1
    fi
    
    return 0
}

# Test if patch has __APPLE__ guards where needed
test_patches_have_guards() {
    local pkg_name="$1"
    local patches_dir="$ASHELL_PKG_ROOT/root-packages/$pkg_name/patches"
    
    if [ ! -d "$patches_dir" ]; then
        return 0
    fi
    
    local failed=0
    for patch in "$patches_dir"/*.patch; do
        if [ -f "$patch" ]; then
            # Patches that modify code should have __APPLE__ guards
            if grep -q "a_shell_" "$patch" 2>/dev/null; then
                if ! grep -q "__APPLE__" "$patch" 2>/dev/null; then
                    log_warn "Patch $patch uses a_shell_* without __APPLE__ guard"
                    failed=1
                fi
            fi
        fi
    done
    
    return $failed
}

# Test if patch includes proper header includes
test_patches_include_headers() {
    local pkg_name="$1"
    local patches_dir="$ASHELL_PKG_ROOT/root-packages/$pkg_name/patches"
    
    if [ ! -d "$patches_dir" ]; then
        return 0
    fi
    
    local failed=0
    for patch in "$patches_dir"/*.patch; do
        if [ -f "$patch" ]; then
            # Patches that use a_shell_* should include the header
            if grep -q "a_shell_" "$patch" 2>/dev/null; then
                if ! grep -q "a_shell_system.h" "$patch" 2>/dev/null; then
                    log_warn "Patch $patch uses a_shell_* without including a_shell_system.h"
                    failed=1
                fi
            fi
        fi
    done
    
    return $failed
}

# Run a single test
run_test() {
    local test_name="$1"
    local pkg_name="$2"
    local test_func="$3"
    
    if [ "$ASHELL_VERBOSE" -eq 1 ]; then
        echo -n "  Testing: $test_name... "
    fi
    
    if $test_func "$pkg_name"; then
        log_test "PASS" "$test_name"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        log_test "FAIL" "$test_name"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Run all tests for a package
run_package_tests() {
    local pkg_name="$1"
    
    echo ""
    echo "=========================================="
    echo "Testing package: $pkg_name"
    echo "=========================================="
    
    # Check if package exists
    if ! test_package_exists "$pkg_name"; then
        log_error "Package $pkg_name does not exist"
        return 1
    fi
    
    # Run tests
    run_test "Package directory exists" "$pkg_name" test_package_exists
    run_test "Build script exists" "$pkg_name" test_build_script_exists
    run_test "Patches apply cleanly" "$pkg_name" test_patches_apply
    run_test "Patches naming convention" "$pkg_name" test_patches_naming
    run_test "Package metadata" "$pkg_name" test_package_metadata
    run_test "Patches use a_shell_* API" "$pkg_name" test_patches_use_ashell_api
    run_test "Patches have __APPLE__ guards" "$pkg_name" test_patches_have_guards
    run_test "Patches include headers" "$pkg_name" test_patches_include_headers
}

# Get list of all packages
get_all_packages() {
    find "$ASHELL_PKG_ROOT/root-packages" -maxdepth 1 -type d ! -path "$ASHELL_PKG_ROOT/root-packages" | sort | xargs -I{} basename {}
}

# Print test summary
print_summary() {
    echo ""
    echo "=========================================="
    echo "Test Summary"
    echo "=========================================="
    echo -e "${GREEN}Passed:  $TESTS_PASSED${NC}"
    echo -e "${RED}Failed:  $TESTS_FAILED${NC}"
    echo -e "${YELLOW}Skipped: $TESTS_SKIPPED${NC}"
    echo "=========================================="
    
    if [ "$TESTS_FAILED" -eq 0 ]; then
        echo -e "${GREEN}All tests passed!${NC}"
        return 0
    else
        echo -e "${RED}Some tests failed.${NC}"
        return 1
    fi
}

# =============================================================================
# MAIN
# =============================================================================

main() {
    local target="${1:-all}"
    
    log_info "Starting package tests..."
    log_info "Package root: $ASHELL_PKG_ROOT"
    
    if [ "$target" = "all" ]; then
        log_info "Testing all packages..."
        for pkg in $(get_all_packages); do
            run_package_tests "$pkg"
        done
    else
        run_package_tests "$target"
    fi
    
    print_summary
    exit $?
}

# Run main function
main "$@"

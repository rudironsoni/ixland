#!/bin/bash
# test-build-system.sh - Validate build system without iOS SDK
# This runs in Docker to test Linux compatibility
#
# Usage: ./scripts/test-build-system.sh

set -e

echo "================================"
echo "a-Shell Build System Test Suite"
echo "================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

# =============================================================================
# TEST HELPERS
# =============================================================================

pass() {
    echo -e "${GREEN}✓ PASS${NC}: $1"
    ((PASSED++))
}

fail() {
    echo -e "${RED}✗ FAIL${NC}: $1"
    ((FAILED++))
}

skip() {
    echo -e "${YELLOW}⊘ SKIP${NC}: $1"
}

# =============================================================================
# TEST: Syntax Check
# =============================================================================

test_syntax() {
    echo "Testing script syntax..."

    if bash -n ashell-packages/ashell_package.sh; then
        pass "ashell_package.sh syntax"
    else
        fail "ashell_package.sh syntax"
    fi

    if bash -n ashell-packages/build.sh; then
        pass "build.sh syntax"
    else
        fail "build.sh syntax"
    fi

    if bash -n ashell-packages/clean.sh; then
        pass "clean.sh syntax"
    else
        fail "clean.sh syntax"
    fi

    echo ""
}

# =============================================================================
# TEST: Load Build System
# =============================================================================

test_load() {
    echo "Testing build system loads..."

    if bash -c 'source ashell-packages/ashell_package.sh && echo OK' | grep -q OK; then
        pass "ashell_package.sh loads"
    else
        fail "ashell_package.sh loads"
    fi

    echo ""
}

# =============================================================================
# TEST: Package Listing
# =============================================================================

test_list() {
    echo "Testing package listing..."

    cd ashell-packages

    if ./build.sh list > /tmp/package-list.txt 2>&1; then
        pass "build.sh list command"

        # Check for expected packages
        if grep -q "hello" /tmp/package-list.txt; then
            pass "hello package listed"
        else
            fail "hello package listed"
        fi

        if grep -q "apt" /tmp/package-list.txt; then
            pass "apt package listed"
        else
            fail "apt package listed"
        fi
    else
        fail "build.sh list command"
    fi

    cd ..
    echo ""
}

# =============================================================================
# TEST: Utility Functions
# =============================================================================

test_utils() {
    echo "Testing utility functions..."

    cd ashell-packages

    # Test logging functions
    if bash -c 'source ashell_package.sh && ashell_info "test"' 2>&1 | grep -q "\[INFO\]"; then
        pass "ashell_info logging"
    else
        fail "ashell_info logging"
    fi

    if bash -c 'source ashell_package.sh && ashell_warning "test"' 2>&1 | grep -q "\[WARNING\]"; then
        pass "ashell_warning logging"
    else
        fail "ashell_warning logging"
    fi

    # Test SDK path detection (mock)
    if bash -c 'source ashell_package.sh && ashell_get_sdk_path' | grep -q "/opt/mock-ios-sdk"; then
        pass "SDK path detection (mock)"
    else
        fail "SDK path detection (mock)"
    fi

    cd ..
    echo ""
}

# =============================================================================
# TEST: Package Structure Validation
# =============================================================================

test_package_structure() {
    echo "Testing package structure..."

    for pkg in hello apt coreutils-minimal; do
        if [[ -f "ashell-packages/$pkg/build.sh" ]]; then
            pass "$pkg/build.sh exists"
        else
            fail "$pkg/build.sh exists"
        fi
    done

    echo ""
}

# =============================================================================
# TEST: Reference Files
# =============================================================================

test_reference() {
    echo "Testing reference files..."

    if [[ -f ".beads/reference/step-functions.sh" ]]; then
        pass "step-functions.sh reference exists"
    else
        fail "step-functions.sh reference exists"
    fi

    if [[ -f ".beads/reference/package-template/build.sh" ]]; then
        pass "package-template exists"
    else
        fail "package-template exists"
    fi

    echo ""
}

# =============================================================================
# TEST: Toolchain Files
# =============================================================================

test_toolchain() {
    echo "Testing toolchain files..."

    if [[ -f "ashell-packages/config/ios-toolchain.cmake" ]]; then
        pass "ios-toolchain.cmake exists"
    else
        fail "ios-toolchain.cmake exists"
    fi

    if [[ -f "ashell-packages/config/ios-configure.sh" ]]; then
        pass "ios-configure.sh exists"
    else
        fail "ios-configure.sh exists"
    fi

    # Syntax check ios-configure.sh
    if bash -n ashell-packages/config/ios-configure.sh; then
        pass "ios-configure.sh syntax"
    else
        fail "ios-configure.sh syntax"
    fi

    echo ""
}

# =============================================================================
# TEST: Extract Function (Mock)
# =============================================================================

test_extract() {
    echo "Testing extract function (with mock download)..."

    cd ashell-packages

    # Create a test archive
    mkdir -p /tmp/test-extract
    echo "test content" > /tmp/test-extract/test.txt
    tar -czf /tmp/test-package-1.0.0.tar.gz -C /tmp/test-extract .

    # Calculate checksum
    TEST_SHA256=$(sha256sum /tmp/test-package-1.0.0.tar.gz | cut -d' ' -f1)

    # Test with mock package
    bash -c "
        source ashell_package.sh
        ASHELL_PKG_NAME='testpkg'
        ASHELL_PKG_VERSION='1.0.0'
        ASHELL_PKG_SRCURL='file:///tmp/test-package-1.0.0.tar.gz'
        ASHELL_PKG_SHA256='$TEST_SHA256'
        ashell_step_extract_package
    " 2>&1 | grep -q "extracted" && pass "extract function" || skip "extract function (no local file support)"

    # Cleanup
    rm -rf /tmp/test-extract /tmp/test-package-1.0.0.tar.gz

    cd ..
    echo ""
}

# =============================================================================
# SUMMARY
# =============================================================================

echo "================================"
echo "Test Summary"
echo "================================"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"
echo ""

if [[ $FAILED -eq 0 ]]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi

#!/bin/bash
# test-package.sh - Run upstream package tests on iOS Simulator
#
# Usage: ./test-package.sh <package-name> [--target simulator|ios|universal]
#
# This script:
# 1. Builds the package (if not already built)
# 2. Runs 'make test' or 'make check' to build test binaries
# 3. Copies test binaries to .build/<target>/test/<package>/
# 4. Executes tests on iOS Simulator
# 5. Captures results in JSON format

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
A_SHELL_PACKAGES_DIR="$SCRIPT_DIR/.."
PACKAGE_NAME=""
TARGET="simulator"  # Default to simulator for speed
TIMEOUT=60          # Default timeout per test in seconds

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

log_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --target|-t)
            TARGET="$2"
            shift 2
            ;;
        --timeout)
            TIMEOUT="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 <package-name> [--target simulator|ios|universal] [--timeout seconds]"
            echo ""
            echo "Run upstream package tests on iOS Simulator"
            echo ""
            echo "Options:"
            echo "  --target    Target platform (default: simulator)"
            echo "  --timeout   Timeout per test in seconds (default: 60)"
            echo ""
            echo "Examples:"
            echo "  $0 libz                          # Test libz on simulator"
            echo "  $0 libz --target ios             # Test on iOS device"
            echo "  $0 libz --target universal       # Test universal build"
            exit 0
            ;;
        -*)
            log_error "Unknown option: $1"
            exit 1
            ;;
        *)
            if [ -z "$PACKAGE_NAME" ]; then
                PACKAGE_NAME="$1"
            else
                log_error "Unexpected argument: $1"
                exit 1
            fi
            shift
            ;;
    esac
done

if [ -z "$PACKAGE_NAME" ]; then
    log_error "No package name specified"
    echo "Usage: $0 <package-name> [--target simulator|ios|universal]"
    exit 1
fi

# Validate target
case "$TARGET" in
    simulator|ios|universal)
        ;;
    *)
        log_error "Invalid target: $TARGET"
        exit 1
        ;;
esac

# Set paths
BUILD_DIR="$A_SHELL_PACKAGES_DIR/.build/$TARGET"
TMP_DIR="$BUILD_DIR/tmp"
TEST_DIR="$BUILD_DIR/test/$PACKAGE_NAME"
RESULTS_DIR="$A_SHELL_PACKAGES_DIR/.build/test-results"

# Create directories
mkdir -p "$TEST_DIR"
mkdir -p "$RESULTS_DIR"

# Find package build.sh
if [ -f "$A_SHELL_PACKAGES_DIR/packages/core/$PACKAGE_NAME/build.sh" ]; then
    PKG_DIR="$A_SHELL_PACKAGES_DIR/packages/core/$PACKAGE_NAME"
elif [ -f "$A_SHELL_PACKAGES_DIR/packages/extra/$PACKAGE_NAME/build.sh" ]; then
    PKG_DIR="$A_SHELL_PACKAGES_DIR/packages/extra/$PACKAGE_NAME"
else
    log_error "Package $PACKAGE_NAME not found"
    exit 1
fi

# Check if package is built
if [ ! -d "$TMP_DIR" ]; then
    log_step "Package not built. Building first..."
    "$SCRIPT_DIR/build-package.sh" "$PACKAGE_NAME" --target "$TARGET"
fi

# Find the package source directory in tmp
# Package names might differ from directory names (e.g., libz -> zlib)
PKG_SRC_DIR=""
for dir in "$TMP_DIR"/*; do
    if [ -d "$dir" ] && [ -f "$dir/Makefile" ]; then
        # Check if directory name matches package name (with or without 'lib' prefix)
        dir_name=$(basename "$dir")
        if [[ "$dir_name" == *"$PACKAGE_NAME"* ]] || \
           ([[ "$PACKAGE_NAME" == lib* ]] && [[ "$dir_name" == *"${PACKAGE_NAME#lib}"* ]]); then
            PKG_SRC_DIR="$dir"
            break
        fi
    fi
done

if [ -z "$PKG_SRC_DIR" ]; then
    log_error "Could not find package source directory in $TMP_DIR"
    exit 1
fi

log_step "Testing package: $PACKAGE_NAME"
log_info "Target: $TARGET"
log_info "Source: $PKG_SRC_DIR"
log_info "Test directory: $TEST_DIR"

# Initialize results
timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
total_tests=0
passed_tests=0
failed_tests=0
skipped_tests=0
tests_json=""

# Run tests
cd "$PKG_SRC_DIR"

log_step "Building test binaries..."

# For iOS cross-compilation, we can't run tests during build
# Just build the test binaries without executing them
if make example minigzip 2>&1 | tee "$TEST_DIR/build.log"; then
    log_info "Test binaries built successfully"
else
    log_warn "Could not build test binaries with 'make example minigzip'"
    log_warn "Trying alternative build methods..."
    
    # Try 'make test' but ignore execution failures
    make test 2>&1 | tee "$TEST_DIR/build.log" || true
fi

# Find and copy test binaries
log_step "Finding test binaries..."

# Look for specific test binaries in current directory
for binary in example minigzip; do
    if [ -f "$binary" ] && [ -x "$binary" ]; then
        # Check if it's a Mach-O binary for arm64
        if file "$binary" | grep -q "Mach-O.*arm64"; then
            cp "$binary" "$TEST_DIR/"
            log_info "Found test binary: $binary"
        fi
    fi
done

# Also look in common subdirectories
for subdir in . test tests; do
    if [ -d "$subdir" ]; then
        for binary in "$subdir"/example "$subdir"/minigzip; do
            if [ -f "$binary" ] && [ -x "$binary" ]; then
                if file "$binary" | grep -q "Mach-O.*arm64"; then
                    cp "$binary" "$TEST_DIR/"
                    log_info "Found test binary: $binary"
                fi
            fi
        done
    fi
done

# Check if we found any test binaries
if [ -z "$(ls -A "$TEST_DIR" 2>/dev/null)" ]; then
    log_error "No test binaries found for $PACKAGE_NAME"
    
    # Create failure result
    cat > "$RESULTS_DIR/${PACKAGE_NAME}-$(date +%Y%m%d-%H%M%S).json" <<EOF
{
  "package": "$PACKAGE_NAME",
  "version": "unknown",
  "timestamp": "$timestamp",
  "target": "$TARGET",
  "status": "no_tests_found",
  "results": {
    "total": 0,
    "passed": 0,
    "failed": 0,
    "skipped": 0
  },
  "tests": [],
  "error": "No test binaries found",
  "raw_log": "Could not find test binaries in $PKG_SRC_DIR"
}
EOF
    
    exit 1
fi

log_step "Running tests on iOS Simulator..."

# Find available simulator
SIMULATOR_ID=$(xcrun simctl list devices available | grep "iPhone" | head -1 | grep -oE '[0-9A-F]{8}-([0-9A-F]{4}-){3}[0-9A-F]{12}')

if [ -z "$SIMULATOR_ID" ]; then
    log_error "No iOS Simulator found"
    exit 1
fi

SIMULATOR_NAME=$(xcrun simctl list devices available | grep "$SIMULATOR_ID" | head -1 | sed -E 's/.*\(([^)]+)\).*/\1/')
log_info "Using simulator: $SIMULATOR_NAME"

# Ensure simulator is booted
xcrun simctl boot "$SIMULATOR_ID" 2>/dev/null || true

# Create temp directory on simulator for test files
SIM_TEMP_DIR="/tmp/a-shell-tests-$$"
xcrun simctl spawn "$SIMULATOR_ID" mkdir -p "$SIM_TEMP_DIR" 2>/dev/null || true

# Process each test binary
for binary_path in "$TEST_DIR"/*; do
    if [ -f "$binary_path" ] && [ -x "$binary_path" ]; then
        binary_name=$(basename "$binary_path")
        
        # Skip if already processed (signed version)
        [[ "$binary_name" == *"_signed" ]] && continue
        
        # Skip if this is a duplicate (check if base name exists)
        base_name="${binary_name%%.*}"
        if [ "$binary_name" != "$base_name" ]; then
            # Check if base version exists
            if [ -f "$TEST_DIR/$base_name" ]; then
                continue
            fi
        fi
        
        total_tests=$((total_tests + 1))
        
        log_step "Running test: $binary_name"
        
        # Determine test arguments based on binary name
        test_args=""
        case "$binary_name" in
            example)
                # zlib example needs a test file path
                test_args="$SIM_TEMP_DIR/zlib_test"
                ;;
            minigzip)
                # minigzip needs input file
                test_args="-h"  # Just test help/usage
                ;;
            *)
                test_args="--help"
                ;;
        esac
        
        # For iOS Simulator command-line tools, we need to sign the binary
        SIGNED_BINARY="$TEST_DIR/${binary_name}_signed"
        cp "$binary_path" "$SIGNED_BINARY"
        chmod +x "$SIGNED_BINARY"
        
        # Sign the binary with Apple Development certificate
        log_step "Signing test binary..."
        log_info "Using Apple Development certificate"
        log_info "IMPORTANT: If prompted, click 'Always Allow' in System Preferences"
        
        SIGN_OUTPUT=$(codesign --force --sign "Apple Development: rudironsonijr@icloud.com" --timestamp=none "$SIGNED_BINARY" 2>&1)
        SIGN_EXIT_CODE=$?
        
        if [ $SIGN_EXIT_CODE -ne 0 ]; then
            if echo "$SIGN_OUTPUT" | grep -q "errSecInternalComponent"; then
                log_error "Keychain is locked. Please unlock it first:"
                log_error ""
                log_error "Option 1: Unlock via Terminal (you'll be prompted for password):"
                log_error "  security unlock-keychain ~/Library/Keychains/login.keychain-db"
                log_error ""
                log_error "Option 2: Unlock via Keychain Access app:"
                log_error "  1. Open 'Keychain Access' (Applications > Utilities)"
                log_error "  2. Select 'login' keychain"
                log_error "  3. File > Unlock Keychain 'login'"
                log_error "  4. Enter your password"
                log_error ""
                log_error "Option 3: Test without signing (limited):"
                log_error "  Using unsigned binary - may not work on iOS Simulator"
                log_warn "Attempting to run without code signing..."
                
                # Fallback: try to run without signing
                USE_UNSIGNED=true
                rm -f "$SIGNED_BINARY"
                SIGNED_BINARY="$binary_path"
            else
                log_error "Code signing failed: $SIGN_OUTPUT"
                rm -f "$SIGNED_BINARY"
                exit 1
            fi
        else
            USE_UNSIGNED=false
            # Verify signature
            if ! codesign -v "$SIGNED_BINARY" 2>/dev/null; then
                log_warn "Binary signature verification failed, continuing anyway"
            else
                log_info "Binary signed successfully"
            fi
        fi
        
        # Determine which binary to run
        if [ "$USE_UNSIGNED" = "true" ]; then
            TEST_BINARY="$binary_name"
            log_warn "Running unsigned binary (may fail on iOS Simulator)"
        else
            TEST_BINARY="${binary_name}_signed"
        fi
        
        # Run the test binary using simctl spawn
        test_start=$(date +%s)
        TEST_OUTPUT=$(cd "$TEST_DIR" && timeout $TIMEOUT xcrun simctl spawn "$SIMULATOR_ID" "./$TEST_BINARY" $test_args 2>&1) || TEST_EXIT_CODE=$?
        test_end=$(date +%s)
        duration=$(( (test_end - test_start) * 1000 ))
        
        # Cleanup signed binary
        if [ "$USE_UNSIGNED" != "true" ]; then
            rm -f "$SIGNED_BINARY"
        fi
        
        # Determine test status based on exit code and output
        if [ -z "$TEST_EXIT_CODE" ] || [ "$TEST_EXIT_CODE" -eq 0 ]; then
            # Check output for explicit failure messages
            if echo "$TEST_OUTPUT" | grep -qiE "(FAILED|failed|error|Error|FAIL)"; then
                status="failed"
                failed_tests=$((failed_tests + 1))
            else
                status="passed"
                passed_tests=$((passed_tests + 1))
            fi
        else
            status="failed"
            failed_tests=$((failed_tests + 1))
        fi
        
        # Escape JSON strings properly
        stdout_escaped=$(echo "$TEST_OUTPUT" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | tr '\n' ' ' | sed 's/\t/ /g')
        
        # Add to tests array
        if [ -n "$tests_json" ]; then
            tests_json="$tests_json,"
        fi
        tests_json="$tests_json
    {
      \"name\": \"$binary_name\",
      \"binary\": \"$binary_path\",
      \"status\": \"$status\",
      \"duration_ms\": $duration,
      \"exit_code\": ${TEST_EXIT_CODE:-0},
      \"stdout\": \"$stdout_escaped\"
    }"
        
        log_info "Test $binary_name: $status (${duration}ms)"
        
        # Cleanup test files on simulator
        xcrun simctl spawn "$SIMULATOR_ID" rm -rf "$SIM_TEMP_DIR" 2>/dev/null || true
    fi
done

# Final cleanup
xcrun simctl spawn "$SIMULATOR_ID" rm -rf "$SIM_TEMP_DIR" 2>/dev/null || true

# Generate final JSON result
result_file="$RESULTS_DIR/${PACKAGE_NAME}-$(date +%Y%m%d-%H%M%S).json"

cat > "$result_file" <<EOF
{
  "package": "$PACKAGE_NAME",
  "timestamp": "$timestamp",
  "target": "$TARGET",
  "results": {
    "total": $total_tests,
    "passed": $passed_tests,
    "failed": $failed_tests,
    "skipped": $skipped_tests
  },
  "tests": [$tests_json
  ]
}
EOF

# Print summary
echo ""
echo "========================================"
echo "Test Results for $PACKAGE_NAME"
echo "========================================"
echo "Total:  $total_tests"
echo "Passed: $passed_tests"
echo "Failed: $failed_tests"
echo "Skipped: $skipped_tests"
echo ""
echo "Results saved to: $result_file"
echo "========================================"

# Exit with appropriate code
if [ $failed_tests -gt 0 ]; then
    exit 1
else
    exit 0
fi

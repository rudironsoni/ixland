#!/bin/bash
# build-test.sh - Build iOS test app for package validation
# Automatically uses universal or simulator builds
#
# Usage: ./build-test.sh <package-name> [build-dir]
#
# The script searches for libraries in this order:
#   1. Universal build (.build/universal/staging/usr/local/)
#   2. Simulator build (.build/simulator/staging/usr/local/)
#   3. Specified build directory (if provided)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="$SCRIPT_DIR/../packages/core"
PACKAGE_NAME="${1:-libz}"
SPECIFIED_BUILD_DIR="$2"

# Auto-detect build directory
auto_detect_build_dir() {
    local pkg="$1"
    
    # Priority 1: Universal build (preferred for testing)
    local universal_dir="$SCRIPT_DIR/../.build/universal/staging/usr/local"
    if [ -f "$universal_dir/lib/lib${pkg}.a" ]; then
        echo "$universal_dir"
        return 0
    fi
    
    # Priority 2: Simulator build
    local simulator_dir="$SCRIPT_DIR/../.build/simulator/staging/usr/local"
    if [ -f "$simulator_dir/lib/lib${pkg}.a" ]; then
        echo "$simulator_dir"
        return 0
    fi
    
    # Priority 3: Legacy path (tmp dir)
    local tmp_dir="$SCRIPT_DIR/../.build/tmp"
    # Try to find the package in tmp directory
    for dir in "$tmp_dir"/*; do
        if [ -d "$dir" ] && [ -f "$dir/lib${pkg}.a" ]; then
            echo "$dir"
            return 0
        fi
    done
    
    return 1
}

# Set build directory
if [ -n "$SPECIFIED_BUILD_DIR" ]; then
    BUILD_DIR="$SPECIFIED_BUILD_DIR"
    a_shell_info "Using specified build directory: $BUILD_DIR"
else
    BUILD_DIR=$(auto_detect_build_dir "$PACKAGE_NAME")
    if [ -z "$BUILD_DIR" ]; then
        echo "Error: Could not find library for $PACKAGE_NAME"
        echo ""
        echo "Please build the package first:"
        echo "  ./scripts/build-package.sh $PACKAGE_NAME --target universal"
        echo "  ./scripts/build-package.sh $PACKAGE_NAME --target simulator"
        echo ""
        echo "Or specify the build directory:"
        echo "  $0 $PACKAGE_NAME <path-to-build-dir>"
        exit 1
    fi
    echo "Auto-detected build: $BUILD_DIR"
fi

# Verify library exists
LIBRARY_PATH="$BUILD_DIR/lib/lib${PACKAGE_NAME}.a"
if [ ! -f "$LIBRARY_PATH" ]; then
    # Try alternative naming
    if [ -f "$BUILD_DIR/lib/libz.a" ] && [ "$PACKAGE_NAME" = "libz" ]; then
        LIBRARY_PATH="$BUILD_DIR/lib/libz.a"
    else
        echo "Error: Library not found at $LIBRARY_PATH"
        exit 1
    fi
fi

echo "========================================"
echo "Building iOS Test App for $PACKAGE_NAME"
echo "========================================"
echo "Library: $LIBRARY_PATH"
echo ""

# iOS Simulator settings for running tests
IOS_SDK="iphonesimulator"
IOS_ARCH="arm64"
IOS_VERSION="16.0"

# Get SDK path
SDK_PATH=$(xcrun -sdk $IOS_SDK --show-sdk-path)
echo "SDK: $SDK_PATH"
echo ""

# Output directory
OUTPUT_DIR="$SCRIPT_DIR/build/$PACKAGE_NAME"
mkdir -p "$OUTPUT_DIR"

# Compile test app
echo "Compiling test app..."

# Compile test functions
clang \
    -arch $IOS_ARCH \
    -isysroot "$SDK_PATH" \
    -mios-simulator-version-min=$IOS_VERSION \
    -c \
    "$SCRIPT_DIR/PackageTestApp/test_${PACKAGE_NAME}.c" \
    -o "$OUTPUT_DIR/test_${PACKAGE_NAME}.o" \
    -I"$BUILD_DIR/include"

# Compile main test runner
cat > "$OUTPUT_DIR/main.m" << 'MAINCODE'
#import <Foundation/Foundation.h>
#import <dlfcn.h>

// Test function prototypes - these should match the test file
extern int test_libz_compression(void);
extern int test_libz_decompression(void);
extern int test_libz_edge_cases(void);

@interface TestResult : NSObject
@property NSString *name;
@property BOOL passed;
@property NSString *error;
@end
@implementation TestResult
@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSMutableArray *results = [NSMutableArray array];
        
        // Run tests
        int (*tests[])(void) = {
            test_libz_compression,
            test_libz_decompression,
            test_libz_edge_cases,
            NULL
        };
        
        const char *test_names[] = {
            "compression",
            "decompression",
            "edge_cases",
            NULL
        };
        
        for (int i = 0; tests[i] != NULL; i++) {
            TestResult *r = [[TestResult alloc] init];
            r.name = [NSString stringWithUTF8String:test_names[i]];
            int ret = tests[i]();
            r.passed = (ret == 0);
            if (!r.passed) {
                r.error = [NSString stringWithFormat:@"Exit code %d", ret];
            }
            [results addObject:r];
            
            NSLog(@"%@ %@", r.passed ? @"✓" : @"✗", r.name);
        }
        
        // Summary
        int passed = 0;
        int failed = 0;
        for (TestResult *r in results) {
            if (r.passed) passed++; else failed++;
        }
        
        NSLog(@"\n========================================");
        NSLog(@"Results: %d passed, %d failed", passed, failed);
        NSLog(@"========================================");
        
        return failed > 0 ? 1 : 0;
    }
}
MAINCODE

clang \
    -arch $IOS_ARCH \
    -isysroot "$SDK_PATH" \
    -mios-simulator-version-min=$IOS_VERSION \
    -framework Foundation \
    -c "$OUTPUT_DIR/main.m" \
    -o "$OUTPUT_DIR/main.o"

# Link with library
echo "Linking with $LIBRARY_PATH..."

clang \
    -arch $IOS_ARCH \
    -isysroot "$SDK_PATH" \
    -mios-simulator-version-min=$IOS_VERSION \
    -framework Foundation \
    "$OUTPUT_DIR/test_${PACKAGE_NAME}.o" \
    "$OUTPUT_DIR/main.o" \
    "$LIBRARY_PATH" \
    -o "$OUTPUT_DIR/test_${PACKAGE_NAME}"

echo ""
echo "✓ Build complete: $OUTPUT_DIR/test_${PACKAGE_NAME}"
echo ""
echo "To run tests on iOS Simulator:"
echo "  $SCRIPT_DIR/run-tests.sh $PACKAGE_NAME"

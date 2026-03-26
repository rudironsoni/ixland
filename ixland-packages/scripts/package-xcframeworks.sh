#!/bin/bash
# Package libiox and libiwasm as XCFrameworks for iOS distribution
# Creates: LibIOX.xcframework and LibIWasm.xcframework

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$PROJECT_ROOT/build/xcframeworks"

echo "========================================"
echo "Creating XCFrameworks for iOS"
echo "========================================"
echo ""

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Check for required libraries
if [ ! -f "$PROJECT_ROOT/lib/libiox-device.a" ] || [ ! -f "$PROJECT_ROOT/lib/libiox-sim.a" ]; then
    echo "Error: libiox libraries not found. Run: make iox-device && make iox-sim"
    exit 1
fi

if [ ! -f "$PROJECT_ROOT/build/wamr-device/libiwasm.a" ] || [ ! -f "$PROJECT_ROOT/build/wamr-simulator/libiwasm.a" ]; then
    echo "Error: libiwasm libraries not found. Run: ./build_wamr_ios_static.sh all"
    exit 1
fi

echo "Packaging LibIOX.xcframework..."

# Create LibIOX.xcframework
xcodebuild -create-xcframework \
    -library "$PROJECT_ROOT/lib/libiox-device.a" \
    -headers "$PROJECT_ROOT/include" \
    -library "$PROJECT_ROOT/lib/libiox-sim.a" \
    -headers "$PROJECT_ROOT/include" \
    -output "$OUTPUT_DIR/LibIOX.xcframework"

echo "✓ Created LibIOX.xcframework"
echo ""

echo "Packaging LibIWasm.xcframework..."

# Create LibIWasm.xcframework
xcodebuild -create-xcframework \
    -library "$PROJECT_ROOT/build/wamr-device/libiwasm.a" \
    -headers "$PROJECT_ROOT/deps/wamr/core/iwasm/include" \
    -library "$PROJECT_ROOT/build/wamr-simulator/libiwasm.a" \
    -headers "$PROJECT_ROOT/deps/wamr/core/iwasm/include" \
    -output "$OUTPUT_DIR/LibIWasm.xcframework"

echo "✓ Created LibIWasm.xcframework"
echo ""

# Verify frameworks
echo "Verifying frameworks..."
ls -lh "$OUTPUT_DIR"/*.xcframework
echo ""

# Create distribution package
echo "Creating distribution package..."
DIST_DIR="$OUTPUT_DIR/libiox-ios-sdk"
mkdir -p "$DIST_DIR"

# Copy frameworks
cp -R "$OUTPUT_DIR/LibIOX.xcframework" "$DIST_DIR/"
cp -R "$OUTPUT_DIR/LibIWasm.xcframework" "$DIST_DIR/"

# Copy headers
cp -R "$PROJECT_ROOT/include" "$DIST_DIR/"

# Create README
cat > "$DIST_DIR/README.md" << 'EOF'
# libiox iOS SDK

## Contents

- **LibIOX.xcframework** - iOS Subsystem for Linux (syscalls, VFS, networking)
- **LibIWasm.xcframework** - WebAssembly Micro Runtime (WAMR)
- **include/** - Public headers

## Supported Platforms

- iOS 16.0+ (arm64 device)
- iOS Simulator (arm64/x86_64)

## Integration

### Xcode

1. Drag both `.xcframework` files into your project
2. Add to your target's **Frameworks, Libraries, and Embedded Content**
3. Import headers:
   ```objc
   #include <iox/iox.h>
   #include <iox/iox_wamr.h>
   ```

### Swift Package Manager

Add to `Package.swift`:
```swift
.binaryTarget(
    name: "LibIOX",
    path: "LibIOX.xcframework"
),
.binaryTarget(
    name: "LibIWasm",
    path: "LibIWasm.xcframework"
)
```

## API Usage

### Basic Syscalls
```c
#include <iox/iox.h>

pid_t pid = iox_getpid();
char cwd[1024];
iox_getcwd(cwd, sizeof(cwd));
iox_chdir("/tmp");
```

### WebAssembly
```c
#include <iox/iox_wamr.h>

iox_wamr_init();
iox_wamr_module_t *mod = iox_wamr_load_module(wasm_data, wasm_size);
int32_t result = iox_wamr_call_function(mod, "main", NULL, 0);
iox_wamr_unload_module(mod);
iox_wamr_deinit();
```

## Architecture

- **LibIOX** - Kernel interposition layer providing Linux syscall compatibility
- **LibIWasm** - WebAssembly runtime with WASI support
- **App** - Your application links both frameworks

## Notes

- Do not use raw WAMR APIs directly
- Use only `iox_*` and `iox_wamr_*` functions
- Both frameworks must be linked together
EOF

# Create version file
echo "1.0.0" > "$DIST_DIR/VERSION"

# Create zip archive
cd "$OUTPUT_DIR"
zip -r "libiox-ios-sdk-1.0.0.zip" "libiox-ios-sdk"

echo ""
echo "========================================"
echo "Packaging Complete!"
echo "========================================"
echo ""
echo "Output: $OUTPUT_DIR/libiox-ios-sdk-1.0.0.zip"
echo ""
echo "Contents:"
ls -lh "$DIST_DIR/"
echo ""
echo "To use in Xcode:"
echo "1. Unzip libiox-ios-sdk-1.0.0.zip"
echo "2. Drag LibIOX.xcframework and LibIWasm.xcframework to your project"
echo "3. Add -lpthread to Other Linker Flags"

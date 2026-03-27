# iXland Local Build Matrix

This document describes the supported local validations for the current repository state.

## Quick Start

```bash
# Using CMake presets (recommended)
cmake --preset ios-simulator
cmake --build --preset ios-simulator

# Or manual configuration
mkdir build && cd build
cmake .. -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator \
         -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build .
```

## Supported Build Paths

### 1. Root Configure (iOS Simulator)

**Status:** ✅ Supported

Configure the monorepo for iOS Simulator builds:

```bash
cmake .. -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator \
         -DCMAKE_OSX_ARCHITECTURES=arm64
```

Or using presets:

```bash
cmake --preset ios-simulator
```

**Components built:**
- `ixland-libc-headers` - Public libc headers
- `ixland-libc-core` - Self-contained libc utilities
- `ixland-wasm-contracts` - Wasm contract headers
- `iox-core` - Core system implementation

### 2. Root Configure (iOS Device)

**Status:** ⚠️ Supported (requires code signing)

Configure for iOS Device builds:

```bash
cmake .. -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos \
         -DCMAKE_OSX_ARCHITECTURES=arm64
```

Or using presets:

```bash
cmake --preset ios-device
```

**Note:** Device builds require a development team for code signing.

### 3. ixland-libc-core Build

**Status:** ✅ Supported

Build the libc core static library:

```bash
cmake --build . --target ixland-libc-core
```

### 4. ixland-libc-core Tests

**Status:** ✅ Supported

Build and run libc smoke tests:

```bash
cmake .. -DIXLAND_LIBC_BUILD_TESTS=ON
cmake --build . --target ixland-libc-test
```

### 5. Wasm Contract Headers

**Status:** ✅ Supported

The `ixland-wasm-contracts` target provides public Wasm contract headers:

```bash
cmake --build . --target ixland-wasm-contracts
```

Currently consumed by:
- `runtime/wasi/iox_wamr.h` - WAMR integration
- `runtime/wasi/wasm_adapter.h/c` - Adapter layer

### 6. ixland-packages Validation

**Status:** ✅ Supported

Validate package repository structure:

```bash
cmake .. -DIXLAND_BUILD_PACKAGES=ON
cmake --build . --target ixland-packages-validate
```

### 7. Local Development (Non-iOS)

**Status:** ✅ Supported

Build libc and packages without iOS SDK:

```bash
cmake --preset local-dev
```

This builds:
- `ixland-libc`
- `ixland-wasm`
- `ixland-packages` validation

## Build Presets

Available CMake presets (see `CMakePresets.json`):

| Preset | Platform | Use Case |
|--------|----------|----------|
| `ios-simulator` | iOS Simulator | Primary development |
| `ios-device` | iOS Device | Device testing |
| `local-dev` | Host | Documentation, validation |

## Not Yet Supported

### Running Tests on Simulator

**Status:** ❌ Not yet supported

XCTest-based tests require:
- Simulator runtime
- Test host app
- Proper bundle signing

Workaround: Build tests, run manually in Xcode.

### Full Package Builds

**Status:** ❌ Not yet supported

Package builds are handled by `ixland-packages/scripts/build-package.sh`,
not CMake. See `ixland-packages/README.md` for package build instructions.

### Cross-compilation from Linux

**Status:** ❌ Not supported

iOS builds require macOS with Xcode.

## Component Build Matrix

| Component | Configure | Build | Test | Notes |
|-----------|-----------|-------|------|-------|
| ixland-libc-headers | ✅ | ✅ | N/A | Headers only |
| ixland-libc-core | ✅ | ✅ | ✅ | Smoke tests |
| ixland-wasm-contracts | ✅ | ✅ | N/A | Headers only |
| iox-core | ✅ | ✅ | ⚠️ | XCTest requires Xcode |
| ixland-packages | ✅ | ✅ | N/A | Validation only |
| ixland-toolchain | ✅ | N/A | N/A | Toolchain files |

## Troubleshooting

### "No Xcode or CLT version detected"

Install Xcode Command Line Tools:
```bash
xcode-select --install
```

### "Could not find a working profiler"

This is a warning, not an error. Safe to ignore for local builds.

### Code signing errors (device builds)

Set development team in CMake:
```bash
cmake .. -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=YOUR_TEAM_ID
```

## Validation Checklist

Before committing changes, verify:

- [ ] `cmake --preset ios-simulator` configures successfully
- [ ] `cmake --build --preset ios-simulator` builds without errors
- [ ] `ixland-libc-core` target builds
- [ ] `ixland-packages-validate` passes (if packages modified)

## See Also

- `docs/BUILD_GRAPH.md` - Build system architecture
- `docs/ARCHITECTURE.md` - Repository structure
- `CMakePresets.json` - Preset definitions
- `ixland-packages/README.md` - Package build instructions

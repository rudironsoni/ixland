# iXland Local Build Matrix

This document describes the supported local validations for the current repository state.

**Last updated:** March 2026 (Epic 30)
**Scope:** Post-libc and wasm boundary deepening

## Quick Start

```bash
# iOS Simulator (full build with iox-core)
cmake --preset ios-simulator -B build-ios

# Local development (no iOS SDK required)
cmake --preset local-dev -B build-local
cmake --build build-local
```

## Build Presets

Available CMake presets (see `CMakePresets.json`):

| Preset | Platform | Components | Use Case |
|--------|----------|------------|----------|
| `ios-simulator` | iOS Simulator | libc, system, wasm | Primary development |
| `ios-device` | iOS Device | libc, system, wasm | Device testing |
| `local-dev` | Host (macOS) | libc, wasm, packages | Documentation, validation |

### Preset Configuration Details

```json
// ios-simulator
{
  "inherits": "default",
  "toolchainFile": "ixland-toolchain/cmake/ixland-ios-simulator.cmake",
  "cacheVariables": {
    "CMAKE_OSX_SYSROOT": "iphonesimulator",
    "CMAKE_OSX_ARCHITECTURES": "arm64"
  }
}

// local-dev
{
  "inherits": "default",
  "cacheVariables": {
    "IXLAND_BUILD_SYSTEM": "OFF",  // No iOS SDK required
    "IXLAND_BUILD_PACKAGES": "ON",
    "IXLAND_BUILD_TESTS": "ON"
  }
}
```

## Supported Build Targets

### 1. ixland-libc-core (with tests)

**Status:** ✅ Supported

Self-contained libc-facing utilities library (`src/iox_version.c`, etc.).

**Configure via preset (recommended):**
```bash
cmake --preset local-dev -B build
```

**Or manual configure:**
```bash
cmake -B build \
    -DIXLAND_BUILD_LIBC=ON \
    -DIXLAND_BUILD_TESTS=ON
```

**Build:**
```bash
cmake --build build --target ixland-libc-core
```

**Build and run tests:**
```bash
cmake --build build --target ixland-libc-test
./build/ixland-libc/ixland-libc-test
```

**Outputs:**
- `build/ixland-libc/libixland-libc-core.a` - Static library
- `build/ixland-libc/ixland-libc-test` - Test executable

### 2. ixland-libc-usersdb

**Status:** ❌ NOT supported

User/group database utilities are not yet extracted from ixland-system.
The headers exist (`include/grp.h`, `include/pwd.h`) but no separate library target exists.

### 3. ixland-wasm-contracts

**Status:** ✅ Supported (headers only)

Wasm contract headers for engine abstraction. This is an INTERFACE library
(headers only, no compiled artifact).

**Configure:**
```bash
cmake --preset ios-simulator -B build
# or
cmake --preset local-dev -B build
```

**Headers provided:**
- `ixland/wasm/engine.h` - Engine abstraction
- `ixland/wasm/host.h` - Host contract
- `ixland/wasm/types.h` - Wasm types
- `ixland/wasm/wasi.h` - WASI ABI policy

**Note:** No build target to run (headers-only). Headers are consumed via:
```cmake
target_link_libraries(my_target PUBLIC ixland-wasm-contracts)
```

### 4. iox-core

**Status:** ✅ Supported (iOS only)

Core system implementation library from ixland-system.

**Configure (requires iOS SDK):**
```bash
cmake --preset ios-simulator -B build-ios
```

**Build:**
```bash
cmake --build build-ios --target iox-core
```

**Standalone build (ixland-system only):**
```bash
cd ixland-system
mkdir build && cd build
cmake ..  # Automatically sets iOS settings
cmake --build . --target iox-core
```

**Note:** iox-core requires iOS SDK. Use `ios-simulator` or `ios-device` preset.
The target consumes `ixland-libc-headers` and `ixland-wasm-contracts`.

### 5. iox-runtime

**Status:** ❌ NOT supported

No separate runtime library target exists yet. Runtime code lives in:
- `ixland-system/runtime/wasi/` - WASI implementation
- `ixland-system/runtime/native/` - Native runtime

Future extraction planned (see `docs/BUILD_GRAPH.md`).

### 6. ixland-packages-validate

**Status:** ✅ Supported

Validates package repository structure (not a full package build).

**Configure:**
```bash
cmake --preset local-dev -B build
# (sets IXLAND_BUILD_PACKAGES=ON automatically)
```

**Build:**
```bash
cmake --build build --target ixland-packages-validate
```

**Output:**
```
===============================================
Validating ixland-packages repository structure
===============================================

Checking directory structure...
  packages/        - OK
  core-packages/   - OK
  root-packages/   - OK
  scripts/         - OK
...
ixland-packages validation PASSED
```

**Note:** Full package builds use shell scripts, not CMake:
```bash
cd ixland-packages
./scripts/build-package.sh <package-name>
```

## Component Build Matrix

| Component | Preset | Build Target | Test | Notes |
|-----------|--------|--------------|------|-------|
| **ixland-libc-headers** | any | N/A (INTERFACE) | N/A | Headers only, consumed by iox-core |
| **ixland-libc-core** | local-dev, ios-* | `ixland-libc-core` | ✅ `ixland-libc-test` | Smoke tests run on host |
| **ixland-libc-usersdb** | ❌ N/A | ❌ N/A | ❌ N/A | Not extracted yet |
| **ixland-wasm-contracts** | any | N/A (INTERFACE) | N/A | Headers only, not yet consumed by runtime |
| **iox-core** | ios-simulator, ios-device | `iox-core` | ⚠️ `iox-core-tests` | XCTest requires Xcode run |
| **iox-runtime** | ❌ N/A | ❌ N/A | ❌ N/A | Not extracted yet |
| **ixland-packages** | local-dev | `ixland-packages-validate` | N/A | Validation only, not full builds |
| **ixland-toolchain** | any | N/A | N/A | Referenced via `CMAKE_TOOLCHAIN_FILE` |

### Quick Reference: Local Validations

| What | Command | Status |
|------|---------|--------|
| Configure local-dev | `cmake --preset local-dev -B build` | ✅ Supported |
| Build libc + tests | `cmake --build build` | ✅ Supported |
| Run libc tests | `./build/ixland-libc/ixland-libc-test` | ✅ Supported |
| Validate packages | `cmake --build build --target ixland-packages-validate` | ✅ Supported |
| Configure iOS sim | `cmake --preset ios-simulator -B build-ios` | ✅ Supported |
| Build iox-core | `cmake --build build-ios --target iox-core` | ✅ Supported |
| Build iox tests | `cmake --build build-ios --target iox-core-tests` | ✅ Supported |
| Run iox tests on sim | Requires Xcode test runner | ❌ Not yet |

## Not Yet Supported

These are intentionally NOT supported - documented for clarity:

### ixland-libc-usersdb

**Status:** ❌ Not yet supported

User/group database target does not exist. Headers exist but implementation
remains in ixland-system. No timeline for extraction.

### iox-runtime Separate Target

**Status:** ❌ Not yet supported

Runtime code is compiled into iox-core. Separate runtime library is a
future extraction (see `docs/BUILD_GRAPH.md`).

### Running XCTest on Simulator (without Xcode)

**Status:** ❌ Not yet supported

XCTest-based tests (`iox-core-tests`) require:
- Simulator runtime
- Test host app bundle
- Proper bundle signing
- xcodebuild or Xcode GUI

Workaround: Build tests, run manually in Xcode.

### Full Package Builds via CMake

**Status:** ❌ Not yet supported

Package builds are handled by `ixland-packages/scripts/build-package.sh`,
not CMake. See `ixland-packages/README.md` for package build instructions.

### Cross-compilation from Linux

**Status:** ❌ Not supported

iOS builds require macOS with Xcode.

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
cmake -B build-ios-device \
    --preset ios-device \
    -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=YOUR_TEAM_ID
```

### "No rule to make target ixland-wasm-contracts"

This is an INTERFACE library (headers only). No build target exists.
Headers are available via:
```cmake
target_link_libraries(my_target PUBLIC ixland-wasm-contracts)
```

### "ixland-system requires ixland-libc headers"

When building iox-core, ensure you have configured the full preset:
```bash
# Correct (has libc headers available)
cmake --preset ios-simulator -B build-ios

# Incorrect (standalone system without libc)
cmake -B build -DIXLAND_BUILD_SYSTEM=ON  # May fail
```

## Validation Checklist

Before committing changes, verify local validations work:

- [ ] `cmake --preset local-dev -B build` configures successfully
- [ ] `cmake --build build` builds `ixland-libc-core` and `ixland-libc-test`
- [ ] `./build/ixland-libc/ixland-libc-test` runs successfully
- [ ] `cmake --build build --target ixland-packages-validate` passes
- [ ] `cmake --preset ios-simulator -B build-ios` configures successfully
- [ ] `cmake --build build-ios --target iox-core` builds successfully

## See Also

- `docs/BUILD_GRAPH.md` - Build system architecture and dependencies
- `docs/ARCHITECTURE.md` - Repository structure
- `CMakePresets.json` - Preset definitions
- `ixland-packages/README.md` - Package build instructions

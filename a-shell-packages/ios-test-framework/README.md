# iOS Test Framework

Automated testing framework for cross-compiled packages on iOS Simulator.

## Overview

This framework runs package unit tests on iOS Simulator to validate cross-compiled libraries work correctly in the iOS environment.

## Features

- **Auto-detection**: Automatically finds universal or simulator builds
- **Universal support**: Prefers universal binaries (work on both device and simulator)
- **Simple usage**: Single command to build and run tests
- **Clear output**: Pass/fail results with detailed error reporting

## Quick Start

```bash
# Build your package first (universal is preferred)
cd a-shell-packages
./scripts/build-package.sh libz --target universal

# Run tests (auto-detects build)
./ios-test-framework/run-tests.sh libz
```

## Build Targets

The test framework works with all build targets:

| Target | Library Location | Works in Simulator |
|--------|-----------------|-------------------|
| `universal` | `.build/universal/staging/usr/local/lib/` | ✅ Yes (preferred) |
| `simulator` | `.build/simulator/staging/usr/local/lib/` | ✅ Yes |
| `ios` | `.build/ios/staging/usr/local/lib/` | ❌ No (device only) |

**Recommendation**: Use `--target universal` for development (default).

## Usage

### Run Tests

```bash
# Auto-detect and run tests
./ios-test-framework/run-tests.sh <package-name>

# Examples
./ios-test-framework/run-tests.sh libz
./ios-test-framework/run-tests.sh libssl
```

### Build Test Only (Without Running)

```bash
# Auto-detect build directory
./ios-test-framework/build-test.sh libz

# Specify build directory explicitly
./ios-test-framework/build-test.sh libz ../.build/universal/staging/usr/local
```

## How It Works

1. **Auto-detect**: Searches for library in this order:
   - Universal build (`.build/universal/`)
   - Simulator build (`.build/simulator/`)
   - Legacy tmp directories

2. **Build test app**:
   - Compiles test functions (C)
   - Compiles test runner (Objective-C)
   - Links with target library
   - Creates iOS Simulator binary

3. **Run on simulator**:
   - Finds available iPhone simulator
   - Creates app bundle with Info.plist
   - Installs and launches app
   - Captures results

## Architecture

```
ios-test-framework/
├── build-test.sh              # Build test app
├── run-tests.sh               # Build + run tests
├── README.md                  # This file
├── PackageTestApp/
│   ├── test_libz.c           # libz test cases
│   └── main.m                # Test runner (Objective-C)
└── build/
    └── <package>/            # Built test apps
        ├── test_<package>    # Test binary
        └── test_<package>.o  # Object files
```

## Adding Tests for New Packages

1. Create test file: `PackageTestApp/test_<package>.c`
2. Implement test functions (return 0 for pass, non-zero for fail)
3. Update test list in `build-test.sh` (lines 47-80)
4. Run: `./run-tests.sh <package>`

### Test Function Template

```c
// PackageTestApp/test_mylib.c
#include <mylib.h>

int test_mylib_feature(void) {
    // Test implementation
    if (mylib_init() != 0) {
        return 1;  // Fail
    }
    return 0;  // Pass
}
```

## Troubleshooting

### "Library not found"

Build the package first:
```bash
./scripts/build-package.sh libz --target universal
```

### "No iOS Simulator found"

Create a simulator:
```bash
xcrun simctl create 'iPhone 15' 'iPhone 15'
```

### Tests fail to link

Check that the library supports simulator architecture:
```bash
lipo -info .build/universal/staging/usr/local/lib/libz.a
# Should show: arm64 (for simulator)
```

## Integration with Build System

The test framework integrates with the three-target build system:

```bash
# Build for development (includes simulator support)
./scripts/build-package.sh libz --target universal

# Test immediately after building
./ios-test-framework/run-tests.sh libz

# Build for production (device only, smaller)
./scripts/build-package.sh libz --target ios
```

## Future Enhancements

- [ ] JSON test report output
- [ ] Multiple test suites per package
- [ ] Performance benchmarking
- [ ] Memory leak detection
- [ ] Code coverage reporting
- [ ] CI/CD integration

## Requirements

- macOS with Xcode
- iOS Simulator (arm64)
- Built package libraries

## See Also

- `../AGENTS.md` - Package build system documentation
- `../scripts/build-package.sh` - Package builder with target support
- `../WAVE-1-VALIDATION.md` - Validation methodology

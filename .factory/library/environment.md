# Environment

## Build Environment

### Prerequisites

**Required:**
- macOS 13.0+ (for iOS development)
- Xcode 15.0+ with iOS SDK
- CMake 3.20+
- clang/clang++ (from Xcode)

**Optional:**
- Docker (for osxcross budget builds)
- osxcross (for Linux cross-compilation)

### Build Configurations

| Configuration | CMake Option | Use Case |
|--------------|--------------|----------|
| Debug | `-DCMAKE_BUILD_TYPE=Debug` | Development, debugging |
| Release | `-DCMAKE_BUILD_TYPE=Release` | Production builds |
| Simulator | `-DCMAKE_OSX_SYSROOT=iphonesimulator` | iOS Simulator testing |
| Device | `-DCMAKE_OSX_SYSROOT=iphoneos` | Physical device |

### CMake Options

```bash
# Component selection
-DIXLAND_BUILD_LIBC=ON          # Build ixland-libc
-DIXLAND_BUILD_SYSTEM=ON        # Build ixland-system
-DIXLAND_BUILD_WASM=ON          # Build ixland-wasm
-DIXLAND_BUILD_PACKAGES=OFF     # Skip ixland-packages
-DIXLAND_BUILD_TESTS=ON         # Build tests
```

## Environment Variables

### Build Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `CMAKE_OSX_DEPLOYMENT_TARGET` | Minimum iOS version | 16.0 |
| `CMAKE_OSX_ARCHITECTURES` | Target architectures | arm64 |
| `IXLAND_BUILD_ROOT` | Root build directory | `${CMAKE_BINARY_DIR}` |

### Runtime Variables (Future)

| Variable | Description |
|----------|-------------|
| `IXLAND_PREFIX` | Installation prefix |
| `IXLAND_PATH` | Command search path |
| `IXLAND_HOME` | User home directory |

## Platform-Specific Notes

### iOS Device

- Code signing required for execution
- Sandbox restrictions apply
- File system access limited to app container

### iOS Simulator

- No code signing required
- Can access host filesystem (with restrictions)
- Preferred for development and testing

### macOS (for testing)

- Some syscalls may behave differently
- Used for unit testing without iOS simulator
- Not a primary target

## Dependency Management

### Git Submodules

None currently used. All dependencies are:
- Included in repository (WAMR in vendor/)
- System-provided (iOS SDK)
- Fetched at build time (if any)

### External Dependencies

| Dependency | Purpose | Location |
|------------|---------|----------|
| WAMR | WebAssembly runtime | ixland-system/vendor/wamr/ |
| iOS SDK | Platform APIs | Xcode toolchain |

## IDE Integration

### Xcode

```bash
# Generate Xcode project
cmake -B build -G Xcode
open build/ixland.xcodeproj
```

### VS Code

Recommended extensions:
- C/C++ (Microsoft)
- CMake Tools
- clangd (for LSP)

### Cursor

Works with CMake Tools extension for IntelliSense and build integration.

## Troubleshooting

### Common Build Issues

**Issue:** `CMAKE_SYSTEM_NAME is not iOS`
```bash
# Solution: Explicitly set
-DCMAKE_SYSTEM_NAME=iOS
```

**Issue:** `No iOS SDK found`
```bash
# Solution: Check Xcode path
xcode-select -p
sudo xcode-select -s /Applications/Xcode.app
```

**Issue:** `Architecture not supported`
```bash
# Solution: Use simulator for Intel Macs
-DCMAKE_OSX_SYSROOT=iphonesimulator
-DCMAKE_OSX_ARCHITECTURES=x86_64
```

### Debug Build

```bash
cmake -B build-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphonesimulator
cmake --build build-debug --verbose
```

## CI/CD Environment

### GitHub Actions

Build matrix:
- macOS 14 + Xcode 15
- iOS Simulator (arm64, x86_64)
- Debug and Release configurations

### Local CI Simulation

```bash
# Run the same checks as CI
./scripts/lint.sh --check
./scripts/format.sh --check
cmake -B build-ci -DCMAKE_SYSTEM_NAME=iOS
cmake --build build-ci
cmake --build build-ci --target test
```

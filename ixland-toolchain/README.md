# ixland-toolchain

Toolchain integration boundary for iXland.

## Purpose

This directory provides CMake toolchain files for cross-compilation and build system integration.

## Contents

### cmake/

Toolchain definition files for iOS builds:

- `ixland-ios-simulator.cmake` - Toolchain for iOS Simulator (arm64)
  - SDK: iphonesimulator
  - Architecture: arm64
  - Deployment target: iOS 16.0+
  - Code signing: Disabled (simulator)

- `ixland-ios-device.cmake` - Toolchain for iOS Device (arm64)
  - SDK: iphoneos
  - Architecture: arm64
  - Deployment target: iOS 16.0+
  - Code signing: Required (configure development team)

## Usage

### With Toolchain File Directly

```bash
cd ixland
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../ixland-toolchain/cmake/ixland-ios-simulator.cmake
```

### With CMake Presets (Recommended)

See `CMakePresets.json` at repository root for pre-configured presets.

## Design Principle

This directory is not a dumping ground for random scripts. Contents should be well-structured toolchain components with clear integration points.

## Does Not Replace

- CMake (build orchestration)
- `.github/workflows` (CI configuration)
- `ixland-packages` (package build recipes)

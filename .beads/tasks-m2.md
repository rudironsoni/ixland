# M2 Tasks

## M2-I2: Create build.sh main orchestration script

**Priority:** P0
**Parent:** M2: Forge Alpha

### Description

## Goal
Create the main build.sh orchestration script with build/clean/install/package/list commands.

## Success Criteria
- [ ] build command works for any package
- [ ] clean command removes artifacts
- [ ] install command for local testing
- [ ] package command creates distributable
- [ ] list command shows all packages
- [ ] Help/usage display

## Context
This is the CLI developers use to interact with the build system.

## Dependencies
- **Blocked by:** M2-I1
- **Blocks:** M2-I3

## Implementation Details
Create ashell-packages/build.sh:
- Parse command line arguments
- Dispatch to package build.sh
- Handle global options

## Files to Modify
- ashell-packages/build.sh - Create

## When to Stop
- All commands work
- Well documented

## Verification Steps
1. ./build.sh list (shows packages)
2. ./build.sh hello (builds hello)
3. ./build.sh hello clean (cleans)

---

## M2-I3: Create hello reference package

**Priority:** P0
**Parent:** M2: Forge Alpha

### Description

## Goal
Create the hello reference package demonstrating the full build pattern.

## Success Criteria
- [ ] Minimal C program that prints greeting
- [ ] Complete build.sh following conventions
- [ ] Uses ios_system.h for output
- [ ] Builds to XCFramework
- [ ] Can be installed via apt

## Context
This is the canonical example all future packages reference.

## Dependencies
- **Blocked by:** M2-I1, M2-I2
- **Blocks:** None

## Files to Modify
- ashell-packages/hello/build.sh - Create
- ashell-packages/hello/README.md - Create

## When to Stop
- Builds successfully
- Serves as template

## Verification Steps
1. ./build.sh hello
2. Check .build/hello/ contains XCFramework
3. Manually install and test

---

## M2-I4: Add iOS cross-compilation toolchain

**Priority:** P0
**Parent:** M2: Forge Alpha

### Description

## Goal
Create iOS cross-compilation toolchain configuration.

## Success Criteria
- [ ] CC/CXX configured for iOS targets
- [ ] Supports arm64 and x86_64 simulator
- [ ] cmake toolchain file
- [ ] autotools configuration

## Dependencies
- **Blocked by:** M2-I1
- **Blocks:** M2-I5

## Files to Modify
- ashell-packages/config/ios-toolchain.cmake
- ashell-packages/config/ios-configure.sh

## When to Stop
- Builds work on macOS with Xcode
- List works on Linux

## Verification Steps
1. Build hello for arm64
2. Build hello for x86_64 simulator
3. Verify on Linux: ./build.sh list works

---

## M2-I5: Implement XCFramework generation

**Priority:** P0
**Parent:** M2: Forge Alpha

### Description

## Goal
Implement XCFramework creation from build artifacts.

## Success Criteria
- [ ] Combines device and simulator builds
- [ ] Includes headers
- [ ] Includes Info.plist
- [ ] Code signing optional but supported

## Dependencies
- **Blocked by:** M2-I4
- **Blocks:** Package distribution

## Files to Modify
- ashell-packages/ashell_package.sh - Add XCFramework step

## When to Stop
- Creates valid XCFramework
- Loads in a-Shell

## Verification Steps
1. Build hello
2. Check XCFramework contents
3. Test loading in a-Shell

---

## M2-I6: Replace pkg extraction with libarchive

**Priority:** P0
**Parent:** M2: Forge Alpha

### Description

## Goal
Replace URLSession-based extraction with libarchive integration.

## Success Criteria
- [ ] ArchiveExtractor.swift uses libarchive
- [ ] Supports .tar.gz, .tar.bz2, .tar.xz, .zip
- [ ] Progress callbacks
- [ ] Error handling

## Dependencies
- **Blocked by:** M2-I1
- **Blocks:** apt install functionality

## Files to Modify
- ashell-core/Sources/PackageManager/ArchiveExtractor.swift

## When to Stop
- All test archives extract
- Progress reported

## Verification Steps
1. Test with .tar.gz
2. Test with .zip
3. Verify progress callbacks

---

## M2-I7: Add plist generation from build metadata

**Priority:** P0
**Parent:** M2: Forge Alpha

### Description

## Goal
Generate commands.plist from ASHELL_PKG_COMMANDS metadata.

## Success Criteria
- [ ] Parses ASHELL_PKG_COMMANDS array
- [ ] Generates valid plist XML
- [ ] Includes framework path
- [ ] Includes entry point

## Dependencies
- **Blocked by:** M2-I1
- **Blocks:** Command registration

## Files to Modify
- ashell-packages/ashell_package.sh - Add plist generation

## When to Stop
- Generated plist loads in ios_system

## Verification Steps
1. Build hello
2. Check generated plist
3. Load in ios_system

---

## M2-I8: Create CI pipeline for automated builds

**Priority:** P0
**Parent:** M2: Forge Alpha

### Description

## Goal
Create GitHub Actions CI pipeline for automated package builds.

## Success Criteria
- [ ] Builds on macOS runner
- [ ] Matrix build for packages
- [ ] Artifact upload
- [ ] Status badges

## Dependencies
- **Blocked by:** All other M2 issues
- **Blocks:** None (final M2 task)

## Files to Modify
- .github/workflows/packages.yml - Create

## When to Stop
- CI passes for all packages

## Verification Steps
1. Push to trigger CI
2. Verify all packages build
3. Download and verify artifacts

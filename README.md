# IXLand

IXLand is an iOS-first monorepo with Xcode as the only build truth.

## Repository Structure

- `IXLand/` - brand-new iOS app at repo root
- `IXLandSystem/` - canonical syscall-facing and kernel-facing implementation
- `IXLandLibC/` - libc and ABI-facing boundaries
- `IXLandWasm/` - WebAssembly-facing boundaries
- `IXLandPackages/` - package metadata and package tooling
- `IXLandToolchain/` - Xcode-oriented toolchain assets and metadata
- `docs/` - architecture and cutover documentation
- `scripts/` - repository maintenance scripts

## Build Truth

Xcode is the only build truth.

Use `xcodebuild` against `IXLand/IXLand.xcodeproj`.

```bash
xcodebuild -project IXLand/IXLand.xcodeproj -list
xcodebuild -project IXLand/IXLand.xcodeproj -scheme IXLand -sdk iphonesimulator build
```

CMake, CTest, and Make are not part of the active build graph.

## Current Architecture

Canonical syscall-facing and kernel-facing ownership lives under:

- `IXLandSystem/fs/*`
- `IXLandSystem/kernel/*`

Historical migration material may mention old names or deleted layouts, but active architecture and active build truth use only the `IXLand*` roots.

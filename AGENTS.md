# Agent Guide: IXLand Repository

## Current Architecture (April 2026)

### Repository Structure

```
IXLand/                 # iOS app at repo root
IXLandSystem/           # Linux-like virtual subsystem
IXLandLibC/            # libc and ABI boundaries
IXLandWasm/            # WebAssembly contract boundaries
IXLandPackages/        # Package metadata and tooling
IXLandToolchain/       # Toolchain configuration
```

### Build Truth

**Single source of truth**: `IXLand/IXLand.xcodeproj` plus shared schemes

**CLI authority**: `xcodebuild` only

**Forbidden**: CMake, CTest, Make as active build truth

### Canonical Ownership

- **Filesystem syscalls**: `IXLandSystem/fs/*`
- **Kernel syscalls**: `IXLandSystem/kernel/*`
- **LibC boundary**: `IXLandLibC/include/ixland/*`
- **WASM boundary**: `IXLandWasm/include/ixland/wasm/*`

### Naming Rules

- Use `IXLand*` roots for all active architecture references
- Use `ixland_*` prefixes for C functions and types
- No active references to old lowercase component roots
- No active references to `a-Shell` or `a_shell`

### Historical References

Documents in `docs/archive/` or explicitly marked "HISTORICAL" may reference old names. These do not override the active Xcode-only build truth.

## Development Workflow

```bash
# Build app
xcodebuild -project IXLand/IXLand.xcodeproj -scheme IXLand -sdk iphonesimulator build

# Run tests
xcodebuild -project IXLand/IXLand.xcodeproj -scheme IXLandTests -sdk iphonesimulator test
```

## Constraints

- No Make/CMake/CTest in active build graph
- No old lowercase component roots as active architecture
- No compatibility shims or backward compatibility layers
- Historical docs must be explicitly labeled as historical

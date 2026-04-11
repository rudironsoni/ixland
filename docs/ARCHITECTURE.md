# iXland Architecture

## Purpose of the Monorepo

iXland is a Linux-like virtual kernel subsystem for iOS, designed for maximum practical Linux userland compatibility within App Store constraints. This monorepo contains all components needed to build, run, and distribute the iXland environment.

## Current Top-Level Layout

```
ixland/
├── IXLand/              # iOS terminal application
├── IXLandSystem/        # Core kernel/system layer (current implementation home)
├── IXLandLibC/          # C library and ABI boundary (emerging)
├── IXLandWasm/          # WebAssembly boundaries (neutral container)
│   ├── ixland-wasm-engine/   # Engine-neutral backend contract
│   ├── ixland-wasm-host/     # Host service contract for guests
│   └── ixland-wasm-wasi/    # WASI guest ABI policy
├── IXLandPackages/      # Package build system and distribution
├── IXLandToolchain/     # Toolchain integration
└── docs/                # Documentation
```

## Role of Each Component

### IXLand

The iOS terminal application. Contains the UI (WebView-based terminal), session management, and package management integration.

### IXLandSystem

**Current home of the implementation.** Contains the virtual kernel, syscall implementations, process management, signal handling, VFS, TTY drivers, and runtime backends (native, WASI via WAMR, script interpreter).

This component remains the source of truth for implementation until narrow extraction to other boundaries occurs.

### IXLandLibC

**Emerging boundary.** Future home for:
- Public user-facing libc/ABI material
- POSIX compatibility headers
- System call interface definitions

Currently skeletal. First extraction target is headers and ABI-facing material, not runtime policy.

### IXLandWasm

**Neutral container** for WebAssembly boundaries. Does not currently contain extracted implementation.

- **ixland-wasm-engine**: Future engine-neutral contract for WebAssembly runtime backends. Current backend (WAMR) implementation lives elsewhere.
- **ixland-wasm-host**: Future host-service contract from guest runtimes into iXland semantics (fd, fs, path, clock, random, socket, process).
- **ixland-wasm-wasi**: Future guest ABI policy for WASI. Defines how WASI maps to iXland semantics.

### IXLandPackages

Package definitions, build recipes, and distribution concerns. Also manages future compiled Wasm artifacts.

### IXLandToolchain

Future place for build/toolchain integration. Does not replace Xcode build.

## Current Reality

Most implementation still lives under `IXLandSystem`. This includes:
- Virtual kernel subsystems (task, signal, exec, VFS, TTY)
- Runtime backends (native, WASI/script via WAMR)
- Platform compatibility layers

`IXLandLibC` and `IXLandWasm` are currently boundary-forming areas, not full extracted implementations. They document intended future contracts but do not yet contain the bulk of implementation.

## Near-Term Direction

1. **Identity cleanup** (current) - Normalize repository naming and documentation
2. **Root build truth** - Establish minimal root CMake direction and `.github/workflows` baseline
3. **First narrow libc extraction** - Extract public headers and ABI-facing material from `IXLandSystem` into `IXLandLibC`
4. **Wasm interface specification** - Document boundary contracts before runtime abstraction coding

## Not Yet

These are explicitly deferred:

- No deep split of `IXLandSystem` - extraction is narrow and incremental
- No separate top-level WAMR repository - WAMR remains a backend detail
- No separate top-level WASI repository - WASI is under `IXLandWasm/ixland-wasm-wasi`
- No artifact/AOT layer - compiled Wasm artifacts are an `IXLandPackages` concern

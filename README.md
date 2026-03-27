# ixland

This repository is the monorepo for iXland, a Linux-like environment for iOS.

## Repository Structure

This monorepo contains:
- `ixland-app/` - iOS terminal application
- `ixland-system/` - Core kernel/system layer with the current implementation
- `ixland-packages/` - Package build system and distribution
- `ixland-libc/` - C library boundary (emerging; contains public headers and ABI-facing material as they are extracted)
- `ixland-wasm/` - WebAssembly boundaries (neutral container)
  - `ixland-wasm-engine/` - Engine abstraction layer
  - `ixland-wasm-host/` - Host contract between guest runtimes and iXland
  - `ixland-wasm-wasi/` - WASI guest ABI policy and conformance
- `ixland-toolchain/` - Toolchain integration boundary

## Setup

Submodules are no longer used. Use the monorepo root for checkout and build.

```bash
git clone git@github.com:rudironsoni/ixland.git
cd ixland
# No submodule initialization required
```

## Build

Build orchestration is expected to flow through CMake.
See individual component READMEs for specific build instructions.

## CI

CI configuration lives in `.github/workflows/`.

## Current Reality

Most implementation still lives under `ixland-system`. The `ixland-libc` and `ixland-wasm` boundaries are currently forming areas for future extraction, not yet full extracted implementations.

See `docs/ARCHITECTURE.md` for the canonical architecture documentation and `docs/BOUNDARIES.md` for boundary definitions.

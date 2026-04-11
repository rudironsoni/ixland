# IXLandWasm

WebAssembly contract boundary for IXLand.

## Purpose

This component defines the public contract for WebAssembly support in IXLand.

## Structure

- `IXLandWasmEngine/` - Engine-neutral backend contract
- `IXLandWasmHost/` - Host-service contract for guests
- `IXLandWasmWASI/` - WASI guest ABI policy
- `include/ixland/wasm/` - Public contract headers
- `docs/` - Boundary specifications

## Public Headers

| Header | Purpose |
|--------|---------|
| `include/ixland/wasm/types.h` | Fundamental types, opaque handles |
| `include/ixland/wasm/engine.h` | Engine-neutral runtime contract |
| `include/ixland/wasm/host.h` | Host-service areas (vtable) |
| `include/ixland/wasm/wasi.h` | WASI policy and mapping |

## Current State

- Contract headers are defined and build-visible
- `docs/WASM_BOUNDARY_SPEC.md` documents the boundary
- Implementation remains in `IXLandSystem` (WAMR backend)

## Build Integration

This component provides contract headers for use by the IXLand app.

## Documentation

- `docs/WASM_BOUNDARY_SPEC.md` - Canonical boundary specification

## Compiled Artifacts

Compiled Wasm artifacts are a concern of `IXLandPackages`.

## Future Direction

See `docs/BOUNDARIES.md` for detailed boundary definitions.

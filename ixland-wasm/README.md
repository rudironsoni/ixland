# ixland-wasm

WebAssembly contract boundary for iXland.

## Purpose

This component defines the public contract for WebAssembly support in iXland.

## Structure

- `ixland-wasm-engine/` - Engine-neutral backend contract
- `ixland-wasm-host/` - Host-service contract for guests
- `ixland-wasm-wasi/` - WASI guest ABI policy
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
- Implementation remains in `ixland-system` (WAMR backend)

## Build Integration

This component provides:
- `ixland-wasm-contracts` - INTERFACE target for contract headers

## Documentation

- `docs/WASM_BOUNDARY_SPEC.md` - Canonical boundary specification

## Compiled Artifacts

Compiled Wasm artifacts are a concern of `ixland-packages`.

## Future Direction

See `docs/BOUNDARIES.md` for detailed boundary definitions.

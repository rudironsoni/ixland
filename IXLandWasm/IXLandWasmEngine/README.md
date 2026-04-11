# IXLandWasmEngine

Engine-neutral WebAssembly backend contract.

## Purpose

This boundary defines the engine-neutral interface that allows IXLand to work with different WebAssembly engines without tight coupling.

## Scope

- Engine abstraction layer
- Common interface for runtime backends
- WASM module loading and execution

## Public Headers

- `../include/ixland/wasm/types.h` - Fundamental types
- `../include/ixland/wasm/engine.h` - Engine-neutral contract

## Current State

- Contract is defined in public headers
- Current backend is WAMR, implemented in `IXLandSystem`
- WAMR remains an implementation detail, not a top-level boundary

## Documentation

- `../../docs/WASM_BOUNDARY_SPEC.md` - Section on IXLandWasmEngine

## Future Work

- Backend implementations (WAMR, Wasmtime)
- Engine registration system

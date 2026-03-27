# ixland-wasm

Neutral container for WebAssembly boundaries in iXland.

## Purpose

This directory is the neutral Wasm boundary area. It contains future contracts and interfaces for WebAssembly support.

## Structure

- `ixland-wasm-engine/` - Future engine-neutral backend contract
- `ixland-wasm-host/` - Future host-service contract for guests
- `ixland-wasm-wasi/` - Future WASI guest ABI policy
- `docs/` - Boundary specifications and design documents

## Current State

The current WAMR WASI implementation lives in `ixland-system`. Interface contracts are being defined in `docs/WASM_BOUNDARY_SPEC.md`.

## Documentation

- `docs/WASM_BOUNDARY_SPEC.md` - Complete boundary specification with service areas and mapping strategy

## Compiled Artifacts

Compiled Wasm artifacts are a future concern of `ixland-packages`.

## Future Direction

See `docs/BOUNDARIES.md` for detailed boundary definitions and extraction rules.

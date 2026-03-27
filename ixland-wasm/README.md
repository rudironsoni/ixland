# ixland-wasm

Neutral container for WebAssembly boundaries in iXland.

## Purpose

This directory is the neutral Wasm boundary area. It contains future contracts and interfaces for WebAssembly support, but does not currently contain the extracted implementation.

## Structure

- `ixland-wasm-engine/` - Future engine-neutral backend contract
- `ixland-wasm-host/` - Future host-service contract for guests
- `ixland-wasm-wasi/` - Future WASI guest ABI policy

## Current State

The current WAMR WASI implementation lives in `ixland-system`. These boundaries will receive narrow extractions once the interface contracts are well-defined.

## Compiled Artifacts

Compiled Wasm artifacts are a future concern of `ixland-packages`.

## Future Direction

See `docs/BOUNDARIES.md` for detailed boundary definitions and extraction rules.

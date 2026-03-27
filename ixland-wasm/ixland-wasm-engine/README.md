# ixland-wasm-engine

Engine-neutral WebAssembly backend contract.

## Purpose

This boundary defines the future engine-neutral interface that allows iXland to work with different WebAssembly engines without tight coupling to any specific implementation.

## Scope

- Engine abstraction layer
- Common interface for runtime backends
- Engine-neutral WASM module loading and execution

## Current State

The current backend is WAMR (WebAssembly Micro Runtime), implemented within `ixland-system`. This directory will receive narrow extraction once the engine-neutral contract is well-defined.

## Note on WAMR

WAMR is the current runtime backend detail, but it is not a top-level repository boundary. It remains an implementation detail until abstraction is warranted.

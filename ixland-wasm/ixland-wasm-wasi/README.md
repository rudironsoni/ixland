# ixland-wasm-wasi

WASI guest ABI policy and conformance for iXland.

## Purpose

This boundary defines how the WebAssembly System Interface (WASI) maps onto iXland host semantics.

## Scope

- WASI syscall definitions and mappings
- WASI-specific adaptations for iOS constraints
- WASI conformance testing
- WASI capability-based security integration

## Mapping Strategy

WASI operations should map to iXland host services, not directly to iOS APIs. This ensures:

- Consistent behavior with native iXland commands
- Proper virtualization (e.g., virtual PIDs work in WASI too)
- Shared policy enforcement

## Current State

WASI support is currently implemented via WAMR within `ixland-system`. This boundary will define and receive extraction of the WASI-specific policy layer.

## Note

This is not a separate top-level repository. It is contained within `ixland-wasm/` as part of the neutral Wasm boundary.

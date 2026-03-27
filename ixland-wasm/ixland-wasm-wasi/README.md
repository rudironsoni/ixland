# ixland-wasm-wasi

WASI guest ABI policy and conformance for iXland.

## Purpose

This boundary defines how the WebAssembly System Interface (WASI) maps onto iXland host semantics.

## Scope

- WASI syscall definitions and mappings
- WASI error codes and types
- WASI capability-based rights
- WASI version support (Preview 0/1/2)

## Public Headers

- `../include/ixland/wasm/types.h` - Fundamental types
- `../include/ixland/wasm/host.h` - Host services (underlying implementation)
- `../include/ixland/wasm/wasi.h` - WASI policy and mapping

## Mapping Strategy

WASI operations map to iXland host services:

| WASI Operation | iXland Mapping |
|----------------|----------------|
| `fd_read` | `host->fd_read` |
| `fd_write` | `host->fd_write` |
| `path_open` | `host->path_open` |
| `clock_gettime` | `host->clock_gettime` |
| `random_get` | `host->random_get` |
| `proc_exit` | `host->proc_exit` |

This ensures consistent behavior with native iXland commands.

## Current State

- Contract is defined in public headers (wasi.h)
- WASI implementation remains in `ixland-system`
- Policy layer documented in WASM_BOUNDARY_SPEC.md

## Documentation

- `../../docs/WASM_BOUNDARY_SPEC.md` - Section on ixland-wasm-wasi

## Note

This is not a separate top-level repository. It is contained within `ixland-wasm/`.

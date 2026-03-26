# WAMR Integration for libiox

## Overview

libiox integrates WebAssembly Micro Runtime (WAMR) on iOS through a **libiox-owned adapter layer** and an **externally built upstream runtime**.

This repository follows these rules:

- `deps/wamr/` is upstream source and must remain read-only
- WAMR is built **out-of-tree** for iOS device and simulator
- libiox and libiwasm are separate artifacts
- app customers must use **`iox_wamr_*` only**

## Artifact Model

Expected build outputs:

- `lib/libiox-sim.a`
- `lib/libiox-device.a`
- `build/wamr-simulator/libiwasm.a`
- `build/wamr-device/libiwasm.a`

App/test targets link both libraries explicitly.

## Ownership Boundary

### libiox-owned

- `include/iox/iox_wamr.h`
- `src/iox/wamr/iox_wamr_simple.c`
- any WASI-to-libiox bridge code
- all iOS toolchain/build policy outside `deps/wamr`

### upstream WAMR-owned

- runtime implementation under `deps/wamr/`
- upstream headers and internal APIs

## Building WAMR

Use the repository-owned wrapper script:

```bash
./build_wamr_ios_static.sh simulator
./build_wamr_ios_static.sh device
```

Or build both:

```bash
./build_wamr_ios_static.sh all
```

This script configures upstream WAMR with iOS settings **without editing the submodule**.

## Building libiox and WAMR Together

```bash
make sdk-sim
make sdk-device
make sdk
```

## Public API

Applications should use only these adapter APIs:

- `iox_wamr_init()`
- `iox_wamr_deinit()`
- `iox_wamr_is_initialized()`
- `iox_wamr_load_module()`
- `iox_wamr_unload_module()`
- `iox_wamr_validate_wasm()`
- `iox_wamr_call_function()`
- `iox_wamr_function_exists()`
- `iox_wamr_get_memory_size()`
- `iox_wamr_memory_read()`
- `iox_wamr_memory_write()`
- `iox_wamr_get_export_list()`
- `iox_wamr_free_export_list()`
- `iox_wamr_get_error()`

## Unsupported Consumer Usage

The following are not part of the supported app-facing contract:

- direct `wasm_runtime_*` usage
- direct inclusion of upstream WAMR headers in app code
- linking apps as though WAMR were the primary SDK surface

## iOS Constraints

- interpreter mode only
- no JIT
- no AOT/JIT runtime code generation
- sandbox-aware file and WASI behavior
- validate all crossings between WASI and libiox

# libiox: iOS Subsystem for Linux

<p align="center">
<img src="https://img.shields.io/badge/Platform-iOS%2016.0+-lightgrey.svg" alt="Platform: iOS">
<img src="https://img.shields.io/badge/arch-arm64%20device%20%7C%20arm64%2Fx86__64%20sim-blue.svg" alt="Architecture: iOS only">
</p>

**libiox** is an iOS-only Linux syscall compatibility layer. It provides symbol interposition, virtual process behavior, sandbox-aware filesystem handling, and an iOS-facing runtime surface for Linux-oriented software.

It also integrates **WAMR** as an external WebAssembly runtime, but with a strict SDK boundary:

- `libiox` and `libiwasm` are built as separate iOS artifacts
- `deps/wamr/` is upstream source and must remain untouched
- iOS toolchain and build configuration for WAMR are owned outside `deps/wamr`
- app customers must use **`iox_wamr_*` only** and must not integrate against raw WAMR APIs

## Overview

libiox allows Linux-oriented code to target iOS while preserving standard syscall-facing source patterns:

```c
#include <unistd.h>

pid_t pid = fork();
```

Under libiox, those calls are interposed onto iOS-safe implementations.

For WebAssembly support, applications do not talk to WAMR directly. They use the public adapter surface in `include/iox/iox_wamr.h`.

## Platform Policy

- **Platform**: iOS only
- **Minimum deployment target**: iOS 16.0+
- **Supported architectures**:
  - arm64 device
  - arm64/x86_64 simulator
- **Not authoritative for validation**: macOS runtime behavior

## Architecture

```text
┌─────────────────────────────────────────────────────────────┐
│                        iOS App                              │
├─────────────────────────────────────────────────────────────┤
│                App-visible supported surface                │
│            iox_* syscalls + iox_wamr_* adapter              │
├─────────────────────────────────────────────────────────────┤
│                         libiox                              │
│  interpose | process | vfs | env | signal | file | net     │
│                 WASI/libiox bridge logic                    │
├─────────────────────────────────────────────────────────────┤
│                        libiwasm                             │
│             upstream WAMR runtime (external)                │
├─────────────────────────────────────────────────────────────┤
│                         iOS APIs                            │
└─────────────────────────────────────────────────────────────┘
```

## Ownership Boundary

### libiox owns

- syscall interposition and public `iox_*` APIs
- process/thread simulation
- VFS, file descriptor, environment, signal, and network layers
- public WAMR adapter API: `iox_wamr_*`
- any WASI-to-libiox bridge code

### WAMR owns

- upstream runtime implementation
- upstream internal/runtime headers
- upstream platform/runtime source in `deps/wamr/`

### Customer-facing rule

- supported: `iox_*`, `iox_wamr_*`
- unsupported: raw WAMR APIs and direct dependence on `deps/wamr/`

## Build Model

WAMR is an **external dependency** built out-of-tree.

The correct artifact model is:

- `lib/libiox-sim.a`
- `lib/libiox-device.a`
- `build/wamr-simulator/libiwasm.a`
- `build/wamr-device/libiwasm.a`

`libiox` must not embed or archive `libiwasm.a` into its own archive. App and test targets must link both libraries explicitly.

## Quick Start

### Clone

```bash
git clone --recursive https://github.com/yourusername/a-shell-kernel.git
cd a-shell-kernel
```

### Build libiox only

```bash
make iox-sim
make iox-device
```

### Build WAMR only

```bash
./build_wamr_ios_static.sh simulator
./build_wamr_ios_static.sh device
```

### Build the full iOS SDK artifact set

```bash
make sdk-sim
make sdk-device
make sdk
```

## App Integration

### Static library integration

Link both artifacts explicitly:

```text
lib/libiox-<variant>.a
build/wamr-<variant>/libiwasm.a
```

Use libiox public headers only:

```c
#include <iox/iox.h>
#include <iox/iox_wamr.h>
```

Do not include or expose raw WAMR headers in app integration code.

## WAMR Integration Policy

libiox provides WebAssembly support through an adapter layer owned in this repository.

### Publicly supported API

- `iox_wamr_init()`
- `iox_wamr_deinit()`
- `iox_wamr_load_module()`
- `iox_wamr_unload_module()`
- `iox_wamr_call_function()`
- `iox_wamr_function_exists()`
- `iox_wamr_validate_wasm()`
- `iox_wamr_memory_read()`
- `iox_wamr_memory_write()`
- `iox_wamr_get_error()`

### Not part of the app-facing contract

- direct `wasm_runtime_*` calls
- raw WAMR headers as public SDK surface
- edits inside `deps/wamr/`

## Project Structure

```text
a-shell-kernel/
├── include/iox/                 # Public libiox headers
├── src/iox/core/                # Syscall implementations
├── src/iox/interpose/           # Strong-symbol interposition
├── src/iox/wamr/                # libiox-owned WAMR adapter layer
├── src/iox/internal/            # Private headers
├── deps/wamr/                   # Upstream WAMR source (read-only)
├── build/wamr-device/           # Out-of-tree WAMR device output
├── build/wamr-simulator/        # Out-of-tree WAMR simulator output
├── lib/                         # libiox build outputs
├── ios-test-app/                # iOS validation app and tests
└── tests/                       # Additional test assets
```

## Syscall Coverage

Implemented coverage includes process, file I/O, filesystem, environment, signal, memory, time, TTY, and network layers, plus the `iox_wamr_*` adapter surface.

See `docs/SYSCALLS.md` and `TEST_COVERAGE.md` for detailed coverage.

## Packaging Model

Recommended distribution model:

- `LibIOX.xcframework`
- `LibIWasm.xcframework`
- one top-level SDK bundle containing both artifacts and libiox public headers

This preserves architecture boundaries while still shipping one iOS SDK to consumers.

## Constraints

- no real `fork()` or `execve()`
- no setuid
- no sandbox bypass
- no JIT or dynamic code generation
- no executable writable memory
- all WASI/libiox crossings must be validated

## Documentation

- `AGENTS.md`
- `BUILD.md`
- `WAMR_INTEGRATION.md`
- `docs/LIBIOX_ARCHITECTURE.md`
- `docs/SYSCALLS.md`
- `docs/PORTING.md`

## Status

- iOS-only architecture established
- libiox/WAMR separation being standardized
- external WAMR build model in progress

---

**Primary Rule**: `deps/wamr/` stays external, and app customers access WAMR only through `iox_wamr_*`.

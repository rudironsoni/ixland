# Agent Guide: libiox

## Overview

**libiox** is an iOS-only Linux syscall compatibility layer. It provides symbol interposition, virtual process abstractions, sandbox-aware filesystem behavior, and an iOS-facing runtime surface for Linux-oriented software.

This repository also integrates **WAMR** for WebAssembly execution, but WAMR is treated as an **external upstream dependency**:

- `deps/wamr/` is read-only upstream source
- do not edit or fork WAMR as part of normal libiox development
- all iOS-specific build/toolchain policy for WAMR is owned outside the submodule
- app customers must use **`iox_wamr_*` only** and must not integrate against raw WAMR APIs directly

## Platform Policy

- **iOS only**
- minimum deployment target: **iOS 16.0+**
- supported architectures: **arm64 device** and **arm64/x86_64 simulator**
- macOS runtime validation is not authoritative for libiox behavior

## Architectural Boundary

### libiox owns

- Linux syscall interposition (`fork`, `execve`, `open`, etc.)
- thread-based process simulation and virtual PIDs
- VFS, sandbox-aware path translation, file descriptors, environment, signals
- network adaptation for iOS
- public C APIs under `include/iox/`
- WAMR adapter layer under `src/iox/wamr/`
- any WASI-to-libiox bridge logic

### WAMR owns

- upstream WebAssembly runtime implementation
- upstream runtime headers and internals
- upstream platform/runtime source under `deps/wamr/`

### Customers may use

- `iox_*` syscall-facing public APIs
- `iox_wamr_*` APIs only

### Customers may not rely on

- raw WAMR APIs
- headers under upstream WAMR internals as public SDK surface
- direct edits to `deps/wamr/`

## Packaging Model

The correct model for this repository is:

- build `libiox` and `libiwasm` as **separate iOS artifacts**
- distribute them together as one iOS SDK bundle
- preserve clean architecture boundaries instead of embedding WAMR into libiox

Recommended outputs:

- `lib/libiox-sim.a`
- `lib/libiox-device.a`
- `build/wamr-simulator/libiwasm.a`
- `build/wamr-device/libiwasm.a`

Long-term distribution target:

- `LibIOX.xcframework`
- `LibIWasm.xcframework`
- one top-level SDK bundle containing both artifacts and libiox public headers

## Naming Convention

### Three-Level Naming

| Level | Pattern | Purpose | Example |
|-------|---------|---------|---------|
| Internal | `__iox_*_impl()` | Implementation details | `__iox_fork_impl()` |
| Public | `iox_*()` | Supported libiox API | `iox_fork()` |
| Interposed | Standard libc/Linux names | Symbol interposition | `fork()` |

### WAMR Naming

| Layer | Pattern | Purpose |
|------|---------|---------|
| Public adapter | `iox_wamr_*` | Only supported WAMR-facing app API |
| Internal bridge | `src/iox/wamr/*` | libiox-owned adapter and bridge logic |
| Upstream runtime | `deps/wamr/*` | Read-only external dependency |

## Directory Structure

```
a-shell-kernel/
├── include/iox/                 # Public libiox headers
│   ├── iox.h
│   ├── iox_syscalls.h
│   ├── iox_wamr.h               # Public WAMR adapter API
│   └── sys/
├── src/iox/
│   ├── core/                    # Syscall implementations
│   ├── interpose/               # Strong-symbol interposition
│   ├── wamr/                    # libiox-owned WAMR adapter layer
│   └── internal/                # Private headers
├── deps/wamr/                   # Upstream WAMR source (do not edit)
├── build/wamr-device/           # Out-of-tree WAMR device build output
├── build/wamr-simulator/        # Out-of-tree WAMR simulator build output
├── lib/                         # libiox build outputs
├── ios-test-app/                # iOS runtime validation app/tests
└── tests/                       # Additional test assets
```

## Build Rules

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

### Build the full SDK artifact set

```bash
make sdk-sim
make sdk-device
make sdk
```

### Important build constraints

- WAMR must be built **out-of-tree**
- libiox must **not** archive `libiwasm.a` into `libiox.a`
- app/test targets must link both libraries explicitly
- libiox compiles against upstream WAMR headers, but app customers use only `iox_wamr_*`

## How To Add New Syscalls

1. Declare in `include/iox/iox_syscalls.h`
2. Implement in `src/iox/core/iox_<category>.c`
3. Add interposition in `src/iox/interpose/iox_interpose.c`
4. Add or update iOS tests
5. Verify simulator and device builds

## How To Extend WAMR Support

1. Keep upstream WAMR untouched in `deps/wamr/`
2. Implement adapter/bridge logic only in `src/iox/wamr/`
3. Expose supported public API only through `include/iox/iox_wamr.h`
4. Build WAMR with out-of-tree scripts owned by this repository
5. Validate that apps can use the new behavior through `iox_wamr_*` only

## Agent Instructions

### Always do

1. Keep `deps/wamr/` pristine
2. Put iOS toolchain/config logic outside WAMR
3. Preserve the boundary between libiox and upstream runtime code
4. Prefer simulator/device validation over macOS-only checks
5. Document customer-facing API limits clearly when changing WAMR integration

### Never do

- do not edit `deps/wamr/` unless the user explicitly asks for upstream patch work
- do not make raw WAMR APIs part of the supported app-facing surface
- do not merge `libiwasm.a` into `libiox.a` as the primary architecture
- do not rely on macOS behavior to validate iOS runtime correctness

## Integration Points

### iOS app consumers

- link `libiox` and `libiwasm` separately
- include `iox/iox.h` and optionally `iox/iox_wamr.h`
- use `iox_wamr_*` only for WASM operations

### libiox test app

- validates runtime behavior on iOS Simulator and device
- should link both libiox and libiwasm explicitly

## Security Constraints

- no real fork/exec
- no setuid
- no sandbox bypass
- no JIT or dynamic code generation
- no executable writable memory
- validate all WASI/libiox crossings

## Documentation

- `README.md`
- `WAMR_INTEGRATION.md`
- `BUILD.md`
- `docs/LIBIOX_ARCHITECTURE.md`
- `docs/SYSCALLS.md`
- `docs/PORTING.md`

---

**Last Updated**: 2026-03-23  
**Status**: iOS-only architecture defined; libiox/WAMR separation in progress  
**Primary Rule**: `deps/wamr/` is external and app customers access WAMR only through `iox_wamr_*`

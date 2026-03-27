# iXland WebAssembly Boundary Specification

## Purpose

This document defines the first concrete public contract for the `ixland-wasm` boundary. It specifies how WebAssembly guests interact with the iXland host environment and how different Wasm engines can be integrated without tight coupling.

## Current Backend Reality

The current WebAssembly backend is **WAMR (WebAssembly Micro Runtime)**.

- WAMR implementation lives inside `ixland-system`
- WAMR is an implementation detail, not a top-level boundary
- This specification is intentionally engine-neutral
- Future backends (e.g., Wasmtime) can implement the same contract

## ixland-wasm-engine

The `ixland-wasm-engine` boundary defines an **engine-neutral runtime contract** for WebAssembly execution backends.

### Goals

- **Backend interchangeability**: Switch between WAMR, Wasmtime, etc. without changing guest code
- **Clean abstraction**: Engine-specific details hidden behind common interface
- **Lifecycle management**: Consistent module/instance creation and destruction

### Key Abstractions

| Type | Purpose |
|------|---------|
| `ixland_wasm_engine_t` | Engine handle (backend instance) |
| `ixland_wasm_module_t` | Compiled WASM module (bytecode) |
| `ixland_wasm_instance_t` | Runtime instance with state |

### Operations

- **Engine lifecycle**: `create`, `destroy`
- **Module operations**: `load`, `unload`, `get_info`
- **Instance operations**: `create`, `destroy`, `run`, `interrupt`
- **Memory access**: `get_ptr`, `size`, `grow`

### Backend Kinds

- `WAMR` - WebAssembly Micro Runtime (current)
- `WASMTIME` - Wasmtime (future)
- `WAMR_AOT` - WAMR AOT compiled (future)

## ixland-wasm-host

The `ixland-wasm-host` boundary defines the **host-service contract** between WebAssembly guests and iXland.

### Service Areas

#### 1. File Descriptor and stdio

File descriptor operations for guest I/O:
- `fd_read`, `fd_write`, `fd_close`
- `fd_seek`, `fd_get_flags`, `fd_set_flags`

All file descriptors are iXland virtual FDs (not host OS FDs).

#### 2. Path and Filesystem

Path-based operations resolved through iXland VFS:
- `path_open`, `path_stat`, `fd_stat`
- `path_mkdir`, `path_unlink`, `path_rmdir`
- `path_readlink`, `path_symlink`, `path_rename`, `path_access`

Paths are resolved through the iXland virtual filesystem, not directly on the host.

#### 3. Clocks

Time operations:
- `clock_gettime` (realtime, monotonic, process, thread)
- `clock_getres`
- `nanosleep`

Clocks are mapped to iXland kernel timekeeping, not host OS clocks directly.

#### 4. Random

Cryptographically secure random number generation:
- `random_get(buf, buflen)`

Uses iXland secure random source.

#### 5. Sockets

Network operations:
- `socket_create`, `socket_connect`, `socket_bind`
- `socket_listen`, `socket_accept`
- `socket_send`, `socket_recv`, `socket_sendto`, `socket_recvfrom`
- `socket_shutdown`

Socket FDs are iXland virtual FDs.

#### 6. Process Exit and Interruption Semantics

Process lifecycle:
- `proc_exit(status)` - Terminate guest
- `proc_is_interrupted()` - Check if guest should stop
- `proc_getpid()`, `proc_getppid()` - Virtual PIDs

Virtual PIDs work the same in WASM guests as in native iXland processes.

## ixland-wasm-wasi

The `ixland-wasm-wasi` boundary defines the **WASI guest ABI policy** - how WASI concepts map onto iXland host semantics.

### WASI Mapping Strategy

| WASI Concept | iXland Mapping |
|--------------|----------------|
| File descriptors | iXland virtual FDs |
| Paths | iXland VFS paths |
| Clocks | iXland kernel clocks |
| Random | iXland secure random |
| Sockets | iXland socket layer |
| Process exit | iXland process exit |

### WASI Versions

- Preview 0 (legacy snapshot)
- Preview 1 (current stable)
- Preview 2 (future components)

### Capability Model

WASI rights map to iXland FD rights:
- `RIGHT_FD_READ` → iXland FD read permission
- `RIGHT_FD_WRITE` → iXland FD write permission
- `RIGHT_PATH_OPEN` → iXland VFS open permission

### Preopens

WASI preopened directories map to iXland bind mounts:
- `"/"` → iXland VFS root
- `"."` → Current working directory
- `"/tmp"` → Temp directory

## Initial Public Header Surface

The public contract is defined in these headers:

| Header | Path | Purpose |
|--------|------|---------|
| `types.h` | `ixland-wasm/include/ixland/wasm/types.h` | Fundamental types, opaque handles |
| `engine.h` | `ixland-wasm/include/ixland/wasm/engine.h` | Engine-neutral runtime contract |
| `host.h` | `ixland-wasm/include/ixland/wasm/host.h` | Host-service areas (vtable) |
| `wasi.h` | `ixland-wasm/include/ixland/wasm/wasi.h` | WASI policy and mapping |

## What This Spec Does Not Do Yet

Explicitly **not included** in this phase:

1. **No runtime abstraction implementation** - Only headers/spec, no working backend abstraction
2. **No Wasmtime backend** - WAMR is the only current backend
3. **No artifact/AOT layer** - Compiled WASM artifacts are a `ixland-packages` concern
4. **No implementation extraction** - WAMR code remains in `ixland-system`
5. **No Component Model** - Preview 2 components are future work

This specification establishes the **contract boundary**. Implementation extraction follows later.

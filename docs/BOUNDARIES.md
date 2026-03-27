# iXland Boundary Definitions

This document defines what belongs in each top-level component boundary.

## ixland-system

**Owns**: Current kernel/subsystem/runtime implementation

- Virtual kernel subsystems (task, signal, exec, time, resource)
- Filesystem implementations (VFS, proc, dev, pipe)
- TTY/PTY drivers
- Runtime backends (native command registry, WASI bridge, script interpreter)
- Platform compatibility and interposition layers

**Role**: Remains source of truth for current implementation until intentional extraction. Should not absorb new top-level boundary concerns indefinitely.

## ixland-libc

**Owns**: Future public user-facing libc/ABI material

- POSIX compatibility headers
- System call interface definitions (public)
- User-facing libc implementations
- ABI-facing boundary material

**First Extraction Target**: Headers and ABI-facing material, not runtime policy.

**Not Yet**: Runtime and kernel policy stays in `ixland-system` for now.

## ixland-wasm-engine

**Owns**: Future engine-neutral contract for WebAssembly runtime backends

- Engine-neutral backend interface
- Abstractions that allow switching between WAMR, Wasmtime, etc.

**Current State**: No concrete backend code belongs here yet. Current backend (WAMR) is a detail of `ixland-system`.

## ixland-wasm-host

**Owns**: Future host-service contract from guest runtimes into iXland semantics

Service areas:
- File descriptor I/O
- Path and filesystem operations
- Clocks and timers
- Random number generation
- Sockets and networking
- Process lifecycle (exit, interruption semantics)

## ixland-wasm-wasi

**Owns**: Future guest ABI policy for WASI

- WASI syscall mappings to iXland host semantics
- WASI conformance testing
- WASI-specific adaptations

**Not**: A separate top-level repository. Contained within `ixland-wasm/`.

## ixland-packages

**Owns**: Package definitions, build, and distribution

- Package build recipes and scripts
- Package metadata and manifests
- Distribution and registry concerns
- Future compiled Wasm artifact management

## ixland-toolchain

**Owns**: Future toolchain integration

- Cross-compilation toolchain definitions
- Build system integration helpers
- Development environment setup

**Does Not Replace**: CMake (build orchestration) or `.github/workflows` (CI).

---

## Rules for Extraction

When extracting code from `ixland-system` to other boundaries:

1. **Narrow extractions only** - Extract specific, well-defined boundaries, not large subsystems
2. **Public boundary first** - Extract headers and interface definitions before implementation
3. **No speculative large moves** - Don't move code "just in case" it might be useful elsewhere
4. **Docs/specs before large code moves** - When abstractions are still unsettled, write the contract first

These rules prevent premature abstraction and maintain repository coherence during incremental refactoring.

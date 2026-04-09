# Header Extraction Backlog - Wave 3

> **Epic:** Epic 5 - Header Organization & API Boundaries
> **Status:** Planning Phase
> **Last Updated:** 2026-03-30

## Overview

This document tracks the backlog of header files that need classification and potential extraction as part of the Epic 5 header organization effort. The goal is to establish clear boundaries between:

- **ixland-libc**: Public libc APIs (portable, stable)
- **ixland-system**: System-level APIs (kernel, runtime, vfs)
- **Internal**: Private implementation headers

---

## Current Inventory

### ixland-system/include/ (Public System Headers)

| Header | Description | Current Status |
|--------|-------------|----------------|
| `vfs.h` | Legacy path translation VFS (simple API) | **Deprecated** - Superceded by fs/vfs.h |

**Note:** The `vfs.h` in `include/` is the old path translation layer. The full VFS implementation lives in `fs/vfs.h`.

### ixland-system/ Root Level Headers

| Header | Description | Classification Needed |
|--------|-------------|----------------------|
| `ixland_error.h` | Error handling, stdio redirection, thread-local streams | **TBD** - Core system API |

### ixland-system/fs/ (Filesystem Layer)

| Header | Description | Classification Needed |
|--------|-------------|----------------------|
| `fs/vfs.h` | Full VFS implementation (vnode, mount, operations) | **keep-public** |
| `fs/fdtable.h` | File descriptor table management | **move-internal** |

### ixland-system/kernel/ (Kernel Layer)

| Header | Description | Classification Needed |
|--------|-------------|----------------------|
| `kernel/internal/ixland_kernel.h` | Kernel internal structures | **move-internal** |
| `kernel/exec/exec.h` | Process execution internals | **move-internal** |
| `kernel/task/task.h` | Task/thread management | **move-internal** |
| `kernel/signal/ixland_signal.h` | Signal handling internals | **move-internal** |

### ixland-system/runtime/ (Runtime Layer)

| Header | Description | Classification Needed |
|--------|-------------|----------------------|
| `runtime/native/registry.h` | Native command registry | **keep-public** |
| `runtime/wasi/ixland_wamr.h` | WAMR WASM runtime integration | **move-internal** |
| `runtime/wasi/wasm_adapter.h` | WASI adapter layer | **move-internal** |

### ixland-system/src/ (Implementation Layer)

| Header | Description | Classification Needed |
|--------|-------------|----------------------|
| `src/ixland/internal/ixland_internal.h` | Internal ixland system header | **move-internal** |

---

## Classification Categories

### move-to-libc

Headers that should move to `ixland-libc/include/`:

- Standard C library extensions
- POSIX compatibility headers
- Public, stable APIs with ixland_ prefixes
- Database APIs (pwd, grp - already done)

**Candidates:**
- None currently - most libc-extraction work completed in Waves 1-2

### keep-public

Headers that should stay public in `ixland-system/include/`:

- System-level APIs used by external commands
- VFS public interface
- Runtime registry APIs
- iXland-specific system calls

**Current:**
- `ixland_error.h` - Keep public (core system interface)
- `fs/vfs.h` - Move to include/ and keep public
- `runtime/native/registry.h` - Keep public

### move-internal

Headers that should become internal (move to `src/` or mark as private):

- Implementation details not for external consumption
- Kernel internals
- Data structure definitions that change frequently
- Headers with unstable APIs

**Candidates:**
- `fs/fdtable.h` - Move to internal/
- `kernel/internal/ixland_kernel.h` - Already internal
- `kernel/exec/exec.h` - Move to kernel/internal/
- `kernel/task/task.h` - Move to kernel/internal/
- `kernel/signal/ixland_signal.h` - Move to kernel/internal/
- `runtime/wasi/*.h` - Already effectively internal
- `src/ixland/internal/ixland_internal.h` - Already internal

### unclear

Headers needing design decision:

| Header | Question | Context |
|--------|----------|---------|
| `include/vfs.h` (old) | Remove or deprecate? | Superceded by `fs/vfs.h` |
| `ixland_error.h` | Keep in system/ or move to libc? | Defines thread_stdout, etc. |

---

## Already Extracted (Wave 1-2)

### Wave 1: Core Database Headers

Extracted to `ixland-libc/include/`:

| Header | Description | Status |
|--------|-------------|--------|
| `grp.h` | Group database operations | ✅ Complete |
| `pwd.h` | User password database | ✅ Complete |

### Wave 2: ixland Umbrella Headers

Extracted to `ixland-libc/include/ixland/`:

| Header | Description | Status |
|--------|-------------|--------|
| `ixland/ixland.h` | Master umbrella header | ✅ Complete |
| `ixland/ixland_types.h` | Public type definitions | ✅ Complete |
| `ixland/ixland_syscalls.h` | Syscall declarations | ✅ Complete |
| `ixland/sys/types.h` | System types wrapper | ✅ Complete |

### Wave 2: Linux-Compatible Headers

Extracted to `ixland-libc/include/linux/`:

| Header | Description | Status |
|--------|-------------|--------|
| `linux/unistd.h` | Process and I/O syscalls | ✅ Complete |
| `linux/signal.h` | Signal handling | ✅ Complete |
| `linux/wait.h` | Process waiting | ✅ Complete |
| `linux/stat.h` | File status | ✅ Complete |
| `linux/fcntl.h` | File control | ✅ Complete |
| `linux/time.h` | Time functions | ✅ Complete |
| `linux/poll.h` | Polling | ✅ Complete |
| `linux/epoll.h` | Epoll interface | ✅ Complete |
| `linux/resource.h` | Resource limits | ✅ Complete |
| `linux/mman.h` | Memory management | ✅ Complete |
| `linux/types.h` | Linux kernel types | ✅ Complete |

---

## Top 3 Extraction Candidates

### 1. Consolidate VFS Headers

**Priority:** Medium
**Complexity:** Medium
**Action:** Merge old `include/vfs.h` with `fs/vfs.h`

**Details:**
- The old `vfs.h` in `include/` is a minimal path translation API
- `fs/vfs.h` contains the full VFS implementation
- Decision needed: Remove old API or maintain backward compatibility

**Tasks:**
- [ ] Audit all consumers of old `include/vfs.h`
- [ ] Decide on backward compatibility strategy
- [ ] Either delete old header or provide compatibility shim
- [ ] Move `fs/vfs.h` to `include/ixland_vfs.h` or keep in fs/

### 2. Classify ixland_error.h

**Priority:** High
**Complexity:** Low
**Action:** Decide on final location for ixland_error.h

**Details:**
- Contains thread-local stdio redirection (thread_stdout, etc.)
- Defines ixland_* wrappers for system calls
- Used by both system and app layers

**Decision Options:**
- **Option A:** Keep in `ixland-system/` (system-level API)
- **Option B:** Move to `ixland-libc/include/ixland/` (public libc API)
- **Option C:** Split into public API + internal implementation

**Recommendation:** Option A - keep as system-level API since it's tightly coupled with the runtime implementation.

### 3. Internalize Kernel Headers

**Priority:** Medium
**Complexity:** Low
**Action:** Move kernel internals to `kernel/internal/`

**Details:**
Several kernel headers are implementation details that should not be public:

| Header | Current Location | Target Location |
|--------|-----------------|-----------------|
| `exec.h` | `kernel/exec/` | `kernel/internal/` |
| `task.h` | `kernel/task/` | `kernel/internal/` |
| `ixland_signal.h` | `kernel/signal/` | `kernel/internal/` |

**Tasks:**
- [ ] Move headers to internal/ subdirectories
- [ ] Update all #include references
- [ ] Update CMakeLists.txt if needed

---

## Proposed Directory Structure (Target)

```
ixland-system/
├── include/
│   ├── ixland_error.h          # Keep: System error/stdio API
│   └── ixland_vfs.h               # Rename from fs/vfs.h
│
├── kernel/
│   ├── internal/               # Private kernel headers
│   │   ├── ixland_kernel.h
│   │   ├── exec.h
│   │   ├── task.h
│   │   └── signal.h
│   └── ... (implementation files)
│
├── fs/
│   ├── vfs.c                   # Implementation
│   └── internal/
│       └── fdtable.h           # Private fdtable API
│
├── runtime/
│   ├── native/
│   │   ├── registry.h          # Public: Native command registry
│   │   └── registry.c
│   └── wasi/
│       └── internal/           # Private WASI headers
│           ├── ixland_wamr.h
│           └── wasm_adapter.h
│
└── src/
    └── internal/
        └── ixland_internal.h      # Already internal
```

---

## Next Steps

1. **Immediate (Wave 3A):**
   - [ ] Make decision on old `include/vfs.h`
   - [ ] Finalize location for `ixland_error.h`
   - [ ] Create `kernel/internal/` and move private headers

2. **Short-term (Wave 3B):**
   - [ ] Move `fs/vfs.h` to `include/ixland_vfs.h`
   - [ ] Move `fs/fdtable.h` to `fs/internal/`
   - [ ] Move WASI headers to `runtime/wasi/internal/`

3. **Verification:**
   - [ ] Run full build after each move
   - [ ] Update any hardcoded paths in CMakeLists.txt
   - [ ] Document public API changes

---

## Notes

- **termcap.h / ncurses_dll.h**: These were mentioned in Epic 5 planning but do not currently exist in the codebase. They may be future additions or were removed in prior refactoring.
- **ixland-libc** is now the canonical location for all public libc-style headers
- **ixland-system** should focus on system-level APIs (VFS, kernel, runtime)
- Internal headers should never be included by external command packages

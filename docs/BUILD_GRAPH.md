# iXland Build Graph

## Purpose

This document describes the actual monorepo build graph, including how components depend on each other and how boundary targets are consumed.

## Current Root Build

The root `CMakeLists.txt` is the monorepo entry point.

### Optional Components

| Option | Target | Status |
|--------|--------|--------|
| `IXLAND_BUILD_LIBC` | `ixland-libc` | ✅ Active |
| `IXLAND_BUILD_SYSTEM` | `ixland-system` | ✅ Active |
| `IXLAND_BUILD_WASM` | `ixland-wasm` | ✅ Active |
| `IXLAND_BUILD_PACKAGES` | `ixland-packages` | ⏸️ Not yet real CMake target |
| `IXLAND_BUILD_TESTS` | (various) | ✅ Optional test builds |

### Component Include Order

The root build includes subdirectories in dependency order:

1. `ixland-libc` - Headers-only, no dependencies
2. `ixland-system` - Consumes `ixland-libc-headers`
3. `ixland-wasm` - Headers-only, no dependencies

`ixland-packages` provides a validation target (`ixland-packages-validate`). `ixland-toolchain` is referenced via `CMAKE_TOOLCHAIN_FILE`, not via `add_subdirectory`.

## Boundary Targets

### ixland-libc-headers

- **Type**: `INTERFACE` library
- **Provides**: Public `iox` headers at `ixland-libc/include/iox/` and POSIX headers at `ixland-libc/include/`
- **Consumers**: `ixland-system/iox-core`, `ixland-libc-core`
- **Usage**:
  ```cmake
  target_link_libraries(my_target PUBLIC ixland-libc-headers)
  ```
- **Headers**: `iox/*.h`, `grp.h`, `pwd.h`

## ixland-libc-core

- **Type**: `STATIC` library
- **Provides**: Self-contained libc-facing utilities (`iox_version`, `iox_strerror`, `iox_perror`)
- **Consumers**: None yet (ixland-system can consume once fully extracted)
- **Usage**:
  ```cmake
  target_link_libraries(my_target PUBLIC ixland-libc-core)
  ```
- **Note**: Depends on `ixland-libc-headers` for public API definitions

### ixland-wasm-contracts

- **Type**: `INTERFACE` library
- **Provides**: Wasm contract headers at `ixland-wasm/include/`
- **Consumers**: `iox-runtime-wasi` (internal target within ixland-system)
- **Usage**:
  ```cmake
  target_link_libraries(my_target PUBLIC ixland-wasm-contracts)
  ```
- **Status**: ✅ Consumed by `iox-runtime-wasi` target in ixland-system

## ixland-system as Consumer

`ixland-system` consumes boundary targets cleanly:

### Current Consumption

- `ixland-libc-headers` → `iox-core` (via `target_link_libraries`)
- `ixland-libc-headers` → `iox-runtime` (via `target_link_libraries`)
- `ixland-wasm-contracts` → `iox-runtime` (via `target_link_libraries`)
- `iox-runtime` → `iox-core` (runtime layer consumed by core)

### Standalone vs Subproject

`ixland-system/CMakeLists.txt` supports both modes:

| Mode | Detection | Behavior |
|------|-----------|----------|
| Standalone | `CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR` | Sets iOS, defines project, finds headers by source tree |
| Subproject | Otherwise | Uses parent project settings, consumes targets via linkage |

## Build Commands

### Root Configure (Monorepo)

```bash
mkdir build && cd build
cmake .. -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator \
         -DCMAKE_OSX_ARCHITECTURES=arm64
```

### Root Build

```bash
cmake --build build
```

### Standalone ixland-system

```bash
cd ixland-system
mkdir build && cd build
cmake ..  # iOS settings applied automatically
cmake --build .
```

## Target Dependencies

```
ixland-libc-headers (INTERFACE)
    │
    ├─ PUBLIC include → iox-core
    └─ PUBLIC include → iox-runtime-wasi

ixland-wasm-contracts (INTERFACE)
    │
    └─ PUBLIC include → iox-runtime-wasi

iox-runtime-wasi (STATIC) - INTERNAL to ixland-system
    │
    ├─ PRIVATE sources: runtime/wasi/, src/iox/wamr/
    ├─ PUBLIC includes: runtime/wasi/, src/iox/wamr/, runtime/native/
    ├─ PUBLIC link: ixland-libc-headers (boundary consumption)
    ├─ PUBLIC link: ixland-wasm-contracts (boundary consumption)
    └─ Note: Internal target, not exposed outside ixland-system

ixland-wasm-contracts (INTERFACE)
    │
    └─ PUBLIC include → iox-runtime

iox-runtime (STATIC)
    │
    ├─ PRIVATE sources: runtime/wasi/, runtime/native/
    ├─ PUBLIC link: ixland-wasm-contracts
    └─ PUBLIC link: ixland-libc-headers

iox-core (STATIC)
    │
    ├─ PRIVATE sources: kernel/, fs/, runtime/native/
    ├─ PUBLIC includes: internal/, kernel/, fs/, runtime/native/, drivers/tty/
    ├─ PUBLIC link: ixland-libc-headers (boundary consumption)
    ├─ PUBLIC link: pthread
    └─ PUBLIC link: iox-runtime-wasi (internal dependency)

iox-core-tests (MODULE)
    │
    ├─ PRIVATE sources: Tests/
    └─ PRIVATE link: iox-core (inherits includes), XCTest
```

## Internal Target Architecture

### iox-runtime-wasi (Internal)

The `iox-runtime-wasi` target is an **internal** target within `ixland-system` that separates the WASI/WebAssembly runtime implementation from the core kernel code.

**Rationale:**
- Prevents monolithic build where everything is in one giant target
- Keeps WASI-specific code (WAMR integration, wasm_adapter) isolated
- Allows clean dependency on `ixland-wasm-contracts` boundary target
- Internal to ixland-system - consumers only link against `iox-core`

**Sources:**
- `runtime/wasi/wasm_adapter.c` - Wasm contract adapter
- `src/iox/wamr/iox_wamr.c` - WAMR integration
- `src/iox/wamr/iox_wamr_simple.c` - Simplified WAMR interface

**Dependencies:**
- `ixland-libc-headers` - For iox types
- `ixland-wasm-contracts` - For wasm contract types

**Consumers:**
- `iox-core` - Links publicly, so runtime is available to all iox-core consumers

## Current Limitations

1. **Implementation still mostly lives in ixland-system**
   - No extracted runtime/backend implementation targets yet
   - WAMR is still directly integrated, not behind contract

2. **ixland-packages is not a real CMake participant**
   - Package build system is shell-based
   - Future: CMake package definitions

3. **ixland-toolchain is skeletal**
   - No real toolchain CMake integration yet

4. **Root iOS build is evolving**
   - Simulator builds work
   - Device builds need signing setup

5. **ixland-wasm-contracts not yet consumed**
   - Contract headers exist
   - Runtime still uses WAMR directly
   - Future: Refactor runtime/wasi to use contract

## Next Build Steps

### Immediate

- Real configure validation in CI (done in this PR)
- Ensure standalone and subproject builds both work

### Near-term

- Additional boundary consumers (more header extractions)
- Begin extracted implementation targets (e.g., `ixland-libc` getting `.c` files)
- Runtime using Wasm contract headers

### Long-term

- Full monorepo build graph with all components
- CI running real builds, not just configure checks
- Extracted backend implementations behind `ixland-wasm-contracts`

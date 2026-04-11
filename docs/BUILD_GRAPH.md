# HISTORICAL: iXland Build Graph (Pre-Coherence)

**Status**: HISTORICAL DOCUMENT - Describes pre-coherence CMake-based build system

> **WARNING**: This document describes a historical CMake-based build system that is no longer active. The current build truth is `IXLand/IXLand.xcodeproj` with Xcode-only builds. See `AGENTS.md` for current build instructions.

## Purpose

This document describes the historical monorepo build graph using CMake, including how components depended on each other and how boundary targets were consumed. This is preserved for historical reference only.

## Historical Root Build

The root `CMakeLists.txt` was the monorepo entry point (now removed).

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

## Internal Targets (ixland-system)

### ixland-runtime-native (Internal)

- **Type**: `STATIC` library
- **Provides**: Native command registry for registering and looking up native commands
- **Consumers**: `ixland-core`
- **Usage**:
  ```cmake
  target_link_libraries(my_target PUBLIC ixland-runtime-native)
  ```
- **Sources**: `runtime/native/registry.c`
- **Dependencies**: `ixland-libc-headers`

### ixland-runtime-wasi (Internal)

- **Type**: `STATIC` library
- **Provides**: WASI/WebAssembly runtime adapter implementation
- **Consumers**: `ixland-core`
- **Usage**:
  ```cmake
  target_link_libraries(my_target PUBLIC ixland-runtime-wasi)
  ```
- **Sources**: `runtime/wasi/wasm_adapter.c`, `src/ixland/wamr/ixland_wamr.c`, `src/ixland/wamr/ixland_wamr_simple.c`
- **Dependencies**: `ixland-libc-headers`, `ixland-wasm-contracts`

## Boundary Targets

### ixland-libc-headers

- **Type**: `INTERFACE` library
- **Provides**: Public `ixland` headers at `ixland-libc/include/ixland/` and POSIX headers at `ixland-libc/include/`
- **Consumers**: `ixland-system/ixland-core`, `ixland-libc-core`
- **Usage**:
  ```cmake
  target_link_libraries(my_target PUBLIC ixland-libc-headers)
  ```
- **Headers**: `ixland/*.h`, `grp.h`, `pwd.h`

## ixland-libc-core

- **Type**: `STATIC` library
- **Provides**: Self-contained libc-facing utilities (`ixland_version`, `ixland_strerror`, `ixland_perror`)
- **Consumers**: None yet (ixland-system can consume once fully extracted)
- **Usage**:
  ```cmake
  target_link_libraries(my_target PUBLIC ixland-libc-core)
  ```
- **Note**: Depends on `ixland-libc-headers` for public API definitions

### ixland-wasm-contracts

- **Type**: `INTERFACE` library
- **Provides**: Wasm contract headers at `ixland-wasm/include/`
- **Consumers**: `ixland-runtime-wasi` (internal target within ixland-system)
- **Usage**:
  ```cmake
  target_link_libraries(my_target PUBLIC ixland-wasm-contracts)
  ```
- **Status**: ✅ Consumed by `ixland-runtime-wasi` target in ixland-system

## ixland-system as Consumer

`ixland-system` consumes boundary targets cleanly:

### Current Consumption

- `ixland-libc-headers` → `ixland-core` (via `target_link_libraries`)
- `ixland-libc-headers` → `ixland-runtime-native` (via `target_link_libraries`)
- `ixland-libc-headers` → `ixland-runtime-wasi` (via `target_link_libraries`)
- `ixland-wasm-contracts` → `ixland-runtime-wasi` (via `target_link_libraries`)
- `ixland-runtime-native` → `ixland-core` (native runtime layer consumed by core)
- `ixland-runtime-wasi` → `ixland-core` (WASI runtime layer consumed by core)

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
    ├─ PUBLIC include → ixland-core
    ├─ PUBLIC include → ixland-runtime-native
    └─ PUBLIC include → ixland-runtime-wasi

ixland-wasm-contracts (INTERFACE)
    │
    └─ PUBLIC include → ixland-runtime-wasi

ixland-runtime-native (STATIC) - INTERNAL to ixland-system
    │
    ├─ PRIVATE sources: runtime/native/
    ├─ PUBLIC includes: runtime/native/, kernel/task/
    ├─ PUBLIC link: ixland-libc-headers (boundary consumption)
    └─ Note: Internal target, not exposed outside ixland-system

ixland-runtime-wasi (STATIC) - INTERNAL to ixland-system
    │
    ├─ PRIVATE sources: runtime/wasi/, src/ixland/wamr/
    ├─ PUBLIC includes: runtime/wasi/, src/ixland/wamr/, runtime/native/
    ├─ PUBLIC link: ixland-libc-headers (boundary consumption)
    ├─ PUBLIC link: ixland-wasm-contracts (boundary consumption)
    └─ Note: Internal target, not exposed outside ixland-system

ixland-core (STATIC)
    │
    ├─ PRIVATE sources: kernel/, fs/
    ├─ PUBLIC includes: internal/, kernel/, fs/, runtime/native/, drivers/tty/
    ├─ PUBLIC link: ixland-libc-headers (boundary consumption)
    ├─ PUBLIC link: pthread
    ├─ PUBLIC link: ixland-runtime-native (internal dependency)
    └─ PUBLIC link: ixland-runtime-wasi (internal dependency)

ixland-core-tests (MODULE)
    │
    ├─ PRIVATE sources: Tests/
    └─ PRIVATE link: ixland-core (inherits includes), XCTest
```

## Internal Target Architecture

### ixland-runtime-native (Internal)

The `ixland-runtime-native` target is an **internal** target within `ixland-system` that separates the native command registry from the core kernel code.

**Rationale:**
- Prevents monolithic build where everything is in one giant target
- Keeps native runtime code (command registry) isolated
- Allows clean dependency on `ixland-libc-headers` boundary target
- Internal to ixland-system - consumers only link against `ixland-core`

**Sources:**
- `runtime/native/registry.c` - Native command registry implementation

**Dependencies:**
- `ixland-libc-headers` - For ixland types

**Consumers:**
- `ixland-core` - Links publicly, so native runtime is available to all ixland-core consumers

### ixland-runtime-wasi (Internal)

The `ixland-runtime-wasi` target is an **internal** target within `ixland-system` that separates the WASI/WebAssembly runtime implementation from the core kernel code.

**Rationale:**
- Prevents monolithic build where everything is in one giant target
- Keeps WASI-specific code (WAMR integration, wasm_adapter) isolated
- Allows clean dependency on `ixland-wasm-contracts` boundary target
- Internal to ixland-system - consumers only link against `ixland-core`

**Sources:**
- `runtime/wasi/wasm_adapter.c` - Wasm contract adapter
- `src/ixland/wamr/ixland_wamr.c` - WAMR integration
- `src/ixland/wamr/ixland_wamr_simple.c` - Simplified WAMR interface

**Dependencies:**
- `ixland-libc-headers` - For ixland types
- `ixland-wasm-contracts` - For wasm contract types

**Consumers:**
- `ixland-core` - Links publicly, so runtime is available to all ixland-core consumers

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

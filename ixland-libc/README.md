# ixland-libc

C library and ABI boundary for iXland.

## Status

**Authoritative boundary for public iox headers and self-contained libc utilities.**

This component owns:
- Public libc/ABI header boundary
- Self-contained libc-facing utility implementations

## Contents

### Headers
- `include/iox/iox.h` - Master umbrella header
- `include/iox/iox_types.h` - Public type definitions
- `include/iox/iox_syscalls.h` - Public syscall API
- `include/iox/sys/types.h` - Linux-compatible types
- `include/grp.h` - POSIX group database operations
- `include/pwd.h` - POSIX user database operations

### Implementation
- `src/iox_version.c` - Version and error string utilities
  - `iox_version()` - Library version string
  - `iox_strerror()` - Error code to string conversion
  - `iox_perror()` - Print error message

## What Stays in ixland-system

- Runtime and kernel policy
- Implementation of syscall functions (non-trivial, requires kernel/VFS)
- Internal headers (non-public)
- WAMR-specific headers

## Build Integration

This component provides:
- `ixland-libc-headers` - INTERFACE target for public headers only
- `ixland-libc-core` - STATIC library with self-contained utilities
- `ixland-libc-usersdb` - STATIC library with user/group database stubs

Include this component to get access to public iox headers and utilities.

## Build Modes

### Standalone Build

When built independently, ixland-libc automatically configures iOS settings:

```bash
cd ixland-libc
cmake -B build .
cmake --build build
```

In standalone mode, the following iOS settings are automatically configured:
- `CMAKE_SYSTEM_NAME=iOS`
- `CMAKE_OSX_DEPLOYMENT_TARGET=16.0`
- `CMAKE_OSX_SYSROOT=iphonesimulator`
- `CMAKE_OSX_ARCHITECTURES=arm64`

### Monorepo Build

When built as part of the ixland monorepo, ixland-libc uses the parent project's settings:

```bash
cd /path/to/ixland
cmake -B build .
cmake --build build --target ixland-libc-core ixland-libc-usersdb
```

Both build modes produce identical targets (`libixland-libc-core.a`, `libixland-libc-usersdb.a`).

## Local Testing

Build and run the smoke tests:

```bash
mkdir build && cd build
cmake .. -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator \
         -DCMAKE_OSX_ARCHITECTURES=arm64 \
         -DIXLAND_LIBC_BUILD_TESTS=ON
cmake --build . --target ixland-libc-test
```

The test executable is built as `ixland-libc-test.app/ixland-libc-test`.

## Future Work

- Additional self-contained libc utility implementations
- Complete syscall implementation extraction (where feasible)
- POSIX compatibility layer implementation
- User-facing libc implementations

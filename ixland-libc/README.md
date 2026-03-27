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

Include this component to get access to public iox headers and utilities.

## Future Work

- Additional self-contained libc utility implementations
- Complete syscall implementation extraction (where feasible)
- POSIX compatibility layer implementation
- User-facing libc implementations

# IXLandLibC

C library and ABI boundary for IXLand.

## Status

**Authoritative boundary for public IXLand headers and self-contained libc utilities.**

This component owns:
- Public libc/ABI header boundary
- Self-contained libc-facing utility implementations

## Contents

### Headers
- `include/ixland/ixland.h` - Master umbrella header
- `include/ixland/ixland_types.h` - Public type definitions
- `include/ixland/ixland_syscalls.h` - Public syscall API
- `include/ixland/sys/types.h` - Linux-compatible types
- `include/grp.h` - POSIX group database operations
- `include/pwd.h` - POSIX user database operations

### Implementation
- `src/ixland_version.c` - Version and error string utilities
  - `ixland_version()` - Library version string
  - `ixland_strerror()` - Error code to string conversion
  - `ixland_perror()` - Print error message

## What Stays in IXLandSystem

- Runtime and kernel policy
- Implementation of syscall functions (non-trivial, requires kernel/VFS)
- Internal headers (non-public)
- WAMR-specific headers

## Build Integration

This component provides public headers and self-contained utilities for use by the IXLand app and other components.

Include this component to get access to public IXLand headers and utilities.

## Local Testing

Build and run the smoke tests through the Xcode project.

## Future Work

- Additional self-contained libc utility implementations
- Complete syscall implementation extraction (where feasible)
- POSIX compatibility layer implementation
- User-facing libc implementations

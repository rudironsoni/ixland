# ixland-libc

C library and ABI boundary for iXland.

## Status

**Authoritative boundary for public iox headers.**

This component now owns the public libc/ABI header boundary.

## Contents

- `include/iox/iox.h` - Master umbrella header
- `include/iox/iox_types.h` - Public type definitions
- `include/iox/iox_syscalls.h` - Public syscall API
- `include/iox/sys/types.h` - Linux-compatible types

## What Stays in ixland-system

- Runtime and kernel policy
- Implementation of syscall functions
- Internal headers (non-public)
- WAMR-specific headers

## Build Integration

This component provides:
- `ixland-libc-headers` - INTERFACE target for public headers

Include this component to get access to public iox headers.

## Future Work

- Complete syscall implementation extraction
- POSIX compatibility layer implementation
- User-facing libc implementations

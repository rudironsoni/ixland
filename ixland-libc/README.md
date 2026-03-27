# ixland-libc

C library and ABI boundary for iXland.

## Status

**Not yet fully extracted.** This directory is the emerging boundary for public libc/ABI material.

## First Extraction Target

The first extraction from `ixland-system` will include:
- Public headers and ABI-facing material
- POSIX syscall interface definitions
- User-facing libc headers

## What Stays in ixland-system

Runtime and kernel policy remains in `ixland-system` for now. Extraction is narrow and incremental.

## Future Contents

When extraction is complete, this boundary will contain:
- POSIX compatibility layer
- System call interface definitions
- User-facing libc headers and implementations

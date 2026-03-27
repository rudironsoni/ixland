# ixland-libc

C library and ABI boundary for iXland.

## Status

**Emerging boundary.** Extraction has begun with public headers.

## Extracted

- `include/iox/iox_types.h` - Public type definitions

## What Stays in ixland-system

- Runtime and kernel policy
- Implementation of syscall functions
- Internal headers

Extraction is narrow and incremental. Headers move first; implementations follow when interfaces stabilize.

## Future Contents

When extraction is complete, this boundary will contain:
- POSIX compatibility layer
- System call interface definitions (headers complete, implementations migrating)
- User-facing libc headers and implementations

## Header Migration Pattern

Headers are being moved from `ixland-system/include/iox/` to `ixland-libc/include/iox/`. During transition:
1. Copy header to `ixland-libc/include/iox/`
2. Update header comments to reflect new ownership
3. Replace original with symlink pointing to new location
4. Update `ixland-system/CMakeLists.txt` include paths if needed

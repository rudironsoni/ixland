# ixland-libc

This directory is reserved for the future libc and ABI split out of `ixland-system`.

No deep split is performed in this migration. The kernel code remains in `ixland-system/` for now.

This boundary will eventually contain:
- POSIX compatibility layer
- System call interface definitions
- User-facing libc headers and implementations

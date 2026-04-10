# Linux-shaped reclassification

## Scope

This document records canonical ownership reclassification for `ixland-system` to enforce one semantic owner per subsystem and remove split ownership between `fs/*` and `src/ixland/fs/*`.

## Reclassification table

| Area | Previous owner(s) | Canonical owner | Decision |
|---|---|---|---|
| File syscall surface (open/read/write/stat/fcntl/ioctl/select/epoll/namei/readdir) | `ixland-system/src/ixland/fs/*` | `ixland-system/fs/*` | Move syscall-family owners to `fs/*`; remove split-brain ownership in `src/ixland/fs/*`. |
| FD syscall backing table for syscall-family wrappers | `ixland-system/src/ixland/fs/fdtable.c` | `ixland-system/fs/fdtable.c` | Merge syscall helper layer into canonical `fs/fdtable.c`; delete duplicate canonical owner in `src/ixland/fs/`. |
| Task-owned fd graph APIs (`ixland_files_t`, `ixland_file_t`) | `ixland-system/fs/fdtable.c` | `ixland-system/fs/fdtable.c` | Preserve existing kernel/task owner and extend in place where required by syscall wrappers. |
| Directory path/entry syscall implementation (`namei` + placeholder `readdir`) | `ixland-system/src/ixland/fs/namei.c`, placeholder `src/ixland/fs/readdir.c` | `ixland-system/fs/namei.c`, `ixland-system/fs/readdir.c` | Promote `namei` to canonical owner under `fs/*`; replace placeholder with real canonical `fs/readdir.c`. |
| Process identity API (`ixland_getuid/geteuid/getgid/getegid/setuid/setgid`) | `ixland-system/src/ixland/core/ixland_identity.c` | `ixland-system/kernel/task/ixland_identity.c` | Drain canonical ownership from `src/ixland/core/*` and colocate identity with task ownership. |

## Explicit contract decisions

### Path-based contract

Path-based filesystem syscalls must be canonicalized through VFS-backed translation paths (`ixland_vfs_*` and `ixland_vfs_translate`) with no host-first fallback roulette.

### FD-based contract

FD-based syscalls must operate through one canonical fd-entry owner in `ixland-system/fs/fdtable.c` for syscall wrappers, while preserving task-owned `ixland_files_t` APIs used by kernel/task.

## Prohibitions enforced by this reclassification

- No duplicate canonical owner for the same syscall family across `ixland-system/fs/*` and `ixland-system/src/ixland/fs/*`.
- No placeholder canonical owner for `readdir`.
- No canonical syscall ownership retained in `ixland-system/src/ixland/core/*` when equivalent subsystem ownership exists under kernel/fs.
- No compatibility shims, aliases, or forwarding wrappers introduced to preserve old paths.

## Expected post-cutover shape

- Canonical fs syscall family ownership: `ixland-system/fs/*`
- Canonical process identity ownership: `ixland-system/kernel/task/*`
- `ixland-system/src/ixland/fs/*`: removed from canonical build ownership
- `ixland-system/src/ixland/core/*`: drained of canonical syscall ownership in this tranche

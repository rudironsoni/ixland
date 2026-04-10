# Linux-Shaped Reclassification - Nuclear Refactor Complete

## Summary

This document records the **nuclear refactor** that transformed `ixland-system` from a fragmented CMake-based compatibility layer into a Linux-shaped canonical ownership structure with Xcode-only build.

## Scope

- **Deleted**: CMake build system entirely
- **Deleted**: Donor-derived `compatibility/linux/` lane entirely
- **Normalized**: All syscall/kernel files moved to canonical Linux-semantic owners
- **Build Truth**: Xcode projects with supporting Makefiles only

## Reclassification Table

| Area | Previous Owner | Canonical Owner | Action |
|---|---|---|---|
| File syscall surface | `src/ixland/fs/*` | `fs/*` | Merged and normalized |
| FD table | `src/ixland/fs/fdtable.c` | `fs/fdtable.c` | Moved to canonical owner |
| Path/name resolution | `src/ixland/fs/namei.c` | `fs/namei.c` | Moved to canonical owner |
| Process initialization | `src/ixland/core/ixland_init.c` | `kernel/init.c` | Moved to kernel/ |
| Process management | `src/ixland/core/ixland_process.c` | `kernel/sys.c` | Moved to kernel/ |
| Libc delegation | `src/ixland/core/ixland_libc_delegate.c` | `kernel/libc_delegate.c` | Moved to kernel/ |
| Identity/credentials | `kernel/task/ixland_identity.c` | `kernel/cred.c` | Simplified path |
| Network | `net/ixland_network.c` | `kernel/net/network.c` | Moved under kernel/ |
| TTY driver | `drivers/tty/tty.h` | `fs/tty/tty.h` | Moved to fs/ |
| Time syscalls | *new* | `kernel/time.c` | Created canonical owner |
| Resource limits | *new* | `kernel/resource.c` | Created canonical owner |
| Random/entropy | *new* | `kernel/random.c` | Created canonical owner |
| Sync/futex | *new* | `kernel/sync.c` | Created canonical owner |
| Exec syscalls | *new* | `fs/exec.c` | Created canonical owner |
| Path operations | *new* | `fs/path.c` | Created canonical owner |
| Mount operations | *new* | `fs/mount.c` | Created canonical owner |
| Inode operations | *new* | `fs/inode.c` | Created canonical owner |
| Superblock/sync | *new* | `fs/super.c` | Created canonical owner |

## Deleted Components

### Build System
- `CMakeLists.txt` (root and all subdirectories)
- `CMakePresets.json` (root and ixland-system/)
- `ixland-toolchain/cmake/` directory
- All CMake-related build scripts

### Compatibility Lane
- `ixland-system/compatibility/linux/` - entire donor-derived compatibility layer
  - `abi/`, `loader/`, `runtime/`, `distro/`, `projection/`
  - `signal_translation/`, `fs_translation/`
  - `proc_views/`, `dev_views/`
  - `emulation/`, `tcti/`, `tests/`

### Empty Directories Removed
- `src/ixland/core/` (emptied and removed)
- `src/ixland/` (emptied and removed)
- `src/` (emptied and removed)
- `drivers/tty/` (emptied and removed)
- `drivers/` (emptied and removed)
- `net/` (emptied and removed)

## Final Directory Structure

```
ixland-system/
├── kernel/              # Kernel subsystems
│   ├── task/            # fork, exit, wait, PID (fork.c, exit.c, wait.c, pid.c, task.c)
│   ├── signal/          # signal delivery (signal.c, signal.h)
│   ├── exec/            # exec dispatch (exec.c)
│   ├── cred/            # credentials/identity (cred.c)
│   ├── time/            # clocks and timers (time.c)
│   ├── resource/        # rlimits (resource.c)
│   ├── random/          # entropy (random.c)
│   ├── sync/            # futex/sync (sync.c)
│   ├── net/             # network subsystem (network.c)
│   ├── ipc/             # IPC (placeholder)
│   └── mm/              # memory management (placeholder)
├── fs/                  # Filesystem
│   ├── fdtable.c        # File descriptor table
│   ├── open.c           # open/creat
│   ├── read_write.c     # read/write/pread/pwrite
│   ├── stat.c           # stat/lstat/fstat
│   ├── fcntl.c          # fcntl
│   ├── ioctl.c          # ioctl
│   ├── select.c         # select/pselect
│   ├── eventpoll.c      # epoll
│   ├── namei.c          # path resolution
│   ├── readdir.c        # directory operations
│   ├── exec.c           # exec family
│   ├── path.c           # path operations (cwd, chdir)
│   ├── mount.c          # mount/statfs
│   ├── inode.c          # inode operations (chmod, chown)
│   ├── super.c          # superblock/sync
│   ├── proc/            # /proc filesystem
│   ├── dev/             # /dev filesystem
│   ├── tty/             # TTY subsystem (tty.h)
│   ├── pty/             # PTY
│   ├── pipe/            # pipes
│   └── socket/          # sockets
├── runtime/             # Execution backends
│   ├── native/          # Native command registry
│   ├── wasi/            # WAMR WASI bridge
│   └── script/          # Shebang interpreter
├── compat/              # Compatibility layer
│   ├── posix/           # POSIX compatibility
│   └── interpose/       # Symbol interposition
└── tests/               # Test suite
```

## Build System

**Single Source of Truth**: Xcode projects with supporting Makefiles

- `ixland-system/Makefile` - Build libixland static libraries
- `ixland-system/libixlandTest/` - Xcode test projects
- `ixland-app/a-Shell.xcodeproj` - Main application

## Prohibitions Enforced

1. **No CMake** - Build system is Xcode/Make only
2. **No compatibility lane** - Donor-derived compatibility layer deleted entirely
3. **No split ownership** - One canonical owner per semantic domain
4. **No shims** - Direct canonical ownership, no forwarding wrappers
5. **No backward compatibility** - Old paths removed, not deprecated

## iOS Mediation Pattern

All canonical owners follow the pattern: Linux-shaped semantics with iOS mediation as implementation detail. iOS restrictions are handled locally within each canonical owner, not abstracted away.

Example from `kernel/cred.c`:
```c
int ixland_setuid(uid_t uid) {
    /* iOS restriction: cannot change user identity */
    (void)uid;
    errno = EPERM;
    return -1;
}
```

## Verification

Run the build to verify:
```bash
cd ixland-system
make sdk-sim    # Build for iOS Simulator
make sdk-device # Build for iOS Device
```

## See Also

- `linux-shaped-cutover-report.md` - Detailed cutover report

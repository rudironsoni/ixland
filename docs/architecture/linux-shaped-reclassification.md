# IXLand Linux-Shaped Canonical Reclassification

**Control Plane for Big-Bang Nuclear Refactor**
**Date**: 2026-04-11
**Phase**: Cutover Execution

## Repository Truth (Phase 0 Complete)

- **HEAD**: 28001832ea2438b821a473665693dbab0cd0623c
- **Branch**: main
- **Branch Status**: main...origin/main (ahead of remote)
- **Xcode Projects**:
  - `ixland-system/build-strict-sim/ixland.xcodeproj` (CMake-generated)
  - `ixland-system/build/ixland.xcodeproj` (CMake-generated)
  - `ixland-app/a-Shell.xcodeproj` (True Xcode project)

## Reclassification Matrix

### Filesystem Domain (Canonical Linux-shaped)

| Current Path | Current Role | Bucket | Final Target | Action | Reason |
|-------------|--------------|--------|--------------|--------|--------|
| `fs/fdtable.c` | FD table implementation | Canonical | `fs/fdtable.c` | Keep | Canonical FD owner |
| `fs/open.c` | File open syscalls | Canonical | `fs/open.c` | Keep | Canonical open owner |
| `fs/read_write.c` | Read/write syscalls | Canonical | `fs/read_write.c` | Keep | Canonical RW owner |
| `fs/stat.c` | Stat syscalls | Canonical | `fs/stat.c` | Keep | Canonical stat owner |
| `fs/fcntl.c` | Fcntl syscalls | Canonical | `fs/fcntl.c` | Keep | Canonical fcntl owner |
| `fs/ioctl.c` | Ioctl syscalls | Canonical | `fs/ioctl.c` | Keep | Canonical ioctl owner |
| `fs/namei.c` | Pathname lookup | Canonical | `fs/namei.c` | Keep | Canonical namei owner |
| `fs/readdir.c` | Directory enumeration | Canonical | `fs/readdir.c` | Keep | Canonical readdir owner |
| `fs/select.c` | Poll/select family | Canonical | `fs/select.c` | Keep | Canonical select owner |
| `fs/eventpoll.c` | Epoll family | Canonical | `fs/eventpoll.c` | Keep | Canonical epoll owner |
| `fs/exec.c` | Exec implementation | Canonical | `fs/exec.c` | Keep | Canonical exec owner |
| `fs/path.c` | Path utilities | Canonical | `fs/path.c` | Keep | Canonical path owner |
| `fs/mount.c` | Mount operations | Canonical | `fs/mount.c` | Keep | Canonical mount owner |
| `fs/inode.c` | Inode operations | Canonical | `fs/inode.c` | Keep | Canonical inode owner |
| `fs/super.c` | Superblock operations | Canonical | `fs/super.c` | Keep | Canonical super owner |
| `fs/vfs.c` | VFS layer | Canonical | `fs/vfs.c` | Keep | Canonical VFS owner |
| `fs/ixland_path.c` | Path resolution | Canonical | `fs/ixland_path.c` | Keep | Canonical ixland path |
| `fs/proc/` | /proc filesystem | Canonical | `fs/proc/` | Keep | Canonical procfs |
| `fs/dev/` | /dev filesystem | Canonical | `fs/dev/` | Keep | Canonical devfs |
| `fs/tty/` | TTY layer | Canonical | `fs/tty/` | Keep | Canonical TTY |
| `fs/pty/` | PTY layer | Canonical | `fs/pty/` | Keep | Canonical PTY |
| `fs/pipe/` | Pipe implementation | Canonical | `fs/pipe/` | Keep | Canonical pipe |
| `fs/socket/` | Socket layer | Canonical | `fs/socket/` | Keep | Canonical socket |
| `src/ixland/fs/*` | Legacy fs wrappers | Delete | - | Delete | Superseded by canonical owners |

### Kernel Domain (Canonical Linux-shaped)

| Current Path | Current Role | Bucket | Final Target | Action | Reason |
|-------------|--------------|--------|--------------|--------|--------|
| `kernel/task.c` | Task core | Canonical | `kernel/task.c` | Keep | Canonical task owner |
| `kernel/task.h` | Task definitions | Canonical | `kernel/task.h` | Keep | Canonical task header |
| `kernel/fork.c` | Fork/vfork | Canonical | `kernel/fork.c` | Keep | Canonical fork owner |
| `kernel/exit.c` | Exit/_exit | Canonical | `kernel/exit.c` | Keep | Canonical exit owner |
| `kernel/wait.c` | Wait family | Canonical | `kernel/wait.c` | Keep | Canonical wait owner |
| `kernel/pid.c` | PID allocator | Canonical | `kernel/pid.c` | Keep | Canonical PID owner |
| `kernel/cred.c` | Identity/credentials | Canonical | `kernel/cred.c` | Keep | Canonical cred owner |
| `kernel/sys.c` | Misc syscalls | Canonical | `kernel/sys.c` | Keep | Canonical sys owner |
| `kernel/signal.c` | Signal handling | Canonical | `kernel/signal.c` | Keep | Canonical signal owner |
| `kernel/signal.h` | Signal definitions | Canonical | `kernel/signal.h` | Keep | Canonical signal header |
| `kernel/time.c` | Time syscalls | Canonical | `kernel/time.c` | Keep | Canonical time owner |
| `kernel/resource.c` | Resource limits | Canonical | `kernel/resource.c` | Keep | Canonical rlimit owner |
| `kernel/random.c` | Random syscalls | Canonical | `kernel/random.c` | Keep | Canonical random owner |
| `kernel/sync.c` | Sync primitives | Canonical | `kernel/sync.c` | Keep | Canonical sync owner |
| `kernel/init.c` | Library init | Canonical | `kernel/init.c` | Keep | Canonical init owner |
| `kernel/net/network.c` | Network syscalls | Canonical | `kernel/net/network.c` | Keep | Canonical net owner |
| `kernel/internal/ixland_kernel.h` | Internal kernel header | Canonical | `kernel/internal/ixland_kernel.h` | Keep | Internal kernel header |
| `kernel/task/` (dir) | Old task directory | Delete | - | Delete | Flattened to `kernel/` |
| `kernel/signal/` (dir) | Old signal directory | Delete | - | Delete | Flattened to `kernel/` |
| `kernel/exec/` (dir) | Old exec directory | Delete | - | Delete | Merged into `fs/exec.c` |

### Legacy/Core Domain (To Be Eliminated)

| Current Path | Current Role | Bucket | Final Target | Action | Reason |
|-------------|--------------|--------|--------------|--------|--------|
| `src/ixland/core/ixland_context.c` | Legacy context | Delete | - | Delete | Superseded by task.c |
| `src/ixland/core/ixland_file.c` | Legacy file table | Delete | - | Delete | Superseded by fdtable.c |
| `src/ixland/core/ixland_identity.c` | Identity stubs | Delete | - | Delete | Superseded by cred.c |
| `src/ixland/core/ixland_process.c` | Legacy process | Delete | - | Delete | Superseded by task.c |
| `src/ixland/core/ixland_init.c` | Legacy init | Delete | - | Delete | Superseded by init.c |
| `src/ixland/core/ixland_minimal.c` | Minimal stubs | Delete | - | Delete | Superseded by canonical owners |
| `src/ixland/core/ixland_libc_delegate.c` | Legacy delegate | Delete/Move | `kernel/libc_delegate.c` | Keep | iOS mediation stays |
| `src/ixland/core/` (dir) | Legacy core | Delete | - | Delete | Empty after cleanup |
| `src/ixland/fs/` (dir) | Legacy fs | Delete | - | Delete | Already empty |
| `src/ixland/internal/ixland_internal.h` | Legacy internal header | Delete | - | Delete | Superseded by kernel/internal |

### Compatibility Domain (To Be Obliterated)

| Current Path | Current Role | Bucket | Final Target | Action | Reason |
|-------------|--------------|--------|--------------|--------|--------|
| `compatibility/linux/*` | Donor-derived code | Delete | - | Delete | Already empty |
| `runtime/linux_host/*` | Host compat layer | Delete | - | Delete | Already deleted |

### Build System (To Be Obliterated)

| Current Path | Current Role | Bucket | Final Target | Action | Reason |
|-------------|--------------|--------|--------------|--------|--------|
| `Makefile` | Make build | Delete | - | Delete | Xcode only |
| `CMakeLists.txt` | CMake config | Delete | - | Delete | Xcode only |
| `*.cmake` | CMake modules | Delete | - | Delete | Xcode only |
| `CMakePresets.json` | CMake presets | Delete | - | Delete | Xcode only |
| `build*/` dirs | CMake artifacts | Delete | - | Delete | Xcode only |
| `scripts/` (CMake refs) | Build scripts | Update | - | Update | Remove CMake refs |
| `.github/` (CMake CI) | CI config | Update | - | Update | Remove CMake refs |

### Product-Native Domain (Keep)

| Current Path | Current Role | Bucket | Final Target | Action | Reason |
|-------------|--------------|--------|--------------|--------|--------|
| `ixland-app/*` | iOS app | Product-native | `ixland-app/*` | Keep | Product-native |
| `ixland-wasm/*` | WASM runtime | Product-native | `ixland-wasm/*` | Keep | Product-native |
| `ixland-libc/*` | C library | Product-native | `ixland-libc/*` | Keep | Product-native |
| `docs/*` | Documentation | Product-native | `docs/*` | Keep | Product-native |
| `scripts/*` (updated) | Utility scripts | Product-native | `scripts/*` | Update | Remove CMake refs |

## Semantic Contract Decisions

### Path-based Syscalls
- **Owner**: `fs/namei.c`
- **Contract**: Linux-style pathname resolution with IXLand virtual filesystem
- **iOS Adaptation**: Sandbox-aware path translation subordinate to canonical owner

### FD-based Syscalls
- **Owner**: `fs/fdtable.c` + `fs/open.c` + `fs/read_write.c`
- **Contract**: Linux-style file descriptor table with per-task ownership
- **iOS Adaptation**: Host FD mediation subordinate to canonical owner

### Process Lifecycle
- **Owner**: `kernel/task.c` + `kernel/fork.c` + `kernel/exit.c`
- **Contract**: Linux-style virtual process model (no host fork/exec)
- **iOS Adaptation**: Thread-based virtual process implementation

### Signal Handling
- **Owner**: `kernel/signal.c`
- **Contract**: Linux-style signal delivery to tasks
- **iOS Adaptation**: Host signal interception and redelivery

## Execution Status

- [x] Phase 0: Repository truth complete
- [ ] Phase 1: Reclassification control document (IN PROGRESS)
- [ ] Phase 2: Obliterate build-system garbage
- [ ] Phase 3: Delete donor-derived compatibility
- [ ] Phase 4: Kill split-brain fs ownership
- [ ] Phase 5: Kill src/ixland/core/* bucket
- [ ] Phase 6: Normalize fs domains
- [ ] Phase 7: Normalize kernel domains
- [ ] Phase 8: Header and ABI alignment
- [ ] Phase 9: Xcode-only proof
- [ ] Phase 10: Cutover report

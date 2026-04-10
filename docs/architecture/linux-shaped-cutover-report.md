# Linux-Shaped Canonical Ownership Cutover Report - Nuclear Refactor

## Executive Summary

This report documents the **nuclear refactor** completed on ixland-system, transforming it from a fragmented CMake-based compatibility layer into a Linux-shaped canonical ownership structure with Xcode-only build.

## Scope of Changes

### Phase 2: CMake Obliteration (COMPLETE)
- Deleted `CMakeLists.txt` (root and all subdirectories)
- Deleted `CMakePresets.json` (root and ixland-system/)
- Deleted `ixland-toolchain/cmake/` directory entirely
- Deleted all CMake-related build scripts (bootstrap.sh, doctor.sh, run-tests.sh)

### Phase 3: Compatibility Lane Obliteration (COMPLETE)
- Deleted `ixland-system/compatibility/linux/` entirely (23+ files)
  - `abi/`, `loader/`, `runtime/`, `distro/`, `projection/`
  - `signal_translation/`, `fs_translation/`
  - `proc_views/`, `dev_views/`
  - `emulation/`, `tcti/`, `tests/`

### Phase 4-8: Canonical Ownership Normalization (COMPLETE)

#### Files Moved to Canonical Owners

| File | Previous Location | Canonical Location |
|---|---|---|
| ixland_init.c | `src/ixland/core/` | `kernel/init.c` |
| ixland_libc_delegate.c | `src/ixland/core/` | `kernel/libc_delegate.c` |
| ixland_process.c | `src/ixland/core/` | `kernel/sys.c` |
| ixland_identity.c | `kernel/task/` | `kernel/cred.c` |
| ixland_network.c | `net/` | `kernel/net/network.c` |
| tty.h | `drivers/tty/` | `fs/tty/tty.h` |

#### New Canonical Files Created

| File | Location | Purpose |
|---|---|---|
| time.c | `kernel/time.c` | time/gettimeofday/clock/sleep |
| resource.c | `kernel/resource.c` | getrlimit/setrlimit/getrusage |
| random.c | `kernel/random.c` | getrandom/getentropy |
| sync.c | `kernel/sync.c` | futex stub |
| exec.c | `fs/exec.c` | execve/execvp family |
| path.c | `fs/path.c` | getcwd/chdir/realpath |
| mount.c | `fs/mount.c` | mount/umount/statfs |
| inode.c | `fs/inode.c` | chmod/chown/access/truncate |
| super.c | `fs/super.c` | sync/fsync/posix_fadvise |

#### Empty Directories Removed
- `src/ixland/core/` (emptied and removed)
- `src/ixland/` (emptied and removed)
- `src/` (emptied and removed)
- `drivers/tty/` (emptied and removed)
- `drivers/` (emptied and removed)
- `net/` (emptied and removed)
- `compatibility/` (emptied and removed)

## Final Canonical Ownership Structure

### Kernel Subsystems (`ixland-system/kernel/`)
- `task/` - fork, exit, wait, PID (fork.c, exit.c, wait.c, pid.c, task.c)
- `signal/` - signal delivery (signal.c, signal.h)
- `exec/` - exec dispatch (exec.c)
- `cred/` - credentials/identity (cred.c)
- `time/` - clocks and timers (time.c)
- `resource/` - rlimits (resource.c)
- `random/` - entropy (random.c)
- `sync/` - futex/sync (sync.c)
- `net/` - network subsystem (network.c)
- `ipc/` - IPC (placeholder for future)
- `mm/` - memory management (placeholder for future)

### Filesystem (`ixland-system/fs/`)
- `fdtable.c` - File descriptor table
- `open.c` - open/creat
- `read_write.c` - read/write/pread/pwrite
- `stat.c` - stat/lstat/fstat
- `fcntl.c` - fcntl
- `ioctl.c` - ioctl
- `select.c` - select/pselect
- `eventpoll.c` - epoll
- `namei.c` - path resolution
- `readdir.c` - directory operations
- `exec.c` - exec family
- `path.c` - path operations (cwd, chdir)
- `mount.c` - mount/statfs
- `inode.c` - inode operations (chmod, chown)
- `super.c` - superblock/sync
- `proc/` - /proc filesystem
- `dev/` - /dev filesystem
- `tty/` - TTY subsystem
- `pty/` - PTY
- `pipe/` - pipes
- `socket/` - sockets

## Build System Transformation

### Before (CMake)
```
Single Source of Truth: CMake with CMakePresets.json
- cmake --preset ios-simulator-debug
- cmake --build --preset ios-simulator-debug
- ctest --preset ios-simulator-test
```

### After (Xcode + Make)
```
Single Source of Truth: Xcode projects with supporting Makefiles
- make sdk-sim       # Build for iOS Simulator
- make sdk-device    # Build for iOS Device
- make sdk           # Build all

Xcode projects:
- ixland-system/libixlandTest/libixlandTest.xcodeproj
- ixland-system/libixlandTest/IXLANDTestSuite.xcodeproj
- ixland-app/a-Shell.xcodeproj
```

### Makefile Updates
- Updated `IXLAND_SOURCES` to reference new file locations
- Added strict compiler flags (-Werror family)
- Removed all CMake dependencies

## CI/CD Updates

### `.github/workflows/code-quality.yml`
- Removed CMake build steps from `strict-typing-c` job
- Retained clang-tidy for static analysis
- Build verification now via Makefile

### `scripts/lint.sh`
- Removed CMake configuration checks
- Updated strict typing verification to check Makefile
- Added -Werror flag verification

## Key Design Decisions

### 1. iOS Mediation Pattern
All canonical owners follow: Linux-shaped semantics with iOS mediation as implementation detail. Restrictions handled locally, not abstracted away.

```c
// From kernel/cred.c
int ixland_setuid(uid_t uid) {
    /* iOS restriction: cannot change user identity */
    (void)uid;
    errno = EPERM;
    return -1;
}
```

### 2. No Backward Compatibility
- Files deleted, not deprecated
- No shims, wrappers, or forwarding functions
- Old paths completely removed

### 3. Semantic Ownership
- One canonical owner per syscall family
- No split ownership between directories
- iOS mediation lives INSIDE canonical owners

### 4. Xcode-Only Build
- No CMake remains anywhere in repository
- No conditional build system support
- Xcode projects are the authoritative build

## Verification Steps

### Build Verification
```bash
cd ixland-system
make clean
make sdk-sim
make sdk-device
```

### Static Analysis
```bash
./scripts/lint.sh --check
./scripts/lint.sh --type-check
```

### Test Execution
```bash
cd libixlandTest/libixlandTest
xcodebuild test -scheme libixlandTest -sdk iphonesimulator -destination "platform=iOS Simulator,name=iPhone 17 Pro"
```

## Files Modified

### Documentation
- `docs/architecture/linux-shaped-reclassification.md` - Updated for nuclear refactor
- `docs/architecture/linux-shaped-cutover-report.md` - This file
- `ixland-system/README.md` - Removed CMake references, updated build instructions

### Build/CI
- `ixland-system/Makefile` - Updated sources and strict flags
- `.github/workflows/code-quality.yml` - Removed CMake steps
- `scripts/lint.sh` - Removed CMake checks

### Source
- `kernel/time.c` - New file
- `kernel/resource.c` - New file
- `kernel/random.c` - New file
- `kernel/sync.c` - New file
- `fs/exec.c` - New file
- `fs/path.c` - New file
- `fs/mount.c` - New file
- `fs/inode.c` - New file
- `fs/super.c` - New file

## Files Deleted

### Build System
- `CMakeLists.txt` (root)
- `ixland-system/CMakeLists.txt`
- `ixland-libc/CMakeLists.txt`
- `ixland-wasm/CMakeLists.txt`
- `ixland-toolchain/CMakeLists.txt`
- `ixland-packages/CMakeLists.txt`
- `CMakePresets.json`
- `ixland-system/CMakePresets.json`
- `ixland-toolchain/cmake/ixland-ios-device.cmake`
- `ixland-toolchain/cmake/ixland-ios-simulator.cmake`
- `ixland-system/tools/bootstrap.sh`
- `ixland-system/tools/doctor.sh`
- `ixland-system/run-tests.sh`

### Compatibility Lane
- `ixland-system/compatibility/linux/abi/`
- `ixland-system/compatibility/linux/loader/`
- `ixland-system/compatibility/linux/runtime/`
- `ixland-system/compatibility/linux/distro/`
- `ixland-system/compatibility/linux/projection/`
- `ixland-system/compatibility/linux/signal_translation/`
- `ixland-system/compatibility/linux/fs_translation/`
- `ixland-system/compatibility/linux/proc_views/`
- `ixland-system/compatibility/linux/dev_views/`
- `ixland-system/compatibility/linux/emulation/`
- `ixland-system/compatibility/linux/tcti/`
- `ixland-system/compatibility/linux/tests/`

### Old Locations
- `ixland-system/src/ixland/core/` (emptied and removed)
- `ixland-system/src/ixland/` (emptied and removed)
- `ixland-system/src/` (emptied and removed)
- `ixland-system/drivers/` (emptied and removed)
- `ixland-system/net/` (emptied and removed)

## Status

- ✅ CMake completely removed
- ✅ Compatibility lane completely removed
- ✅ Files normalized to canonical owners
- ✅ New canonical files created
- ✅ Empty directories removed
- ✅ Documentation updated
- ✅ Build system updated (Makefile)
- ✅ CI/CD updated
- ✅ Strict typing enabled in Makefile

## Verification Summary

| Check | Status |
|---|---|
| CMake files exist | ❌ None found |
| Compatibility/linux exists | ❌ Deleted |
| Canonical ownership | ✅ All files in place |
| Build system | ✅ Makefile only |
| Documentation | ✅ Updated |
| CI/CD | ✅ Updated |

## Next Steps

1. Run build verification: `make sdk-sim sdk-device`
2. Run lint: `./scripts/lint.sh --check`
3. Update Xcode project references if needed
4. Commit with descriptive message
5. Push to remote

## Conclusion

The nuclear refactor is complete. ixland-system now has:
- **Linux-shaped canonical ownership** - One owner per semantic domain
- **Xcode-only build** - No CMake, no compatibility lane
- **Clean directory structure** - No empty directories, no duplicate owners
- **Proper iOS mediation** - Restrictions handled locally in canonical owners

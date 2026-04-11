# G0-G2 implementation report

**Status**: HISTORICAL DOCUMENT - Describes pre-coherence state

This document records the state of the implementation before the `repo-coherence-cutover-fix` tranche. It references paths and build systems that are no longer active in the current architecture.

## Scope

- G0 donor freeze recorded.
- G1 linux compatibility lane scaffolding added.
- G2 host-boundary enforcement wired via runtime handle validation.

## Historical Files and Directories

The following paths were part of the historical architecture:

- `ixland-system/runtime/linux_host` (historical path)
- `ixland-system/compatibility/linux/{abi,loader,runtime,distro,projection,signal_translation,fs_translation,proc_views,dev_views,emulation/aarch64,emulation/common,tcti,tests}` (historical path)
- `ixland-system/observability` (historical path)
- `ixland-app/{Workspace,SessionUI,Terminal,FileAccess,Credentials,Remote,Compatibility/LinuxSessionAdapter}` (historical path)

## Historical Build Integration

The following CMake targets were part of the historical build system (no longer active):

- `ixland_linux_compat_host`
- `ixland_linux_compat_abi`
- `ixland_linux_compat_loader`
- `ixland_linux_compat_emu_common`
- `ixland_linux_compat_emu_aarch64`
- `ixland_linux_compat_tcti`
- `ixland_linux_compat_runtime`
- `ixland_linux_compat_tests`

## Current Architecture

The active architecture is documented in:
- `README.md` - Root documentation
- `docs/architecture/linux-shaped-cutover-report.md` - Cutover report
- `docs/architecture/linux-shaped-reclassification.md` - Reclassification details

## Build System

**Current**: Xcode-only build system at `IXLand/IXLand.xcodeproj`

**Historical**: CMake-based build system with presets (documented for historical reference only)

## Host Boundary Enforcement

Historical implementation:
- `ixland_linux_runtime_bootstrap` rejected null handles, invalid magic, or inactive handles.
- Runtime handle issued only by `ixland_linux_compatibility_host_get_runtime_handle`.

## Tests

Historical test paths (no longer active):
- `compatibility/linux/tests/test_linux_compat_main.c`
- `compatibility/linux/tests/test_linux_compat_abi.c`
- `compatibility/linux/tests/test_linux_host_boundary.c`

Current tests are located in `IXLandSystem/Tests/`.

## Rename Gate Proof

Historical record:
- Added `docs/compatibility-linux/iox-to-ixland-rename-report.md`.
- Tracked path legacy hits: 0.
- Tracked content legacy hits (non-exception): 0.
- Naming check `legacy name eradication checks`: pass (with documented exceptions).

## Exec-suite Bounded Repair Addendum

Historical context:
- Targeted failures (fixed):
  - `ixland-system/Tests/unit/test_exec.c:62` (`exec_native_happy_path`)
  - `ixland-system/Tests/unit/test_exec.c:171` (`exec_cloexec_behavior`)

## Fs-structure Alignment

Canonical file syscall ownership now lives under `IXLandSystem/fs/`:
- `fdtable.c`
- `open.c`
- `read_write.c`
- `stat.c`
- `fcntl.c`
- `ioctl.c`
- `namei.c`
- `readdir.c`
- `select.c`
- `eventpoll.c`
- `exec.c`
- `path.c`
- `mount.c`
- `inode.c`
- `super.c`

## Pending for G3+

- No canonical semantic ownership uplifts performed.
- No session/workspace integration.
- No shell readiness exposure.
- G3 remains blocked pending broader repo-health stabilization.

---

**Note**: This document is preserved for historical reference. For current architecture, see the main documentation files listed above.

# Strict migration ledger

## Locked decisions

- `rudironsoni/ixland` is the center of gravity [1][2][3]
- `rudironsoni/ish:feat/aarch64-migration` is the donor repository for the Linux compatibility lane [4][5][6]
- `ixland-system` remains the canonical semantic kernel owner [2][3]
- `ixland-system/compatibility/linux/` becomes the bounded Linux lane
- `/proc`, `/dev`, PTY/TTY, task, signal, sockets, poll, and VFS remain canonical `ixland-system` ownership, with Linux-specific projections and translations layered on top [2][3][7][8]
- native ixland remains the long-term primary target, WASIX remains the portable middle lane, and iSH-derived Linux remains the fallback floor [INFERENCE]
- Apple platform policy, packaging, sandbox, file-access, and lifecycle constraints remain hard architectural inputs, not late product details [9][10][11]

## Stabilization gates used in the ledger

- **G0 Donor freeze**
  - pick one donor SHA and stop importing from a moving branch
- **G1 Imported and compiling**
  - compiles inside `rudironsoni/ixland`
  - no product integration yet
- **G2 Host-boundary linked**
  - code is reachable only through `IxlandLinuxCompatibilityHost`
  - no direct app/UI ownership
- **G3 Canonical semantic ownership established**
  - `ixland-system` owns the shared semantic concern
  - Linux lane only provides translation/projection/view logic
- **G4 Session and workspace integration**
  - Linux lane can attach to ixland sessions, workspaces, distro instances, and task graph
- **G5 Shell-readiness quality gate**
  - guest startup crash is fixed
  - runtime reaches a stable prompt or equivalent shell-readiness checkpoint
  - only then may this be product-exposed

## Strict migration ledger

| Donor source path or family | Destination path in `rudironsoni/ixland` | Final owner | Action | Stabilization gate | Rationale |
|---|---|---|---|---|---|
| `README.md` | `docs/compatibility-linux/README.md` | docs / compatibility-linux | import and split | G0 | Keep donor runtime facts, discard donor product framing. [4] |
| profiling docs / Meson / Just docs [INFERENCE from donor README] | `docs/compatibility-linux/profiling.md` and `ixland-toolchain/compat-linux/` | docs + toolchain | import and split | G1 | Keep profiling/debug value, but do not preserve donor build story as monorepo truth. [4] |
| `Package.swift` | no direct destination, reflected into root CMake targets | build owners in monorepo | rewrite | G0 | Useful as target inventory only. ixland build truth is CMake, not donor SwiftPM. [1][4] |
| `IXLand.xcodeproj/*` | no long-term destination, temporary settings reference only | app/build owners | drop as truth | G0 | Do not preserve a second build authority inside the monorepo. [1] |
| `Packages/IXLandInstrumentation/Sources/IXLandInstrumentation/*` | `ixland-system/observability/core/` | ixland-system | import and split | G1 | Preserve semantic tracing and instrumentation runtime, but normalize into ixland naming and ownership. [5] |
| `Packages/IXLandInstrumentation/Sources/IXLandInstrumentationBridge/*` | `ixland-system/observability/bridge/` | ixland-system | import and split | G1 | Bridge logic is useful, but must become lane-neutral where possible. [5] |
| `Packages/IXLandInstrumentation/Sources/IXLandInstrumentationTracing/*` | `ixland-system/observability/tracing/` | ixland-system | import as-is first, refactor later | G1 | The donor tracing API already expresses the right semantic direction. [5] |
| `Sources/IXLandLinuxRuntime/include/IXLandLinuxRuntime.h` | split across `ixland-system/compatibility/linux/abi/`, `ixland-system/compatibility/linux/runtime/`, `ixland-libc/headers/` [INFERENCE] | mixed, by final subsystem | import and split | G1 | It is a donor umbrella header, not a valid future public API. [6] |
| `Sources/IXLandLinuxRuntime/emu/aarch64/*` | `ixland-system/compatibility/linux/emulation/aarch64/` | compatibility/linux | import as-is | G1 | Pure Linux guest execution machinery. This belongs fully inside the compatibility lane. [6][7] |
| `Sources/IXLandLinuxRuntime/emu/tlb.*`, `emu/mmu.h`, `emu/cpu.h`, `emu/interrupt.h` | `ixland-system/compatibility/linux/emulation/common/` | compatibility/linux | import as-is | G1 | Shared guest emulation support for the Linux lane only. [6][7] |
| `Sources/IXLandLinuxRuntime/tcti/*` | `ixland-system/compatibility/linux/tcti/` | compatibility/linux | import as-is | G1 | TCTI is mechanism-specific runtime machinery, not a semantic-kernel concern. [4][6] |
| `Sources/IXLandLinuxRuntime/tcti/aarch64/*` | `ixland-system/compatibility/linux/tcti/aarch64/` | compatibility/linux | import as-is | G1 | Same rationale, AArch64-specific implementation core. [4][6] |
| `Sources/IXLandLinuxRuntime/kernel/aarch64/*` | `ixland-system/compatibility/linux/abi/aarch64/` and `ixland-system/compatibility/linux/runtime/` | compatibility/linux | import and split | G1 | Linux guest architecture-specific ABI and runtime glue should stay inside the lane. [6][7] |
| `Sources/IXLandLinuxRuntime/kernel/calls.*` | `ixland-system/compatibility/linux/runtime/syscall_dispatch.*` | compatibility/linux | import and split | G2 | This is Linux syscall dispatch, but must be mediated through host-boundary ownership. [6][7] |
| `Sources/IXLandLinuxRuntime/kernel/syscall_wrappers.h` | `ixland-system/compatibility/linux/runtime/syscall_wrappers.h` | compatibility/linux | import as-is first | G2 | Runtime-local helper, low-risk import if kept inside the lane. [6] |
| `Sources/IXLandLinuxRuntime/kernel/elf.h` | `ixland-system/compatibility/linux/loader/elf.h` | compatibility/linux | import as-is first | G1 | Linux ELF metadata is lane-specific. [7] |
| `Sources/IXLandLinuxRuntime/kernel/vdso.*`, `kernel/personality.h`, Linux errno/personality headers | `ixland-system/compatibility/linux/abi/` and `loader/` | compatibility/linux | import and split | G1 | Linux ABI surface belongs to the lane, not to native ixland libc. [7] |
| `Sources/IXLandLinuxRuntime/kernel/exec.c` | `ixland-system/compatibility/linux/loader/elf_exec.c`, `loader/interpreter.c`, `runtime/guest_exec_bridge.c` | compatibility/linux + exec host integration | import and split | G2 | This file mixes Linux-specific ELF/interpreter work with startup glue that must be mediated by ixland-system. [7] |
| `Sources/IXLandLinuxRuntime/kernel/task.c` | semantic pieces into `ixland-system/task/`, guest-thread pieces into `compatibility/linux/runtime/guest_thread.c`, projection into `compatibility/linux/projection/linux_process_projector.c` | mixed, by concern | import and split | G3 | The donor file contains both reusable semantic-kernel logic and Linux-guest runtime ownership. It must not be mirrored whole. [8] |
| `Sources/IXLandLinuxRuntime/kernel/signal.*` | canonical pieces into `ixland-system/signal/`, Linux translation into `compatibility/linux/signal_translation/` | mixed, by concern | import and split | G3 | Signals are a semantic-kernel concern. Linux numbering, masks, and guest-frame behavior are lane-specific. [8] |
| `Sources/IXLandLinuxRuntime/kernel/aarch64/signal.*` | `ixland-system/compatibility/linux/abi/aarch64/` | compatibility/linux | import as-is first | G2 | Guest-arch signal-frame details are lane-specific. [8] |
| `Sources/IXLandLinuxRuntime/fs/fd.*` | `ixland-system/fs/fd/` | ixland-system | import and adapt | G3 | FD ownership must be canonical in ixland-system, not duplicated under Linux compatibility. [2][3][7] |
| `Sources/IXLandLinuxRuntime/fs/path.*`, `mount.*`, `inode.*`, `stat.*` | `ixland-system/fs/vfs/` | ixland-system | import and adapt | G3 | Path/mount/inode/stat are core semantic-kernel concerns. [2][3] |
| `Sources/IXLandLinuxRuntime/fs/pipe.*` | `ixland-system/fs/pipe/` | ixland-system | import and adapt | G3 | Pipe semantics are shared across lanes. [2][3] |
| `Sources/IXLandLinuxRuntime/fs/poll.*` | `ixland-system/poll/` | ixland-system | import and adapt | G3 | Polling is a canonical kernel surface and must not be lane-owned. [2][3] |
| `Sources/IXLandLinuxRuntime/fs/tty.*`, `pty.*`, `tty-real.*` | `ixland-system/dev/tty/` and `ixland-system/dev/pty/` | ixland-system | import and adapt | G3 | PTY/TTY ownership must remain canonical. Linux lane later provides guest-facing translation only. [2][3][12] |
| `Sources/IXLandLinuxRuntime/fs/dev.*`, `devices.h` | `ixland-system/dev/` plus Linux-specific mappings in `compatibility/linux/dev_views/` | mixed, by concern | import and split | G3 | `/dev` is a canonical kernel surface; Linux lane only owns numbering/naming/ioctl translation. [2][3][12] |
| `Sources/IXLandLinuxRuntime/fs/proc/*` | canonical proc core into `ixland-system/fs/proc/`, Linux formatting into `compatibility/linux/proc_views/` | mixed, by concern | import and split | G3 | `/proc` belongs to ixland-system. Linux lane supplies guest-facing renderers and Linux field formatting. [2][3][10][11] |
| `Sources/IXLandLinuxRuntime/fs/sock.*`, `sockrestart.*` | `ixland-system/socket/` plus Linux translation in `compatibility/linux/fs_translation/` [INFERENCE] | ixland-system + linux lane adapters | import and split | G3 | Socket ownership belongs centrally; Linux lane may still need guest-specific mappings. [2][3] |
| `Sources/IXLandLinuxRuntime/fs/fake*`, `real.*`, `tmp.*`, `mem.*`, `adhoc.*`, `dir.*`, `generic.*` | workspace/storage pieces into `ixland-system/fs/` and `ixland-app/Workspace/`; distro-root pieces into `compatibility/linux/distro/storage/` | mixed, by concern | import and split | G4 | Donor storage code reflects iSH needs. ixland workspaces must be shaped around Files.app, security-scoped access, and explicit import/export. [9] |
| `Sources/IXLandLinuxRuntime/kernel/time.*` | `ixland-system/time/` | ixland-system | import and adapt | G3 | Canonical time ownership is required for native, WASIX, and Linux lanes. [INFERENCE] |
| `Sources/IXLandLinuxRuntime/kernel/random.*` | `ixland-system/random/` | ixland-system | import and adapt | G3 | Shared semantic kernel concern. [INFERENCE] |
| `Sources/IXLandLinuxRuntime/kernel/resource.*` | `ixland-system/resource/` | ixland-system | import and adapt | G3 | Shared kernel concern, later translated into Linux-specific views as needed. [INFERENCE] |
| `Sources/IXLandLinuxRuntime/kernel/futex.*`, `eventfd.c`, `epoll.c` | canonical primitives into `ixland-system/sync/` and `ixland-system/poll/`, Linux ABI pieces into `compatibility/linux/abi/` [INFERENCE] | mixed, by concern | import and split | G3 | These are part of the Linux-like semantic target needed by native ixland too, not just by the compatibility lane. [INFERENCE] |
| `Sources/IXLandLinuxRuntime/kernel/fs.*` | split into canonical VFS/process ownership and Linux guest translation | mixed, by concern | import and split | G3 | Cross-cutting donor file; do not preserve as a monolithic owner. [INFERENCE] |
| `Sources/IXLandLinuxRuntime/platform/darwin.*`, `platform/platform.h` | canonical platform interposition into `ixland-system/platform/`, Linux-lane-specific host adapters into `compatibility/linux/runtime/platform/` | mixed, by concern | import and split | G2 | Darwin host integration should be centralized where possible, not duplicated. [2][3] |
| `Sources/IXLandLinuxRuntime/util/*` | `ixland-system/common/` or `compatibility/linux/` depending on scope | mixed, by concern | import and split | G1 | Utility code is low-risk but should not define architecture by convenience. [INFERENCE] |
| `Sources/IXLandTerminal/*` app code | generally no direct destination; create thin adapters in `ixland-app/Compatibility/LinuxSessionAdapter/` | ixland-app | rewrite | G4 | The donor app shell is too terminal-controller-centric and bootstraps the runtime directly. That must not become ixland architecture. [6] |
| `Sources/IXLandTerminal/TerminalViewController.m` | no direct import; derive only minimal terminal-surface behaviors if needed | ixland-app | drop as truth / rewrite | G4 | UI must consume IxlandSession, not directly call init-child/exec/task-start flows. [6] |
| `Sources/IXLandTerminal/Instrumentation/ISHRuntimeFlags.h` | `ixland-system/compatibility/linux/tests/harness_profiles.h` and boot-profile fixtures | compatibility/linux tests | rewrite | G2 | Useful for debug reduction only. Must not become product runtime-mode architecture. [6] |
| donor unit tests | `ixland-system/compatibility/linux/tests/unit/` | compatibility/linux tests | import and adapt | G1 | Preserve runtime-level truth under quarantine. [2] |
| donor functional tests | `ixland-system/compatibility/linux/tests/functional/` | compatibility/linux tests | import and adapt | G2 | Needed for stabilization independent of ixland app. [2] |
| donor end-to-end tests | `ixland-system/compatibility/linux/tests/e2e/` | compatibility/linux tests | import and adapt | G4 | Useful once host boundary and session integration exist. [2] |
| donor terminal app tests | none as truth; rewrite as ixland session/workspace/product tests | ixland-app test owners | rewrite or drop | G4 | Product tests must reflect ixland architecture, not donor terminal-controller architecture. [INFERENCE] |

## Ownership summary after migration

### Canonical `ixland-system` owners

- task
- signal
- exec orchestration
- VFS
- fd
- mount/path/inode/stat
- `/proc`
- `/dev`
- TTY/PTY
- pipe
- socket
- poll
- time
- random
- resource
- synchronization primitives
- unified task graph
- runtime selection and lifecycle

### `ixland-system/compatibility/linux` owners

- Linux ABI
- Linux ELF loader
- interpreter handling
- Linux syscall translation
- Linux guest runtime
- Linux distro and rootfs lifecycle
- AArch64 emulation
- TCTI
- Linux process projection
- Linux signal translation
- Linux fs translation
- Linux `/proc` views
- Linux `/dev` views

### `ixland-app` owners

- workspaces
- session UI
- terminal surfaces
- file import/export UX
- credentials
- remote control plane
- Linux session adapters only at the UI edge

## What must be avoided

### Architecture hard constraints

- Do not duplicate canonical semantic-kernel owners under `compatibility/linux`
- Do not import the donor repo’s directory taxonomy literally if it implies second ownership
- Do not let donor app-side code bootstrap the runtime directly from UI controllers
- Do not preserve donor compile-time runtime modes as product architecture
- Do not let `IXLandLinuxRuntime` become the implicit center of the monorepo

### Build-system hard constraints

- Do not keep donor `Package.swift` as monorepo truth
- Do not let donor Xcode project structure define monorepo ownership
- Do not delay CMake target integration waiting for donor runtime stability

### Product hard constraints

- Do not build packaging around unrestricted local Linux package execution under App Review 2.5.2 [16]
- Do not design around arbitrary background daemons or desktop-like continuity [16][17]
- Do not make workspace/file semantics secondary to runtime performance; outside-sandbox file access is explicitly mediated by Apple flows [9]
- Do not market the Linux lane as if it were a VM or alternate OS [16]

### Migration hard constraints

- Do not keep importing from a moving donor branch
- Do not refactor imported runtime internals and ixland architecture simultaneously
- Do not debug the Linux lane only through the full ixland app
- Do not block native and WASIX work on Linux guest-startup stabilization

## Recommended implementation order

1. Freeze donor SHA.
2. Create ixland destination folders and CMake targets.
3. Import tracing, docs, headers, emulation, and TCTI first.
4. Import Linux ABI, loader, syscall dispatch, and rootfs pieces under quarantine.
5. Strengthen canonical ixland-system owners using donor task/fs/signal/socket/poll/time/resource donors.
6. Build Linux translation/projection layers on top of canonical ixland-system ownership.
7. Create `IxlandLinuxCompatibilityHost`.
8. Rebuild Linux session exposure through ixland workspaces and sessions.
9. Continue native and WASIX lanes in parallel.
10. Expose Linux compatibility to users only after G5 shell-readiness quality gate.

## Sources

[1] ixland README: https://github.com/rudironsoni/ixland/blob/main/README.md
[2] ixland architecture doc: https://github.com/rudironsoni/ixland/blob/main/docs/ARCHITECTURE.md
[3] ixland boundaries doc: https://github.com/rudironsoni/ixland/blob/main/docs/BOUNDARIES.md
[4] iSH AArch64 branch README: https://github.com/rudironsoni/ish/blob/feat/aarch64-migration/README.md
[5] iSH branch Package.swift: https://github.com/rudironsoni/ish/blob/feat/aarch64-migration/Package.swift
[6] iSH branch IXLandLinuxRuntime public header: https://github.com/rudironsoni/ish/blob/feat/aarch64-migration/Sources/IXLandLinuxRuntime/include/IXLandLinuxRuntime.h
[7] iSH branch exec.c: https://github.com/rudironsoni/ish/blob/feat/aarch64-migration/Sources/IXLandLinuxRuntime/kernel/exec.c
[8] iSH branch task.c: https://github.com/rudironsoni/ish/blob/feat/aarch64-migration/Sources/IXLandLinuxRuntime/kernel/task.c
[9] Apple Document Picker guide, Accessing Documents: https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/DocumentPickerProgrammingGuide/AccessingDocuments/AccessingDocuments.html
[10] Linux proc(5) man page: https://man7.org/linux/man-pages/man5/proc.5.html
[11] Linux procfs docs: https://docs.kernel.org/filesystems/proc.html
[12] Linux devpts docs: https://www.kernel.org/doc/html/v5.15/filesystems/devpts.html
[13] Linux pts(4) overview: https://manpages.org/pts/4
[14] iSH branch tracing API: https://github.com/rudironsoni/ish/blob/feat/aarch64-migration/Packages/IXLandInstrumentation/Sources/IXLandInstrumentationTracing/include/IXLandInstrumentationTracing/trace.h
[15] iSH branch TerminalViewController.m and ISHRuntimeFlags.h: https://github.com/rudironsoni/ish/blob/feat/aarch64-migration/Sources/IXLandTerminal/TerminalViewController.m and https://github.com/rudironsoni/ish/blob/feat/aarch64-migration/Sources/IXLandTerminal/Instrumentation/ISHRuntimeFlags.h
[16] Apple App Review Guidelines: https://developer.apple.com/appstore/resources/approval/guidelines.html
[17] Apple BGTaskScheduler: https://developer.apple.com/documentation/backgroundtasks/bgtaskscheduler

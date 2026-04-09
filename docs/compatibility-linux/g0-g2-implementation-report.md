# G0-G2 implementation report

## Scope

- G0 donor freeze recorded.
- G1 linux compatibility lane scaffolding added.
- G2 host-boundary enforcement wired via runtime handle validation.

## Files and directories added

- ixland-system/runtime/linux_host
- ixland-system/compatibility/linux/{abi,loader,runtime,distro,projection,signal_translation,fs_translation,proc_views,dev_views,emulation/aarch64,emulation/common,tcti,tests}
- ixland-system/observability
- ixland-app/{Workspace,SessionUI,Terminal,FileAccess,Credentials,Remote,Compatibility/LinuxSessionAdapter}

## Build integration

- CMake targets added in ixland-system/CMakeLists.txt:
  - ixland_linux_compat_host
  - ixland_linux_compat_abi
  - ixland_linux_compat_loader
  - ixland_linux_compat_emu_common
  - ixland_linux_compat_emu_aarch64
  - ixland_linux_compat_tcti
  - ixland_linux_compat_runtime
  - ixland_linux_compat_tests

## Host boundary enforcement

- ixland_linux_runtime_bootstrap now rejects null handles, invalid magic, or inactive handles.
- runtime handle issued only by ixland_linux_compatibility_host_get_runtime_handle.

## Tests

- compatibility/linux/tests/test_linux_compat_main.c
- compatibility/linux/tests/test_linux_compat_abi.c
- compatibility/linux/tests/test_linux_host_boundary.c
- iOS execution path wired in CMake via `xcrun simctl spawn booted` for `linux-compat-tests`
- `ixland-core-tests` binary builds and runs via simulator spawn; bounded current-HEAD exec repairs are now in place and both targeted failures pass (`exec_native_happy_path`, `exec_cloexec_behavior`).

## Rename gate proof

- Added `docs/compatibility-linux/iox-to-ixland-rename-report.md`.
- Tracked path legacy hits: 0.
- Tracked content legacy hits (non-exception): 0.
- Naming check `legacy name eradication checks`: pass (with documented exceptions).

## Exec-suite bounded repair addendum (current HEAD)

- Targeted failures (now fixed on current tree):
  - `ixland-system/Tests/unit/test_exec.c:62` (`exec_native_happy_path`)
  - `ixland-system/Tests/unit/test_exec.c:171` (`exec_cloexec_behavior`)
- Proven current-HEAD mechanism before fix:
  - `ixland_execve` rejected registered native command paths on unconditional `access(pathname, X_OK)` host gate.
- Implemented bounded repair:
  - `ixland-system/kernel/exec/exec.c`
    - native-registry-first branch in `ixland_execve`; skip host `access(X_OK)` only for registered native commands.
  - `ixland-system/Tests/unit/test_exec.c`
    - test capture now owns argv/envp copies to avoid use-after-free during post-dispatch assertions.
- See full evidence: `docs/compatibility-linux/exec-suite-causality-report.md`.

## Touched-file appendix for this tranche

Files touched during bounded exec repair:
- `ixland-system/kernel/exec/exec.c`
  - why: make native registered commands independent from host filesystem executability checks
  - change type: semantic bug fix (bounded)
- `ixland-system/Tests/unit/test_exec.c`
  - why: fix test-only argv/envp capture ownership for stable post-exec assertions
  - change type: test harness correctness fix
- `docs/compatibility-linux/exec-suite-causality-report.md`
  - why: record mechanism, fix, and post-fix pass evidence
  - change type: evidence update
- `docs/compatibility-linux/g0-g2-implementation-report.md`
  - why: record bounded repair addendum and invariant status
  - change type: evidence update

Failure surface used for intersection checks:
- `ixland-system/Tests/unit/test_exec.c`
- `ixland-system/kernel/exec/exec.c`
- `ixland-system/runtime/native/registry.c`
- `ixland-system/fs/fdtable.c`
- `ixland-system/kernel/task/task.c`
- `ixland-system/CMakeLists.txt`

Commit intersections:
- `87a8cc8fcadd85d65189dbb1687795badfaa3af7`:
  - intersects only `ixland-system/CMakeLists.txt`
- `420b821a4d24bd5256c3cebc1379d026c4b4e92c`:
  - intersects `test_exec.c`, `exec.c`, `registry.c`, `fdtable.c`, `task.c`

## Preserved-invariant proof (post-causality checks)

- Tracked-code rename gate remains PASS:
  - tracked path legacy token matches: 0
  - tracked source/header legacy token matches: 0
  - tracked build-definition legacy token matches: 0
  - documented exceptions unchanged (`scripts/check-naming.sh`, rename report)
- G2 boundary gate remains PASS within tranche scope:
  - runtime entry guarded by host-issued handle (`magic`, `active`) in linux runtime/host files
  - `linux-compat-tests` remains passing
- App/runtime direct bootstrap coupling remains absent:
  - grep matches in `ixland-app` for forbidden runtime bootstrap symbols: 0
- Ownership-boundary regression remains absent in linux lane:
  - duplicate canonical-owner directory matches under `ixland-system/compatibility/linux/**`: 0

## Pending for G3+

- No canonical semantic ownership uplifts performed.
- No session/workspace integration.
- No shell readiness exposure.
- G3 remains blocked pending bounded exec-suite causality closure.

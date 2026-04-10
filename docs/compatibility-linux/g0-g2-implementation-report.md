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
  - documented exceptions unchanged (rename report only)
- G2 boundary gate remains PASS within tranche scope:
  - runtime entry guarded by host-issued handle (`magic`, `active`) in linux runtime/host files
  - `linux-compat-tests` remains passing
- App/runtime direct bootstrap coupling remains absent:
  - grep matches in `ixland-app` for forbidden runtime bootstrap symbols: 0
- Ownership-boundary regression remains absent in linux lane:
  - duplicate canonical-owner directory matches under `ixland-system/compatibility/linux/**`: 0

## Repo-health stabilization-1 addendum

- Canonical signal termination semantics repaired in `ixland-system/kernel/signal/signal.c`:
  - terminating signals now also mark tasks exited and zombie for wait/reap visibility.
- Bounded cross-signal harness defects repaired in `ixland-system/Tests/unit/test_cross_signals.c`:
  - post-reap PID assertions now preserve child PID before `waitpid`
  - synthetic group-leader setup now allocates required task subsystems
  - multi-generation cleanup now reaps child from the correct current task context
- Proof after repair:
  - `cross_` filtered suite passes `24/24`
  - `linux-compat-tests` remains passing
- Global repo health is still not passing:
  - repo lint check still fails on pre-existing SwiftLint and debt findings outside this tranche
  - type-check is toolchain-blocked in this environment because `clang-tidy` is unavailable
  - full unfiltered `ixland-core-tests` still does not complete within timeout, despite all localized signal/cross-signal clusters now passing in isolation/filter scope

## Repo-health stabilization-2 addendum

- Full-suite blocker narrowed to sequence-sensitive contamination involving `pgrp` ordering before `signal_killpg_basic_delivery`.
- First bad behavior source localized to `ixland-system/Tests/unit/test_pgrp_session.c` (`setpgid_fails_for_non_child`):
  - test manually rewired `task_table` for a task already registered by `ixland_task_alloc`, risking table corruption during later suite traversal.
- Bounded fix implemented:
  - removed manual `task_table` insertion/removal in `setpgid_fails_for_non_child` and relied on canonical allocator/free ownership.
- Post-fix proof:
  - sequence reproducer `pg` passes deterministically (`24/24`) across two runs.
  - `signal_killpg_basic_delivery` remains passing in isolation.
  - previously fixed exec tests remain passing.
  - `linux-compat-tests` remains passing.
- Remaining red state:
  - full unfiltered `ixland-core-tests` still does not complete within timeout in this environment even after reproducer fix.
  - repo lint/type-check status unchanged outside this tranche (pre-existing SwiftLint debt; missing `clang-tidy` toolchain).

## Repo-health stabilization-3 addendum

- Repo truth verified on current branch:
  - exec fix commit `ec9b3916d0a38c0fb1fb15810bab0d74deb3206f` present
  - stabilization-2 commit `c1c855f40b56daefca2d0bf3730045b9ac97d908` present
- Full-suite position moved materially forward:
  - no longer stalls at `signal_killpg_basic_delivery`
  - now reaches and deterministically reports concrete failures in file/poll/directory syscall clusters.
- Next first bad behavior localized to file syscall wrappers now owned under `ixland-system/src/ixland/fs/stat.c`:
  - `__ixland_stat_impl` and `__ixland_fstat_impl` were too restrictive for current mixed host/VFS test paths.
- Bounded product fix implemented:
  - `src/ixland/fs/stat.c::__ixland_stat_impl`: host `stat()` first, fallback to `ixland_vfs_stat()` on `ENOENT`
  - `src/ixland/fs/stat.c::__ixland_fstat_impl`: host `fstat()` first, fallback to ixland fd-table path when needed
- Post-fix proof:
  - `file_syscalls_integration` now passes in targeted run
  - `stat_existing_file_returns_success` and `fstat_open_file_returns_correct_size` now pass in targeted runs
  - exec regressions remain passing (`exec_native_happy_path`, `exec_cloexec_behavior`)
  - `linux-compat-tests` remains passing
- Remaining red state:
  - full unfiltered suite still reports deterministic failures, but now as explicit assertions (not sequence stall), primarily in poll and directory syscall tests.

## Repo-health stabilization-4 addendum

- Repo truth remained clean entering this tranche:
  - `ec9b3916d0a38c0fb1fb15810bab0d74deb3206f` present on `main`
  - `8b607b804b6a05a2e552ebdbff2f55bd559753b2` present on `main` and remote-visible
  - `src/ixland/fs/stat.c` contains the stabilization-3 stat/fstat fallback logic claimed by docs
- Full-suite deterministic next blocker cluster after stabilization-3:
  - poll and directory syscall assertions (no longer signal killpg sequence stall)
- Bounded stabilization-4 fixes implemented:
  - `ixland-system/src/ixland/core/ixland_poll.c`
    - `poll` negative fd entries now produce `POLLNVAL` as test contract expects
    - `epoll_ctl` now validates `epfd == fd` before generic fd validity checks to preserve `EINVAL` semantics
  - `ixland-system/Tests/unit/test_poll_syscalls.c`
    - `select_invalid_fd_returns_ebadf` now constructs a real invalid-FD set member (closed fd 0) instead of macro-ignored `FD_SET(-1, ...)`
- Post-fix proof:
  - `poll_invalid_fd_sets_nval` now passes
  - `epoll_ctl_epfd_equals_fd_returns_einval` now passes
  - `exec_native_happy_path` and `exec_cloexec_behavior` remain passing
  - `linux-compat-tests` remains passing
  - full unfiltered suite pass count moved from ~`337/347` to `339/347` in this environment
- Remaining red state:
  - `select_invalid_fd_returns_ebadf` still failing in full-suite context
  - directory syscall cluster still failing deterministically

## Fs-structure alignment addendum

- Canonical file syscall ownership no longer lives in `src/ixland/core/ixland_file_v2.c`.
- The canonical syscall core is now split into Linux-like semantic files under `ixland-system/src/ixland/fs/`:
  - `fdtable.c`
  - `open.c`
  - `read_write.c`
  - `stat.c`
  - `fcntl.c`
  - `ioctl.c`
- `ixland_file_v2.c` has been removed from canonical build use and deleted.
- CMake build truth now points `IXLAND_SYSCALL_SOURCES` at the new `src/ixland/fs/*` files.
- Targeted proof after cutover:
  - `exec_native_happy_path` PASS
  - `exec_cloexec_behavior` PASS
  - `file_syscalls_integration` PASS
  - `poll_invalid_fd_sets_nval` PASS
  - `epoll_ctl_epfd_equals_fd_returns_einval` PASS
  - `mkdir_new_directory_returns_success` PASS
  - `linux-compat-tests` PASS

## Pending for G3+

- No canonical semantic ownership uplifts performed.
- No session/workspace integration.
- No shell readiness exposure.
- G3 remains blocked pending broader repo-health stabilization beyond the exec and signal-sequence tranche fixes.

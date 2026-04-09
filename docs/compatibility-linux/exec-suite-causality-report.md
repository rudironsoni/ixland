# Exec-suite causality report (bounded)

## Scope and constraints

- Scope is limited to causality analysis for:
  - `ixland-system/Tests/unit/test_exec.c:62`
  - `ixland-system/Tests/unit/test_exec.c:171`
- This report does not implement G3, G4, or G5.
- Accepted baseline preserved:
  - G0 PASS
  - G1 PASS
  - G2 PASS (within tranche scope)
  - tracked-code rename gate PASS
  - global repository health NOT PASSING
  - G3 NO-GO

## Phase 1: Clean-room three-point replay matrix

Tested states via isolated `git worktree` instances, using unique build directories per point:

1. **Pre-tranche parent** (`90321e0ad38193721bcdb2f7bed6e447184acffd`)
   - Worktree path: `/tmp/ixland-replay-90321e0`
   - Build path: `/tmp/ixland-replay-90321e0/ixland-system/build-replay-90321e0`
   - Command: `cmake -S . -B build-replay-90321e0 -DIOX_BUILD_TESTS=ON && cmake --build build-replay-90321e0 --target iox-core-tests`
   - Configure: PASS
   - Build: FAIL
   - Test launch possible: NO
   - Exact blocker: `fatal error: 'ixland/wasm/types.h' file not found` compiling `runtime/wasi/wasm_adapter.c`.
2. **Scaffold tranche** (`87a8cc8fcadd85d65189dbb1687795badfaa3af7`)
   - Worktree path: `/tmp/ixland-replay-87a8cc8`
   - Build path: `/tmp/ixland-replay-87a8cc8/ixland-system/build-replay-87a8cc8`
   - Command: `cmake -S . -B build-replay-87a8cc8 -DIXLAND_BUILD_TESTS=ON && cmake --build build-replay-87a8cc8 --target ixland-core-tests`
   - Configure: FAIL
   - Build: FAIL
   - Test launch possible: NO
   - Exact blocker: `CMake Error at CMakeLists.txt:35: Standalone build requires ixland-libc headers at /tmp/ixland-replay-87a8cc8/ixland-system/../ixland-libc/include`. (The standalone CMake logic expects `ixland/ixland.h` which does not yet exist because libc headers were not renamed until the subsequent commit).
3. **Rename tranche / HEAD** (`420b821a4d24bd5256c3cebc1379d026c4b4e92c`)
   - Worktree path: `/tmp/ixland-replay-420b821`
   - Build path: `/tmp/ixland-replay-420b821/ixland-system/build-replay-420b821`
   - Command: `cmake -S . -B build-replay-420b821 -DIXLAND_BUILD_TESTS=ON && cmake --build build-replay-420b821 --target ixland-core-tests`
   - Configure: PASS
   - Build: PASS
   - Test launch possible: YES
   - Exact failing tests:
     - `exec_native_happy_path` (FAIL at `test_exec.c:62` with `(ret) == (0)`)
     - `exec_cloexec_behavior` (FAIL at `test_exec.c:171` with `(ret) == (0)`)

*Note: The previous replay attempt was invalid because it used `git checkout` inside the main working tree and reused a single build directory, leading to CMake cache/artifact contamination.*

## Phase 2: Scaffold vs rename diff isolation

For the exact intersecting failure-surface files, `87a8cc8` was compared against `420b821` using a token-normalized semantic diff (`iox`/`ixland` replacement).

| File | Changed between 87a8 and 420b? | Rename-only? | Semantic change? | Failure relevance |
|---|---|---|---|---|
| `ixland-system/Tests/unit/test_exec.c` | YES | YES | NO | NO (semantic equivalent) |
| `ixland-system/kernel/exec/exec.c` | YES | YES | NO | NO (semantic equivalent) |
| `ixland-system/runtime/native/registry.c` | YES | YES | NO | NO (semantic equivalent) |
| `ixland-system/fs/fdtable.c` | YES | YES | NO | NO (semantic equivalent) |
| `ixland-system/kernel/task/task.c` | YES | YES | NO | NO (semantic equivalent) |
| `ixland-system/CMakeLists.txt` | NO | NO | NO | NO (semantic equivalent) |

## Phase 3: Simulator-runner delta check

- Compared simulator-runner and test-launch wiring (`add_test(NAME core-tests COMMAND ...)`) between `87a8cc8` and `420b821`.
- Result: **Identical**. No `COMMAND` lines or runner configurations changed between the scaffold and rename tranches.

## Phase 4: Rename-fallout check

- Did exec failures depend on renamed paths, symbols, or targets?
- **Rename fallout is not implicated.** The file-by-file semantic audit proved zero logic changes across the rename tranche for the failure surface. The failures trigger on the exact same logic paths (e.g. `access(pathname, X_OK)`) that existed pre-rename.

## Phase 5: Host-boundary and simulator-runner check

- Do the failing exec tests traverse `ixland-system/runtime/linux_host/*` or `ixland-system/compatibility/linux/runtime/*`?
- **No.** The tests invoke `ixland_native_register` and `ixland_execve`, traversing only canonical native execution paths. They do not cross the `IxlandLinuxCompatibilityHost` boundary.
- Do the failing exec tests intersect simulator spawn runner changes?
- **Yes.** They run under `xcrun simctl spawn booted`, where the simulator filesystem environment lacks executability context for `/bin/testcmd`, causing the `access()` check in `ixland_execve` to return `-1`.

## Phase 6: Causality classification

- `test_exec.c:62` (`exec_native_happy_path`): **blocked by historical replay failure**, with exact blocker: `87a8` configure fails (`ixland-libc` headers not yet renamed) and `90321e0` build fails (`ixland/wasm/types.h` missing).
- `test_exec.c:171` (`exec_cloexec_behavior`): **blocked by historical replay failure**, with same exact blockers.
- Later full-suite `core-tests` stall/hang behavior: **environment-induced and not yet attributable**.

## Invariant preservation checks

- **Tracked-code rename gate**: PASS (0 tracked path hits, 0 source/header hits, 0 build-target hits outside documented exceptions).
- **Linux host boundary**: PASS (files exist, validation enforced, boundary test passes, app-side bypass searches return 0).
- **Ownership boundaries**: PASS (no duplicate canonical owners introduced under `compatibility/linux`).

## Exact next action

1. Acknowledge that direct `87a8` and `90321e0` replay are impossible without altering historical commits.
2. Proceed with bounded diagnostics around the simulator `access(pathname, X_OK)` environment behavior on current HEAD to prove whether the failures are pure environment-induced transport issues or semantic regressions.
3. Keep G3 blocked until that proof is complete.

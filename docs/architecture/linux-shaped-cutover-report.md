# Linux-shaped canonical ownership cutover report

## Scope

This report records the canonical syscall/filesystem ownership cutover in `ixland-system` and captures the resulting source-of-truth layout, drained duplicates, and proof commands.

## Canonical ownership after cutover

- Filesystem syscall-family ownership: `ixland-system/fs/*`
  - `open.c`
  - `read_write.c`
  - `stat.c`
  - `fcntl.c`
  - `ioctl.c`
  - `select.c`
  - `eventpoll.c`
  - `namei.c`
  - `readdir.c`
- Process identity ownership: `ixland-system/kernel/task/ixland_identity.c`

## Drained/removed duplicate owners

- Removed duplicate directory/link owner:
  - `ixland-system/src/ixland/core/ixland_directory.c`
- Removed duplicate legacy fd-table helper owner:
  - `ixland-system/fs/legacy_fdtable_helpers.c`
- Removed old fs split-brain source directory from canonical ownership:
  - `ixland-system/src/ixland/fs/*` (directory is empty)
- Removed old identity owner path:
  - `ixland-system/src/ixland/core/ixland_identity.c`

## Build truth

`ixland-system/CMakeLists.txt` `IXLAND_SYSCALL_SOURCES` now points at:

- `fs/open.c`
- `fs/read_write.c`
- `fs/stat.c`
- `fs/fcntl.c`
- `fs/ioctl.c`
- `fs/select.c`
- `fs/eventpoll.c`
- `fs/namei.c`
- `fs/readdir.c`
- `kernel/task/ixland_identity.c`

## Contract checks

- One canonical owner per syscall family in this tranche.
- No canonical `src/ixland/fs/*` owner left.
- Real canonical `fs/readdir.c` implementation present (`ixland_getdents64`, `ixland_getdents`).
- Host boundary remains under `ixland-system/runtime/linux_host/*`.

## Proof commands

Run from repository root:

```bash
cmake --preset ios-simulator
cmake --build --preset ios-simulator
ctest --preset ios-simulator --output-on-failure
./scripts/lint.sh --check
./scripts/lint.sh --type-check
```

## Status

- Cutover structure is in place and duplicate owners were drained from core/fs split points.
- Build proof: `cmake --preset ios-simulator-debug` and `cmake --build --preset ios-simulator-debug` in `ixland-system/` succeeded.
- Test proof in this environment: `ctest --test-dir build -C Debug --output-on-failure` failed at simulator launch (`xcrun simctl spawn booted`) with LaunchdSimError 111 / SimXPCErrorDomain 111 (`Invalid or missing Program/ProgramArguments`), so tests did not execute userland assertions.
- Repo lint/type-check gates remain red due pre-existing repo-wide issues and missing local `clang-tidy` tooling.

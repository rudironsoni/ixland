# Linux-Shaped Canonical Ownership Cutover Report

## Current Truth

This repository is in the IXLand component-root naming state:

- `IXLand/`
- `IXLandSystem/`
- `IXLandLibC/`
- `IXLandWasm/`
- `IXLandPackages/`
- `IXLandToolchain/`

The app lives at repo root under `IXLand/`.

## Build Truth

The only active build graph is:

- `IXLand/IXLand.xcodeproj`
- shared schemes inside that project
- `xcodebuild` as the only CLI build and test authority

CMake, CTest, and Make are not active build truth.

## Canonical Owner Tree

Active syscall-facing and kernel-facing ownership lives under `IXLandSystem/`.

### Filesystem

- `IXLandSystem/fs/fdtable.c`
- `IXLandSystem/fs/open.c`
- `IXLandSystem/fs/read_write.c`
- `IXLandSystem/fs/stat.c`
- `IXLandSystem/fs/fcntl.c`
- `IXLandSystem/fs/ioctl.c`
- `IXLandSystem/fs/namei.c`
- `IXLandSystem/fs/readdir.c`
- `IXLandSystem/fs/select.c`
- `IXLandSystem/fs/eventpoll.c`
- `IXLandSystem/fs/exec.c`
- `IXLandSystem/fs/path.c`
- `IXLandSystem/fs/mount.c`
- `IXLandSystem/fs/inode.c`
- `IXLandSystem/fs/super.c`

### Kernel

- `IXLandSystem/kernel/fork.c`
- `IXLandSystem/kernel/exit.c`
- `IXLandSystem/kernel/pid.c`
- `IXLandSystem/kernel/cred.c`
- `IXLandSystem/kernel/sys.c`
- `IXLandSystem/kernel/signal.c`
- `IXLandSystem/kernel/time.c`
- `IXLandSystem/kernel/resource.c`
- `IXLandSystem/kernel/random.c`
- `IXLandSystem/kernel/sync.c`
- `IXLandSystem/kernel/task.c`
- `IXLandSystem/kernel/wait.c`
- `IXLandSystem/kernel/init.c`
- `IXLandSystem/kernel/libc_delegate.c`
- `IXLandSystem/kernel/net/network.c`

## Coherence Rules

- No active lowercase component roots
- No active `a-Shell`
- No active donor-derived `compatibility/linux` lane
- No active `src/ixland/core/*` or `src/ixland/fs/*` canonical ownership
- No CMake, CTest, or Make as active build truth

## Status

This document describes current repository structure only. Historical migration details belong in explicitly historical documents and do not override the active Xcode-only build graph.

# IXLand Linux-Shaped Canonical Reclassification

## Repository Truth

- Active app root: `IXLand/`
- Active system root: `IXLandSystem/`
- Active libc root: `IXLandLibC/`
- Active wasm root: `IXLandWasm/`
- Active packages root: `IXLandPackages/`
- Active toolchain root: `IXLandToolchain/`
- Active build graph: `IXLand/IXLand.xcodeproj`
- Active CLI build authority: `xcodebuild`

## Canonical Linux-Shaped Owners

### Filesystem Domain

| Current Path | Role | Bucket | Final Target | Final Owner | Action | Reason |
|---|---|---|---|---|---|---|
| `IXLandSystem/fs/fdtable.c` | FD table | Canonical Linux-shaped | `IXLandSystem/fs/fdtable.c` | fd/file-description layer | keep | single canonical owner |
| `IXLandSystem/fs/open.c` | open family | Canonical Linux-shaped | `IXLandSystem/fs/open.c` | open | keep | single canonical owner |
| `IXLandSystem/fs/read_write.c` | read/write family | Canonical Linux-shaped | `IXLandSystem/fs/read_write.c` | read/write | keep | single canonical owner |
| `IXLandSystem/fs/stat.c` | stat family | Canonical Linux-shaped | `IXLandSystem/fs/stat.c` | stat | keep | single canonical owner |
| `IXLandSystem/fs/fcntl.c` | fcntl family | Canonical Linux-shaped | `IXLandSystem/fs/fcntl.c` | fcntl | keep | single canonical owner |
| `IXLandSystem/fs/ioctl.c` | ioctl family | Canonical Linux-shaped | `IXLandSystem/fs/ioctl.c` | ioctl | keep | single canonical owner |
| `IXLandSystem/fs/namei.c` | pathname lookup | Canonical Linux-shaped | `IXLandSystem/fs/namei.c` | namei | keep | single canonical owner |
| `IXLandSystem/fs/readdir.c` | directory enumeration | Canonical Linux-shaped | `IXLandSystem/fs/readdir.c` | readdir | keep | single canonical owner |
| `IXLandSystem/fs/select.c` | poll/select family | Canonical Linux-shaped | `IXLandSystem/fs/select.c` | select | keep | single canonical owner |
| `IXLandSystem/fs/eventpoll.c` | epoll family | Canonical Linux-shaped | `IXLandSystem/fs/eventpoll.c` | eventpoll | keep | single canonical owner |
| `IXLandSystem/fs/exec.c` | exec dispatch | Canonical Linux-shaped | `IXLandSystem/fs/exec.c` | exec | keep | single canonical owner |
| `IXLandSystem/fs/path.c` | cwd/path operations | Canonical Linux-shaped | `IXLandSystem/fs/path.c` | path | keep | single canonical owner |
| `IXLandSystem/fs/mount.c` | mount/statfs | Canonical Linux-shaped | `IXLandSystem/fs/mount.c` | mount | keep | single canonical owner |
| `IXLandSystem/fs/inode.c` | chmod/chown/access/truncate | Canonical Linux-shaped | `IXLandSystem/fs/inode.c` | inode | keep | single canonical owner |
| `IXLandSystem/fs/super.c` | sync/superblock operations | Canonical Linux-shaped | `IXLandSystem/fs/super.c` | super | keep | single canonical owner |
| `IXLandSystem/src/ixland/fs/*` | dead split-brain layout | Delete from current branch | - | - | delete | dead legacy ownership |

### Kernel Domain

| Current Path | Role | Bucket | Final Target | Final Owner | Action | Reason |
|---|---|---|---|---|---|---|
| `IXLandSystem/kernel/task.c` | task core | Canonical Linux-shaped | `IXLandSystem/kernel/task.c` | task | keep | single canonical owner |
| `IXLandSystem/kernel/fork.c` | fork/vfork | Canonical Linux-shaped | `IXLandSystem/kernel/fork.c` | fork | keep | single canonical owner |
| `IXLandSystem/kernel/exit.c` | exit family | Canonical Linux-shaped | `IXLandSystem/kernel/exit.c` | exit | keep | single canonical owner |
| `IXLandSystem/kernel/wait.c` | wait family | Canonical Linux-shaped | `IXLandSystem/kernel/wait.c` | wait | keep | single canonical owner |
| `IXLandSystem/kernel/pid.c` | pid allocator | Canonical Linux-shaped | `IXLandSystem/kernel/pid.c` | pid | keep | single canonical owner |
| `IXLandSystem/kernel/cred.c` | credentials | Canonical Linux-shaped | `IXLandSystem/kernel/cred.c` | cred | keep | single canonical owner |
| `IXLandSystem/kernel/sys.c` | syscall misc surface | Canonical Linux-shaped | `IXLandSystem/kernel/sys.c` | sys | keep | single canonical owner |
| `IXLandSystem/kernel/signal.c` | signal delivery | Canonical Linux-shaped | `IXLandSystem/kernel/signal.c` | signal | keep | single canonical owner |
| `IXLandSystem/kernel/time.c` | time syscalls | Canonical Linux-shaped | `IXLandSystem/kernel/time.c` | time | keep | single canonical owner |
| `IXLandSystem/kernel/resource.c` | rlimit/rusage | Canonical Linux-shaped | `IXLandSystem/kernel/resource.c` | resource | keep | single canonical owner |
| `IXLandSystem/kernel/random.c` | entropy/random | Canonical Linux-shaped | `IXLandSystem/kernel/random.c` | random | keep | single canonical owner |
| `IXLandSystem/kernel/sync.c` | synchronization | Canonical Linux-shaped | `IXLandSystem/kernel/sync.c` | sync | keep | single canonical owner |
| `IXLandSystem/kernel/net/network.c` | network subsystem | Canonical Linux-shaped | `IXLandSystem/kernel/net/network.c` | net | keep | single canonical owner |
| `IXLandSystem/src/ixland/core/*` | dead core bucket | Delete from current branch | - | - | delete | dead legacy ownership |

### Product-Native Domain

| Current Path | Role | Bucket | Final Target | Final Owner | Action | Reason |
|---|---|---|---|---|---|---|
| `IXLand/*` | iOS app | Product-native | `IXLand/*` | app | keep | root app owner |
| `IXLandLibC/*` | libc boundary | Product-native | `IXLandLibC/*` | libc | keep | named component root |
| `IXLandWasm/*` | wasm boundary | Product-native | `IXLandWasm/*` | wasm | keep | named component root |
| `IXLandPackages/*` | packages | Product-native | `IXLandPackages/*` | packages | keep | named component root |
| `IXLandToolchain/*` | toolchain metadata | Product-native | `IXLandToolchain/*` | toolchain | keep | named component root |
| `docs/*` | documentation | Product-native | `docs/*` | docs | keep | non-kernel surface |
| `scripts/*` | maintenance tooling | Product-native | `scripts/*` | scripts | keep | non-kernel surface |

## Build Truth

- Xcode project/workspace plus shared schemes are the only active build graph
- `xcodebuild` is the only CLI build and test authority
- CMake, CTest, and Make are forbidden as active build truth

## Semantic Decisions

- Path-based syscalls: one IXLand contract under `IXLandSystem/fs/namei.c`
- FD-based syscalls: one IXLand contract under `IXLandSystem/fs/fdtable.c`, `open.c`, and `read_write.c`
- iOS adaptation stays subordinate inside the canonical owner that needs it

# Ownership boundaries (G0-G2)

## Canonical ixland-system ownership

- task
- signal
- exec orchestration
- VFS
- fd
- mount/path/inode/stat
- /proc
- /dev
- TTY/PTY
- pipe
- socket
- poll
- time
- random
- resource
- sync primitives
- unified task graph
- runtime selection and lifecycle

## Bounded linux compatibility ownership

- Linux ABI
- ELF/interpreter loader
- syscall translation
- guest runtime
- distro/rootfs lifecycle
- AArch64 emulation
- TCTI
- Linux process projection
- Linux signal translation
- Linux fs translation
- Linux /proc views
- Linux /dev views

## Host boundary ownership

- ixland-system/runtime/linux_host
- ixland_linux_compatibility_host_* API is the only entry to linux runtime bootstrap

## ixland-app ownership

- workspace UX
- session UX
- terminal UX
- file-access UX
- credentials UX
- remote UX
- Compatibility/LinuxSessionAdapter only

## Prohibitions

- No duplicate semantic owners under compatibility/linux.
- No donor UI runtime bootstrap calls.
- No donor build artifacts as monorepo truth.

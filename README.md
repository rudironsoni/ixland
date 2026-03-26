# a-shell-next

This repository is now the monorepo for the a-shell project.

## Repository Structure

This monorepo contains:
- `ixland-app/` - iOS terminal application (imported from a-shell-app with history preserved)
- `ixland-system/` - Core kernel/system layer (imported from a-shell-kernel with history preserved)
- `ixland-packages/` - Package build system (imported from a-shell-packages with history preserved)
- `ixland-libc/` - C library boundary (skeletal, for future split)
- `ixland-wasm/` - WebAssembly boundaries (skeletal, contains ixland-wasm-engine, ixland-wasm-host, ixland-wasm-wasi)
- `ixland-toolchain/` - Toolchain boundary (skeletal, for future toolchain integration)

## Setup

Submodules are no longer used. Use the monorepo root for checkout and build.

```bash
git clone git@github.com:rudironsoni/a-shell-next.git
cd a-shell-next
# No submodule initialization required
```

## Build

Build orchestration is expected to flow through CMake.
See individual component READMEs for specific build instructions.

## CI

CI configuration lives in `.github/workflows/`.

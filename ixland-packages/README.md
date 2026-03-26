> [!IMPORTANT]
> This component now lives inside the `a-shell-next` monorepo.
> Do not run `git submodule update --init --recursive`.
> Use the monorepo root checkout and see the root `README.md` for current setup and build instructions.
> Historical instructions below may be outdated.

# ixland-packages

Package build system for the iXland environment.

## Overview

This directory contains the package management and build infrastructure for a-shell, imported from `rudironsoni/a-shell-packages` with full Git history preserved.

## Purpose

- Termux-style package build system
- Package definitions and build scripts
- Dependency resolution for iXland components
- Future: compiled Wasm artifact management

## Structure

```
ixland-packages/
├── packages/           # Package definitions
├── scripts/           # Build automation
├── docker/            # Containerized builds (optional)
└── docs/              # Package documentation
```

## Build System

Package builds are orchestrated through the monorepo's CMake-based build system.

## Future Work

- Wasm artifact compilation and caching
- Package repository management
- Cross-compilation toolchain integration

## Documentation

- Root `README.md` - Monorepo setup and build instructions
- `docs/` - Package-specific documentation

## License

[License information]

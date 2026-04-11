# IXLandPackages

Package build system and distribution for the IXLand environment.

## Overview

This directory contains the package management infrastructure for IXLand, providing build recipes for native iOS packages and future WASM packages.

## Contents

- `packages/` - Package definitions and build scripts
  - `packages/core/` - Core native packages (bash, coreutils, etc.)
  - `packages/wasm/` - WebAssembly packages (future)
- `core-packages/` - Core package definitions
- `root-packages/` - Root-level packages
- `scripts/` - Build automation scripts
- `wheel-index/` - Python wheel index

## Build System

Package builds are integrated with the monorepo's Xcode-based build system. See the root `README.md` and `docs/ARCHITECTURE.md` for build instructions.

## Wasm Package Layout

Future compiled Wasm artifacts will be managed as a package concern within this component.

See `docs/WASM_PACKAGE_LAYOUT.md` for the detailed specification of:
- Wasm package directory structure
- Artifact naming conventions
- Metadata schema
- Validation rules
- Integration with the build system

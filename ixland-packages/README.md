# ixland-packages

Package build system and distribution for the iXland environment.

## Overview

This directory contains the package management infrastructure for iXland, providing Termux-style build recipes for native XCFrameworks and future WASM packages.

## Contents

- `packages/` - Package definitions and build scripts
- `core-packages/` - Core package definitions
- `root-packages/` - Root-level packages
- `scripts/` - Build automation scripts
- `wheel-index/` - Python wheel index

## Build System

Package builds are integrated with the monorepo's CMake-based build system. See the root `README.md` and `docs/ARCHITECTURE.md` for build instructions.

## Compiled Artifacts

Future compiled Wasm artifacts will be managed as a package concern within this component.

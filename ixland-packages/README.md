# ixland-packages

Package build system and distribution for the iXland environment.

## Overview

This directory contains the package management infrastructure for a-shell, imported from `rudironsoni/a-shell-packages` with full Git history preserved.

## Contents

- `packages/` - Package definitions and build scripts
- `core-packages/` - Core package definitions
- `root-packages/` - Root-level packages
- `scripts/` - Build automation scripts
- `wheel-index/` - Python wheel index

## Build System

Package builds are integrated with the monorepo's CMake-based build system. See the root `README.md` for build instructions.

## Note

This component now lives inside the `a-shell-next` monorepo. Do not run `git submodule update --init --recursive`. Use the monorepo root checkout and see the root `README.md` for current setup and build instructions.

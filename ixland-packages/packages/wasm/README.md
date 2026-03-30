# Wasm Package Layout

This directory contains WebAssembly module packages for iXland.

## Directory Structure

```
wasm/
├── README.md           # This file
├── *.wasm              # Wasm module files
└── metadata.json       # Package metadata
```

## Build Output

During package builds, Wasm artifacts are staged in:
```
.build/wasm/<PKG>/staging/
```

## Naming Conventions

- `*.wasm` - Compiled WebAssembly module files
- `metadata.json` - Package manifest with version and dependencies

## Portable vs Engine-Specific

- **Portable Wasm**: Raw `.wasm` files (engine-agnostic)
- **Engine-Specific**: Future AOT-compiled artifacts (per-backend)

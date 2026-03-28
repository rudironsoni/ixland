# Wasm Packages

This directory is reserved for future WebAssembly packages.

See [docs/WASM_PACKAGE_LAYOUT.md](../../../docs/WASM_PACKAGE_LAYOUT.md) for the full specification.

## Future Layout

```
wasm/
├── README.md
├── <package-name>/
│   ├── build.sh          # Package build script
│   ├── metadata.json     # Package metadata
│   └── patches/          # Wasm-specific patches
│       └── 01-fix.patch
└── ...
```

## Package Structure

Each Wasm package contains:
- `build.sh` - Build script (sourced by build system)
- `metadata.json` - Package metadata with schema defined in WASM_PACKAGE_LAYOUT.md
- `patches/` - Optional patches directory

## Naming Conventions

- Package directory: lowercase, hyphens allowed (e.g., `coreutils`, `lua-runtime`)
- Wasm artifacts: `<package>-<version>.wasm` or `<package>.wasm`
- Metadata: `metadata.json`

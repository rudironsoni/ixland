# ixland-packages WASM Layout

This directory contains WebAssembly (WASM) package definitions and validation for iXland packages.

## Directory Structure

```
ixland-packages/packages/wasm/
├── README.md              # This file
├── layout.json            # WASM layout schema definition
└── validate.sh            # Layout validation script
```

## WASM Package Layout

WASM packages in iXland follow a strict layout for consistency and security:

### Required Structure

```
<package-name>.wasm/
├── bin/
│   └── <command-name>     # WASM executable (required)
├── lib/
│   └── *.wasm             # WASM library dependencies (optional)
├── share/
│   ├── doc/
│   │   └── README.md      # Package documentation (optional)
│   └── man/
│       └── man1/
│           └── <command>.1 # Man page (optional)
└── manifest.json          # Package metadata (required)
```

### manifest.json Schema

```json
{
  "name": "string",           // Package name (required)
  "version": "string",        // SemVer version (required)
  "description": "string",    // Brief description (required)
  "author": "string",         // Author or maintainer (optional)
  "license": "string",        // SPDX license identifier (required)
  "homepage": "string",       // Project URL (optional)
  "entrypoint": "string",     // Relative path to main binary (required)
  "arch": ["wasm32"],         // Supported architectures (required)
  "dependencies": {           // Dependency mapping (optional)
    "<dep-name>": "<version-constraint>"
  },
  "capabilities": [           // Required WASI capabilities (optional)
    "filesystem",
    "network",
    "stdio"
  ]
}
```

## Validation Rules

1. **Binary Integrity**: All WASM binaries must be valid WebAssembly modules
2. **Manifest Presence**: Every package must include a valid `manifest.json`
3. **Entrypoint Validity**: The entrypoint must reference an existing executable
4. **Architecture Check**: Only `wasm32` architecture is supported
5. **Dependency Resolution**: All declared dependencies must be resolvable
6. **Capability Whitelist**: Only declared WASI capabilities may be used

## Running Validation

```bash
# Validate all WASM packages
cmake --build --preset local-dev --target ixland-packages-validate

# Validate specific package
./validate.sh <package-name>
```

## Security Considerations

- All WASM modules are sandboxed by the iXland runtime
- File system access is restricted to package-specific directories
- Network access requires explicit capability declaration
- Capabilities are enforced at the runtime level

## References

- [WebAssembly Spec](https://webassembly.github.io/spec/)
- [WASI](https://wasi.dev/)
- [iXland Package System](../../README.md)

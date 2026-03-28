# Wasm Package Layout Specification

## Overview

This document defines the expected directory structure and conventions for WebAssembly (Wasm) packages within the iXland package system. This is a forward-looking specification for future artifact support.

## Design Rationale

Wasm packages follow a layout parallel to native XCFramework packages, allowing the same build infrastructure to support both native and WebAssembly targets. This enables:

- Single source packages that build for both native iOS and Wasm
- Shared metadata and patch infrastructure
- Unified package discovery and validation
- Future artifact caching and distribution

## Directory Structure

```
ixland-packages/
├── packages/
│   ├── core/
│   │   ├── bash/               # Native package (existing)
│   │   ├── coreutils/          # Native package (existing)
│   │   └── ...
│   └── wasm/                   # Wasm packages directory
│       ├── coreutils/          # Wasm-compiled coreutils
│       │   ├── build.sh        # Package build script
│       │   ├── patches/        # Patches for Wasm compatibility
│       │   └── metadata.json   # Package metadata
│       ├── lua/                # Wasm Lua runtime
│       ├── quickjs/            # Wasm QuickJS
│       └── ...
├── core-packages/
│   └── python/                 # Core packages (native)
├── root-packages/
│   └── wamr/                   # Root-level packages
└── wheel-index/                # Python wheel index
```

## Wasm Package Structure

Each Wasm package resides in `packages/wasm/<name>/` and contains:

### Required Files

| File | Purpose |
|------|---------|
| `build.sh` | Package build script (sourced by build system) |
| `metadata.json` | Package metadata (name, version, description, deps) |

### Optional Directories

| Directory | Purpose |
|-----------|---------|
| `patches/` | Patches applied before build (Wasm-specific) |
| `src/` | Source files (if not downloaded from URL) |
| `assets/` | Static assets bundled with package |

### metadata.json Schema

```json
{
  "name": "coreutils",
  "version": "9.4",
  "description": "GNU Core Utilities compiled for Wasm",
  "type": "wasm",
  "arch": "wasm32-wasi",
  "dependencies": [],
  "source": {
    "url": "https://ftp.gnu.org/gnu/coreutils/coreutils-9.4.tar.gz",
    "sha256": "..."
  },
  "artifact": {
    "name": "coreutils.wasm",
    "entrypoint": "_start",
    "exports": ["ls", "cat", "cp", "mv", "rm"]
  },
  "wasi": {
    "version": "preview1",
    "features": ["wasi-cli"]
  }
}
```

## Artifact Naming Conventions

### Wasm Module Files

- Extension: `.wasm` (standard WebAssembly binary format)
- Naming: `<package>-<version>.wasm` or `<package>.wasm` (for latest)
- Location: `.build/wasm/<package>/staging/`

### Example Artifacts

```
.build/wasm/
├── coreutils/
│   └── staging/
│       └── coreutils-9.4.wasm
├── lua/
│   └── staging/
│       ├── lua-5.4.6.wasm
│       └── luac-5.4.6.wasm
└── quickjs/
    └── staging/
        ├── qjs.wasm
        └── qjsc.wasm
```

## Build Output Structure

When built, Wasm packages produce:

```
.build/wasm/<package>/
├── staging/
│   ├── <package>.wasm          # Main Wasm module
│   ├── metadata.json           # Copied/validated metadata
│   └── resources/              # Any bundled resources
├── tmp/
│   └── ...                     # Build intermediates
└── log/
    └── build.log               # Build log
```

## Validation Rules

The `ixland-packages-validate` target checks for:

1. **Directory presence**: `packages/wasm/` exists
2. **Package structure**: Each Wasm package has `build.sh` and `metadata.json`
3. **Metadata validity**: JSON schema validation
4. **Naming conventions**: Package names follow iXland conventions

## Integration with Build System

Wasm packages integrate with the existing build system:

- Use `ixland_package.sh` library (same as native packages)
- Support `ixland_step_*` build functions
- Patches applied from `patches/` directory
- Metadata validated during `ixland-packages-validate`

## Future Considerations

### Artifact Distribution

Future versions may support:
- Pre-compiled artifact caching
- Registry-based package distribution
- Version pinning and resolution

### WASI Versions

The specification supports multiple WASI versions:
- `preview0` - Legacy WASI snapshot
- `preview1` - Current stable WASI
- `preview2` - Component Model (future)

### Multi-Target Packages

Packages may support both native and Wasm from a single source:

```
packages/core/coreutils/
├── build.sh          # Common build configuration
├── patches/
│   ├── 01-fix.patch  # Common patches
│   └── wasm/         # Wasm-specific patches
├── metadata.json     # Package metadata
└── targets/
    ├── ios/          # iOS-specific configuration
    ├── simulator/    # Simulator configuration
    └── wasm/         # Wasm configuration
```

## Relationship to Other Components

- **ixland-wasm**: Provides the Wasm runtime engine
- **ixland-system**: Provides host services (WASI implementation)
- **ixland-packages**: This component - package build and distribution

## Migration Path

Native packages can be ported to Wasm by:

1. Creating `packages/wasm/<name>/` directory
2. Adapting `build.sh` for Wasm target (using WASI SDK)
3. Adding Wasm-specific patches if needed
4. Creating `metadata.json` with `type: "wasm"`

The parallel structure allows gradual migration without disrupting native packages.

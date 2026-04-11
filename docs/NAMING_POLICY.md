# iXland Naming Policy

**Version:** 1.0
**Last Updated:** 2026-03-30

This document defines the canonical naming conventions for the iXland project. All new code must follow these conventions, and legacy naming should be migrated to these standards.

## Overview

iXland uses consistent naming patterns across all languages and components to ensure clarity, maintainability, and namespace isolation.

## C Code Naming Conventions

### Public Functions

| Pattern | Example | Description |
|---------|---------|-------------|
| `ixland_` prefix | `ixland_task_alloc()`, `ixland_vfs_init()` | Core system functions |
| `ixland_` prefix | `ixland_wasm_engine_create()` | High-level iXland APIs |
| `vfs_` prefix | `vfs_translate_path()` | VFS-specific functions |

**Rationale:** The `ixland_` prefix provides namespace isolation and makes iXland APIs instantly recognizable.

### Types and Structs

| Construct | Pattern | Example |
|-----------|---------|---------|
| **Struct types** | `ixland_lowercase_t` | `ixland_task_t`, `ixland_files_t` |
| **Typedefs** | `ixland_lowercase_t` | `ixland_pid_t`, `ixland_mode_t` |
| **Enum types** | `ixland_lowercase` | `ixland_task_state_t` |

### Macros and Constants

| Construct | Pattern | Example |
|-----------|---------|---------|
| **Macros** | `IXLAND_UPPER_CASE` | `IXLAND_MAX_NAME`, `IXLAND_NSIG` |
| **Enum values** | `IXLAND_PREFIX_VALUE` | `IXLAND_TASK_RUNNING`, `IXLAND_MAX_FD` |
| **Constants** | `IXLAND_UPPER_CASE` | `IXLAND_MAX_PATH`, `IXLAND_MAX_ARGS` |
| **Package variables** | `IXLAND_UPPER_CASE` | `IXLAND_PKG_NAME`, `IXLAND_PREFIX` |

### Include Guards

| Header Location | Pattern | Example |
|-------------------|---------|---------|
| `include/ixland/*.h` | `IXLAND_FILENAME_H` | `IXLAND_VFS_H`, `IXLAND_TASK_H` |
| `include/*.h` (standard) | `_IXLAND_FILENAME_H` | `_IXLAND_PWD_H`, `_IXLAND_GRP_H` |
| `include/linux/*.h` | `_LINUX_FILENAME_H` | `_LINUX_TYPES_H` |

## Shell Script Naming Conventions

### Environment Variables

| Pattern | Example | Description |
|---------|---------|-------------|
| `IXLAND_` prefix | `IXLAND_PREFIX`, `IXLAND_PKG_NAME` | Build system variables |
| `IXLAND_` prefix | `IXLAND_MAX_FDS` | Runtime configuration |

### Function Names

| Pattern | Example | Description |
|---------|---------|-------------|
| `ixland_` prefix | `ixland_step_configure()` | Build step functions |
| `ixland_` prefix | `ixland_info()`, `ixland_error()` | Utility functions |
| `ixland_pkg_` prefix | `ixland_pkg_configure()` | Package-specific hooks |

### Directory Variables

| Pattern | Example | Description |
|---------|---------|-------------|
| `IXLAND_` prefix | `IXLAND_PKG_BUILD_DIR` | Build directories |
| `IXLAND_` prefix | `IXLAND_PKG_STAGING` | Staging directories |

## Swift Code Naming Conventions

Swift code follows standard Swift guidelines:

| Construct | Pattern | Example |
|-----------|---------|---------|
| **Types (classes/structs/enums)** | `PascalCase` | `ContentView`, `TerminalManager` |
| **Protocols** | `PascalCase` | `CommandExecutable` |
| **Functions** | `camelCase` | `loadView()`, `executeCommand()` |
| **Variables/Properties** | `camelCase` | `currentDirectory`, `taskCount` |
| **Constants** | `camelCase` | `defaultTimeout`, `maxBufferSize` |
| **Enum Cases** | `camelCase` | `.running`, `.completed` |

## Directory and File Naming

| Type | Pattern | Example |
|------|---------|---------|
| **Component directories** | `IXLand{Name}` | `IXLandSystem`, `IXLandLibC`, `IXLandWasm`, `IXLandPackages`, `IXLandToolchain` |
| **Sub-component directories** | `ixland-{name}` | `ixland-wasm-engine` (legacy), `ixland-packages-validate` |
| **Package directories** | `IXLandPackages/` | - |
| **Toolchain directories** | `IXLandToolchain/` | - |
| **Build scripts** | `build.sh`, `clean.sh` | - |
| **Script libraries** | `ixland_*.sh` | `ixland_package.sh` |

## Legacy Patterns (Deprecated)

The following patterns have been deprecated and should not be used:

| Legacy Pattern | Replacement | Status |
|----------------|-------------|--------|
| `a_shell_` | `ixland_` | ✅ Migrated |
| `ashell_` | `ixland_` | ✅ Migrated |
| `ashell-` | `ixland-` | ✅ Migrated |
| `a-shell` | `ixland` | ✅ Migrated |
| `A_SHELL_` | `IXLAND_` or `IXLAND_` | ✅ Migrated |
| `_A_SHELL_` | `_IXLAND_` or `IXLAND_` | ✅ Migrated |
| `a_shell_pkg_*` | `ixland_pkg_*` | ✅ Migrated |
| `A_SHELL_PACKAGES_DIR` | `IXLAND_PACKAGES_DIR` | ✅ Migrated |
| `A_SHELL_BUILD_TARGET` | `IXLAND_BUILD_TARGET` | ✅ Migrated |

## Enforcement

Naming conventions are enforced via:

- **clang-tidy**: Checks C/C++ naming on every build
- **CI validation**: `naming-c` job validates conventions
- **Pre-commit hooks**: Check staged files for violations

## Migration Guidelines

When migrating legacy code:

1. Rename functions and variables to use `ixland_` or `ixland_` prefixes
2. Update macro constants to use `IXLAND_` or `IXLAND_` prefixes
3. Update include guards to follow documented patterns
4. Update shell script variables to use `IXLAND_` prefix
5. Update package build hooks from `a_shell_pkg_*` to `ixland_pkg_*`

## Examples

### C Function
```c
// ✅ Correct
ixland_task_t *ixland_task_alloc(const char *name);

// ❌ Incorrect (legacy)
a_shell_task_t *a_shell_task_alloc(const char *name);
```

### Shell Script Variable
```bash
# ✅ Correct
IXLAND_PKG_NAME="bash"
IXLAND_PREFIX="/usr/local"

# ❌ Incorrect (legacy)
A_SHELL_PKG_NAME="bash"
```

### Shell Script Function
```bash
# ✅ Correct
ixland_pkg_configure() {
    ./configure --prefix="$IXLAND_PREFIX"
}

# ❌ Incorrect (legacy)
a_shell_pkg_configure() {
    ./configure --prefix="$A_SHELL_PREFIX"
}
```

## Summary

| Category | Prefix | Example |
|----------|--------|---------|
| C Functions | `ixland_` | `ixland_task_alloc()` |
| C Types | `ixland_` + `_t` | `ixland_task_t` |
| C Macros | `IXLAND_` | `IXLAND_MAX_PATH` |
| Shell Variables | `IXLAND_` | `IXLAND_PKG_NAME` |
| Shell Functions | `ixland_` | `ixland_pkg_configure()` |
| Swift Types | `PascalCase` | `TerminalManager` |
| Swift Functions | `camelCase` | `executeCommand()` |
| Directories | `ixland-` | `ixland-system/` |

---

*This document is part of the iXland project. For questions or updates, see the project repository.*

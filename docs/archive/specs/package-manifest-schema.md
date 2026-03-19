# Package Manifest Schema

Specification for a-Shell package manifests (build.sh variables).

## Overview

Package manifests are bash scripts that define metadata and build configuration. The build system (ashell_package.sh) sources these files and uses the variables to control the build process.

## Format

```bash
#!/bin/bash
# Package manifest for <name>

# Required variables
ASHELL_PKG_NAME="pkgname"
ASHELL_PKG_VERSION="1.0.0"
ASHELL_PKG_SRCURL="https://example.com/source.tar.gz"
ASHELL_PKG_SHA256="sha256_hash"

# Optional variables
ASHELL_PKG_DEPENDS="dep1 dep2"
ASHELL_PKG_BUILD_DEPENDS="cmake"
ASHELL_PKG_COMMANDS="cmd1:entry1::type1 cmd2:entry2::type2"

# Source the build system
source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"
```

## Required Variables

### ASHELL_PKG_NAME

- **Type**: String
- **Pattern**: `^[a-z][a-z0-9-]*$` (lowercase, alphanumeric, hyphens)
- **Example**: `hello`, `coreutils-minimal`, `lib-z`
- **Description**: Package identifier used for directories, filenames, and references

### ASHELL_PKG_VERSION

- **Type**: String
- **Pattern**: Semantic versioning preferred
- **Example**: `1.0.0`, `2.12.1`, `3.2.2-beta1`
- **Description**: Version string displayed in listings and used for caching

### ASHELL_PKG_SRCURL

- **Type**: String (URL)
- **Format**: `https://`, `http://`, `file://`, or `git+https://`
- **Example**: `https://ftp.gnu.org/gnu/hello/hello-2.12.1.tar.gz`
- **Description**: Source archive download URL

### ASHELL_PKG_SHA256

- **Type**: String (hex)
- **Pattern**: `^[a-f0-9]{64}$`
- **Example**: `cf04e86aa1c5a3fb29789eb6575f85096f32a286f73497926ab539c21185cfa7`
- **Description**: SHA256 checksum of source archive for verification

## Optional Variables

### ASHELL_PKG_DESCRIPTION

- **Type**: String
- **Max Length**: 200 characters
- **Example**: `GNU Hello - A program that prints a friendly greeting`
- **Description**: Brief description for package listings

### ASHELL_PKG_HOMEPAGE

- **Type**: String (URL)
- **Example**: `https://www.gnu.org/software/hello/`
- **Description**: Project homepage URL

### ASHELL_PKG_LICENSE

- **Type**: String
- **Examples**: `GPL-3.0`, `MIT`, `BSD-3-Clause`, `Apache-2.0`
- **Description**: SPDX license identifier

### ASHELL_PKG_DEPENDS

- **Type**: Space-separated list
- **Example**: `libz libssl coreutils`
- **Description**: Runtime dependencies (packages required to run)

### ASHELL_PKG_BUILD_DEPENDS

- **Type**: Space-separated list
- **Example**: `cmake automake libtool`
- **Description**: Build-time dependencies (packages required to build)

### ASHELL_PKG_EXTRA_CONFIGURE_ARGS

- **Type**: String
- **Example**: `--disable-nls --enable-static --prefix=@ASHELL_PREFIX@`
- **Description**: Extra arguments passed to configure

### ASHELL_PKG_EXTRA_MAKE_ARGS

- **Type**: String
- **Example**: `V=1 -j4`
- **Description**: Extra arguments passed to make

### ASHELL_PKG_COMMANDS

- **Type**: Space-separated list
- **Format**: `command:entry_point:auth:type`
- **Example**: `hello:hello_main::no ls:ls_main::file`

#### Command Format

| Field | Description | Default |
|-------|-------------|---------|
| `command` | Command name users type | (required) |
| `entry_point` | C function called | `{command}_main` |
| `auth` | Authentication string | (empty) |
| `type` | Command type | `no` |

#### Command Types

| Type | Description |
|------|-------------|
| `file` | Operates on files (shows in file menu) |
| `directory` | Operates on directories |
| `no` | No special handling |

## Build Hooks (Override Functions)

Packages can override these functions for custom behavior:

### ashell_step_pre_configure()

Run before configure. Use for:
- Setting environment variables
- Running autoreconf
- Applying dynamic patches

```bash
ashell_step_pre_configure() {
    export CFLAGS="$CFLAGS -O3"
    cd "$(ashell_pkg_srcdir)" && autoreconf -fi
}
```

### ashell_step_configure()

Replace the configure step. Use for:
- Custom configure scripts
- Manual configuration

```bash
ashell_step_configure() {
    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)
    ashell_mkdir_p "$builddir"
    cd "$builddir"
    "$srcdir/configure" --prefix="$ASHELL_PREFIX"
}
```

### ashell_step_post_configure()

Run after configure. Use for:
- Patching generated files
- Adjusting build flags

### ashell_step_pre_make()

Run before make. Use for:
- Setting parallel job count
- Pre-build validation

### ashell_step_make()

Replace the build step. Use for:
- Custom build commands
- Non-standard build systems

### ashell_step_post_make()

Run after make. Use for:
- Running tests
- Verifying build outputs

### ashell_step_post_make_install()

Run after make install. Use for:
- Additional file installation
- Shebang fixing
- Plist generation

## Validation

### JSON Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "ASHELL Package Manifest",
  "type": "object",
  "required": ["ASHELL_PKG_NAME", "ASHELL_PKG_VERSION"],
  "properties": {
    "ASHELL_PKG_NAME": {
      "type": "string",
      "pattern": "^[a-z][a-z0-9-]*$"
    },
    "ASHELL_PKG_VERSION": {
      "type": "string"
    },
    "ASHELL_PKG_SRCURL": {
      "type": "string",
      "format": "uri"
    },
    "ASHELL_PKG_SHA256": {
      "type": "string",
      "pattern": "^[a-f0-9]{64}$"
    },
    "ASHELL_PKG_DEPENDS": {
      "type": "string"
    },
    "ASHELL_PKG_BUILD_DEPENDS": {
      "type": "string"
    },
    "ASHELL_PKG_COMMANDS": {
      "type": "string",
      "pattern": "^[a-z][a-z0-9_]*:[a-z_]*::[a-z]*$"
    }
  }
}
```

### Validation Script

```bash
#!/bin/bash
# validate-manifest.sh

validate_manifest() {
    local file="$1"
    local errors=0

    # Source to check variables
    source "$file"

    # Check required variables
    [[ -z "$ASHELL_PKG_NAME" ]] && { echo "ERROR: ASHELL_PKG_NAME not set"; ((errors++)); }
    [[ -z "$ASHELL_PKG_VERSION" ]] && { echo "ERROR: ASHELL_PKG_VERSION not set"; ((errors++)); }

    # Check name pattern
    [[ "$ASHELL_PKG_NAME" =~ ^[a-z][a-z0-9-]*$ ]] || { echo "ERROR: Invalid package name"; ((errors++)); }

    # Check SHA256 format (if set)
    if [[ -n "$ASHELL_PKG_SHA256" ]]; then
        [[ "$ASHELL_PKG_SHA256" =~ ^[a-f0-9]{64}$ ]] || { echo "ERROR: Invalid SHA256"; ((errors++)); }
    fi

    return $errors
}
```

## Examples

### Minimal Package

```bash
#!/bin/bash
ASHELL_PKG_NAME="hello"
ASHELL_PKG_VERSION="1.0.0"
ASHELL_PKG_SRCURL="https://example.com/hello-1.0.0.tar.gz"
ASHELL_PKG_SHA256="abc123..."
ASHELL_PKG_COMMANDS="hello:hello_main::no"

source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"
```

### Complex Package

```bash
#!/bin/bash
ASHELL_PKG_NAME="coreutils"
ASHELL_PKG_VERSION="9.4"
ASHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/coreutils/coreutils-9.4.tar.xz"
ASHELL_PKG_SHA256="bc..."
ASHELL_PKG_DESCRIPTION="GNU core utilities"
ASHELL_PKG_HOMEPAGE="https://www.gnu.org/software/coreutils/"
ASHELL_PKG_LICENSE="GPL-3.0"
ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS="automake libtool"
ASHELL_PKG_COMMANDS="ls:ls_main::file cat:cat_main::file"
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--disable-nls --enable-install-program=hostname"

ashell_step_pre_configure() {
    export ac_cv_func_malloc_0_nonnull=yes
}

source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"
```

## Migration from Termux

For packages migrating from Termux:

| Termux Variable | ASHELL Variable | Notes |
|-----------------|-----------------|-------|
| `TERMUX_PKG_NAME` | `ASHELL_PKG_NAME` | Same format |
| `TERMUX_PKG_VERSION` | `ASHELL_PKG_VERSION` | Same format |
| `TERMUX_PKG_SRCURL` | `ASHELL_PKG_SRCURL` | Same format |
| `TERMUX_PKG_SHA256` | `ASHELL_PKG_SHA256` | Same format |
| `TERMUX_PKG_DEPENDS` | `ASHELL_PKG_DEPENDS` | Same format |
| `TERMUX_PKG_BUILD_DEPENDS` | `ASHELL_PKG_BUILD_DEPENDS` | Same format |
| `TERMUX_PKG_EXTRA_CONFIGURE_ARGS` | `ASHELL_PKG_EXTRA_CONFIGURE_ARGS` | Same format |

Key differences:
- iOS-specific flags in configure
- Different PREFIX structure
- XCFramework generation instead of ELF binaries

## References

- Termux build.sh format: https://github.com/termux/termux-packages/wiki/Creating-new-package
- Semantic versioning: https://semver.org/
- SPDX licenses: https://spdx.org/licenses/

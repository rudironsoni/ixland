# ashell-packages - Package Forge

Termux-inspired package building system for a-Shell native iOS packages.

Part of the a-Shell Next modernization (see `docs/plans/20260318-a-shell-next.md`).

## Overview

This module contains the package forge - a build system for creating native iOS XCFramework packages that work with ashell-system.

## Quick Start

```bash
# Build the reference hello package
cd ashell-packages
./build.sh hello

# List available packages
./build.sh list

# Clean build artifacts
./build.sh hello clean

# Create distributable archive
./build.sh hello package
```

## Directory Structure

```
ashell-packages/
├── ashell_package.sh       # Build system library (sourced by packages)
├── build.sh                # Main build orchestration script
├── clean.sh                # Clean all build artifacts (TODO)
├── README.md               # This file
├── hello/                  # Reference package
│   ├── build.sh            # Package build recipe
│   └── patches/            # Git diff patches (01-*.patch, 02-*.patch, ...)
├── <package-name>/         # Other packages
│   ├── build.sh
│   └── patches/
└── .build/                 # Build outputs (.gitignore)
    ├── <package>/
    │   ├── src/            # Downloaded/extracted source
    │   ├── build/          # Build directory
    │   ├── staging/        # Installation staging
    │   └── <package>.framework/  # Generated XCFramework
```

## Creating a New Package

1. Create package directory:
```bash
mkdir ashell-packages/mypackage
touch ashell-packages/mypackage/build.sh
chmod +x ashell-packages/mypackage/build.sh
```

2. Write the build recipe:
```bash
#!/bin/bash
# ashell-packages/mypackage/build.sh

ASHELL_PKG_NAME="mypackage"
ASHELL_PKG_VERSION="1.0.0"
ASHELL_PKG_SRCURL="https://example.com/mypackage-1.0.0.tar.gz"
ASHELL_PKG_SHA256="abc123..."
ASHELL_PKG_COMMANDS="mypackage:mypackage_main::no myutil:myutil_main::no"
ASHELL_PKG_DEPENDS=""
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--disable-shared"

source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"
ashell_build_package
```

3. Add patches if needed:
```bash
# patches are applied in order (01-*.patch, 02-*.patch, ...)
vim ashell-packages/mypackage/patches/01-ios-prefix.patch
```

4. Build:
```bash
./build.sh mypackage
```

## Package Recipe Format

### Required Variables

| Variable | Description |
|----------|-------------|
| `ASHELL_PKG_NAME` | Package name (lowercase, no spaces) |
| `ASHELL_PKG_VERSION` | Package version |
| `ASHELL_PKG_SRCURL` | Source download URL (optional for local source) |
| `ASHELL_PKG_SHA256` | SHA256 checksum for verification |
| `ASHELL_PKG_COMMANDS` | Space-separated command specs |

### Command Specification Format

```
command:entry_point:auth_type:type
```

- **command**: The name users type to run
- **entry_point**: C function called (default: `{command}_main`)
- **auth_type**: Authentication requirement (empty for none)
- **type**: Command type (`no`, `file`, `directory`)

Example:
```bash
ASHELL_PKG_COMMANDS="hello:hello_main::no ls:ls_main::file grep:grep_main::file"
```

### Optional Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `ASHELL_PKG_DEPENDS` | Runtime dependencies | `""` |
| `ASHELL_PKG_BUILD_DEPENDS` | Build dependencies | `""` |
| `ASHELL_PKG_EXTRA_CONFIGURE_ARGS` | Extra args for configure | `""` |
| `ASHELL_PKG_EXTRA_MAKE_ARGS` | Extra args for make | `""` |

### Build Step Hooks

Override these functions in your `build.sh` to customize:

```bash
ashell_step_pre_configure() {
    # Run before configure
    export CFLAGS="$CFLAGS -DFOO"
}

ashell_step_post_make_install() {
    # Run after install
    ashell-fix-shebang "$ASHELL_PKG_STAGINGDIR/bin/"
}
```

Available hooks:
- `ashell_step_pre_configure`
- `ashell_step_configure` (override entirely)
- `ashell_step_post_configure`
- `ashell_step_pre_make`
- `ashell_step_make` (override entirely)
- `ashell_step_post_make`
- `ashell_step_make_install` (override entirely)
- `ashell_step_post_make_install`
- `ashell_step_create_xcframework` (override entirely)
- `ashell_step_generate_plist` (override entirely)

## Patch Format

Patches are standard unified diff format:

```diff
--- a/src/original.c
+++ b/src/original.c
@@ -10,7 +10,7 @@
 #include <stdio.h>
-#include <linux/fs.h>
+#include <sys/mount.h>

 int main() {
```

Naming convention:
- `01-prefix.patch` - Replace `/usr` with `@ASHELL_PREFIX@`
- `02-sandbox.patch` - iOS sandbox fixes
- `03-hardcoded-paths.patch` - Fix hardcoded paths

## Build System Variables

Set these environment variables to customize:

| Variable | Description | Default |
|----------|-------------|---------|
| `ASHELL_PREFIX` | Installation prefix | `$HOME/Library/ashell` |
| `ASHELL_CONFIG` | Config directory | `$HOME/Documents/.ashell` |
| `ASHELL_TARGET_PLATFORM` | Target platform | `arm64-apple-ios16.0` |
| `ASHELL_DEPLOYMENT_TARGET` | iOS version | `16.0` |
| `ASHELL_SDK_PATH` | iOS SDK path | Auto-detected |

## iOS Cross-Compilation

The build system automatically sets up:

- Architecture: `arm64`
- SDK: iOS (auto-detected)
- Minimum version: `16.0`
- Bitcode: Enabled
- Deployment target: Configurable

## XCFramework Output

Each package produces:

```
.build/<package>/<package>.framework/
├── <package>              # Binary executable
├── Info.plist            # Framework metadata
├── commands.plist        # Command registration
└── Headers/              # Public headers (if any)
```

## Package Installation

After building, install to a-Shell:

```bash
# Copy to app bundle
# (In production, this is done by `pkg install` command)
cp -R .build/hello/hello.framework \
  /path/to/a-Shell.app/Frameworks/
```

The `commands.plist` is merged into a-Shell's `commandDictionary.plist` at runtime.

## Difference from holzschu/a-Shell-commands

This forge uses a **Termux-inspired approach**:

| Aspect | a-Shell-commands | ashell-packages |
|--------|------------------|-----------------|
| Build scripts | Individual shell scripts | Unified `ashell_package.sh` library |
| Patches | Embedded in scripts | Separate `patches/*.patch` files |
| Build steps | Ad-hoc | Standardized `ashell_step_*` functions |
| XCFramework | Manual creation | Automated |
| Plist generation | Manual | From `ASHELL_PKG_COMMANDS` metadata |
| Dependencies | Manual handling | Declarative with automatic resolution |

## Troubleshooting

### Build fails with "SDK not found"
```bash
export ASHELL_SDK_PATH=$(xcrun --sdk iphoneos --show-sdk-path)
```

### Checksum verification fails
```bash
# Skip verification (not recommended for production)
ASHELL_PKG_SHA256="" ./build.sh mypackage
```

### Patch fails to apply
```bash
# Check patch format
cd .build/mypackage/src
patch -p1 --dry-run < ../../mypackage/patches/01-fix.patch
```

## License

Same as a-Shell project.

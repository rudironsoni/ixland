# Porting Guide for Package Authors

This guide walks you through porting Unix commands to a-Shell, from source code to working iOS package. No prior iOS development experience required.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start](#quick-start)
3. [The Porting Workflow](#the-porting-workflow)
4. [Identifying iOS Incompatibilities](#identifying-ios-incompatibilities)
5. [Creating Patches](#creating-patches)
6. [Writing build.sh](#writing-buildsh)
7. [Testing Your Package](#testing-your-package)
8. [Troubleshooting](#troubleshooting)
9. [Walkthrough Examples](#walkthrough-examples)
10. [Submitting Your Package](#submitting-your-package)

---

## Prerequisites

### Required

- **macOS** (11.0 or later) - iOS development requires Apple tools
- **Xcode** (15.0 or later) - Available free from Mac App Store
- **Command line tools**: `xcode-select --install`
- **Basic Unix knowledge** - Familiarity with shell, make, compilers

### Optional but Recommended

- **Docker** - For testing build scripts on Linux
- **Git** - For patch management
- **a-Shell app** - For testing on actual iOS device

### Verify Your Setup

```bash
# Check macOS version
sw_vers -productVersion

# Check Xcode
xcode-select -p
xcodebuild -version

# Check SDK
xcrun --sdk iphoneos --show-sdk-path
```

---

## Quick Start

If you want to skip the details and just see a working example:

```bash
cd ashell-packages
./build.sh hello
cat hello/build.sh
```

The `hello` package demonstrates the minimal working pattern.

---

## The Porting Workflow

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  1. Get Source  │────▶│ 2. Identify     │────▶│ 3. Create       │
│                 │     │    iOS Issues   │     │    Patches      │
└─────────────────┘     └─────────────────┘     └─────────────────┘
                                                         │
┌─────────────────┐     ┌─────────────────┐              │
│ 6. Test & Fix   │◀────│ 5. Build on Mac │◀─────────────┘
│                 │     │                 │
└─────────────────┘     └─────────────────┘
         │
         ▼
┌─────────────────┐
│ 7. Submit PR    │
└─────────────────┘
```

---

## Identifying iOS Incompatibilities

### Common Issues Checklist

When examining source code, look for these iOS-incompatible patterns:

#### ❌ Fork/Exec Family

```c
// These will fail on iOS
fork();           // Returns -1, errno=ENOSYS
vfork();          // Returns -1, errno=ENOSYS
execv();          // May work via ios_system
execve();         // May work via ios_system
system();         // Works via ios_system
```

**Solution**: Use threads (`pthread_create`) instead of fork. For command execution, use `ios_system()`.

#### ❌ Process Management

```c
// Problematic
ptrace();         // Not allowed (security)
setuid();         // Not allowed in sandbox
setgid();         // Not allowed in sandbox
getpwnam();       // May return dummy data
```

**Solution**: Remove or stub out these calls.

#### ❌ Hardcoded Paths

```c
// Won't work on iOS
"/bin/sh"         // No /bin on iOS
"/usr/bin/awk"    // No /usr/bin on iOS
"/etc/passwd"     // Not accessible
```

**Solution**: Use `$PREFIX` variables. The build system substitutes `@ASHELL_PREFIX@`.

#### ❌ Global I/O Streams

```c
// Problematic in multi-session
fprintf(stdout, ...);   // Goes to wrong terminal
fprintf(stderr, ...);   // May not appear
getchar();              // Reads from wrong stdin
```

**Solution**: Use ios_system thread-local I/O:

```c
#include "ios_system.h"
fprintf(ios_stdout(), "Hello\n");
```

#### ⚠️ Signal Handling

```c
// May behave differently
signal(SIGTERM, handler);   // Limited signal support
sigaction();                 // May not work as expected
alarm();                     // Not available
```

**Solution**: Test thoroughly. Many signals are ignored on iOS.

#### ⚠️ File System Assumptions

```c
// May not exist or be accessible
"/tmp"              // Use $TMPDIR instead
"/dev/tty"          // Use ios_ttyname()
"/proc/self"        // Not available
```

**Solution**: Use standard temp file APIs, ios_system APIs.

### Using the Compatibility Checker

We provide a script to scan for common issues:

```bash
./scripts/check-compatibility.sh source-file.c
```

This outputs potential iOS compatibility issues.

---

## Creating Patches

### Patch Naming Convention

Patches go in `patches/` directory with numeric prefixes:

```
patches/
├── 01-fix-prefix.patch       # Applied first
├── 02-remove-fork.patch      # Applied second
└── 03-ios-compat.patch       # Applied third
```

### Creating a Patch

1. **Make a clean copy of the original source**:

```bash
cd mypackage-src
cp -r source source.original
```

2. **Make your changes** to `source/`:

```bash
# Edit files as needed
vim source/main.c
```

3. **Generate the patch**:

```bash
diff -ruN source.original/ source/ > patches/01-fix-prefix.patch
```

### Patch Format

A good patch looks like this:

```diff
--- a/src/main.c
+++ b/src/main.c
@@ -10,7 +10,7 @@
 #include <stdio.h>

-#define DEFAULT_PATH "/usr/bin"
+#define DEFAULT_PATH "@ASHELL_PREFIX@/bin"

 int main(int argc, char** argv) {
```

### Patch Best Practices

1. **One logical change per patch** - Don't mix unrelated fixes
2. **Use `@ASHELL_PREFIX@`** - Build system substitutes this
3. **Minimize changes** - Only patch what's necessary
4. **Comment why** - Add comments explaining iOS-specific changes
5. **Keep original behavior** - Don't change functionality, just compatibility

### Common Patch Types

#### Type 1: Prefix Substitution

Replace hardcoded paths with `@ASHELL_PREFIX@`:

```diff
-#define BINDIR "/usr/bin"
+#define BINDIR "@ASHELL_PREFIX@/bin"
```

#### Type 2: Remove Fork/Exec

Replace with error or alternative:

```diff
-    pid = fork();
-    if (pid == 0) {
-        execv(cmd, args);
-    }
+#ifdef __APPLE__
+    // iOS doesn't support fork()
+    fprintf(stderr, "fork() not supported on iOS\n");
+    return -1;
+#else
+    pid = fork();
+    if (pid == 0) {
+        execv(cmd, args);
+    }
+#endif
```

#### Type 3: I/O Redirection

Use ios_system I/O:

```diff
-#include <stdio.h>
+#include "ios_system.h"

-    fprintf(stdout, "Hello\n");
+    fprintf(ios_stdout(), "Hello\n");
```

#### Type 4: Feature Removal

Remove unsupported features gracefully:

```diff
-    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
-        perror("ptrace");
-        exit(1);
-    }
+    // ptrace not available on iOS - skip debugging support
```

---

## Writing build.sh

### Minimal build.sh

```bash
#!/bin/bash
# mypackage/build.sh

ASHELL_PKG_NAME="mypackage"
ASHELL_PKG_VERSION="1.0.0"
ASHELL_PKG_SRCURL="https://example.com/mypackage-1.0.0.tar.gz"
ASHELL_PKG_SHA256="abc123..."
ASHELL_PKG_COMMANDS="mycmd:mycmd_main::no"

source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"
```

That's it! The build system handles the rest.

### Required Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `ASHELL_PKG_NAME` | Package name (lowercase) | `hello` |
| `ASHELL_PKG_VERSION` | Version string | `1.0.0` |
| `ASHELL_PKG_SRCURL` | Download URL | `https://...` |
| `ASHELL_PKG_SHA256` | Source checksum | `abc123...` |
| `ASHELL_PKG_COMMANDS` | Command definitions | `cmd:entry::type` |

### Commands Format

```bash
ASHELL_PKG_COMMANDS="cmd1:entry1::type1 cmd2:entry2::type2"
# Format: "command:entry_point:auth:type"
```

- **command**: Name users type
- **entry_point**: C function to call (default: `{command}_main`)
- **auth**: Authentication string (usually empty)
- **type**: `file`, `directory`, `no`

### Optional Override Functions

Override any step function for custom behavior:

```bash
ashell_step_pre_configure() {
    # Run before configure
    export CFLAGS="$CFLAGS -O3"
}

ashell_step_configure() {
    # Custom configure
    cd "$(ashell_pkg_srcdir)"
    ./configure --prefix="$ASHELL_PREFIX" \
                --disable-feature-x \
                --enable-feature-y
}

ashell_step_post_make() {
    # Run after make
    echo "Build complete!"
}
```

### Complete Example with Overrides

```bash
#!/bin/bash

ASHELL_PKG_NAME="htop"
ASHELL_PKG_VERSION="3.2.2"
ASHELL_PKG_SRCURL="https://github.com/htop-dev/htop/releases/download/3.2.2/htop-3.2.2.tar.xz"
ASHELL_PKG_SHA256="bac9aebdeeede27464a225210bb1c1589256364e3c7453544a59e4278ccf6567"
ASHELL_PKG_DEPENDS="ncurses"
ASHELL_PKG_COMMANDS="htop:htop_main::no"

source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"

ashell_step_pre_configure() {
    # htop needs these set for cross-compilation
    export ac_cv_file__proc_stat=yes
    export ac_cv_file__proc_meminfo=yes
}

ashell_step_configure() {
    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)

    ashell_mkdir_p "$builddir"
    cd "$builddir"

    # htop uses a custom configure
    "$srcdir/configure" \
        --prefix="$ASHELL_PREFIX" \
        --host="$ASHELL_TARGET_PLATFORM" \
        --disable-unicode \
        --enable-selinux=no
}
```

---

## Testing Your Package

### Test Checklist

```bash
cd ashell-packages

# 1. Syntax check
bash -n mypackage/build.sh

# 2. Try to build
./build.sh mypackage

# 3. Verify output
ls -la .build/mypackage/
# Should see:
# - mypackage.xcframework/
# - mypackage.framework/

# 4. Check XCFramework structure
find .build/mypackage/*.xcframework -type f

# 5. Verify plist
plutil -lint .build/mypackage/*/commands.plist
```

### Testing on Real iOS (a-Shell App)

1. **Build the package** on your Mac
2. **Copy XCFramework** to a-Shell project:
   ```bash
   cp -r .build/mypackage/mypackage.xcframework \
         /path/to/a-shell-project/Frameworks/
   ```
3. **Add plist entries** to `commandDictionary.plist`
4. **Build and run** a-Shell in Simulator
5. **Test the command** in the terminal

### Testing with Docker (Script Validation)

```bash
# Validate scripts without macOS
docker-compose run test
```

This checks syntax and validates metadata without needing Xcode.

---

## Troubleshooting

### Build fails with "No targets specified"

**Cause**: Makefile not found or empty build directory
**Fix**: Override `ashell_step_make()` to build in correct directory

```bash
ashell_step_make() {
    local srcdir=$(ashell_pkg_srcdir)
    cd "$srcdir"
    make -j$(nproc)
}
```

### "Configure: error: cannot run C compiled programs"

**Cause**: Cross-compilation detection failing
**Fix**: Set cache variables:

```bash
ashell_step_pre_configure() {
    export ac_cv_func_malloc_0_nonnull=yes
    export ac_cv_func_realloc_0_nonnull=yes
}
```

### "Undefined symbols for architecture arm64"

**Cause**: Missing libraries or wrong architecture
**Fix**: Check dependencies and linker flags

```bash
ASHELL_PKG_DEPENDS="libz libcurl"
```

### XCFramework not created

**Cause**: Binary not found in expected location
**Fix**: Override install step:

```bash
ashell_step_post_make_install() {
    local srcdir=$(ashell_pkg_srcdir)
    local stagingdir=$(ashell_pkg_stagingdir)
    ashell_mkdir_p "$stagingdir/bin"
    cp "$srcdir/mybinary" "$stagingdir/bin/"
}
```

### Command not found after install

**Cause**: Missing or incorrect commands.plist
**Fix**: Verify ASHELL_PKG_COMMANDS format

```bash
# Correct
ASHELL_PKG_COMMANDS="mycmd:mycmd_main::no"

# Check generated plist
cat .build/mypackage/*/commands.plist
```

### "dyld: Library not loaded"

**Cause**: Dynamic library linking
**Fix**: Use static linking:

```bash
ashell_step_pre_configure() {
    export LDFLAGS="$LDFLAGS -static"
}
```

---

## Walkthrough Examples

### Example 1: Simple C Program (hello)

See `ashell-packages/hello/build.sh` for the minimal example.

Key points:
- No external source (embedded C code)
- No patches needed
- Overrides `ashell_step_extract_package()` to create source

### Example 2: Autotools Project (coreutils)

```bash
ASHELL_PKG_NAME="coreutils"
ASHELL_PKG_VERSION="9.4"
ASHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/coreutils/coreutils-9.4.tar.xz"
ASHELL_PKG_SHA256="...
ASHELL_PKG_COMMANDS="ls:ls_main::no cat:cat_main::no"

# Patches needed:
# - 01-disable-fork.patch (replace fork with error)
# - 02-prefix-paths.patch (use @ASHELL_PREFIX@)
```

### Example 3: CMake Project

```bash
ASHELL_PKG_NAME="myproject"
...

ashell_step_configure() {
    local srcdir=$(ashell_pkg_srcdir)
    local builddir=$(ashell_pkg_builddir)

    ashell_mkdir_p "$builddir"
    cd "$builddir"

    cmake "$srcdir" \
        -DCMAKE_INSTALL_PREFIX="$ASHELL_PREFIX" \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0
}
```

---

## Submitting Your Package

### Before Submitting

1. [ ] Build succeeds on your Mac
2. [ ] XCFramework is created
3. [ ] Commands.plist is valid
4. [ ] You've tested in a-Shell (if possible)
5. [ ] Documentation is complete

### Directory Structure

```
ashell-packages/mypackage/
├── build.sh           # Build recipe
├── patches/           # (optional)
│   ├── 01-*.patch
│   └── 02-*.patch
└── README.md          # Package-specific notes (optional)
```

### Pull Request Template

```markdown
## Package: mypackage v1.0.0

### Description
Brief description of what this package does.

### Changes Required
- Patch 01: Remove fork() calls
- Patch 02: Fix hardcoded paths

### Testing
- [ ] Builds on macOS
- [ ] XCFramework created
- [ ] Tested in a-Shell (simulator/device)

### Checklist
- [ ] build.sh has correct metadata
- [ ] SHA256 verified
- [ ] Patches apply cleanly
- [ ] No new warnings
```

### Review Process

1. **Automated checks** run on PR:
   - Syntax validation
   - Metadata validation

2. **Maintainer review**:
   - Code review
   - Build test

3. **Merge** when approved

---

## Resources

- **ios_system Contract**: `docs/api/ios_system_contract.md`
- **Example packages**: `ashell-packages/hello/`
- **Build system**: `ashell-packages/ashell_package.sh`
- **Reference implementations**: `.beads/reference/`

## Getting Help

- Open an issue: https://github.com/rudironsoni/a-shell-next/issues
- Check existing packages for examples
- Read the ios_system contract for API details

---

**Happy Porting!**

# Patch Template Guide for IXLand Packages

This guide provides templates and examples for creating iOS compatibility patches.

## API Reference

When patching packages for iOS, you'll need to use the IXLand kernel APIs. Include the kernel header and use the `ixland_*` prefixed functions:

```c
// Include the IXLand kernel header
extern "C" {
    #include "ixland_system.h"  // or <ixland_system.h> if installed
}
```

### Core Syscalls (Process Management)

```c
// Process management
pid_t ixland_fork(void);
void ixland_waitpid(pid_t pid);
pid_t ixland_full_waitpid(pid_t pid, int *stat_loc, int options);
int ixland_execv(const char *path, char* const argv[]);
int ixland_execve(const char *path, char* const argv[], char** envlist);
void ixland_exit(int errorCode);

// Signal handling
sig_t ixland_signal(int signal, sig_t function);
int ixland_killpid(pid_t pid, int sig);
```

### I/O Functions

```c
// Stream operations
FILE *ixland_popen(const char *command, const char *type);
FILE* ixland_stdin(void);
FILE* ixland_stdout(void);
FILE* ixland_stderr(void);

// I/O redirection
int ixland_dup2(int fd1, int fd2);
int ixland_isatty(int fd);
int ixland_getstdin(void);
int ixland_gettty(void);
int ixland_opentty(void);
void ixland_closetty(void);

// I/O functions (for #defines)
ssize_t ixland_write(int fildes, const void *buf, size_t nbyte);
size_t ixland_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
int ixland_puts(const char *s);
int ixland_fputs(const char* s, FILE *stream);
int ixland_fputc(int c, FILE *stream);
int ixland_fflush(FILE *stream);
```

### Environment

```c
char *ixland_getenv(const char *name);
int ixland_setenv(const char* variableName, const char* value, int overwrite);
int ixland_unsetenv(const char* variableName);
int ixland_putenv(char *string);
int ixland_fchdir(const int fd);
```

### Session Management

```c
void ixland_switchSession(const void* sessionid);
void ixland_closeSession(const void* sessionid);
void ixland_setStreams(FILE* _stdin, FILE* _stdout, FILE* _stderr);
void ixland_setContext(const void *context);
const void* ixland_getContext(void);
const char* ixland_progname(void);
int ixland_getCommandStatus(void);
```

## Patch File Format

All patches use **unified diff format** (`diff -u`):

```diff
--- a/path/to/original/file
+++ b/path/to/modified/file
@@ -line,context +line,context @@
 context line
-context line to remove
+new context line to add
 context line
```

## Naming Convention

```
patches/XX-ios-<category>.patch
```

- **XX**: Order number (01, 02, 03... applied sequentially)
- **category**: Type of patch
  - `functions`: Missing function replacement
  - `paths`: Path substitutions
  - `platform`: Platform detection
  - `config`: Build configuration
  - `debian`: Removing Debian dependencies

## Categories

### 1. Missing Functions (functions)

**When to use:** Package uses a function iOS doesn't provide.

**Common examples:**
- `getentropy()` → `arc4random()`
- `gethostbyname_r()` → `getaddrinfo()`
- `clock_settime()` → skip/disable

**Template:**

```diff
--- a/lib/sh/random.c
+++ b/lib/sh/random.c
@@ -195,7 +195,12 @@ get_random_number (max)
   if (max == 0)
     return 0;

+#if defined(__APPLE__) && defined(__MACH__)
+  /* iOS doesn't have getentropy, use arc4random */
+  ret = (unsigned long)arc4random();
+#else
   ret = getentropy(&rand_bytes, sizeof(rand_bytes));
+#endif
   if (ret == 0)
     return (rand_bytes % max);
```

**Real example (bash-minimal):**
```diff
--- a/lib/sh/random.c
+++ b/lib/sh/random.c
@@ -195,7 +195,12 @@ get_random_number (max)
   if (max == 0)
     return 0;

+#if defined(__APPLE__) && defined(__MACH__)
+  /* iOS doesn't have getentropy, use arc4random */
+  ret = (unsigned long)arc4random();
+#else
   ret = getentropy(&rand_bytes, sizeof(rand_bytes));
+#endif
   if (ret == 0)
     return (rand_bytes % max);
```

### 2. Path Substitutions (paths)

**When to use:** Package has hardcoded Unix paths.

**Common substitutions:**
- `/etc/` → `@A_SHELL_PREFIX@/etc/`
- `/tmp/` → `$TMPDIR` or `@A_SHELL_PREFIX@/tmp/`
- `/var/` → `~/Library/Caches/` or `@A_SHELL_PREFIX@/var/`
- `/usr/share/` → `@A_SHELL_PREFIX@/share/`

**Template (configure.ac):**

```diff
--- a/configure.ac
+++ b/configure.ac
@@ -50,7 +50,7 @@ AC_INIT([package], [1.0.0])
 # Check for system directories
 AC_MSG_CHECKING([for system configuration directory])
-sysconfdir=/etc
+sysconfdir=@A_SHELL_PREFIX@/etc
 AC_MSG_RESULT([$sysconfdir])
 AC_SUBST([sysconfdir])
```

**Template (C source):**

```diff
--- a/src/config.c
+++ b/src/config.c
@@ -30,7 +30,11 @@
 #define CONFIG_FILE "/etc/myconfig.conf"

 const char *get_config_path(void) {
+#if defined(__APPLE__) && defined(__MACH__)
+    return "@A_SHELL_PREFIX@/etc/myconfig.conf";
+#else
     return CONFIG_FILE;
+#endif
 }
```

### 3. Platform Detection (platform)

**When to use:** Code checks `__linux__` but not `__APPLE__`.

**Template:**

```diff
--- a/src/platform.c
+++ b/src/platform.c
@@ -25,7 +25,7 @@

 void platform_init(void) {
-#ifdef __linux__
+#if defined(__linux__) || defined(__APPLE__)
     /* Linux-specific initialization */
     setup_linux_features();
 #elif defined(__FreeBSD__)
```

### 4. Build Configuration (config)

**When to use:** Configure script needs iOS-specific settings.

**Cache variables template:**

```diff
--- a/build.sh
+++ b/build.sh
@@ -10,6 +10,12 @@ ixland_pkg_configure() {
     export ac_cv_func_<function_name>=no
     export ac_cv_lib_<library_name>=no

+    # iOS-specific settings
+    export ac_cv_func_clock_settime=no
+    export ac_cv_func_getentropy=no
+    export gl_cv_func_getentropy=no
+
     ./configure \
         --prefix="$A_SHELL_PREFIX" \
         --host="arm-apple-darwin"
```

### 5. Removing Debian Dependencies (debian)

**When to use:** Package has dpkg/apt/deb dependencies.

**Remove dpkg check:**

```diff
--- a/configure.ac
+++ b/configure.ac
@@ -100,10 +100,15 @@ AC_CHECK_PROG([HAS_MAKE], [make], [yes], [no])
 # Check for required libraries
 PKG_CHECK_MODULES([OPENSSL], [openssl >= 1.0.0])

-# Check for package manager (REMOVE FOR iOS)
-AC_PATH_PROG([DPKG], [dpkg])
-AC_PATH_PROG([APT_GET], [apt-get])
-AM_CONDITIONAL([HAVE_DPKG], [test -n "$DPKG"])
+# iOS: No dpkg/apt, skip these checks
+# AC_PATH_PROG([DPKG], [dpkg])
+# AC_PATH_PROG([APT_GET], [apt-get])
+# AM_CONDITIONAL([HAVE_DPKG], [test -n "$DPKG"])
+
+# Disable Debian features
+DPKG=""
+APT_GET=""
+AM_CONDITIONAL([HAVE_DPKG], [false])

 AC_CONFIG_FILES([
     Makefile
```

**Remove deb-specific paths:**

```diff
--- a/src/paths.c
+++ b/src/paths.c
@@ -30,9 +30,15 @@

 /* Default paths */
 #ifndef DEFAULT_ETC_DIR
+#if defined(__APPLE__) && defined(__MACH__)
+#define DEFAULT_ETC_DIR "@A_SHELL_PREFIX@/etc"
+#define DEFAULT_VAR_DIR "@A_SHELL_PREFIX@/var"
+#define DEFAULT_CACHE_DIR "~/Library/Caches/myapp"
+#else
 #define DEFAULT_ETC_DIR "/etc"
 #define DEFAULT_VAR_DIR "/var"
 #define DEFAULT_CACHE_DIR "/var/cache"
 #endif
+#endif

 const char *get_etc_dir(void) {
     return DEFAULT_ETC_DIR;
```

## Common Patterns

### Replacing getentropy

```diff
--- a/src/random.c
+++ b/src/random.c
@@ -45,7 +45,12 @@ int get_random_bytes(void *buf, size_t len) {
     #ifdef HAVE_GETENTROPY
     return getentropy(buf, len);
     #else
+    #if defined(__APPLE__) && defined(__MACH__)
+    arc4random_buf(buf, len);
+    return 0;
+    #else
     return -1;
+    #endif
     #endif
 }
```

### Fixing /etc references

```diff
--- a/src/config.c
+++ b/src/config.c
@@ -20,7 +20,11 @@
 #include "config.h"

 #ifndef CONFIG_DIR
+#if defined(__APPLE__) && defined(__MACH__)
+#define CONFIG_DIR "@A_SHELL_PREFIX@/etc"
+#else
 #define CONFIG_DIR "/etc"
+#endif
 #endif

 int load_config(void) {
```

### Adding iOS to platform checks

```diff
--- a/src/os.c
+++ b/src/os.c
@@ -30,7 +30,7 @@

 int get_os_type(void) {
-    #ifdef __linux__
+    #if defined(__linux__) || defined(__APPLE__)
     return OS_UNIX;
     #elif defined(_WIN32)
     return OS_WINDOWS;
```

### Disabling features with cache variables

Instead of patching source, use configure cache variables in build.sh:

```bash
ixland_pkg_configure() {
    # Disable features iOS doesn't support
    export ac_cv_func_getentropy=no
    export ac_cv_func_clock_settime=no
    export ac_cv_func_fanotify_init=no
    export ac_cv_func_inotify_init=no

    # Disable Debian tools
    export ac_cv_path_DPKG=/bin/false
    export ac_cv_path_APTGET=/bin/false
    export ac_cv_path_DPKG_DEB=/bin/false

    ./configure \
        --prefix="$A_SHELL_PREFIX" \
        --host="arm-apple-darwin"
}
```

## Creating a Patch

### Method 1: Manual edit and diff

1. Extract package source
2. Make a copy: `cp -r package-1.0 package-1.0.orig`
3. Edit files in `package-1.0/`
4. Generate patch: `diff -u package-1.0.orig/ package-1.0/ > 01-ios-fix.patch`

### Method 2: Using git

```bash
# In package source directory
git init
git add .
git commit -m "original"

# Make changes
# ... edit files ...

git diff > ../patches/01-ios-fix.patch
```

### Method 3: quilt (for multiple patches)

```bash
# In package source directory
quilt init
quilt new 01-ios-functions.patch
quilt add src/file.c
# ... edit src/file.c ...
quilt refresh
quilt new 02-ios-paths.patch
# ... next patch ...
```

## Testing Patches

1. **Place patch in correct location:**
   ```
   packages/core/<name>/patches/01-ios-<category>.patch
   ```

2. **Build package:**
   ```bash
   ./scripts/build-package.sh <name> --target universal
   ```

3. **Verify patch was applied:**
   - Check build output for "Applying patches..."
   - Check `.build/tmp/<name>-<version>/` for modified files

4. **If patch fails:**
   - Check line numbers in patch match source version
   - Use `patch --dry-run -p1 < patchfile` to test
   - Regenerate patch if needed

## Patch Documentation

Create `patches/README.md`:

```markdown
# Patches for <package>

## 01-ios-functions.patch
- **Category:** Missing Functions
- **Issue:** iOS lacks getentropy()
- **Solution:** Use arc4random() instead
- **Files modified:** lib/sh/random.c

## 02-ios-paths.patch
- **Category:** Path Substitutions
- **Issue:** Hardcoded /etc paths
- **Solution:** Use @A_SHELL_PREFIX@/etc
- **Files modified:** src/config.c, configure.ac
```

## Best Practices

1. **One logical change per patch**
   - Don't mix function fixes with path fixes
   - Create separate patches: 01-functions, 02-paths

2. **Keep patches minimal**
   - Only change what's necessary
   - Avoid whitespace changes
   - Use `diff -u` (unified format)

3. **Document the patch**
   - Header comment explaining the issue
   - Reference to iOS limitation

4. **Test before committing**
   - Build with and without patch
   - Verify iOS Simulator tests pass

5. **Version-specific patches**
   - Patches may need updates for new versions
   - Document which version patch applies to

## Common Issues

### Patch fails to apply

```
patch: **** malformed patch at line 42
```

**Fix:**
- Check line endings (must be LF, not CRLF)
- Verify file paths in patch headers
- Ensure patch level (-p0, -p1) matches directory structure

### Patch applies but doesn't work

**Check:**
- Correct source version?
- Patch applied in right order?
- Check `configure.log` for feature detection

### Function still missing

**Solution:**
- Use cache variable in build.sh instead:
  ```bash
  export ac_cv_func_getentropy=no
  ```
- Or configure flag:
  ```bash
  ./configure --disable-feature
  ```

## Quick Reference

```bash
# Create patch
diff -u original.c modified.c > 01-ios-fix.patch

# Test patch (dry run)
cd package-source/
patch --dry-run -p1 < ../patches/01-ios-fix.patch

# Apply patch
patch -p1 < ../patches/01-ios-fix.patch

# Reverse patch
patch -R -p1 < ../patches/01-ios-fix.patch
```

## Examples

See existing patches:
- `packages/core/bash-minimal/patches/01-ios-getentropy.patch`

## See Also

- `AGENTS.md` - Full patching guide
- `build-package.sh --help` - Build documentation

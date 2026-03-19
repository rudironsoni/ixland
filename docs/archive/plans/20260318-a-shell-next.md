# a-Shell-next: Modernization Plan

## Project Structure Initiative

### Objective
Create a comprehensive beads-based project management structure with milestones, epics, and detailed tasks that enable future coding agents to work with zero context. Each artifact must be self-contained with full implementation guidance, code snippets, copy-paste examples, and agent-specific instructions.

### Package Manager: `apt` (Debian-style)

**Critical Decision**: Use `apt` as the sole package manager command, not `pkg`. This provides a familiar Debian-style interface:
- `apt install <package>` - Install packages
- `apt remove <package>` - Remove packages
- `apt search <query>` - Search packages
- `apt list` - List installed packages
- `apt update` - Update package catalog

The `apt` package itself is built from Debian upstream source (apt 2.8.1) with iOS-specific patches, following Termux's approach: https://github.com/termux/termux-packages/tree/master/packages/apt

### Package Repository Structure (ashell-packages)

Packages are organized in two categories:

**1. root-packages/ - Core Distribution Packages**
Located in `ashell-packages/root-packages/` - these are the building blocks of the a-Shell distribution. They must exist for the core system to function.

Examples (per user direction):
- `apt/` - The package manager (built from Debian 2.8.1 with iOS patches)
- `dpkg/` - Debian package management system
- `coreutils/` - Essential file/text/shell utilities
- `libz/` - zlib compression library
- `libssl/` - OpenSSL library
- `python/` - Python runtime (BeeWare Python-Apple-support)

**2. packages/ - Optional User Packages**
Located in `ashell-packages/packages/` - these are optional packages users can install later.

Examples:
- `nvim/` - Neovim text editor
- `vim/` - Vim text editor
- `python/` - Python runtime
- `node/` - Node.js runtime
- `git/` - Git version control

### Directory Layout
```
ashell-packages/
├── ashell_package.sh              # Build system library
├── build.sh                       # Main build orchestration
├── clean.sh                       # Clean build artifacts
├── root-packages/                 # Core distro packages (REQUIRED)
│   ├── apt/                       # apt package manager
│   │   ├── build.sh               # Build recipe
│   │   └── patches/
│   │       ├── 01-prefix.patch    # PREFIX path patches
│   │       ├── 02-ios-sandbox.patch   # iOS sandbox fixes
│   │       └── 03-filesystem.patch    # iOS filesystem handling
│   ├── dpkg/
│   ├── libz/
│   └── ...
├── packages/                      # Optional packages
│   ├── nvim/
│   ├── vim/
│   ├── python/
│   └── ...
└── .build/                        # Build outputs (.gitignore)
```

### Four-Part Enhancement Plan

#### 1. Enhanced Issue Descriptions

Update all 35+ existing issues to include:

**Code Snippets Section:**
```markdown
## Code Snippets

### Function Signature Pattern
```bash
# From ashell-packages/ashell_package.sh (if exists)
ashell_step_<name>() {
    # Inputs: $1 = arg1, $2 = arg2
    # Returns: 0 on success, 1 on failure
    # Side effects: Creates/modifies files
}
```

### Reference Implementation
```bash
# Exact code from existing working implementation
# If none exists, provide starter template:
ashell_step_extract_package() {
    local url="$1"
    local sha256="$2"
    local dest_dir="$3"

    # Download
    if ! curl -fsSL "$url" -o "$dest_dir/download.tmp"; then
        ashell_log_error "Download failed: $url"
        return 1
    fi

    # Verify SHA256
    if [[ "$(shasum -a 256 "$dest_dir/download.tmp" | cut -d' ' -f1)" != "$sha256" ]]; then
        ashell_log_error "SHA256 mismatch"
        return 1
    fi

    # Extract
    tar -xzf "$dest_dir/download.tmp" -C "$dest_dir"
    rm "$dest_dir/download.tmp"
}
```

### Error Handling Pattern
```bash
# Check return codes
if ! some_command; then
    ashell_log_error "Command failed: $func_name"
    return 1
fi

# Cleanup on error
trap 'rm -rf "$temp_dir"' EXIT
```
```

**Exact File Paths Section:**
```markdown
## Exact File Paths

### Files to Create
- `/home/rrj/src/github/rudironsoni/a-shell-next/ashell-packages/ashell_package.sh` - Main library
- `/home/rrj/src/github/rudironsoni/a-shell-next/ashell-packages/config/ios-toolchain.cmake` - CMake toolchain

### Files to Modify (if exists)
- `/home/rrj/src/github/rudironsoni/a-shell-next/ashell-packages/build.sh` - Add command dispatch
  - Line 45: Add 'package' case to switch statement
  - Line 78: Update help text

### Directory Structure to Create
```
ashell-packages/
├── ashell_package.sh
├── build.sh
├── config/
│   └── ios-toolchain.cmake
└── .build/
    └── {package}/
```
```

**Copy-Paste Commands Section:**
```markdown
## Copy-Paste Commands

### Setup
```bash
cd /home/rrj/src/github/rudironsoni/a-shell-next/ashell-packages
mkdir -p config .build
```

### Testing
```bash
# Syntax check
bash -n ashell_package.sh

# Test specific function
source ashell_package.sh
ashell_step_extract_package "https://example.com/file.tar.gz" "abc123..." "/tmp/test"

# Full build test
./build.sh hello
```

### Verification
```bash
# Check output exists
ls -la .build/hello/*.xcframework

# Validate plist
cat .build/hello/commands.plist | plutil -lint -
```
```

#### 2. Task Specification Template

Create `.beads/TASK_SPECIFICATION.md`:

```markdown
# Task Specification Template

Every task MUST include these sections with this level of detail:

## Title Format
{Milestone}-I{Number}: {Action verb} {what} {for whom/what}

Example: M2-I1: Create ashell_package.sh build system library

## Required Sections

### Goal (1 sentence)
What to accomplish. Must be testable.

### Success Criteria (checklist)
- [ ] Specific, measurable outcomes
- [ ] Each item has verification method
- [ ] No subjective criteria

### Context (2-3 paragraphs)
- Why this matters
- What depends on it
- Technical background
- iOS-specific constraints

### Dependencies
- **Blocked by:** Exact issue IDs
- **Blocks:** Exact issue IDs
- **Related:** Reference issues

### Code Snippets
Working code examples:
- Function signatures
- Implementation patterns
- Error handling
- Integration points

### Exact File Paths
Absolute paths from repo root:
- Files to create
- Files to modify (with line numbers if applicable)
- Directories to create

### Copy-Paste Commands
Exact commands for:
- Setup
- Implementation
- Testing
- Verification

### Decision Records
| Decision | Rationale | Alternatives Rejected |
|----------|-----------|----------------------|
| Choice | Why | Why not other |

### Do's and Don'ts
Specific guidelines based on past experience:
- DO: Specific action
- DON'T: Specific pitfall

### When to Stop
Explicit conditions:
- Complete when: X, Y, Z
- Escalate if: A, B, C
- Ask for help when: D, E, F

### Verification Steps
Numbered, specific, copy-pasteable:
1. Run: exact command
2. Expect: exact output
3. Check: specific condition

### Agent Notes (NEW)
Special instructions for AI agents:
- Read these files first
- Use these patterns
- Check these invariants
- Common mistakes to avoid
```

#### 3. Reference Implementations

Create `.beads/reference/` with working examples:

**reference/step-functions.sh**
```bash
#!/bin/bash
# Reference implementation of all ashell_step_* functions
# Copy and modify for actual implementation

ashell_step_extract_package() {
    local url="$1"
    local sha256="$2"
    local dest_dir="$3"

    ashell_log_info "Downloading from $url"

    mkdir -p "$dest_dir"
    local tmp_file="$dest_dir/.download.tmp"

    # Download with retry
    local retries=3
    while ((retries-- > 0)); do
        if curl -fsSL --connect-timeout 30 "$url" -o "$tmp_file" 2>/dev/null; then
            break
        fi
        ashell_log_warn "Download failed, retrying... ($retries left)"
        sleep 2
    done

    if [[ ! -f "$tmp_file" ]]; then
        ashell_log_error "Download failed after all retries"
        return 1
    fi

    # Verify SHA256
    if command -v shasum >/dev/null 2>&1; then
        local computed_hash=$(shasum -a 256 "$tmp_file" | cut -d' ' -f1)
    else
        local computed_hash=$(sha256sum "$tmp_file" | cut -d' ' -f1)
    fi

    if [[ "$computed_hash" != "$sha256" ]]; then
        ashell_log_error "SHA256 mismatch!"
        ashell_log_error "  Expected: $sha256"
        ashell_log_error "  Computed: $computed_hash"
        rm -f "$tmp_file"
        return 1
    fi

    # Extract based on extension
    case "$url" in
        *.tar.gz|*.tgz)
            tar -xzf "$tmp_file" -C "$dest_dir" --strip-components=1
            ;;
        *.tar.bz2|*.tbz2)
            tar -xjf "$tmp_file" -C "$dest_dir" --strip-components=1
            ;;
        *.tar.xz)
            tar -xJf "$tmp_file" -C "$dest_dir" --strip-components=1
            ;;
        *.zip)
            unzip -q "$tmp_file" -d "$dest_dir"
            ;;
        *)
            ashell_log_error "Unknown archive format: $url"
            return 1
            ;;
    esac

    rm -f "$tmp_file"
    ashell_log_success "Extracted to $dest_dir"
    return 0
}
```

**reference/package-template/build.sh**
```bash
#!/bin/bash
# Template for new packages - copy and customize

ASHELL_PKG_NAME="{{PACKAGE_NAME}}"
ASHELL_PKG_VERSION="{{VERSION}}"
ASHELL_PKG_SRCURL="{{SOURCE_URL}}"
ASHELL_PKG_SHA256="{{SHA256}}"
ASHELL_PKG_DEPENDS="{{DEPENDENCIES}}"
ASHELL_PKG_BUILD_DEPENDS="{{BUILD_DEPS}}"
ASHELL_PKG_COMMANDS=("{{cmd}}:{{entry_point}}::{{type}}")

# Optional: Override default steps
ashell_step_pre_configure() {
    # Add custom logic before configure
    :  # noop placeholder
}

# Load build system
source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"
```

#### 4. Agent Notes Section

Add to every issue:

```markdown
## 🤖 Agent Notes

### If You're an AI Agent Starting This Task:

**READ FIRST:**
1. Read the entire issue description
2. Read any "reference/" files mentioned
3. Look at existing similar implementations in the codebase
4. Check if tests exist that define expected behavior

**BEFORE CODING:**
1. Verify you can run existing code (if any)
2. Understand the ASHELL_ conventions
3. Check for similar patterns in existing files

**COMMON MISTAKES TO AVOID:**
- Don't hardcode paths - use $ASHELL_PREFIX
- Don't assume macOS/Xcode - use lazy loading
- Don't skip error handling - every command can fail
- Don't use global stdout - use ios_stdout() in C/Swift

**WHEN STUCK:**
1. Check existing working implementations in ashell-packages/
2. Look at ios_system/ for API patterns
3. Review docs/api/ios_system_contract.md
4. If still stuck, document what you tried and why it failed

**VERIFICATION CHECKLIST:**
Before claiming done:
- [ ] Code follows existing style conventions
- [ ] Error handling is comprehensive
- [ ] All success criteria are checked
- [ ] Verification steps all pass
- [ ] No TODOs or placeholder comments remain
```

### Execution Plan

1. **Create reference implementations** first (provides code to reference)
2. **Create task specification template** (defines the format)
3. **Update existing issues** using the template (enhance descriptions)
4. **Add agent notes** to all issues (AI-specific guidance)

### Files to Create
- `.beads/TASK_SPECIFICATION.md` - The template standard
- `.beads/reference/step-functions.sh` - Working bash examples
- `.beads/reference/package-template/build.sh` - Package template
- `.beads/reference/swift-command.swift` - Swift command template
- `.beads/AGENT_GUIDE.md` - AI agent onboarding guide

### Three-Tier Hierarchy

Since beads doesn't have native milestones, we use labels:
- **Milestones** (type=epic, label=milestone): M0-M6 - High-level phases
- **Epics** (type=epic, label=epic): Functional areas within milestones
- **Tasks** (type=task): Individual work items

**Package Manager:** `apt` (built from Debian source in root-packages/apt/)

**Package Locations:**
- `ashell-packages/root-packages/` - Core distro packages (apt, dpkg, libz, etc.)
- `ashell-packages/packages/` - Optional user packages (nvim, vim, python, etc.)

**Milestone M0: Contract & Documentation Foundation**
- Epic: Documentation
  - Task: ios_system contract document
  - Task: Porting guide
  - Task: Package manifest schema
  - Task: Test strategy

**Milestone M1: Platform Hardening**
- Epic: ios_system Core
  - Task: Trace hooks
  - Task: Environment standardization
  - Task: Runtime registration API
  - Task: Thread-safety hardening

**Milestone M2: Forge Alpha (CRITICAL)**
- Epic: Build System
  - Task: ashell_package.sh library
  - Task: build.sh orchestration
  - Task: CI pipeline
- Epic: Toolchain
  - Task: iOS cross-compilation
  - Task: XCFramework generation
- Epic: Root Packages
  - Task: apt (from Debian source with patches)
  - Task: dpkg
  - Task: libz, libssl (dependencies)
- Epic: Reference Package
  - Task: hello package
  - Task: libarchive extraction
  - Task: plist generation

**Milestone M3: Core Shell Beta**
- Epic: Command Hardening
  - Task: Tier A commands
  - Task: Conformance tests
  - Task: File-safety tests
  - Task: Session validation

**Milestone M4: Interactive UX**
- Epic: System Info
  - Task: Library APIs
- Epic: LineEditor
  - Task: Integration
  - Task: History/completion

**Milestone M5: Heavy Packages**
- Epic: Python
  - Task: Python runtime
  - Task: Wheel forge
  - Task: Curated packages
- Epic: SSH
  - Task: SSH tools package
- Epic: Optional Packages
  - Task: nvim (Neovim)
  - Task: vim
  - Task: git

**Milestone M6: Release Candidate**
- Epic: Validation
  - Task: Benchmarks
  - Task: Crash rate
  - Task: Documentation

---

# a-Shell-next: Modernization Plan

## Context

This plan outlines the evolution of a-Shell from its current state to a more robust, maintainable, and extensible Unix-like environment for iOS. The core insight is to treat **ios_system** as the platform contract, **a-Shell** as the host app, and build a proper **package forge** that produces native iOS packages targeting the ios_system contract.

### Approach: Termux-Inspired Package Building (ashell-packages)

Following the model of [termux-packages](https://github.com/termux/termux-packages), the forge uses:
- **Bash build scripts** (`build.sh`) instead of YAML configs
- **Git diff patches** (`patches/*.patch`) for upstream modifications
- **Standard build lifecycle** (`ashell_step_*` functions)
- **iOS-specific additions**: XCFramework generation, plist metadata, code signing

**Naming convention**: `ASHELL` prefix for environment variables, `ashell_` prefix for functions.

### Current State
- **ios_system**: Already provides `ios_system()` as a drop-in replacement for `system()`, command dictionaries (plist-based registration), thread-local I/O (`thread_stdin`, `thread_stdout`), `ios_popen`, `replaceCommand`, `addCommandList`, and various ported Unix commands
- **a-Shell**: Host app with multi-window support, terminal UI, `pkg` command for WASM-based packages, session management, bookmarks, Shortcuts integration
- **libarchive**: Already integrated in ios_system for archive operations
- **Package system**: Currently WASM-focused with `pkg install`

### Target Architecture
Three distinct layers (Termux-inspired naming):
1. **ashell-system** (was ios_system): The Unix system layer - syscall replacements (`fork`→`ashell_fork`), thread-local I/O, process simulation. Drop-in replacement for Linux system calls.
2. **ashell-app** (was a-Shell): The iOS app - terminal UI (SwiftTerm), session management, package UI
3. **ashell-packages**: Package forge - build system for native iOS packages

**Supporting modules:**
- **ashell-shared**: Shared constants, utilities, headers used by all modules
- **ashell-core**: Core command implementations (file, text, shell commands)

### Process Spawning Architecture: Pluggable Backends

**Goal**: Support both App Store-compliant pthread simulation AND true process spawning (SpawnKit-style) via abstraction.

```
┌─────────────────────────────────────────────────────────────┐
│                    ashell-app                               │
│  Terminal UI → Session Manager → Command Dispatcher         │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼ (Protocol-based abstraction)
┌─────────────────────────────────────────────────────────────┐
│              ProcessManager (Protocol)                        │
│  ┌─────────────────────┐    ┌─────────────────────┐          │
│  │   PthreadBackend    │    │   SpawnKitBackend   │          │
│  │   (App Store)       │    │   (Jailbreak/Alt)   │          │
│  │                     │    │                     │          │
│  │ - Simulated PIDs    │    │ - Real posix_spawn  │          │
│  │ - Thread-based      │    │ - Real processes    │          │
│  │ - FILE* redirection │    │ - FD inheritance    │          │
│  │ - ios_system()      │    │ - waitpid()         │          │
│  └─────────────────────┘    └─────────────────────┘          │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              ashell-system (Shims & I/O)                    │
│  - Thread-local I/O (thread_stdin/out/err)                │
│  - Syscall replacements (fork→pthread_create)                 │
│  - Session state management                                  │
└─────────────────────────────────────────────────────────────┘
```

**The Protocol:**
```c
// ashell_process.h

typedef struct {
    pid_t (*spawn)(const char* path, char* const argv[], char* const envp[],
                   int stdin_fd, int stdout_fd, int stderr_fd);
    int (*waitpid)(pid_t pid, int* status, int options);
    int (*kill)(pid_t pid, int sig);
    int (*isRunning)(pid_t pid);
} ashell_process_backend_t;

// Global backend (set at init)
extern ashell_process_backend_t* g_process_backend;

// App Store compliant (default)
ashell_process_backend_t* ashell_pthread_backend(void);

// SpawnKit-based (if available)
ashell_process_backend_t* ashell_spawnkit_backend(void);
```

**Backend Selection (User Decision): Compile-time via `#ifdef USE_SPAWNKIT`**

```c
// ashell_config.h
#ifdef USE_SPAWNKIT
  #define ASHELL_BACKEND_SPAWNKIT 1
  #include <spawn.h>  // Real posix_spawn
#else
  #define ASHELL_BACKEND_PTHREAD 1
  // Pthread simulation only
#endif
```

**Build variants:**
```bash
# App Store build (default)
swift build --target ashell-system

# Jailbreak/AltStore build with real process spawning
swift build --target ashell-system -D USE_SPAWNKIT
```

**Benefits:**
- **App Store**: Pthread backend - no `fork`, no `posix_spawn`, pure thread simulation
- **Jailbreak/AltStore**: SpawnKit backend for real process semantics (compile-time flag)
- **Future-proof**: Can add new backends (e.g., XPC-based, WASM-based)
- **Testable**: Mock backend for unit testing

### SpawnKit Integration Strategy

**SpawnKit** (https://github.com/speedyfriend433/SpawnKit) provides:
- Swift `Process` class wrapping `posix_spawn`
- `FileActions` for FD redirection
- `SpawnAttributes` for spawn flags
- `CInterop` for C string conversions

**Key Findings:**
| Component | Value | Integration |
|-----------|-------|-------------|
| `FileActions` | Clean FD management | Replace manual FILE* manipulation |
| `CInterop` | C/Swift interop | Reuse for argv/envp conversion |
| `Process` | High-level API | Reference for `ashell_process_backend_t` |

**When to use SpawnKit:**
- Sideloaded builds (TestFlight, Enterprise)
- Jailbroken devices
- Future iOS versions if Apple relaxes restrictions

### Syscall Replacement Architecture (Key Insight)

The current ios_system uses a **header-based macro replacement** system:

```c
// ios_error.h
#define exit ios_exit
#define fork ios_fork
#define execv ios_execv
#define getenv ios_getenv
#define system ios_system
// ... etc
```

**How it works for packages:**
1. Packages compile with `-include ashell_error.h` (forced header inclusion)
2. All standard calls are redirected to `ios_*` implementations at compile time
3. `ios_*` functions provide iOS-compatible behavior (e.g., `ios_fork()` returns error instead of crashing)
4. Thread-local I/O (`thread_stdout`, etc.) ensures output goes to correct terminal session

**For the build system:**
- Packages must link against `ashell-system`
- Build recipes must use `-include ashell_error.h` or `#include "ashell_error.h"` in patched sources
- XCFrameworks embed the platform headers for self-contained compilation

---

## Phase 1: Package Forge (HIGHEST PRIORITY) - REVISED POST-VALIDATION

### Goals
Build a build system that produces ios_system-targeted XCFrameworks. **Packages are PRE-BUNDLED in the app, not downloaded.**

### Decisions (Post 4-Subagent Roast)
- **Primary**: Pre-bundled XCFrameworks (native performance, App Store compliant)
- **Secondary**: WebAssembly fallback (downloadable, App Store treats as "content")
- **Package Format**: XCFrameworks + plist for bundled packages
- **Filesystem Model**: Termux-style PREFIX layout (not FHS-compliant)

### Critical Constraints (From Research)

**What We CANNOT Do:**
1. ❌ Download native XCFrameworks at runtime (violates App Store 2.5.2)
2. ❌ Use real apt/dpkg (requires fork/exec)
3. ❌ Install compiled Python extensions via pip
4. ❌ Use libarchive with Swift (no bindings exist)

**What We CAN Do:**
1. ✅ Build XCFrameworks offline, bundle in app
2. ✅ Use familiar `apt` command name (but simpler implementation)
3. ✅ Download WebAssembly commands (proven by original a-Shell)
4. ✅ Pre-bundle Python + scientific stack

### Deliverables
- [ ] Package recipe format specification (Termux-style build.sh)
- [ ] Build toolchain for cross-compiling Unix tools to iOS XCFrameworks
- [ ] Package manifest that generates plist entries for command registration
- [ ] Archive extraction for WASM packages (libcompression + custom tar, NOT libarchive)
- [ ] CI pipeline for automated package builds (GitHub Actions with macOS runners)

### Termux-Inspired Filesystem Model

Following Termux's approach for Android constraints:

```
~/Library/ashell/              # $PREFIX - package root (like Termux's /data/data/.../usr)
├── bin/                        # Commands (not /bin)
├── lib/                        # Libraries (not /lib)
├── include/                    # Headers
├── etc/                        # Configs
├── share/                      # Data files
└── tmp/                        # Temp directory
```

**Key Principles (from Termux):**
- **No FHS compliance**: Don't create `/bin`, `/usr`, `/etc` in the filesystem
- **PREFIX as root**: All packages install to `$PREFIX`, never to system directories
- **Patch hardcoded paths**: At build time, replace `/bin`, `/usr`, `/etc` with `$PREFIX` equivalents
- **Shebang handling**: Provide `ashell-fix-shebang` utility to rewrite `#!/bin/sh` → `#!$PREFIX/bin/sh`
- **Relocatable**: PREFIX can move (e.g., between app versions) without breaking packages

**iOS Differences from Termux:**
- Termux: `$PREFIX = /data/data/com.termux/files/usr`
- a-Shell: `$PREFIX = ~/Library/ashell` (or `~/Documents/.ashell`)
- Termux: Patches for Bionic libc
- a-Shell: Patches for iOS BSD libc + sandbox restrictions

### Deliverables
- [ ] Package recipe format specification
- [ ] Build toolchain for cross-compiling Unix tools to iOS XCFrameworks
- [ ] Package manifest that generates plist entries for command registration
- [ ] Replace pkg extraction backend with libarchive
- [ ] CI pipeline for automated package builds

### Dependencies
- **libcompression** (Apple native) - Gzip/bzip2 decompression
- **ZIPFoundation** - ZIP archive handling (Swift library)
- **Custom Swift tar** - Tar archive extraction (implement in ashell-core)
- **swift-crypto** - Package checksums and signatures (host/tooling layer)

### Archive Handling Strategy (Per User Direction)

Use Apple's native frameworks instead of libarchive:

| Format | Library | Implementation |
|--------|---------|----------------|
| .zip | ZIPFoundation | Swift Package |
| .tar.gz | libcompression + custom tar | Apple native + Swift |
| .tar.bz2 | libcompression + custom tar | Apple native + Swift |
| .tar.xz | liblzma (if available) + custom tar | System library |

Rationale: libarchive has no Swift bindings. Using Apple's native libcompression avoids C bridging complexity.

### Package Recipe Format (Termux-Style)
Inspired by [termux-packages](https://github.com/termux/termux-packages), use bash build scripts + git diff patches.

```
ashell-packages/zip/
├── build.sh           # Build recipe (bash)
├── patches/           # Git diff patches applied in order
│   ├── 01-ios-prefix.patch       # Replace /usr with @ASHELL_PREFIX@
│   ├── 02-sandbox.patch          # iOS sandbox fixes
│   └── 03-hardcoded-paths.patch  # Replace hardcoded /bin, /etc
├── commands.plist     # Generated command metadata
└── LICENSE            # Package license
```

**Patch Philosophy (from Termux):**
- **Minimal patches**: Only patch what's necessary for iOS + PREFIX model
- **Upstream-first**: Submit patches upstream when possible
- **Reusable patterns**: Common patches for PREFIX, sandbox, hardcoded paths
- **Named sequentially**: `01-`, `02-`, etc. for order of application

**Common Patch Types:**
1. **PREFIX patches**: Replace `/usr`, `/bin`, `/etc` with `@ASHELL_PREFIX@` (substituted at build time)
2. **Sandbox patches**: Remove/replace `fork()`, `exec()`, hardcoded paths
3. **Shebang patches**: Fix `#!/bin/sh` → `#!/usr/bin/env sh` or use ashell-fix-shebang
4. **iOS-specific**: `stat()` differences, `dirent` issues, signal handling

**build.sh example:**
```bash
ASHELL_PKG_NAME="zip"
ASHELL_PKG_VERSION="3.0"
ASHELL_PKG_SRCURL="https://example.com/zip-3.0.tar.gz"
ASHELL_PKG_SHA256="abc123..."
ASHELL_PKG_DEPENDS="libz"
ASHELL_PKG_BUILD_DEPENDS="cmake"
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--prefix=@ASHELL_PREFIX@"

ashell_step_pre_configure() {
    # Patch or configure before build
    export CC="$CC -target arm64-apple-ios14.0"

    # Replace @ASHELL_PREFIX@ in patches with actual path
    substitute_prefix "$ASHELL_PKG_BUILDER_DIR/patches"
}

ashell_step_post_make_install() {
    # Fix shebangs in installed scripts
    ashell-fix-shebang "$ASHELL_PKG_PREFIX/bin/"

    # Generate plist from ASHELL_PKG_COMMANDS metadata
    generate_plist "$ASHELL_PKG_COMMANDS"
}
```

**commands.plist (generated):**
```xml
<key>zip</key>
<array>
    <string>zip.framework/zip</string>
    <string>zip_main</string>
    <string>r0qndD...</string>
    <string>file</string>
</array>
```

### Key Files to Create/Modify
- New: `ashell-packages/build.sh` - Main build orchestration (Termux-style)
- New: `ashell-packages/ashell_package.sh` - Build system library (functions like ashell_step_*)
- New: `ashell-packages/<name>/build.sh` - Per-package build recipes
- New: `ashell-packages/<name>/*.patch` - Git diff patches applied in order
- New: `ashell-packages/<name>/commands.plist` - Generated command metadata
- New: `ashell-packages/.build/` - Build outputs (XCFrameworks)
- Modify: `a-shell/a-Shell/ExtraCommands.swift` - Update pkg command

### Termux-Style Build System

**Build lifecycle (mirrors termux-packages):**
```
ashell_step_extract_package()     # Download & extract source
ashell_step_patch_package()       # Apply patches/*.patch files
ashell_step_pre_configure()       # Pre-configure hooks
ashell_step_configure()           # cmake/configure
ashell_step_make()                # Build
ashell_step_make_install()        # Install to staging
ashell_step_post_make_install()   # Generate plist, codesign
```

**iOS-specific additions:**
- `ashell_step_create_xcframework()` - Create XCFramework from build
- `ashell_step_generate_plist()` - Generate commandDictionary entries
- `ashell_step_codesign()` - Sign frameworks for iOS

### Termux-Inspired Utilities

Following Termux's helper tools:

**`ashell-fix-shebang`** (like `termux-fix-shebang`):
```bash
# Rewrite shebangs from #!/bin/sh to #!$PREFIX/bin/sh
ashell-fix-shebang script.sh
# Or to use env:
ashell-fix-shebang --use-env script.sh  # → #!/usr/bin/env sh
```

**Alternative: Runtime shebang handling (like termux-exec)**
Termux uses `LD_PRELOAD` with `libtermux-exec.so` to intercept execve() and rewrite shebangs at runtime:
```bash
export LD_PRELOAD="$PREFIX/lib/libashell-exec.so"
# Now scripts with #!/bin/sh work without modification
```
For iOS, this would require a different approach (perhaps via ios_system's command wrapper), but the concept is valuable.

**`ashell-info`** (like `termux-info`):
```bash
# Show environment details for debugging
$ ashell-info
Application version: 2.1.0
iOS version: 17.2
PREFIX: /var/mobile/Containers/Data/Application/.../Library/ashell
Target SDK: iphoneos17.2
Architecture: arm64
```

**`ashell-chroot`** (like `termux-chroot`, optional):
```bash
# Create a fake FHS environment with symlinks for compatibility
# Not recommended for production, but useful for testing
ashell-chroot
# Now /bin, /usr, /etc exist as symlinks to $PREFIX
```

**Environment setup** (in `.profile`):
```bash
export PREFIX="~/Library/ashell"
export PATH="$PREFIX/bin:$PATH"
export LD_LIBRARY_PATH="$PREFIX/lib"  # If needed for dynamic libs
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
export ACLOCAL_PATH="$PREFIX/share/aclocal"
# etc.
```

---

## Phase 2: ios_system Contract & Documentation

### Goals
Formalize the ios_system platform contract so package authors know what they can rely on. Aim for Linux syscall(2) compatibility where possible, following Termux's example.

### Decisions
- **Synthetic Filesystems**: Use Library API approach - commands call ios_system APIs (`ios_getprocinfo()`, `ios_getsysinfo()`, etc.) instead of file reads for system info.
- **Syscall Compatibility**: Document which Linux syscalls work, which are emulated, which fail

### Termux-Inspired Syscall Compatibility Model

Termux handles Linux differences by:
1. **Patching at build time** - Replace unsupported calls with alternatives
2. **Library shims** - Provide compatible implementations
3. **Clear failure** - Unsupported operations fail explicitly, not silently

**For a-Shell:**

| Syscall Category | iOS Support | Strategy |
|-----------------|-------------|----------|
| `fork()` | No | Return -1, set errno=ENOSYS |
| `exec*()` | Partial | Use ios_execv() shim |
| `ptrace()` | No | Fail with ENOSYS |
| `socket()` | Yes (sandboxed) | Standard BSD sockets |
| `pipe()` | Yes | Standard pipes |
| `open()` | Yes (sandboxed) | Respect miniroot |
| `stat()` | Yes | iOS-compatible stat |
| `getuid()` | Returns fixed | Return mobile user ID |
| `ioctl()` | Limited | Support TTY ioctls |

**Library shims to provide (in ios_system):**
```c
// fork/exec family
pid_t fork(void);           // Returns -1, errno=ENOSYS
int execv(const char*, char* const[]);  // Maps to ios_execv()
int system(const char*);    // Maps to ios_system()

// procfs replacement
int ios_proc_pidpath(pid_t, char*, uint32_t);  // Like Linux /proc/PID/exe
int ios_proc_listallpids(void*, int);          // Like /proc scan

// TTY handling
int isatty(int);            // ios_isatty()
int ioctl(int, unsigned long, ...);  // Filtered ioctls
```

**Documentation:**
- `docs/SYSCALL_COMPATIBILITY.md` - Which syscalls work, which don't
- `docs/PATCHING_GUIDE.md` - How to patch packages for iOS (Termux-style)
- `docs/COMMON_PATCHES.md` - Reusable patch patterns

### Deliverables
- [ ] ios_system Contract Document - APIs, behaviors, guarantees
- [ ] ios_system Porting Guide - step-by-step porting instructions
- [ ] a-Shell Host Boundary Document - what belongs where
- [ ] Add structured trace hooks for debugging

### New APIs to Add (for synthetic filesystem replacement)
Instead of `/proc` files, provide:
```c
// ios_system.h additions
const char* ios_getproc_self(void);      // Process info
const char* ios_getcpuinfo(void);        // CPU info
const char* ios_getmeminfo(void);        // Memory info
const char* ios_getversion(void);        // Version string
```

### Key Files
- `ios_system/ios_system.h` - Document all public APIs
- `ios_system/ios_error.h` - Document stdio redirection
- `ios_system/Resources/commandDictionary.plist` - Reference for command registration
- New: `docs/ios_system_contract.md`
- New: `docs/porting_guide.md`

---

## Phase 3: Shell Profile Core

### Goals
Harden the commands people use daily to behave consistently under the ios_system contract.

### Commands to Harden
- Essential: `sh`, `ls`, `cat`, `cp`, `mv`, `rm`, `mkdir`, `touch`, `ln`, `pwd`, `env`
- Text processing: `grep`, `find`, `sed`, `awk`, `sort`, `head`, `tail`, `tee`, `wc`, `xargs`
- Archives: `tar`, `gzip`
- Info: `stat`

### Note
Many of these already exist in ios_system (awk, curl, files, shell, tar, text frameworks). The work is ensuring consistent behavior under the ios_system contract.

### Existing Command Locations
- `ios_system/file_cmds/` - BSD file commands (cp, mv, rm, etc.)
- `ios_system/text_cmds/` - BSD text utilities (cat, grep, sed, etc.)
- `ios_system/shell_cmds/` - BSD shell commands (sh, ls, etc.)
- `ios_system/awk/` - One true awk
- `ios_system/tar/` - libarchive-based tar
- `ios_system/curl/` - curl

---

## Phase 4: Synthetic System Info (Library API Approach)

### Decision
Use Library API approach instead of filesystem simulation. Commands call ios_system APIs rather than reading `/proc` or `/dev` files.

### New APIs to Add to ios_system.h
```c
// Process info (replaces /proc/self/*)
typedef struct {
    pid_t pid;
    pid_t ppid;  // simulated parent pid
    uid_t uid;
    gid_t gid;
    char cwd[MAXPATHLEN];
    char environ[];  // serialized env vars
} ios_proc_info_t;

ios_proc_info_t* ios_getproc_info(pid_t pid);
void ios_freeproc_info(ios_proc_info_t* info);

// System info (replaces /proc/cpuinfo, /proc/meminfo)
typedef struct {
    uint64_t total_ram;
    uint64_t available_ram;
    uint32_t cpu_count;
    char cpu_arch[16];
    char version[256];
} ios_sys_info_t;

ios_sys_info_t ios_getsys_info(void);

// TTY info
int ios_opentty(void);
void ios_closetty(void);
int ios_gettty(void);
```

### Commands to Update
- `ps` - Use ios_getproc_info() instead of /proc
- `top` - Use ios_getsys_info() for CPU/memory
- `uname` - Use ios_getsys_info() for version/arch
- Interactive shells - Use ios_gettty() for terminal detection

---

## Phase 5: Terminal UI with SwiftTerm

### Decision: Adopt SwiftTerm AFTER M2 Stabilizes

**Merge Timeline**: After Milestone M2 (Forge Alpha) stabilizes, not immediately.

**Rationale**:
- Package forge is highest priority
- SwiftTerm is significant architectural change - introduces risk
- Better to have stable package system before major UI refactor

### SwiftTerm Adoption Plan
**Reference**: https://github.com/holzschu/a-shell/tree/SwiftTerm2

SwiftTerm replaces WKWebView-based terminal rendering with a native Swift terminal emulator.

### Key Changes in SwiftTerm2 Branch
- New file: `SceneDelegate+TerminalView.swift` - Terminal UI management
- Modified: `ContentView.swift` - SwiftUI integration with SwiftTerm
- Modified: `SceneDelegate.swift` - Refactored to use TerminalView instead of WebView
- Added: `TerminalView+KeyCommands.swift` - Keyboard shortcut handling
- Submodule: `SwiftTerm @ 03a7b73` - Specific SwiftTerm version

### Architecture Benefits
1. **Native Performance**: No WebView overhead for terminal rendering
2. **Better Keyboard Support**: Native iOS keyboard handling
3. **Reduced Complexity**: No JavaScript bridge for terminal operations
4. **Better VT100/xterm emulation**: SwiftTerm provides more accurate terminal emulation

### Migration Path
```
Current (master branch):
WKWebView (terminal rendering)
  → JavaScript
  → WebAssembly execution
  → DOM manipulation

Target (SwiftTerm2 branch):
TerminalView (SwiftTerm)
  → Native Swift
  → Direct iOS drawing
  → Better performance
```

### Integration Points
1. **ashell-system provides**: `thread_stdout`, `thread_stderr` FILE* streams
2. **SwiftTerm reads from**: These streams and renders to screen
3. **Keyboard input**: SwiftTerm captures and writes to `thread_stdin`

### New File Structure
```
ashell-app/Sources/
├── Terminal/
│   ├── TerminalView.swift           # SwiftTerm wrapper
│   ├── TerminalView+KeyCommands.swift
│   └── TerminalDelegate.swift       # Bridge to ios_system
├── Session/
│   └── SessionManager.swift         # Was SceneDelegate.swift
└── App/
    ├── App.swift
    └── ContentView.swift
```

---

## Appendix B: Syscall Replacement Deep Dive

### How ashell-system Works for Package Compilation

**Problem**: Packages expect standard Linux syscalls (`fork`, `execve`, `getpwnam`, etc.) that iOS doesn't support.

**Solution**: Header-based macro replacement + weak linking

#### The Three Layers

```
┌─────────────────────────────────────────────────────────────┐
│  Layer 3: Package Source Code                               │
│  - Thinks it's calling standard libc                        │
│  - Calls fork(), execve(), getenv()                         │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼ (compile-time macro replacement)
┌─────────────────────────────────────────────────────────────┐
│  Layer 2: ashell_error.h (Macros)                           │
│  #define fork ios_fork                                      │
│  #define execve ios_execve                                  │
│  #define getenv ios_getenv                                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼ (links to)
┌─────────────────────────────────────────────────────────────┐
│  Layer 1: ashell-system Library                           │
│  ios_fork() { errno = ENOSYS; return -1; }                  │
│  ios_execve() { /* shim to ios_system() */ }                │
│  ios_getenv() { /* per-session env */ }                     │
└─────────────────────────────────────────────────────────────┘
```

#### Package Build Recipe Example

```bash
# ashell-packages/htop/build.sh
ASHELL_PKG_NAME="htop"
ASHELL_PKG_VERSION="3.2.2"

ashell_step_pre_configure() {
    # Force include ashell_error.h for ALL compiled files
    export CFLAGS="$CFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"

    # Also add to configure script environment
    export CPPFLAGS="$CPPFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"
}

ashell_step_configure() {
    ./configure --host=$ASHELL_TARGET_PLATFORM \
                --prefix=$ASHELL_PREFIX \
                --disable-unicode
}
```

### Combined Approach: Header Macros + Dyld Interception

**User Decision**: Use both header-based macros AND dyld interception for maximum compatibility.

**Two-Layer Defense:**

```
Layer 1 (Compile-time): Header macros
  - #define fork ios_fork
  - Catches direct calls in package source code

Layer 2 (Runtime): Dyld symbol interposition
  - Intercepts calls from system libraries (libc, libSystem)
  - Catches dynamic calls and pre-built binaries
```

**Implementation:**

1. **Header macros** (ashell_error.h):
   ```c
   #define fork ios_fork
   #define execve ios_execve
   #define getenv ios_getenv
   // ... etc
   ```

2. **Dyld interception** (ashell_interpose.c):
   ```c
   #include <mach-o/dyld.h>

   // Interpose fork() at runtime
   int ashell_fork(void) {
       errno = ENOSYS;
       return -1;
   }

   // Structure used by dyld for symbol interposition
   typedef struct {
       const void *replacement;
       const void *original;
   } interpose_t;

   // Export interposition table
   __attribute__((used))
   static const interpose_t interposers[] = {
       { (const void *)ashell_fork, (const void *)fork },
       { (const void *)ashell_execve, (const void *)execve },
       // ... etc
   };
   ```

**Why Both?**
- Headers: Compile-time safety, zero runtime overhead, works for packages we build
- Dyld: Runtime interposition, catches system library calls, handles edge cases

**Build Recipe Integration:**
```bash
# In package build.sh, add both:
export CFLAGS="$CFLAGS -include $ASHELL_PREFIX/include/ashell_error.h"
export LDFLAGS="$LDFLAGS -Wl,-force_load,$ASHELL_PREFIX/lib/libashell_interpose.a"
```

#### What Each ios_* Function Does

| Standard Function | ios_* Replacement | Behavior on iOS |
|-------------------|-------------------|-----------------|
| `fork()` | `ios_fork()` | Returns -1, errno=ENOSYS (no fork on iOS) |
| `execve()` | `ios_execve()` | Redirects to `ios_system()` for command execution |
| `getenv()` | `ios_getenv()` | Returns per-session environment variable |
| `setenv()` | `ios_setenv()` | Sets per-session environment variable |
| `exit()` | `ios_exit()` | Calls `pthread_exit()` with return code |
| `write()` | `ios_write()` | Redirects to `thread_stdout` or `thread_stderr` |

#### Unsupported Syscalls Table

| Syscall | iOS Support | ashell-system Behavior |
|---------|-------------|-------------------------|
| `fork()` | No | Returns -1, errno=ENOSYS |
| `vfork()` | No | Returns -1, errno=ENOSYS |
| `execve()` | No | Redirects to `ios_system()` |
| `execvp()` | No | Redirects to `ios_system()` |
| `getpwnam()` | Partial | Returns static data for mobile user |
| `getpwuid()` | Partial | Returns static data |
| `ptrace()` | No | errno=EPERM |
| `socket()` | Limited | Allowed, but sandboxed |

#### Package Compatibility Strategy

1. **Single-Process Apps** (most CLI tools): Work out of the box
   - `ls`, `cat`, `grep`, `sed`, `awk` - no forking needed

2. **Shell Scripts**: Need patching to avoid fork/exec
   - Use `sh -c "cmd"` instead of direct fork+exec
   - Patch: `system("cmd")` works via `ios_system()`

3. **Multi-Process Apps** (like `htop`, `ps`): Need special handling
   - `htop`: Must use `ios_getproc_info()` instead of parsing `/proc`
   - `ps`: Must use iOS APIs for process enumeration
   - These get **runtime patches** in the package build

4. **Network Apps** (like `curl`, `ssh`): Work with limitations
   - iOS allows sockets but requires entitlements
   - DNS resolution works normally

#### Header Installation

```
ashell-system/Headers/
├── ashell.h              # Main API (was ios_system.h)
├── ashell_error.h        # MUST be included by all packages
├── ashell_proc.h         # Process info API
└── ashell_syscall.h      # Syscall replacement declarations
```

#### Example: Compiling a Simple Package

```bash
# htop expects: fork(), exec(), /proc filesystem

# Patch 01-ashell-compat.patch:
--- a/ProcessList.c
+++ b/ProcessList.c
@@ -1,5 +1,5 @@
 #include "ProcessList.h"
-#include <proc/readproc.h>
+#include "ashell_proc.h"  // Use iOS-compatible process API

 ProcessList* ProcessList_new() {
-    // Old: read from /proc
+    // New: use ios_getproc_info()
 }
```

---

## Phase 5: Interactive UX (LineEditor)

### Goals
Add proper interactive input for shell sessions using LineEditor.

### Dependencies
- **LineEditor** - Swift wrapper around libedit (history, tab completion)

### Design Rules
- Use LineEditor only for interactive commands (sh, python, lua)
- Never use for pipes or non-interactive stdin
- Completion hooks fed by command registry and filesystem view

### Implementation Approach
- Add LineEditor as Swift Package Manager dependency in a-Shell
- Create ios_system hooks for interactive mode detection
- Commands declare interactive capability in their plist entries
- Host app (a-Shell) provides the LineEditor integration

---

## Phase 6: Heavyweight Packages

### Python (Priority: High)
**Current**: Uses custom cpython build with iOS patches (Holzschu's fork).
**Target**: BeeWare Python-Apple-support (https://github.com/beeware/Python-Apple-support) + cibuildwheel, built via Termux-style recipes.

**Why BeeWare Python-Apple-support:**
- Officially supported by Python's PEP 730 iOS support
- Clean, reproducible builds with standard Apple tooling
- Framework-based distribution (not custom embedding)
- Integrates with AppleFrameworkLoader for C extension loading
- Maintained by the BeeWare project (active, documented)
- Replaces the ad-hoc Holzschu cpython fork

#### Directory Structure
```
ashell-packages/python/
├── build.sh                    # Main Python build recipe
├── patch/
│   ├── 01-ios-support.patch    # PEP 730 iOS patches (if needed)
│   └── 02-framework-loader.patch
└── scripts/
    └── post-process-wheel.sh   # Convert .so → frameworks
```

#### Actions
- [ ] Create `ashell-packages/python/build.sh` using BeeWare Python-Apple-support
- [ ] Fork/subtree https://github.com/beeware/Python-Apple-support into `ashell-runtimes/python-runtime/`
- [ ] Set up cibuildwheel for iOS wheel production (CI)
- [ ] Create wheel post-processor (`.so` → framework conversion)
- [ ] Create Python package recipes for common extensions (numpy, pillow, etc.)
- [ ] Set up pip index for iOS wheels

### SSH (Priority: Medium)
**Current**: ssh_cmd framework using libssh2.
**Target**: libssh2-iosx with standard Apple-targeted build via Termux recipe.

#### Directory Structure
```
ashell-packages/openssh/
├── build.sh              # Build recipe
├── patch/
│   ├── 01-ios-fixes.patch
│   └── 02-sandbox.patch
└── commands.plist        # Generated (ssh, scp, sftp, etc.)
```

#### Actions
- [ ] Create `ashell-packages/libssh2/build.sh` recipe
- [ ] Create `ashell-packages/openssl/build.sh` recipe (dependency)
- [ ] Create `ashell-packages/openssh/build.sh` recipe
- [ ] Generate unified plist for all SSH commands

### Git (Priority: Low - Future)
**Current**: lg2 command (libgit2 wrapper).
**Target**: Full libgit2 integration if needed.

#### Actions
- [ ] Evaluate if current lg2 is sufficient
- [ ] Add proper libgit2 as native package if needed

---

## Phase 7: Host Polish

### Goals
Improve a-Shell as the best host app for ios_system.

### Areas
- Tab/session isolation
- Keyboard handling
- Terminal rendering
- Files integration
- Package UI
- Package diagnostics

### SwiftTerm Evaluation
- **Current**: Uses WKWebView for terminal rendering
- **Consider SwiftTerm if**: Terminal rendering becomes a bottleneck
- **Decision**: Host polish, not substrate - evaluate only if needed

---

## Dependencies Summary

### Immediate Adoption (for plan phases)
| Package | Purpose | Integration Boundary |
|---------|---------|---------------------|
| libarchive | Archive extraction | ios_system platform layer |
| swift-crypto | Checksums/signatures | Host/tooling layer |
| LineEditor | Interactive input | ios_system optional path |
| BeeWare Python-Apple-support | Python runtime | Native package |
| cibuildwheel | Wheel building CI | Build toolchain |
| libssh2-iosx | SSH commands | Native package |

### Swift Ecosystem (for host/tooling)
| Package | Purpose |
|---------|---------|
| swift-system | System call interfaces |
| swift-log | Unified logging |
| swift-testing | Modern test framework |
| swift-snapshot-testing | Regression testing |
| swift-benchmark | Performance measurement |

### Future Consideration
| Package | Purpose | When |
|---------|---------|------|
| libgit2 | Git operations | After forge is stable |
| Toybox | Command source reservoir | When needed |

---

## Decisions Made

| Question | Decision |
|----------|----------|
| WASM backward compatibility | Native-first with WASM fallback |
| Phase 1 priority | Package Forge (most pressing) |
| Package format | Keep XCFrameworks + plist registration |
| Synthetic filesystems | Library API approach (not file-based) |
| First forge package | hello command (reference implementation) |
| Python C extensions | Drop-in replacement goal - pre-compiled wheels or XCFrameworks |
| Release strategy | Incremental releases (minor version bumps per phase) |

---

## 4-Subagent Validation Report: Reality Check

Based on deep research by 4 parallel subagents (Swift libraries, App Store compliance, Termux build system, Python/C extensions):

### Critical Findings

| Aspect | Original Plan | Reality | Verdict |
|--------|---------------|---------|---------|
| **apt package manager** | Build from Debian source | Requires fork/exec, violates App Store 2.5.2 | ❌ NOT REALISTIC |
| **Downloaded frameworks** | XCFrameworks via apt | Code signing prevents dlopen of downloaded code | ❌ BLOCKED |
| **pip install numpy** | Standard pip workflow | No iOS wheels on PyPI, C extensions can't be downloaded | ❌ PIPE DREAM |
| **Termux approach** | Copy their build system | Termux uses `pkg`, not apt; Android ≠ iOS | ⚠️ MISUNDERSTOOD |
| **SwiftTerm** | Terminal emulator | Production-ready, actively maintained (v1.12.0) | ✅ VIABLE |
| **WebAssembly** | Fallback option | Original a-Shell's proven approach, App Store compliant | ✅ RECOMMENDED |

### What Actually Works on iOS

**From research into existing apps:**

| App | Approach | App Store? |
|-----|----------|------------|
| **a-Shell (original)** | Pre-bundled + WASM downloads | ✅ Yes |
| **iSH** | x86 emulation + Alpine Linux | ✅ Yes |
| **Carnets** | Bundled Python + limited pip | ✅ Yes |
| **Pyto** | Pre-compiled frameworks only | ✅ Yes |
| **Cydia** | Native APT | ❌ Jailbreak only |

**Key insight**: No app on the App Store downloads native ARM binaries. WASM and emulation are the only viable paths for extensibility.

### Revised Strategy (Post-Roast)

**MUST abandon:**
1. Native apt package manager
2. Downloaded XCFrameworks
3. "pip install numpy" for users

**SHOULD pursue:**
1. **Curated pre-bundled packages** (native performance)
2. **WebAssembly command system** (downloadable, App Store compliant)
3. **Honest Python support** (pre-bundled scientific stack, pure Python pip only)

---

### Python C Extension Strategy (USER DECISION: Custom iOS Wheels)

**USER DIRECTION**: Use BeeWare Python-Apple-support and deliver C extensions through custom wheels prebuilt for iOS. We provide the compatibility layer.

**Architecture:**

```
Tier 1: Python Runtime (BeeWare)
├── Python-Apple-support framework
├── AppleFrameworkLoader for C extensions
└── Pre-configured environment

Tier 2: Prebuilt iOS Wheels (delivered via apt)
├── numpy-1.26.0-ios_13_0_arm64.whl
├── pandas-2.0.0-ios_13_0_arm64.whl
├── matplotlib-3.7.0-ios_13_0_arm64.whl
└── Custom wheel index for a-Shell

Tier 3: Pure Python (standard pip)
└── pip install requests # Works normally
```

**Technical Implementation:**

1. **Base Runtime**: BeeWare Python-Apple-support (Python 3.10-3.14)
   - Repo: https://github.com/beeware/Python-Apple-support
   - Provides iOS-compatible CPython with PEP 730 support
   - Framework-based distribution (not custom embedding)

2. **Wheel Building**: Custom build pipeline
   - Use Mobile Forge for creating iOS wheels
   - Cross-compile in CI (GitHub Actions with macOS runners)
   - Platform tag: `ios_13_0_arm64` (custom, not PyPI standard)

3. **Wheel Installation**: apt-based delivery
   - `apt install python-numpy` downloads prebuilt wheel
   - Extract to `~/Library/ashell/lib/python3.12/site-packages/`
   - Frameworks placed in app bundle's `Frameworks/` folder

4. **Compatibility Layer**: ashell-system provides
   - `ios_fork()` → pthread simulation (returns error for real fork)
   - `ios_execve()` → in-process command execution
   - Thread-local I/O for proper output capture

**Key Dependencies:**
- **Python-Apple-support**: Runtime base (actively maintained)
- **Mobile Forge**: Build iOS wheels in CI
- **AppleFrameworkLoader**: Official CPython iOS loader
- **ashell-system**: POSIX compatibility layer

**Reference:**
- PEP 730 (iOS support): https://peps.python.org/pep-0730/
- Mobile Forge: https://github.com/beeware/mobile-forge

---

## Decisions Made (Post Validation + User Direction)

| Question | Original Decision | Validated Decision | User Direction |
|----------|-------------------|-------------------|----------------|
| Package manager | `apt` (Debian-style) | Curated `apt` | **USE APT WITH PATCHES** - Build from Debian 2.8.1 |
| Package delivery | Downloaded XCFrameworks | Pre-bundled only | **DEBIAN-LIKE LINUX FOR iOS** - Custom delivery via apt |
| Python C extensions | pip install numpy | Pre-bundled only | **CUSTOM iOS WHEELS** - We build and deliver via apt |
| Archive handling | libarchive | libarchive has no Swift bindings | **USE APPLE'S LIBCOMPRESSION** + custom tar |
| Build system | Termux-style | ✅ Reuse patterns | **BUILD DEBIAN-LIKE LINUX FOR iOS** |

### User Direction Summary

1. **APT Package Manager**: Use `apt` command name, build from Debian upstream with iOS patches to handle fork/exec limitations through ashell-system

2. **Python C Extensions**: Use BeeWare Python-Apple-support, deliver C extensions as custom iOS wheels through our apt repository

3. **Build System**: Create "Debian-like Linux for iOS" - use Termux patterns but for iOS Mach-O binaries

4. **Archive Handling**: Use Apple's libcompression instead of libarchive (no Swift bindings exist)
| Synthetic filesystems | Library API | ✅ Correct | Avoids /proc simulation issues |
| First forge package | hello command | ✅ Correct | Good reference implementation |
| WASM fallback | Secondary | Primary for downloadable commands | Original a-Shell's proven approach |
| Release strategy | Incremental | ✅ Correct | Milestone-based still valid |
| Timeline | M0-M6 milestones | ✅ Correct | Calendar-agnostic phases still valid |

### Critical Pivots Based on Research

1. **apt → Curated Package Manager**: We're keeping the familiar `apt` command name, but implementing a simpler curated package manager that only installs pre-approved, pre-bundled packages. No dependency resolution, no GPG, no dpkg.

2. **Native Downloads → WebAssembly**: User-downloadable commands must be WebAssembly (WASM), not native. This is the only App Store-compliant approach proven by a-Shell and iSH.

3. **pip install numpy → Bundled Only**: Users cannot install compiled Python extensions. We pre-bundle numpy, pandas, etc. Pure Python packages work via pip.

4. **Termux-style → iOS-Adapted**: We borrow Termux's build script structure but NOT their package manager complexity. iOS constraints are stricter than Android.

---

## Milestone Sequence (M0-M6)

**Calendar-agnostic, capability-based milestones.**

### M0: Contract
- ios_system contract defined
- Porting guide written
- Package manifest schema defined
- Test strategy established

### M1: Platform Hardening
- ios_system APIs strengthened
- Trace hooks added
- Path/environment policy standardized
- Command registration formalized

### M2: Forge Alpha
- Native pkg backend
- Package manifests → plist
- Archive/install/remove flow
- Bootstrap package set (hello, etc.)

### M3: Core Shell Beta (Longest Phase)
- Tier A commands hardened
- Conformance suite in place
- Session behavior stable
- File-safety tests passing

### M4: Interactive Polish
- LineEditor integration
- History, completion working
- Improved shell UX

### M5: Heavy Packages Beta
- Python runtime (Python-Apple-support)
- SSH stack (libssh2-iosx)
- First curated advanced packages

### M6: Platform Release Candidate
- /proc, /dev (Library APIs)
- Package reliability targets
- Crash rate and performance targets

---

## Testing Strategy (Behavior-Based Gates)

**Not coverage-based. Gates defined per phase.**

### ios_system Contract Phase (M0-M1)
- [ ] 100% of documented APIs have smoke tests
- [ ] At least one end-to-end port per major porting pattern
- [ ] No undocumented behavior relied on by core packages

### Package Forge Phase (M2)
- [ ] 100% test pass on install, remove, upgrade, list, files for core packages
- [ ] Reproducible bootstrap from clean environment
- [ ] Integrity verification on every artifact
- [ ] Rollback/clean failure for interrupted installs

### Shell Profile Phase (M3)
- [ ] 90%+ pass rate on Tier A conformance suite
- [ ] 100% pass on file-safety tests (cp, mv, rm, tar, install, uninstall)
- [ ] Zero known crashers in sh, ls, cat, grep, find, sed, awk, tar

### System Info Phase (M4)
- [ ] 100% of documented APIs tested
- [ ] Every unsupported operation fails consistently

### Interactive UX Phase (M4)
- [ ] 100% separation between interactive/non-interactive paths
- [ ] History/completion tested for interactive commands
- [ ] Pipe/script regressions explicitly tested

### Coverage Targets (Supporting Metric)
- 80%+ on host/tooling code
- 90%+ on package manager and state machine
- Lower emphasis on legacy C command code

---

## Success Criteria

- [ ] New ports have one obvious target (ios_system contract)
- [ ] pkg can bootstrap from native package metadata
- [ ] Shell-profile core behaves predictably under session switching, pipes, redirection
- [ ] Interactive commands have history/completion; non-interactive never accidentally use interactive path
- [ ] Python is a first-class native package (pip install numpy works)
- [ ] SSH commands packaged through standard dependency chain
- [ ] Host app stays thin - new commands/packages require minimal UI changes

---

## Suggested PR Sequence (By Milestone)

### M0: Contract
1. Write ios_system contract document
2. Write ios_system porting guide
3. Define package manifest schema
4. Establish test strategy

### M1: Platform Hardening
5. Add structured trace hooks
6. Standardize environment/path initialization
7. Add runtime registration API
8. Harden thread-safety of session management

### M2: Forge Alpha
9. Create `ashell-packages/ashell_package.sh` build system
10. Create `ashell-packages/build.sh` main orchestration script
11. Create `ashell-packages/hello/build.sh` reference package
12. Add iOS cross-compilation toolchain
13. Add XCFramework generation step
14. Replace pkg extraction with libarchive
15. Add plist generation from build metadata
16. CI pipeline for automated package builds

### M3: Core Shell Beta
15. Harden Tier A commands
16. Build conformance test suite
17. File-safety testing
18. Session behavior validation

### M4: Interactive & System Info
19. Add Library APIs for system info (replaces /proc)
20. Integrate LineEditor
21. History/completion implementation

### M5: Heavy Packages
22. Python runtime (Python-Apple-support)
23. Wheel forge and post-processor
24. SSH tools (libssh2-iosx)
25. First curated Python packages

### M6: Release Candidate
26. Performance benchmarks
27. Crash rate validation
28. Documentation completion

---

## Agent Investigation Findings

### Critical Discovery: NO Existing `pkg` Command
The codebase references `pkg install` in error messages, but **no actual implementation exists**. This is a greenfield build, not a refactoring.

### Build System Reality
- No centralized build system exists - uses hybrid Xcode projects + FMake SPM
- Individual .xcodeproj files per command framework
- Binary XCFrameworks downloaded from GitHub releases
- No Termux-style recipe infrastructure exists

### Testing Vacuum
- Zero automated testing for Swift/iOS code
- CI only builds, doesn't test
- BSD upstream tools have C test suites (not integrated)
- Must build test harness from scratch

### Command Registration Architecture
- Works via `commandDictionary.plist` files
- `addCommandList()` dynamically loads additional plists
- `replaceCommand()` swaps implementations at runtime
- Frameworks loaded via dlopen(), entry points are `<command>_main()`

## User Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Minimum iOS Version | **iOS 16** | Enables modern Swift concurrency, drops very old devices |
| pkg Implementation | **Swift in ExtraCommands.swift** | Native Swift with `@_cdecl`, matches existing pattern |
| Package Storage | **Split: Library + Documents** | Binaries in `~/Library/ashell/`, configs in `~/Documents/.ashell/` |
| Testing Framework | **Swift Testing (modern)** | Clean async/await, iOS 16 compatible |

## Updated PREFIX Filesystem Model

Based on split storage decision:

```
~/Library/ashell/              # Binaries, frameworks (not user-editable)
├── Frameworks/                # XCFrameworks
│   ├── hello.framework/
│   └── zip.framework/
├── lib/                       # Shared libraries
└── libexec/                   # Helper programs

~/Documents/.ashell/           # User-editable configs
├── etc/                       # Configuration files
├── share/                     # Data files, man pages
├── var/log/                   # Package logs
└── tmp/                       # Temporary files
```

Environment variables to set:
```bash
export PREFIX="$HOME/Library/ashell"
export ASHELL_CONFIG="$HOME/Documents/.ashell"
export PATH="$PREFIX/bin:$PATH"
export MANPATH="$ASHELL_CONFIG/share/man:$MANPATH"
```

## Detailed Implementation Project Plan

### Milestone M2: Forge Alpha (Target: 8-10 weeks)

#### Week 1-2: Foundation & pkg Command Skeleton

**Deliverables:**
- `ashell-packages/` directory structure created
- `ashell_package.sh` library with core functions:
  - `ashell_step_extract_package()` - Download and verify SHA256
  - `ashell_step_patch_package()` - Apply patches in order
  - `ashell_step_configure()` - Run autotools/cmake
  - `ashell_step_make()` - Build with iOS toolchain
  - `ashell_step_make_install()` - Install to staging
  - `ashell_step_create_xcframework()` - Package as XCFramework
  - `ashell_step_generate_plist()` - Create command metadata
  - `ashell_step_codesign()` - Sign frameworks

**Code Structure:**
```bash
# ashell-packages/ashell_package.sh
set -e

# Configuration
ASHELL_PREFIX="${ASHELL_PREFIX:-$HOME/Library/ashell}"
ASHELL_CONFIG="${ASHELL_CONFIG:-$HOME/Documents/.ashell}"
ASHELL_HOST_PLATFORM="$(uname -m)-apple-darwin"
ASHELL_TARGET_PLATFORM="arm64-apple-ios16.0"

# Step functions (override in package build.sh)
ashell_step_pre_configure() { :; }
ashell_step_configure() { :; }
ashell_step_post_configure() { :; }
ashell_step_pre_make() { :; }
ashell_step_make() { :; }
ashell_step_post_make() { :; }
```

**Testing:**
- Unit tests for each step function
- Mock package that exercises all steps

#### Week 3-4: pkg Command Implementation

**Deliverables:**
- `@_cdecl("pkg")` function in `ExtraCommands.swift`
- Subcommands: `install`, `remove`, `search`, `update`, `list`, `info`
- URLSession-based download with progress
- libarchive integration for extraction
- Plist merging for command registration

**Code Structure:**
```swift
// a-shell/a-Shell/PackageManager.swift
import Foundation
import ios_system
import CryptoKit

public actor PackageManager {
    static let shared = PackageManager()

    let prefixURL: URL
    let configURL: URL
    let frameworksURL: URL

    func install(package: String, version: String?) async throws -> InstallResult
    func remove(package: String) async throws
    func list() async -> [InstalledPackage]
    func search(query: String) async -> [PackageMetadata]
}

// Export for C interop
@_cdecl("pkg")
public func pkg(argc: Int32, argv: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?) -> Int32 {
    // Parse arguments, dispatch to PackageManager
}
```

**Testing:**
- Mock package server (local HTTP server)
- Install/remove round-trip tests
- Error handling (network failure, corrupt archive)

#### Week 5-6: hello Reference Package

**Deliverables:**
- `ashell-packages/hello/build.sh` complete recipe
- `ashell-packages/hello/patches/01-prefix.patch`
- Cross-compilation toolchain verified
- XCFramework generation working
- Manual end-to-end test: `./build.sh hello` → `pkg install hello` → `hello` works

**Code Structure:**
```bash
# ashell-packages/hello/build.sh
ASHELL_PKG_NAME="hello"
ASHELL_PKG_VERSION="2.12.1"
ASHELL_PKG_SRCURL="https://ftp.gnu.org/gnu/hello/hello-${ASHELL_PKG_VERSION}.tar.gz"
ASHELL_PKG_SHA256="..."
ASHELL_PKG_DEPENDS=""
ASHELL_PKG_BUILD_DEPENDS=""
ASHELL_PKG_EXTRA_CONFIGURE_ARGS="--prefix=@ASHELL_PREFIX@"
ASHELL_PKG_COMMANDS=("hello:hello_main::no")

source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"
```

**C Source:**
```c
// ashell-packages/hello/src/hello.c (generated from template)
#include <stdio.h>
#include "ios_system.h"

int hello_main(int argc, char** argv) {
    fputs("Hello from a-Shell!\n", ios_stdout());
    return 0;
}
```

#### Week 7-8: Bootstrap Package Set

**Deliverables:**
- 5-10 essential packages working:
  - `hello` (reference)
  - `coreutils-minimal` (file, readlink, realpath)
  - `less` (pager)
  - `tree` (directory listing)
  - `htop` (system monitor, uses new Library APIs)

**Testing:**
- Bootstrap test: clean install, install all packages, verify commands work
- Integration tests for each package

#### Week 9-10: CI/CD & Polish

**Deliverables:**
- GitHub Actions workflow for package builds
- Automated XCFramework generation
- Checksum verification in CI
- Documentation: `docs/PACKAGE_BUILDING.md`

**CI Workflow:**
```yaml
# .github/workflows/packages.yml
name: Package Builds
on:
  push:
    paths: ['ashell-packages/**']
jobs:
  build:
    runs-on: macos-14
    strategy:
      matrix:
        package: [hello, coreutils-minimal, less, tree, htop]
    steps:
      - uses: actions/checkout@v4
      - name: Build ${{ matrix.package }}
        run: |
          cd packages
          ./build.sh ${{ matrix.package }}
      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          name: ${{ matrix.package }}.xcframework
          path: ashell-packages/.build/${{ matrix.package }}/*.xcframework
```

### Dependencies & Blockers

| Dependency | Status | Blocker For |
|------------|--------|-------------|
| iOS 16 SDK | Required | All Swift concurrency features |
| Xcode 15+ | Required | Swift Testing support |
| swift-crypto | Add to Package.swift | Checksum verification |
| libarchive | Already integrated | Archive extraction |
| GitHub Actions runners | Available | CI/CD |

### Risk Mitigation

| Risk | Mitigation |
|------|------------|
| XCFramework codesigning issues | Test on physical device early, use ad-hoc signing for CI |
| dlopen fails for some frameworks | Validate all architectures present, check deployment target |
| Thread-local I/O bugs | Extensive testing with concurrent sessions |
| Network reliability | Implement retry with exponential backoff, cache downloads |
| Package bloat | Implement cleanup command, track package sizes |

## First Steps (Immediate Actions)

To begin Phase 1 (Package Forge) with Termux-inspired approach:

1. **Create directory structure**:
   ```
   ashell-packages/                           # Top-level package forge
   ├── ashell_package.sh              # Build system library (Termux-style)
   ├── build.sh                       # Main build script
   ├── clean.sh                       # Clean build artifacts
   ├── scripts/
   │   ├── build-xcframework.sh       # XCFramework creation helper
   │   ├── codesign.sh                # Code signing utility
   │   └── generate-plist.swift       # Plist generation from metadata
   ├── hello/                         # Reference package
   │   ├── build.sh                   # Package build recipe
   │   ├── patches/
   │   │   └── 01-prefix.patch        # Patches applied in order
   │   └── commands.plist             # Generated (not committed)
   └── .build/                        # Build outputs (.gitignore)
       └── hello/
           └── src/                   # Downloaded source
   ```

2. **Create build system library** (`ashell-packages/ashell_package.sh`):
   - Implement `ashell_step_*` functions (like Termux's `termux_step_*`)
   - iOS cross-compilation toolchain setup
   - XCFramework generation helpers
   - Plist generation from command metadata
   - PREFIX substitution in patches

3. **Create hello reference package** (`ashell-packages/hello/build.sh`):
   ```bash
   ASHELL_PKG_NAME="hello"
   ASHELL_PKG_VERSION="1.0"
   ASHELL_PKG_SRCURL="..."
   ASHELL_PKG_SHA256="..."
   ASHELL_PKG_COMMANDS=("hello:hello_main::no")
   ```
   - Minimal C program that prints "Hello from a-Shell!"
   - Demonstrates source → XCFramework → plist generation
   - Uses PREFIX-aware paths

4. **Test build locally**:
   ```bash
   cd packages
   ./build.sh hello
   ```
   - Downloads source
   - Applies patches (with @ASHELL_PREFIX@ substitution)
   - Builds XCFramework
   - Generates plist
   - Signs framework

5. **Implement pkg command** (`a-shell/a-Shell/ExtraCommands.swift`):
   Add Swift implementation with `@_cdecl("pkg")`:
   ```swift
   @_cdecl("pkg")
   public func pkg(argc: Int32, argv: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?) -> Int32 {
       // Subcommands: install, remove, search, update, list, info
       // Uses URLSession for downloads, libarchive for extraction
       // Installs XCFrameworks to ~/Library/ashell/Frameworks/
       // Updates commandDictionary via addCommandList()
   }
   ```

6. **Create testing infrastructure**:
   - Add `ios_system/Tests/` directory with Swift Testing
   - Create `CommandTestHarness.swift` for capturing command output
   - Add behavior-based gates for Tier A commands
   - Update CI to run tests

7. **Create utilities**:
   - `ashell-fix-shebang` - Fix script shebangs
   - `ashell-info` - Environment info
   - `.profile` setup for PREFIX environment

8. **Document the contract** (parallel work):
   - Begin `docs/ios_system_contract.md`
   - Document `docs/SYSCALL_COMPATIBILITY.md`
   - Document `replaceCommand()` and `addCommandList()` APIs
   - Define what makes a command "ios_system compatible"

---

## Implementation Guardrails

### Code Quality Standards

1. **Swift Style**: Follow Swift API Design Guidelines
   - Clear, grammatical names
   - Protocol-oriented where appropriate
   - Error handling with `Result` or `throws`, not magic return values

2. **C Interop Safety**:
   - Always check pointer bounds before dereferencing
   - Use `withUnsafePointer`/`withUnsafeMutablePointer` scopes
   - Bridge `String` to C strings with explicit encoding
   - Check `dlopen`/`dlsym` results before use

3. **Thread Safety**:
   - Respect ios_system's thread-local I/O (`thread_stdout`, etc.)
   - Use `ios_switchSession()` before cross-session operations
   - Protect shared mutable state with actors or locks

4. **Memory Safety**:
   - No force-unwrapping (`!`) in production code
   - Use `defer` for cleanup (fclose, free, etc.)
   - Validate all file paths with `ios_setMiniRoot` restrictions

### Testing Requirements

Every new component must include:
- Unit tests for pure logic (Swift Testing)
- Integration tests for ios_system interaction
- Snapshot tests for command output verification
- At least one behavior-based gate per Tier A command

### Documentation Requirements

- All public APIs get doc comments (`///`)
- Complex algorithms get inline comments explaining "why"
- Build scripts get usage headers with examples
- User-facing commands get man page entries

## Appendix A: Codebase Modularization Plan

### User Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Repository Structure | **Multi-repo from start** | Clean boundaries, independent versioning, enables community contributions |
| Python Decoupling | **High priority (immediate)** | Python becomes optional package via `pkg install python` |
| Build System | **Full SPM unification** | Single `swift build` for entire project |

### Repository Structure (Multi-Repo)

```
github.com/rudironsoni/           # Your GitHub org
├── ashell-system/             # Unix system layer - syscall replacements
│   ├── Package.swift
│   ├── Sources/
│   │   ├── ashell_error.h        # ⭐ CRITICAL: syscall macros
│   │   ├── ashell_system.h       # Main system API
│   │   ├── ashell_interpose.c    # Dyld symbol interposition
│   │   └── ashell_shims.c        # ios_fork, ios_execve, etc.
│   └── Tests/
│
├── ashell-shared/             # Shared headers/constants (was part of ios_system)
│   ├── Package.swift
│   ├── Sources/
│   │   ├── include/
│   │   │   ├── ashell_common.h   # Shared constants
│   │   │   └── ashell_proc.h     # Process info API
│   │   └── lib/
│   └── Tests/
│
├── ashell-core/               # Core command implementations
│   ├── Package.swift
│   ├── Sources/
│   │   ├── file_cmds/            # BSD file commands (cp, mv, rm)
│   │   ├── text_cmds/            # BSD text commands (cat, grep, sed)
│   │   └── shell_cmds/           # BSD shell commands (ls, pwd, echo)
│   └── Tests/
│
├── ashell-app/                 # iOS app (was a-shell)
│   ├── Package.swift
│   ├── Sources/
│   │   ├── App.swift
│   │   ├── Terminal/             # SwiftTerm UI
│   │   ├── PackageManager/       # pkg command
│   │   └── Commands/             # Swift commands
│   └── Tests/
│
├── ashell-ashell-packages/             # Package forge build system
│   ├── build.sh
│   ├── ashell-packages/
│   └── .github/workflows/
│
├── ashell-python-runtime/       # Python runtime (decoupled)
├── ashell-lua-runtime/          # Lua runtime
├── ashell-perl-runtime/         # Perl runtime
└── ashell-catalog/              # Package registry web service

# And per-package repos (example):
github.com/ashell-ashell-packages/
├── hello/
├── coreutils/
├── vim/
└── curl/
```

### Repository Dependencies

```
ashell-app/
├── Package.swift
│   ├── ashell-system (from: ../ashell-system or git)
│   ├── ashell-python-runtime (optional)
│   └── swift-crypto
│
ashell-ashell-packages/
├── build.sh (uses system tools)
│   └── Generates: XCFrameworks
│
ashell-python-runtime/
├── Package.swift
│   └── ashell-system
```

### Current Problems Identified

1. **No Clear Module Boundaries**: ios_system mixes platform code (syscall replacements) with command implementations
2. **Hardcoded Python Integration**: Python downloaded via curl in `downloadFrameworks.sh`, superglued to a-shell
3. **Scattered Build Systems**: Xcode projects, FMake, CMake, Makefiles - no unified approach
4. **Developer Environment Lock-in**: Depends on specific developer's setup (holzschu's GitHub releases)
5. **Monolithic Structure**: Everything in one repo makes it hard to work on individual components

### Proposed New Structure

```
a-shell-next/                     # Monorepo root
├── README.md                     # Root readme with module overview
├── CLAUDE.md                     # AI assistant instructions
├── docs/                         # Documentation
│   ├── architecture/             # Architecture decision records
│   ├── api/                      # API documentation
│   └── guides/                   # Developer guides
│
├── ashell-system/              # WAS: ios_system (Unix system layer)
│   ├── README.md
│   ├── Package.swift             # SPM package definition
│   ├── .github/workflows/        # CI for system library
│   ├── Sources/
│   │   ├── include/
│   │   │   ├── ashell_system.h   # Main system API (was ios_system.h)
│   │   │   ├── ashell_error.h    # ⭐ CRITICAL: syscall replacements
│   │   │   └── ashell_interpose.h # Dyld interposition interface
│   │   ├── system/
│   │   │   ├── thread_io.c       # Thread-local I/O (thread_stdin/out)
│   │   │   ├── session.c         # Session management
│   │   │   ├── process.c         # PID simulation, ios_fork, ios_exec
│   │   │   └── command.c         # ios_system(), command dispatch
│   │   └── shims/
│   │       ├── stdio_shim.c      # printf, fprintf → thread stdout
│   │       ├── process_shim.c    # fork, exec, wait replacements
│   │       └── signal_shim.c     # signal, kill replacements
│   └── Tests/
│
├── ashell-shared/              # NEW: Shared constants & utilities
│   ├── README.md
│   ├── Package.swift
│   ├── Sources/
│   │   └── include/
│   │       ├── ashell_common.h   # Shared constants
│   │       └── ashell_proc.h     # Process info API (replaces /proc)
│   └── Tests/
│
├── ashell-core/                # NEW: Core command implementations
│   ├── README.md
│   ├── Package.swift             # Builds file/text/shell cmds
│   ├── Sources/
│   │   ├── file_cmds/            # BSD file commands (cp, mv, rm)
│   │   ├── text_cmds/            # BSD text commands (cat, grep, sed)
│   │   └── shell_cmds/           # BSD shell commands (ls, pwd, echo)
│   └── Tests/
│
├── ashell-app/                  # WAS: a-shell (the app)
│   ├── README.md
│   ├── Package.swift             # SPM-based app
│   ├── Sources/
│   │   ├── App.swift             # Entry point
│   │   ├── Terminal/             # ⭐ SwiftTerm-based terminal UI
│   │   │   ├── TerminalView.swift           # SwiftTerm wrapper
│   │   │   ├── TerminalView+KeyCommands.swift
│   │   │   └── TerminalDelegate.swift       # Bridge to ashell-system
│   │   ├── Commands/             # Swift-implemented commands
│   │   │   ├── pkg.swift         # Package manager
│   │   │   ├── ExtraCommands.swift
│   │   │   └── interactive.swift
│   │   └── PackageManager/       # Native package handling
│   ├── Resources/
│   └── Tests/
│
├── ashell-ashell-packages/              # NEW: Package forge (moved from ashell-packages/)
│   ├── README.md
│   ├── build-system/
│   │   ├── ashell_package.sh     # Build library
│   │   ├── config/
│   │   │   ├── ios-cross-compile.cmake
│   │   │   └── toolchain.xcconfig
│   │   └── scripts/
│   ├── ashell-packages/                 # Individual package recipes
│   │   ├── hello/
│   │   ├── coreutils/
│   │   └── python/
│   └── .github/workflows/        # CI for package builds
│
├── ashell-runtimes/              # NEW: Language runtimes (separate from host)
│   ├── python-runtime/
│   │   ├── Package.swift         # Swift package wrapping Python
│   │   ├── python-apple-support/ # Subtree or submodule
│   │   └── python-stdlib/        # iOS-compatible stdlib
│   ├── lua-runtime/
│   └── perl-runtime/
│
├── ashell-commands/              # NEW: Monolithic commands repo
│   ├── core/                     # Essential commands (Tier A)
│   │   ├── file_cmds/            # BSD file_cmds port
│   │   ├── text_cmds/            # BSD text_cmds port
│   │   └── shell_cmds/           # BSD shell_cmds port
│   ├── extended/                 # Optional heavy commands
│   │   ├── vim/
│   │   ├── curl/
│   │   └── git/
│   └── Package.swift             # Can build all or individual
│
└── ashell-catalog/               # NEW: Package registry (web service)
    ├── packages.json             # Package index
    ├── manifests/                # Per-package metadata
    └── docs/
```

### Multi-Repo Migration Strategy

**Phase 0: Repository Setup (Pre-M2)**
1. Create new GitHub organization: `ashell-dev` (or use existing)
2. Create repositories:
   - `ashell-system` (fork from ios_system, then restructure)
   - `ashell-app` (fork from a-shell)
   - `ashell-packages`
   - `ashell-python-runtime` (extract from a-shell/cpython)
3. Archive original repos with redirects to new locations

**Phase 1: Platform Extraction (Week 1-2, Parallel to M2)**
- Extract `ios_system/` → `ashell-system/`
- Remove command implementations, keep only:
  - Thread-local I/O
  - Session management
  - PID simulation
  - Syscall shims
- Rename headers: `ios_system.h` → `ashell.h`
- Add SPM Package.swift
- First release: `ashell-system v1.0.0`

**Phase 2: Python Decoupling (Week 3-4, High Priority)**
- Move `a-shell/cpython/` → `ashell-python-runtime/`
- Create proper SPM package
- Modify `ashell-app` to use optional dependency:
  ```swift
  .package(url: "https://github.com/rudironsoni/ashell-python-runtime",
           from: "1.0.0")
  ```
- Update `pkg` to handle runtime installation
- Remove Python from default a-Shell build

**Phase 3: Host Migration (Week 5-6)**
- Update `ashell-app` to use `ashell-system` as SPM dependency
- Replace all `#include "ios_system.h"` → `#include "ashell.h"`
- Full SPM migration - remove Xcode project dependencies
- Test on device

**Phase 4: Build System Unification (Week 7-10)**
- Root `Package.swift` in each repo
- Unified CI across repos (trigger builds on dependency updates)
- Automated package publishing to `ashell-catalog`

### Developer Workflow (Multi-Repo)

**Clone and Setup:**
```bash
# Create workspace
mkdir ashell-workspace && cd ashell-workspace

# Clone all repos
git clone https://github.com/rudironsoni/ashell-system.git
git clone https://github.com/rudironsoni/ashell-app.git
git clone https://github.com/rudironsoni/ashell-packages.git
git clone https://github.com/rudironsoni/ashell-python-runtime.git

# Build platform
cd ashell-system
swift build

# Build host (depends on platform)
cd ../ashell-app
swift build

# Build packages
cd ../ashell-packages
./build.sh hello

# Run tests
swift test --package-path ../ashell-system
swift test --package-path ../ashell-app
```

**Cross-Repo Development:**
```bash
# Edit platform + test in host
cd ashell-system
# ... make changes ...
swift build

cd ../ashell-app
swift package edit ashell-system --path ../ashell-system
swift build  # Uses local platform changes
swift test
```

### Migration Strategy

#### Phase 1: Rename and Document (Week 1-2)
- Rename `ios_system/` → `ashell-system/`
- Rename `a-shell/` → `ashell-app/`
- Update all internal references
- Add READMEs explaining each module's purpose

#### Phase 2: Extract Python (Week 3-4)
- Move Python from `a-shell/cpython/` → `ashell-runtimes/python-runtime/`
- Make Python an optional Swift Package dependency
- Update download scripts to use proper package management

#### Phase 3: Unify Build System (Week 5-8)
- Create root `Package.swift` with all modules
- Migrate Xcode projects to SPM where possible
- Keep CMake for C libraries (curl, libarchive)
- Create unified build script

#### Phase 4: Split Repositories (Future)
- Extract `ashell-system` to standalone repo
- Extract `ashell-packages` to standalone repo
- Keep `ashell-app` as main integration repo

### Python Decoupling Strategy

**Current State:**
- Python downloaded via `curl` in `downloadFrameworks.sh` from Holzschu's GitHub releases
- Custom cpython fork with iOS patches at `a-shell/cpython/`
- Tightly coupled to app bundle

**Target State:**
- Python in separate repo: `ashell-python-runtime`
- Based on BeeWare Python-Apple-support (https://github.com/beeware/Python-Apple-support)
- Optional SPM dependency
- Installed via `pkg install python` at runtime
- Supports multiple Python versions

**Why BeeWare:**
- Official PEP 730 iOS support implementation
- Clean separation between Python runtime and host app
- AppleFrameworkLoader handles C extensions natively
- Well-documented, actively maintained
- Build process integrated with Xcode/SwiftPM

**Migration Steps:**
1. **Extract Runtime (Week 1)**
   ```bash
   # Create new repo based on BeeWare Python-Apple-support
   # Instead of subtree splitting the old cpython fork:
   git clone https://github.com/beeware/Python-Apple-support.git ashell-python-runtime
   cd ashell-python-runtime
   # Add ashell-specific Package.swift, README
   git push origin main
   ```

2. **Update Host (Week 2)**
   - Remove `cpython/` directory
   - Add optional dependency to Package.swift:
     ```swift
     .package(url: ".../ashell-python-runtime", from: "1.0.0")
     ```
   - Add placeholder Python command that suggests `pkg install python`

3. **Create Python Package (Week 3)**
   - Create `ashell-ashell-packages/python/build.sh`
   - Downloads Python XCFramework from releases
   - Generates `python` command registration
   - Tests: `pkg install python` → `python --version` works

4. **Remove Download Script (Week 4)**
   - Remove Python curl from `downloadFrameworks.sh`
   - Update documentation
   - Release new a-Shell without bundled Python

### Cross-Repo CI/CD

**Repository Webhooks:**
```yaml
# ashell-system/.github/workflows/publish.yml
on:
  release:
    types: [published]
jobs:
  notify-dependents:
    steps:
      - name: Trigger ashell-app rebuild
        uses: peter-evans/repository-dispatch@v2
        with:
          repository: rudironsoni/ashell-app
          event-type: platform-update
          client-payload: '{"version": "${{ github.ref }}"}'
```

**Versioning Strategy:**
- `ashell-system`: Semver (breaking changes bump major)
- `ashell-app`: Calendar versioning (releases)
- `ashell-packages`: Package versions match upstream
- `ashell-runtimes`: Match language version (Python 3.11.x)

### Build System Unification

**Root Package.swift:**
```swift
// swift-tools-version:5.9
import PackageDescription

let package = Package(
    name: "ashell-next",
    products: [
        .library(name: "ashell-system", targets: ["ashell-system"]),
        .executable(name: "ashell-app", targets: ["ashell-app"])
    ],
    dependencies: [
        .package(path: "./ashell-system"),
        .package(path: "./ashell-runtimes/python-runtime"),
        .package(url: "https://github.com/apple/swift-crypto", from: "3.0.0"),
    ],
    targets: [
        .target(name: "ashell-system", dependencies: []),
        .executableTarget(name: "ashell-app", dependencies: [
            "ashell-system",
            .product(name: "python-runtime", package: "ashell-runtimes"),
        ]),
    ]
)
```

**Build Commands:**
```bash
# Build everything
swift build

# Build specific module
swift build --target ashell-system

# Run tests
swift test

# Build packages
cd ashell-packages
./build.sh hello
```

### Developer Onboarding

**New Developer Workflow:**
```bash
# 1. Clone monorepo
git clone https://github.com/rudironsoni/a-shell-next.git
cd a-shell-next

# 2. Build platform library
swift build --package-path ashell-system

# 3. Build and run host app
swift run --package-path ashell-app

# 4. Build a package
cd ashell-packages
./build.sh hello

# 5. Run tests
swift test --package-path ashell-system
```

**Module-Specific Development:**
- Work on platform: `cd ashell-system/`, edit, `swift build`
- Work on host: `cd ashell-app/`, edit, `swift run`
- Work on packages: `cd ashell-ashell-packages/`, edit recipes, `./build.sh`

### Clear Module Responsibilities

| Module | Responsibility | Boundary |
|--------|---------------|----------|
| `ashell-system` | Syscall replacement, thread-local I/O, session management | No UI, no network |
| `ashell-app` | Terminal UI, command dispatch, package manager UI | Calls platform APIs |
| `ashell-packages` | Build recipes, patches, XCFramework generation | No runtime code |
| `ashell-runtimes/*` | Language interpreters (Python, Lua, etc.) | Plugin to platform |
| `ashell-commands` | Command implementations | Link against platform |
| `ashell-catalog` | Web service for package registry | External service |

## Verification Strategy

### Package Forge Testing
- [ ] `./ashell-packages/build.sh hello` builds successfully
- [ ] Patches apply cleanly (`patch -p1 < patch/01-*.patch`)
- [ ] XCFramework loads and registers commands correctly
- [ ] `pkg install` extracts and installs correctly
- [ ] Uninstall removes framework and plist entries

### ios_system Contract Testing
- [ ] All public APIs documented with examples
- [ ] Test each API edge case
- [ ] Verify thread-safety of session management
- [ ] Validate command registration round-trip

### Swift Testing Integration
```swift
import Testing
import ios_system

@Test func pkgInstallHello() async throws {
    let runner = CommandRunner()
    let (output, _, exitCode) = try await runner.run("pkg install hello")
    #expect(exitCode == 0)
    #expect(output.contains("installed successfully"))
}
```

### Integration Testing
- [ ] Command pipeline works: `ls | grep | wc`
- [ ] Redirection works: `echo test > file.txt`
- [ ] Session switching preserves environment
- [ ] Bookmarks work across sessions

### Performance Benchmarks
- [ ] Command cold start < 50ms
- [ ] Package extraction < 1s per 10MB
- [ ] Tab switching < 100ms
- [ ] Memory usage per session < 50MB baseline

---

## Appendix C: 4-Subagent Validation - Key Technical Findings

### Research Methodology
Four parallel subagents were dispatched to validate the plan:
1. **Swift Libraries & Apple Docs** - Researched current library status, App Store guidelines
2. **App Store Compliance & APT** - Analyzed fork/exec requirements, Section 2.5.2
3. **Termux Build System** - Deep dive into actual Termux implementation
4. **Python/C Extensions** - Researched BeeWare, Mobile Forge, pip feasibility

### Critical Technical Blockers Identified

#### Blocker 1: App Store Guideline 2.5.2
> "Apps may not download, install, or execute code which introduces or changes features or functionality"

**Impact:** Native XCFramework downloads are **prohibited**. Only WebAssembly (treated as "content") is allowed.

**Evidence:**
- Original a-Shell downloads WASM, not native code
- iSH uses x86 emulation (native code never enters the system)
- No terminal emulator on App Store downloads ARM binaries

#### Blocker 2: iOS fork/exec Restrictions
- `fork()` returns -1 with `errno = ENOSYS` (not implemented)
- `posix_spawn()` also blocked
- iOS apps are single-process by design

**Impact:** APT fundamentally requires subprocesses for dpkg, maintainer scripts. Cannot be ported.

#### Blocker 3: Code Signing Requirements
- iOS kernel enforces code signature verification
- Downloaded frameworks fail `dlopen()` with `EPERM`
- Even for developers: ad-hoc signed code has restrictions

**Impact:** Cannot load downloaded native code regardless of App Store policy.

#### Blocker 4: Python C Extensions
- PyPI has **no iOS platform tags**
- cibuildwheel does **not support iOS** (and likely never will)
- Mobile Forge exists but has limited package support
- `pip install numpy` requires pre-built iOS wheel that doesn't exist

**Impact:** Users cannot install compiled Python extensions. Must be pre-bundled.

### What Termux Actually Does (Misconception Clarified)

**Myth:** Termux uses apt and dpkg on Android.

**Reality:**
- Termux has **14 patches** to APT (0000-cmake-fix.patch through 0013-wtf.patch)
- They maintain **26 NDK patches** to make Android more Linux-compatible
- Termux provides **compatibility libraries** (libandroid-glob, libandroid-posix-semaphore)
- Termux uses a **custom `pkg` tool** for basic operations (not apt directly)

**Why this doesn't translate to iOS:**
- Android allows most syscalls; iOS restricts many
- Android uses ELF binaries; iOS requires Mach-O
- Android's Bionic libc is Linux-like; iOS's libSystem is BSD-like
- Android doesn't enforce code signing; iOS does rigorously

### Realistic Implementation Path (Validated)

| Component | Approach | Feasibility | Evidence |
|-----------|----------|-------------|----------|
| Build system (ashell_package.sh) | Termux-style | ✅ High | Patterns are reusable |
| Native commands | Pre-bundled XCFrameworks | ✅ High | Standard Apple practice |
| Downloadable commands | WebAssembly | ✅ High | a-Shell proven |
| Package manager | Curated `apt` (not real apt) | ✅ Medium | Simpler than full apt |
| Python runtime | BeeWare Python-Apple-support | ✅ High | Actively maintained |
| Python extensions | Pre-bundled frameworks | ✅ High | Pyto, Carnets do this |
| pip for pure Python | Standard pip | ✅ High | Works today |
| pip for C extensions | ❌ Not possible | ❌ Impossible | App Store + iOS restrictions |
| libarchive integration | libcompression + Swift tar | ⚠️ Medium | No Swift bindings exist |
| SwiftTerm terminal | v1.12.0 | ✅ High | Production-ready |

### Recommended Technical Stack (Post-Roast)

**Build System:**
- ashell_package.sh (Termux-style build recipes)
- GitHub Actions with macOS runners (Apple license requirement)
- Xcode toolchain (not Android NDK)

**Package Manager:**
- Command name: `apt` (familiar interface)
- Implementation: Curated package list, no dependency resolution
- Packages: Pre-bundled in app, not downloaded

**Command Distribution:**
- Tier 1: Native XCFrameworks (bundled with app)
- Tier 2: WebAssembly (downloadable, App Store compliant)

**Python:**
- Runtime: BeeWare Python-Apple-support (Python 3.10-3.14)
- Extensions: Pre-bundled numpy, pandas, matplotlib, scipy
- pip: Pure Python packages only

**Archive Handling:**
- ZIP: ZIPFoundation library
- Gzip/Bzip2: Apple libcompression framework
- Tar: Custom Swift implementation (libarchive has no Swift bindings)

**Terminal:**
- SwiftTerm v1.12.0 (actively maintained, iOS 14+)

### Honest Effort Estimates

| Component | Original Estimate | Post-Roast Reality |
|-----------|-------------------|-------------------|
| apt package manager | 2-3 months | ❌ Abandon - not feasible |
| Curated `apt` command | Not planned | 2-3 weeks (simpler scope) |
| Build system | 1 month | 1 month (Termux patterns work) |
| Pre-bundled packages | 2 weeks | 2-3 months (signing complexity) |
| WebAssembly support | Existing | Reuse existing |
| Python runtime | 1 month | 2-3 weeks (BeeWare exists) |
| Python extensions | 1 month | Continuous (add per request) |
| Full feature parity | 6 months | 2-3 months (realistic scope) |

### References

**Swift Libraries:**
- SwiftTerm: https://github.com/migueldeicaza/SwiftTerm (v1.12.0, March 2026)
- ZIPFoundation: https://github.com/weichsel/ZIPFoundation
- Alamofire: https://github.com/Alamofire/Alamofire (v5.11.0)

**Apple Documentation:**
- App Store Review Guidelines: https://developer.apple.com/app-store/review/guidelines/
- Section 2.5.2: Downloaded code restrictions
- XCFramework documentation

**Python on iOS:**
- PEP 730: https://peps.python.org/pep-0730/
- BeeWare Python-Apple-support: https://github.com/beeware/Python-Apple-support
- Mobile Forge: https://github.com/beeware/mobile-forge

**Termux:**
- termux-packages: https://github.com/termux/termux-packages
- APT patches: https://github.com/termux/termux-packages/tree/master/packages/apt

**Existing iOS Terminal Apps:**
- a-Shell: https://github.com/holzschu/a-shell (WASM approach)
- iSH: https://github.com/ish-app/ish (x86 emulation)
- Carnets: https://github.com/Carnets/Carnets (Python-focused)
- Pyto: https://github.com/ColdGrub1384/Pyto (Pre-bundled Python)

---

## Implementation Progress

### Completed
- ✅ 4-subagent validation completed with realistic assessment
- ✅ Directory structure: ashell-packages/root-packages/ and ashell-packages/packages/
- ✅ Plan updated with user direction (apt with patches, custom iOS wheels)
- ✅ apt build.sh recipe created with iOS patches
- ✅ M2 epic with 8 child issues and dependencies
- ✅ apt patches: 01-prefix, 02-ios-sandbox, 03-filesystem

### In Progress
- 🔄 Creating comprehensive beads issues for all milestones
- 🔄 Documenting exact file paths and code snippets
- 🔄 Setting up issue dependencies

### Next Steps
1. Create M1 (Platform Hardening) issues
2. Create M0 (Contract & Documentation) issues
3. Create Python runtime issues
4. Create dependency package issues (dpkg, libz, etc.)
5. All issues must have: code snippets, exact paths, copy-paste commands, agent notes

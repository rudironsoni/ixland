# AI Agent Onboarding Guide

Guide for AI assistants working on the a-Shell Next project.

## Quick Start

1. **Read the plan:** `/home/rrj/src/github/rudironsoni/a-shell-next/docs/plans/20260318-a-shell-next.md`
2. **Check current work:** `bd ready` - find issues ready to work
3. **Understand conventions:** Read this guide and `CLAUDE.md`
4. **Find work:** `bd show <id>` for detailed issue information

## Project Structure

```
a-shell-next/
├── .beads/                      # Issue tracking
│   ├── reference/               # Code templates and examples
│   ├── TASK_SPECIFICATION.md    # Issue format standard
│   └── AGENT_GUIDE.md           # This file
├── ashell-packages/             # Package forge
│   ├── ashell_package.sh        # Build system library
│   ├── build.sh                 # Main orchestration
│   ├── root-packages/           # Core distro packages (REQUIRED)
│   │   ├── apt/                 # apt package manager
│   │   ├── dpkg/                # Debian package system
│   │   ├── libz/                # zlib compression
│   │   ├── libssl/              # OpenSSL
│   │   └── coreutils/           # Essential file utilities
│   └── packages/                # Optional user packages
│       ├── hello/               # Reference package
│       └── nvim/                # Neovim editor (future)
├── ashell-core/                 # Core Swift implementation
│   └── Sources/
│       ├── Commands/            # Swift command implementations
│       └── PackageManager/      # Native package manager
├── ios_system/                  # Unix system layer (will rename to ashell-system)
│   └── ios_system.h             # Main API header
└── docs/
    ├── api/
    │   └── ios_system_contract.md
    └── plans/
        └── 20260318-a-shell-next.md
```

## Key Conventions

### Naming
- **ASHELL_** prefix for environment variables
- **ashell_** prefix for functions
- **M{N}-I{N}** for milestone issues (M2-I1, etc.)

### File Paths
- Use full absolute paths in issue descriptions
- `/home/rrj/src/github/rudironsoni/a-shell-next/` is repo root
- Never use relative paths in documentation

### Build System
- Termux-inspired bash build scripts
- Git diff patches in `patches/01-*.patch` format
- XCFramework + plist registration pattern

## Critical Technical Constraints

### What iOS CANNOT Do (and we work around)
1. **fork()** - Returns -1 with ENOSYS. Use pthread simulation
2. **execve()** - Not available. Use ios_system() instead
3. **Download native code** - App Store 2.5.2 prohibits this
4. **Real process spawning** - Only threads in App Store apps

### What We DO Instead
- XCFrameworks are **pre-bundled**, not downloaded
- WebAssembly for downloadable content
- Pthread-based process simulation
- ios_system() for command execution

## Working with Issues

### Finding Work
```bash
bd ready              # Issues with no blockers
bd blocked            # Issues waiting on dependencies
bd show <id>          # Full issue details
bd list --status=open # All open issues
```

### Claiming Work
```bash
bd update <id> --claim  # Mark as in_progress
```

### Completing Work
```bash
bd close <id>          # Mark as done
bd close <id1> <id2>   # Close multiple
```

### Creating Issues
```bash
bd create --title="..." --description="..." --type=task --priority=2
bd dep add <issue> <depends-on>  # Add dependency
```

## Code Patterns

### ASHELL_ Environment Variables
```bash
ASHELL_PREFIX="$HOME/Library/ashell"
ASHELL_CONFIG="$HOME/Documents/.ashell"
ASHELL_TARGET_PLATFORM="arm64-apple-ios16.0"
ASHELL_DEPLOYMENT_TARGET="16.0"
```

### Package Build Script Template
```bash
ASHELL_PKG_NAME="pkgname"
ASHELL_PKG_VERSION="1.0.0"
ASHELL_PKG_SRCURL="https://..."
ASHELL_PKG_SHA256="..."
ASHELL_PKG_COMMANDS="cmd:cmd_main::no"

source "${ASHELL_PKG_BUILDER_DIR}/../ashell_package.sh"
```

### Swift Command Pattern
```swift
@_cdecl("commandName")
public func commandName(argc: Int32, argv: UnsafeMutablePointer<...>) -> Int32 {
    // Parse arguments
    // Execute logic
    // Return exit code
}
```

## Common Mistakes to Avoid

1. **Hardcoding paths** - Always use `$ASHELL_PREFIX`
2. **Skipping error handling** - Every command can fail on iOS
3. **Assuming macOS** - Use lazy SDK loading
4. **Downloaded code** - XCFrameworks must be pre-bundled
5. **Global stdout** - Use ios_stdout() in C/Swift
6. **fork/exec** - These don't work on iOS

## Testing

### Local Testing
```bash
cd ashell-packages
./build.sh hello        # Build hello package
bash -n ashell_package.sh  # Syntax check
```

### Verification
```bash
# Check XCFramework
ls -la .build/hello/*.xcframework

# Validate plist
plutil -lint .build/hello/commands.plist

# Test loading (requires Xcode)
# Add to a-Shell project and build
```

## When Stuck

1. Check existing working implementations in `ashell-packages/`
2. Review reference files in `.beads/reference/`
3. Look at `ios_system/` for API patterns
4. Read `docs/api/ios_system_contract.md`
5. Document what you tried before asking

## Resources

- **Plan:** `docs/plans/20260318-a-shell-next.md`
- **Task Spec:** `.beads/TASK_SPECIFICATION.md`
- **Reference Code:** `.beads/reference/`
- **iOS Contract:** `docs/api/ios_system_contract.md`
- **Build System:** `ashell-packages/ashell_package.sh`

## Escalation Path

**Ask for help when:**
- Stuck for > 2 hours on same problem
- Unclear iOS-specific behavior
- Security/architecture decision needed
- Test failures with mysterious errors

**Document before escalating:**
1. What you tried
2. What error occurred
3. What you expected to happen
4. Relevant code snippets

## Quick Reference Commands

```bash
# Beads workflow
bd ready              # Find work
bd show <id>          # See details
bd update <id> --claim  # Claim
bd close <id>         # Complete

# Build system
cd ashell-packages
./build.sh <package>  # Build
./clean.sh            # Clean

# Verification
ls -la .build/*/      # Check outputs
bash -n *.sh          # Syntax check
```

## Success Checklist

Before claiming an issue complete, verify:
- [ ] All success criteria pass
- [ ] Code follows style conventions
- [ ] Error handling is comprehensive
- [ ] No TODOs or placeholders remain
- [ ] Verification steps all pass
- [ ] Documentation updated
- [ ] `bd close <id>` executed

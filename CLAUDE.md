# CLAUDE.md - a-Shell Next Implementation Guide

## Project Overview

This is the a-Shell Next modernization project following the plan in `docs/plans/20260318-a-shell-next.md`.

## Current Module Structure

```
ixland/
├── ixland-packages/          # ✅ Phase 1: Package Forge (COMPLETE)
│   ├── ashell_package.sh     # Build system library (Termux-style)
│   ├── build.sh              # Main build orchestration
│   ├── hello/                # Reference package
│   ├── coreutils-minimal/    # Bootstrap package
│   ├── scripts/              # Helper utilities
│   └── README.md
│
├── ixland-core/              # ✅ Phase 1: Core Commands (COMPLETE)
│   ├── Sources/
│   │   ├── Commands/
│   │   │   └── pkg.swift     # Package manager command
│   │   └── PackageManager/
│   │       ├── PackageManager.swift
│   │       └── ArchiveExtractor.swift
│   └── README.md
│
├── ixland-system/            # iOS system layer (was ios_system)
│   └── (existing ios_system code)
│
├── ixland-app/               # iOS app (was a-shell)
│   └── (existing a-Shell app code - pkg code removed)
│
└── docs/
    ├── api/
    │   └── ios_system_contract.md
    └── plans/
        └── 20260318-a-shell-next.md  # Master plan
```

## Phase 1: Package Forge (ixland-packages) - Implemented

### Created Files:

1. **ixland-packages/ashell_package.sh** - Build system library
   - `ashell_step_extract_package()` - Download and verify SHA256
   - `ashell_step_patch_package()` - Apply patches in order
   - `ashell_step_configure()` - Run autotools/cmake
   - `ashell_step_make()` - Build with iOS toolchain
   - `ashell_step_make_install()` - Install to staging
   - `ashell_step_create_xcframework()` - Package as XCFramework
   - `ashell_step_generate_plist()` - Create command metadata
   - `ashell_step_codesign()` - Sign frameworks

2. **ixland-packages/build.sh** - Main orchestration script
   - Commands: build, clean, install, package, list
   - Usage: `./build.sh <package> [command]`

3. **ixland-packages/hello/build.sh** - Reference package
   - Demonstrates minimal iOS-compatible command
   - Shows ios_system.h integration pattern

4. **ixland-packages/scripts/ashell-fix-shebang** - Shebang fixer utility
   - Rewrites `#!/bin/sh` to `#!$PREFIX/bin/sh`

5. **ixland-packages/README.md** - Documentation

6. **ixland-app/a-Shell/PackageManager.swift** - Swift package manager actor
   - Async package installation/removal
   - Registry tracking (JSON-based)
   - Command registration via ios_system APIs

7. **ixland-app/a-Shell/ExtraCommands.swift** - Added `@_cdecl("pkg")` function
   - Subcommands: install, remove, update, list, search, info
   - Integrated with PackageManager

### Usage:

```bash
# Build a package
cd ixland-packages
./build.sh hello

# In ixland app
pkg install hello
pkg list
pkg remove hello
```

## Next Steps (Per Plan)

### Phase 1 Completion (Remaining)
- [x] CI pipeline for automated package builds (.github/workflows/packages.yml)
- [x] Create clean.sh script
- [x] ArchiveExtractor.swift (libarchive integration stub)
- [x] Bootstrap package: coreutils-minimal
- [ ] Bootstrap packages: less, tree (templates)
- [ ] Testing infrastructure

### Phase 2: ios_system Contract & Documentation (STARTED)
- [x] Create docs/api/ios_system_contract.md
- [ ] Create docs/guides/porting_guide.md
- [ ] Create docs/SYSCALL_COMPATIBILITY.md
- [x] Rename ios_system/ → ixland-system/

### Phase 2: ios_system Contract & Documentation
- [x] Rename ios_system/ → ixland-system/
- [ ] Document ios_system.h public APIs
- [ ] Create docs/ios_system_contract.md
- [ ] Create docs/porting_guide.md
- [ ] Create docs/SYSCALL_COMPATIBILITY.md

### Phase 3: Codebase Modularization
- [ ] Create ixland-shared/ (shared headers)
- [x] Create ixland-core/ (core command implementations)
- [x] Rename a-shell/ → ixland-app/
- [ ] Create Package.swift files for SPM integration

### Phase 4: Heavy Packages
- [ ] ixland-runtimes/python-runtime/ (BeeWare-based)
- [ ] ixland-runtimes/lua-runtime/
- [ ] Package recipes for Python, SSH

## Development Workflow

```bash
# Build a package
cd ixland-packages
./build.sh hello

# Test the package
# (Copy .build/hello/hello.framework to ixland-app)

# Install via pkg command (in app)
pkg install hello
```

## Conventions

- **ASHELL_** prefix for environment variables
- **ashell_** prefix for functions
- Termux-style build steps (`ashell_step_*`)
- Git diff patches in `patches/01-*.patch` format
- XCFramework + plist registration pattern


<!-- BEGIN BEADS INTEGRATION v:1 profile:minimal hash:b9766037 -->
## Beads Issue Tracker

This project uses **bd (beads)** for issue tracking. Run `bd prime` to see full workflow context and commands.

### Quick Reference

```bash
bd ready              # Find available work
bd show <id>          # View issue details
bd update <id> --claim  # Claim work
bd close <id>         # Complete work
```

### Rules

- Use `bd` for ALL task tracking — do NOT use TodoWrite, TaskCreate, or markdown TODO lists
- Run `bd prime` for detailed command reference and session close protocol
- Use `bd remember` for persistent knowledge — do NOT use MEMORY.md files

## Landing the Plane (Session Completion)

**When ending a work session**, you MUST complete ALL steps below. Work is NOT complete until `git push` succeeds.

**MANDATORY WORKFLOW:**

1. **File issues for remaining work** - Create issues for anything that needs follow-up
2. **Run quality gates** (if code changed) - Tests, linters, builds
3. **Update issue status** - Close finished work, update in-progress items
4. **PUSH TO REMOTE** - This is MANDATORY:
   ```bash
   git pull --rebase
   bd dolt push
   git push
   git status  # MUST show "up to date with origin"
   ```
5. **Clean up** - Clear stashes, prune remote branches
6. **Verify** - All changes committed AND pushed
7. **Hand off** - Provide context for next session

**CRITICAL RULES:**
- Work is NOT complete until `git push` succeeds
- NEVER stop before pushing - that leaves work stranded locally
- NEVER say "ready to push when you are" - YOU must push
- If push fails, resolve and retry until it succeeds
<!-- END BEADS INTEGRATION -->

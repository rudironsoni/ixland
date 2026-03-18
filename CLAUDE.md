# CLAUDE.md - a-Shell Next Implementation Guide

## Project Overview

This is the a-Shell Next modernization project following the plan in `docs/plans/20260318-a-shell-next.md`.

## Current Module Structure

```
a-shell-next/
├── ashell-packages/          # ✅ Phase 1: Package Forge (COMPLETE)
│   ├── ashell_package.sh     # Build system library (Termux-style)
│   ├── build.sh              # Main build orchestration
│   ├── hello/                # Reference package
│   ├── coreutils-minimal/    # Bootstrap package
│   ├── scripts/              # Helper utilities
│   └── README.md
│
├── ashell-core/              # ✅ Phase 1: Core Commands (COMPLETE)
│   ├── Sources/
│   │   ├── Commands/
│   │   │   └── pkg.swift     # Package manager command
│   │   └── PackageManager/
│   │       ├── PackageManager.swift
│   │       └── ArchiveExtractor.swift
│   └── README.md
│
├── ios_system/               # TODO: Rename to ashell-system/
│   └── (existing ios_system code)
│
├── a-shell/                  # TODO: Rename to ashell-app/
│   └── (existing a-Shell app code - pkg code removed)
│
└── docs/
    ├── api/
    │   └── ios_system_contract.md
    └── plans/
        └── 20260318-a-shell-next.md  # Master plan
```

## Phase 1: Package Forge (ashell-packages) - Implemented

### Created Files:

1. **ashell-packages/ashell_package.sh** - Build system library
   - `ashell_step_extract_package()` - Download and verify SHA256
   - `ashell_step_patch_package()` - Apply patches in order
   - `ashell_step_configure()` - Run autotools/cmake
   - `ashell_step_make()` - Build with iOS toolchain
   - `ashell_step_make_install()` - Install to staging
   - `ashell_step_create_xcframework()` - Package as XCFramework
   - `ashell_step_generate_plist()` - Create command metadata
   - `ashell_step_codesign()` - Sign frameworks

2. **ashell-packages/build.sh** - Main orchestration script
   - Commands: build, clean, install, package, list
   - Usage: `./build.sh <package> [command]`

3. **ashell-packages/hello/build.sh** - Reference package
   - Demonstrates minimal iOS-compatible command
   - Shows ios_system.h integration pattern

4. **ashell-packages/scripts/ashell-fix-shebang** - Shebang fixer utility
   - Rewrites `#!/bin/sh` to `#!$PREFIX/bin/sh`

5. **ashell-packages/README.md** - Documentation

6. **a-shell/a-Shell/PackageManager.swift** - Swift package manager actor
   - Async package installation/removal
   - Registry tracking (JSON-based)
   - Command registration via ios_system APIs

7. **a-shell/a-Shell/ExtraCommands.swift** - Added `@_cdecl("pkg")` function
   - Subcommands: install, remove, update, list, search, info
   - Integrated with PackageManager

### Usage:

```bash
# Build a package
cd ashell-packages
./build.sh hello

# In a-Shell app
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
- [ ] Rename ios_system/ → ashell-system/ (TODO)

### Phase 2: ios_system Contract & Documentation
- [ ] Rename ios_system/ → ashell-system/
- [ ] Document ios_system.h public APIs
- [ ] Create docs/ios_system_contract.md
- [ ] Create docs/porting_guide.md
- [ ] Create docs/SYSCALL_COMPATIBILITY.md

### Phase 3: Codebase Modularization
- [ ] Create ashell-shared/ (shared headers)
- [ ] Create ashell-core/ (core command implementations)
- [ ] Rename a-shell/ → ashell-app/
- [ ] Create Package.swift files for SPM integration

### Phase 4: Heavy Packages
- [ ] ashell-runtimes/python-runtime/ (BeeWare-based)
- [ ] ashell-runtimes/lua-runtime/
- [ ] Package recipes for Python, SSH

## Development Workflow

```bash
# Build a package
cd ashell-packages
./build.sh hello

# Test the package
# (Copy .build/hello/hello.framework to a-Shell app)

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

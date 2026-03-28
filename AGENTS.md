# Project Instructions for AI Agents

This file provides instructions and context for AI coding agents working on this project.

<!-- BEGIN BEADS INTEGRATION v:1 profile:full hash:d4f96305 -->
## Issue Tracking with bd (beads)

**IMPORTANT**: This project uses **bd (beads)** for ALL issue tracking. Do NOT use markdown TODOs, task lists, or other tracking methods.

### Why bd?

- Dependency-aware: Track blockers and relationships between issues
- Git-friendly: Dolt-powered version control with native sync
- Agent-optimized: JSON output, ready work detection, discovered-from links
- Prevents duplicate tracking systems and confusion

### Quick Start

**Check for ready work:**

```bash
bd ready --json
```

**Create new issues:**

```bash
bd create "Issue title" --description="Detailed context" -t bug|feature|task -p 0-4 --json
bd create "Issue title" --description="What this issue is about" -p 1 --deps discovered-from:bd-123 --json
```

**Claim and update:**

```bash
bd update <id> --claim --json
bd update bd-42 --priority 1 --json
```

**Complete work:**

```bash
bd close bd-42 --reason "Completed" --json
```

### Issue Types

- `bug` - Something broken
- `feature` - New functionality
- `task` - Work item (tests, docs, refactoring)
- `epic` - Large feature with subtasks
- `chore` - Maintenance (dependencies, tooling)

### Priorities

- `0` - Critical (security, data loss, broken builds)
- `1` - High (major features, important bugs)
- `2` - Medium (default, nice-to-have)
- `3` - Low (polish, optimization)
- `4` - Backlog (future ideas)

### Workflow for AI Agents

1. **Check ready work**: `bd ready` shows unblocked issues
2. **Claim your task atomically**: `bd update <id> --claim`
3. **Work on it**: Implement, test, document
4. **Discover new work?** Create linked issue:
   - `bd create "Found bug" --description="Details about what was found" -p 1 --deps discovered-from:<parent-id>`
5. **Complete**: `bd close <id> --reason "Done"`

### Auto-Sync

bd automatically syncs via Dolt:

- Each write auto-commits to Dolt history
- Use `bd dolt push`/`bd dolt pull` for remote sync
- No manual export/import needed!

### Important Rules

- ✅ Use bd for ALL task tracking
- ✅ Always use `--json` flag for programmatic use
- ✅ Link discovered work with `discovered-from` dependencies
- ✅ Check `bd ready` before asking "what should I work on?"
- ❌ Do NOT create markdown TODO lists
- ❌ Do NOT use external issue trackers
- ❌ Do NOT duplicate tracking systems

For more details, see README.md and docs/QUICKSTART.md.

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


## Build & Test

### Build System

This project uses **Make** for building, following proper engineering practices.

**Build the entire project:**
```bash
make                    # Build everything (default: Release)
make debug              # Build with Debug configuration
make CONFIGURATION=Debug # Explicit configuration
```

**Build specific targets:**
```bash
make build              # Build a-shell-kernel XCFramework
make build-ios          # Build for iOS device only (arm64)
make build-simulator    # Build for iOS Simulator only
```

**Clean build artifacts:**
```bash
make clean              # Clean all build artifacts
make clean-kernel       # Clean only kernel build artifacts
```

**Development helpers:**
```bash
make help               # Show all available targets
make dry-run            # Show what would be built
make verbose            # Build with verbose output
make settings           # Show Xcode build settings
```

**Direct kernel Makefile (in a-shell-kernel/ directory):**
```bash
cd a-shell-kernel
make ios                # Build iOS framework
make simulator          # Build Simulator framework
make xcframework        # Create universal XCFramework
make clean              # Clean build artifacts
```

### Testing

_TODO: Add test commands here_

## Architecture Overview

_Add a brief overview of your project architecture_

## Code Quality & Linting

This project uses automated linting and formatting to maintain code quality:

### Running Linters

**Run all linters:**
```bash
./scripts/lint.sh
```

**Auto-fix issues where possible:**
```bash
./scripts/lint.sh --fix
```

**Check mode (for CI):**
```bash
./scripts/lint.sh --check
```

### C Code (ixland-system, ixland-libc)

**Linting Tools:**
- **clang-tidy**: Static analysis and bug detection
- **clang-format**: Code formatting

**Configuration:**
- `.clang-tidy` - Linting rules and checks
- `.clang-format` - Formatting style rules

**Key Checks Enabled:**
- Bug detection (null pointer dereference, memory leaks)
- Performance optimizations
- Code style consistency (naming, formatting)
- Cyclomatic complexity limits

### Swift Code (ixland-app)

**Linting Tool:**
- **SwiftLint**: Swift-specific linting and formatting

**Configuration:**
- `.swiftlint.yml` - SwiftLint rules

**Key Rules:**
- Line length limit (120 characters)
- Function complexity limits
- Trailing whitespace, unused imports
- Force unwrapping discouraged

### Shell Scripts

**Linting Tool:**
- **ShellCheck**: Shell script analysis

**Usage:**
```bash
shellcheck scripts/*.sh
```

### Code Formatting

**Format all code:**
```bash
./scripts/format.sh
```

**Auto-fix formatting issues:**
```bash
./scripts/format.sh --fix
```

**Check mode (for CI):**
```bash
./scripts/format.sh --check
```

**Editor Integration:**
- Install EditorConfig plugin for your editor to respect `.editorconfig`
- Configure your editor to run clang-format on save for C files
- Configure SwiftLint integration for Swift files

### Formatting Tools by Language

| Language | Formatter | Config File |
|----------|-----------|-------------|
| C/C++ | clang-format | `.clang-format` |
| Swift | SwiftLint | `.swiftlint.yml` |
| Shell | shfmt | `.editorconfig` |
| All | EditorConfig | `.editorconfig` |

### CI Integration

Linting and formatting checks run automatically on:
- All pull requests
- Pushes to main/develop branches

See `.github/workflows/code-quality.yml` for CI configuration.

**CI Jobs:**
- `format-c` - Verifies C/C++ code formatting
- `format-swift` - Verifies Swift code formatting
- `lint-c` - Runs clang-tidy static analysis
- `lint-swift` - Runs SwiftLint analysis
- `lint-scripts` - Runs ShellCheck on shell scripts

### Pre-commit Hooks

Pre-commit hooks run automatically before each commit to ensure code quality.

**Install hooks:**
```bash
./scripts/install-hooks.sh
```

**What hooks check:**
- C/C++ code formatting (clang-format)
- Swift code formatting (SwiftLint)
- Shell script quality (ShellCheck)
- Trailing whitespace detection
- Large file warnings (>1MB)

**Hooks location:** `.pre-commit-hooks/hooks/`

**To bypass hooks temporarily:**
```bash
git commit --no-verify
# or
SKIP_CODE_QUALITY_CHECKS=1 git commit
```

**Uninstall hooks:**
```bash
./scripts/install-hooks.sh --uninstall
```

### Pre-commit vs CI

| Check | Pre-commit | CI |
|-------|-----------|-----|
| Runs on | Staged files only | All files |
| Speed | Fast | Complete |
| Enforcement | Blocks commit | Blocks merge |
| Best for | Development feedback | Final validation |

Use pre-commit hooks during development for fast feedback, CI for comprehensive validation.

## Conventions & Patterns

### C Code Naming Conventions

- **Structs/Types**: `iox_` prefix with lowercase (e.g., `iox_task_t`)
- **Functions**: `iox_` prefix with lowercase (e.g., `iox_task_alloc()`)
- **Macros**: UPPER_CASE (e.g., `IOX_MAX_NAME`)
- **Constants**: UPPER_CASE (e.g., `IOX_NSIG`)

### Swift Code Naming Conventions

- Follow standard Swift naming guidelines
- Use PascalCase for types (e.g., `ContentView`)
- Use camelCase for functions and variables
- Private outlets and actions marked accordingly

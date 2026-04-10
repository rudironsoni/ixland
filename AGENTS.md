# Project Instructions for AI Agents

This file provides instructions and context for AI coding agents working on this project.

## Landing the Plane (Session Completion)

**When ending a work session**, you MUST complete ALL steps below. Work is NOT complete until `git push` succeeds.

**MANDATORY WORKFLOW:**

1. **Run quality gates** (if code changed) - Tests, linters, builds
2. **PUSH TO REMOTE** - This is MANDATORY:
   ```bash
   git pull --rebase
   git push
   git status  # MUST show "up to date with origin"
   ```
3. **Clean up** - Clear stashes, prune remote branches
4. **Verify** - All changes committed AND pushed
5. **Hand off** - Provide context for next session

**CRITICAL RULES:**
- Work is NOT complete until `git push` succeeds
- NEVER stop before pushing - that leaves work stranded locally
- NEVER say "ready to push when you are" - YOU must push
- If push fails, resolve and retry until it succeeds

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

**Type checking only:**
```bash
./scripts/lint.sh --type-check
```

**Strict typing verification:**
```bash
./scripts/lint.sh --strict-typing
```

**Dead code detection:**
```bash
./scripts/lint.sh --dead-code
```

**Duplicate code detection:**
```bash
jscpd --config .jscpd.json .
```

**Technical debt scanner:**
```bash
git grep -n -E "TODO|FIXME" -- '*.c' '*.h' '*.swift' '*.sh'
```

### C Code (ixland-system, ixland-libc)

**Linting Tools:**
- **clang-tidy**: Static analysis, type checking, dead code detection, and bug detection
- **clang-format**: Code formatting

**Configuration:**
- `.clang-tidy` - Linting rules and checks (strict mode enabled with `WarningsAsErrors: '*'`)
- `.clang-format` - Formatting style rules

**Strict Type Checking:**
When `IXLAND_STRICT_TYPING=ON` (default), CMake adds strict compiler flags:
- `-Werror` - Treat all warnings as errors
- `-Werror=implicit-function-declaration` - Catch undefined functions
- `-Werror=incompatible-pointer-types` - Prevent pointer type mismatches
- `-Werror=int-conversion` - Catch implicit int conversions
- `-Werror=return-type` - Ensure functions return correct types
- `-Werror=uninitialized` - Catch uninitialized variables
- `-Werror=strict-prototypes` - Enforce proper function prototypes
- `-Werror=missing-prototypes` - Require function declarations
- `-Werror=implicit-int` - Require explicit int declarations

**Dead Code Detection:**
The following tools detect unused/dead code:
- **clang-tidy checks**: `misc-unused-parameters`, `misc-unused-alias-decls`, `readability-redundant-*`, `cppcoreguidelines-*-unused-parameters`
- **Compiler flags**: `-Wunused-variable`, `-Wunused-function`, `-Wunused-parameter`
- **ShellCheck**: Detects unused variables in shell scripts (SC2034)
- **SwiftLint**: Detects unused imports and declarations

**Technical Debt Tracking:**
All TODO/FIXME comments must be linked to issues using these formats:
- `TODO(TICKET-123)` or `FIXME(TICKET-123)`
- `TODO(#456)` or `FIXME(#456)`
- `TODO: See issue #123` or `FIXME: See issue #123`
- `TODO(PROJ-789)` or `FIXME(PROJ-789)`

The technical debt scanner enforces this via CI.

**Key Checks Enabled:**
- Type checking (clang-analyzer type analysis)
- Bug detection (null pointer dereference, memory leaks)
- Dead code detection (unused parameters, variables, functions)
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
- `type-check-c` - Runs clang-tidy type checking on all C components
- `strict-typing-c` - Enforces strict type checking with -Werror flags
- `dead-code-check` - Detects unused/dead code across all components
- `duplicate-code-check` - Detects copy-paste/duplicate code (jscpd)
- `tech-debt-check` - Scans TODO/FIXME comments and enforces issue linking
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

All C code follows strict naming conventions enforced by clang-tidy:

| Construct | Pattern | Example |
|-----------|---------|---------|
| **Functions** | `ixland_lowercase()` | `ixland_task_alloc()`, `ixland_pid_alloc()` |
| **Structs/Types** | `ixland_lowercase_t` | `ixland_task_t`, `ixland_files_t` |
| **Typedefs** | `ixland_lowercase_t` | `ixland_pid_t`, `ixland_mode_t` |
| **Macros** | `IXLAND_UPPER_CASE` | `IXLAND_MAX_NAME`, `IXLAND_NSIG` |
| **Enum Types** | `ixland_lowercase` | `ixland_task_state_t` |
| **Enum Values** | `IXLAND_PREFIX_VALUE` | `IXLAND_TASK_RUNNING`, `IXLAND_MAX_FD` |
| **Constants** | `IXLAND_UPPER_CASE` | `IXLAND_MAX_PATH`, `IXLAND_MAX_ARGS` |
| **Global Variables** | `ixland_lowercase` | `ixland_current_task` |
| **Parameters** | `lowercase` | `task`, `pid`, `fd` |

**Enforcement:**
- clang-tidy checks naming on every build
- CI validates naming conventions in `naming-c` job
- Pre-commit hooks check staged files

**Check naming:**
```bash
clang-tidy <file> --checks='readability-identifier-naming' -- <includes>
git ls-files | grep -E '^ixland-system/(src/ixland/(core|fs)|kernel|fs)/.*_v2\.(c|h)$'
```

**Exceptions:**
- `main()` function
- `test_*` functions in test files
- Standard library callbacks
- Platform-specific entry points (e.g., `ios_system_*`)

### Swift Code Naming Conventions

Swift code follows standard Swift guidelines enforced by SwiftLint:

| Construct | Pattern | Example |
|-----------|---------|---------|
| **Types (classes/structs/enums)** | `PascalCase` | `ContentView`, `TerminalManager` |
| **Protocols** | `PascalCase` | `CommandExecutable` |
| **Functions** | `camelCase` | `loadView()`, `executeCommand()` |
| **Variables/Properties** | `camelCase` | `currentDirectory`, `taskCount` |
| **Constants** | `camelCase` | `defaultTimeout`, `maxBufferSize` |
| **Enum Cases** | `camelCase` | `.running`, `.completed` |
| **Private IBOutlet** | `camelCase` | `terminalView`, `inputField` |
| **Private IBAction** | `camelCase` with action verb | `handleInput()`, `dismissKeyboard()` |

**Enforcement:**
- SwiftLint validates naming on every build
- CI validates naming conventions in `naming-swift` job
- Identifier minimum length: 2 characters
- Type names must start with uppercase
- Function/variable names must start with lowercase

### Naming Convention Rationale

**Why `ixland_` prefix for C APIs?**
1. **Namespace isolation**: Prevents symbol collisions
2. **Clarity**: Makes iXland APIs instantly recognizable
3. **Consistency**: All public APIs follow same pattern
4. **Future-proof**: Allows coexistence with other libraries

**Why strict enforcement?**
1. **Maintainability**: Easy to identify code origin
2. **Documentation**: Naming conveys API boundaries
3. **Tooling**: Enables automated analysis
4. **Onboarding**: New developers learn conventions quickly

# AI Agent Guide for a-Shell Next

## Quick Start

When starting work on this codebase:

1. **Read AGENTS.md** - Project-specific rules
2. **Check `bd ready`** - See available work
3. **Read docs/ARCHITECTURE.md** - Understand the system
4. **Claim your task** - `bd update <id> --claim`
5. **Follow conventions** - See below

## Agent Capabilities

### ✅ You CAN Do
- Read any file in the codebase
- Run analysis commands (grep, find, etc.)
- Use `bd` (beads) to track work
- Ask clarifying questions
- Propose code changes

### ❌ You CANNOT Do
- Modify files without claiming via `bd`
- Delete files without confirmation
- Run destructive git operations
- Access files outside the workspace
- Run external network commands

## Common Tasks

### Working with beads

```bash
# See ready work
bd ready

# Claim an issue
bd update <id> --claim

# Create discovered work
bd create "Found bug" -p 1 --deps discovered-from:<parent-id>

# Close completed
bd close <id> --reason "Done"
```

### Working with a-shell-kernel

```c
// Use ios_* functions, never standard calls directly
pid_t pid = fork();  // ✅ Becomes ios_fork() via macros

// Process table functions
pid_t vpid = ios_vfork();     // Allocate PID without thread
pid_t pid = ios_waitpid(pid, &status, 0);  // Wait for child
void ios_exit(status);         // Exit current process
```

### Working with a-shell-packages

```bash
# Build a package
./build.sh <package>

# Test build
./build.sh --test <package>

# Package variables in build.sh
ASHELL_PKG_NAME="bash"
ASHELL_PKG_VERSION="5.2.15"
ASHELL_PKG_DEPENDS="libncurses"
```

## Code Patterns

### Adding a Syscall

1. Add to `a_shell_system.h`:
```c
#define wait ios_wait
```

2. Implement in `a_shell_syscalls.c`:
```c
pid_t ios_wait(int *status) {
    return ios_waitpid(-1, status, 0);
}
```

3. Update docs

### Porting a Package

1. Create `a-shell-packages/core-packages/<name>/build.sh`
2. Add patches in `patches/*.patch`
3. Use `@ASHELL_PREFIX@` for paths
4. Test with `./build.sh <name>`

## Decision Tree

**Native or WASM?**
- Core tools (apt, bash, python) → Native XCFramework
- User tools (vim, node) → WASM
- Hardware access needed → Must be native

**When to ask user:**
- Architecture decisions
- Deleting files
- Security implications
- Unclear requirements

## Troubleshooting

**Build fails with "fork: not implemented"**
→ Check if `-include a_shell_system.h` is in CFLAGS

**Patches fail to apply**
→ Verify patches use `@ASHELL_PREFIX@` not hardcoded paths

**XCFramework not found**
→ Check codesign step completed

**Process table full**
→ Cleanup thread not running, or zombie processes not reaped

## Resources

- ARCHITECTURE.md - System overview
- MASTER_PLAN.md - Implementation roadmap
- Common issues: docs/a-shell-common-porting-problems.md

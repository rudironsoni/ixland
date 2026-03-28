# Backend Worker Skill

## Purpose

Implement kernel system components including syscall implementations, process management, and libc boundary extraction.

## When to Use

Use this skill for:
- Implementing C/Objective-C kernel code
- Creating syscall functions
- Process table management
- Header file creation
- CMake target configuration
- Unit test implementation

## Procedure

### 1. Read Prerequisites

Before starting, read:
- `/Users/rudironsoni/.factory/missions/3c036201-017c-4063-ad49-81b58aa3f05e/mission.md` - Mission overview
- `/Users/rudironsoni/src/github/rudironsoni/ixland/.factory/library/architecture.md` - System architecture
- `/Users/rudironsoni/src/github/rudironsoni/ixland/.factory/library/environment.md` - Build environment
- `/Users/rudironsoni/src/github/rudironsoni/ixland/AGENTS.md` - Coding conventions and boundaries

### 2. Understand the Feature

Read the feature from features.json:
- What milestone does it belong to?
- What are the preconditions?
- What assertions does it fulfill from validation-contract.md?
- What are the expected behaviors and verification steps?

### 3. Explore Existing Code

Before implementing:
- Search for similar implementations in the codebase
- Read existing headers in the target location
- Check for patterns in existing syscall implementations
- Verify naming conventions match AGENTS.md

### 4. Implement the Feature

**For Headers:**
1. Create header file with proper include guard (IOX_FILENAME_H)
2. Add Doxygen documentation for all public APIs
3. Define function prototypes with iox_ prefix
4. Define macros with IOX_ prefix
5. Add extern "C" guards for C++ compatibility

**For Syscall Implementations:**
1. Include required headers (ixland_kernel.h for kernel code)
2. Implement function with proper error handling
3. Set errno on errors
4. Use atomic operations for thread safety
5. Follow naming conventions strictly

**For CMake Targets:**
1. Define target with appropriate type (INTERFACE, STATIC, MODULE)
2. Set include directories
3. Link to dependencies
4. Handle both monorepo and standalone builds

### 5. Write Tests

Create unit tests for:
- Happy path (normal operation)
- Error paths (invalid inputs, edge cases)
- Boundary conditions (NULL pointers, max values)
- Thread safety (if applicable)

### 6. Run Quality Checks

Before committing:
```bash
# Format check
clang-format --dry-run <your-files>

# Lint check
clang-tidy <your-files>

# Build test
cmake --build build --target <your-target>

# Run tests
ctest --output-on-failure
```

### 7. Verify Fulfillment

Check that your implementation fulfills the claimed assertions:
- Review each assertion ID in the feature's "fulfills" field
- Ensure behavioral descriptions are met
- Run any specific verification steps listed

### 8. Commit and Handoff

**Commit Message Format:**
```
feat(area): brief description

- Detailed change 1
- Detailed change 2

Fulfills: VAL-XXX-NNN, VAL-YYY-NNN
```

**Handoff Report:**
```yaml
feature: <feature-id>
success: true|false|partial

what_was_done:
  - <specific accomplishment 1>
  - <specific accomplishment 2>

discovered_issues:
  - id: bd-XXX (if created in beads)
    summary: <brief description>
    severity: blocking|non-blocking

what_was_left_undone:
  - <reason and plan>

critical_context:
  - <important information for next worker>

return_to_orchestrator: false
```

## Tools Available

- **File editing**: Create, Read, Edit tools for source files
- **Build**: Execute tool for cmake, make, clang
- **Testing**: Execute tool for ctest, test runners
- **Search**: Grep, Glob for code exploration
- **Git**: Execute for git operations

## Constraints

1. **Naming**: Must follow iox_/IOX_ conventions
2. **Headers**: Must have include guards and Doxygen docs
3. **Thread Safety**: Use atomic types for shared state
4. **Error Handling**: Set errno, return -1 on error
5. **Boundaries**: Respect AGENTS.md off-limits areas

## Common Patterns

### Header Template
```c
#ifndef IOX_FILENAME_H
#define IOX_FILENAME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Brief description
 * @param param Parameter description
 * @return Return value description
 */
int iox_function_name(int param);

#ifdef __cplusplus
}
#endif

#endif /* IOX_FILENAME_H */
```

### Syscall Template
```c
#include "ixland_kernel.h"
#include <errno.h>

int iox_syscall_name(int arg) {
    if (invalid_condition) {
        errno = EINVAL;
        return -1;
    }

    // Implementation

    return result;
}
```

### Process Table Access
```c
#include "kernel/task/task.h"

iox_task_t *task = iox_task_lookup(pid);
if (!task) {
    errno = ESRCH;
    return -1;
}
// Use task...
iox_task_unref(task); // If using reference counting
```

## Error Codes to Use

| Situation | errno Value |
|-----------|-------------|
| Invalid argument | EINVAL |
| No such process | ESRCH |
| Out of memory | ENOMEM |
| Permission denied | EPERM |
| No child processes | ECHILD |
| Resource busy | EBUSY |
| Interrupted | EINTR |

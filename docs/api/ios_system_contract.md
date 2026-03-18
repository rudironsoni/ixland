# ios_system Contract

## Overview

ios_system provides a Unix-like environment for iOS apps. This document defines the platform contract that package authors can rely on when porting commands to a-Shell.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│  Package (your command)                                 │
│  - Uses standard libc headers                           │
│  - Calls `system()`, `fork()`, `getenv()`, etc.         │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼ (redirected by ashell_error.h macros)
┌─────────────────────────────────────────────────────────┐
│  ios_system API Layer                                   │
│  - ios_system() - execute commands                      │
│  - ios_fork() - returns ENOSYS                          │
│  - ios_getenv() - per-session environment               │
│  - thread_stdout/err/in - thread-local I/O              │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│  a-Shell Host                                           │
│  - Terminal UI (SwiftTerm/WKWebView)                    │
│  - Session management                                   │
│  - Package management                                   │
└─────────────────────────────────────────────────────────┘
```

## Public API Reference

### Core Functions

#### `int ios_system(const char* command)`
Execute a shell command.

- **Parameters**: `command` - Shell command string to execute
- **Returns**: Exit status of the command
- **Thread-safe**: Yes
- **Example**:
```c
int status = ios_system("ls -la");
```

#### `pid_t ios_fork(void)`
Simulate process fork (returns error on iOS).

- **Returns**: -1, sets `errno = ENOSYS`
- **Note**: iOS doesn't support true fork(), use threading instead

#### `int ios_execv(const char* path, char* const argv[])`
Execute a command with arguments.

- **Redirects to**: `ios_system()`
- **Note**: Reconstructs command string from argv

### Environment Functions

#### `char* ios_getenv(const char* name)`
Get environment variable (session-local).

- **Parameters**: `name` - Variable name
- **Returns**: Variable value or NULL
- **Thread-safe**: Yes (per-session)

#### `int ios_setenv(const char* name, const char* value, int overwrite)`
Set environment variable (session-local).

- **Parameters**:
  - `name` - Variable name
  - `value` - Variable value
  - `overwrite` - Non-zero to replace existing
- **Returns**: 0 on success, -1 on error

### I/O Functions

#### `FILE* ios_stdout(void)`
Get thread-local stdout stream.

- **Returns**: FILE* for current session's stdout
- **Note**: Use instead of global `stdout`

#### `FILE* ios_stderr(void)`
Get thread-local stderr stream.

- **Returns**: FILE* for current session's stderr

#### `FILE* ios_stdin(void)`
Get thread-local stdin stream.

- **Returns**: FILE* for current session's stdin

### Session Management

#### `void ios_switchSession(const char* sessionId)`
Switch to a different session context.

- **Parameters**: `sessionId` - Unique session identifier
- **Note**: Called automatically by ios_system()

#### `void ios_setContext(void* context)`
Set opaque context pointer for the current session.

- **Parameters**: `context` - Opaque pointer (usually session identifier)

#### `void* ios_getContext(void)`
Get the current session context.

- **Returns**: Context pointer set by ios_setContext()

### Command Registration

#### `void addCommandList(char* plistPath)`
Register commands from a plist file.

- **Parameters**: `plistPath` - Path to commands.plist
- **Note**: Used by package manager to register new commands

#### `void replaceCommand(char* commandName, char* functionName)`
Replace or unregister a command.

- **Parameters**:
  - `commandName` - Command to replace
  - `functionName` - New function (NULL to unregister)

### Path/Filesystem

#### `void ios_setMiniRoot(char* path)`
Set the root directory for file operations.

- **Parameters**: `path` - New root path
- **Note**: Commands cannot access files outside this root

#### `char* ios_getMiniRoot(void)`
Get the current root directory.

- **Returns**: Current miniroot path

## Header File: ios_system.h

```c
#ifndef IOS_SYSTEM_H
#define IOS_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Core execution
int ios_system(const char* command);
pid_t ios_fork(void);
int ios_execv(const char* path, char* const argv[]);

// Environment (per-session)
char* ios_getenv(const char* name);
int ios_setenv(const char* name, const char* value, int overwrite);

// Thread-local I/O
FILE* ios_stdout(void);
FILE* ios_stderr(void);
FILE* ios_stdin(void);

// Session management
void ios_switchSession(const char* sessionId);
void ios_setContext(void* context);
void* ios_getContext(void);

// Command registration
void addCommandList(char* plistPath);
void replaceCommand(char* commandName, char* functionName);

// Filesystem sandboxing
void ios_setMiniRoot(char* path);
char* ios_getMiniRoot(void);

// Process info (simulated)
pid_t ios_getpid(void);
pid_t ios_getppid(void);

#endif
```

## Header File: ios_error.h

This header provides macro replacements for standard functions:

```c
#ifndef IOS_ERROR_H
#define IOS_ERROR_H

#define system ios_system
#define fork ios_fork
#define execv ios_execv
#define execve ios_execve
#define execvp ios_execvp
#define getenv ios_getenv
#define setenv ios_setenv
#define unsetenv ios_unsetenv
#define exit ios_exit
#define _exit ios__exit
#define wait ios_wait
#define waitpid ios_waitpid

// Standard I/O redirection
#define stdout thread_stdout
#define stderr thread_stderr
#define stdin thread_stdin

#endif
```

## Porting Guide

### 1. Include Headers

```c
#include "ios_system.h"
#include "ios_error.h"  // Optional: provides macro replacements
```

### 2. Use Thread-Local I/O

```c
// Instead of:
fprintf(stdout, "Hello\n");

// Use:
fprintf(ios_stdout(), "Hello\n");
```

### 3. Entry Point Convention

```c
// Commands must provide: int command_main(int argc, char** argv)
int mycommand_main(int argc, char** argv) {
    // Your command logic here
    return 0;
}
```

### 4. Build Flags

```bash
# Include ios_system headers
CFLAGS="-I/path/to/ios_system/Headers $CFLAGS"

# Force include ios_error.h for automatic redirection
CFLAGS="-include /path/to/ios_system/Headers/ios_error.h $CFLAGS"

# Link against ios_system (if needed)
LDFLAGS="-framework ios_system $LDFLAGS"
```

### 5. Plist Registration

Create `commands.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>mycommand</key>
    <array>
        <string>myframework.framework/myframework</string>
        <string>mycommand_main</string>
        <string></string>
        <string>file</string>
    </array>
</dict>
</plist>
```

Fields:
1. Framework path (relative to Frameworks/)
2. Entry point function name
3. Authentication string (optional)
4. Command type: `file`, `directory`, `no`

## Syscall Compatibility

| Syscall | iOS Support | ashell-system Behavior |
|---------|-------------|------------------------|
| `fork()` | ❌ No | Returns -1, errno=ENOSYS |
| `exec*()` | ⚠️ Partial | Redirects to ios_system() |
| `system()` | ✅ Yes | Via ios_system() |
| `getenv()` | ✅ Yes | Per-session |
| `setenv()` | ✅ Yes | Per-session |
| `pipe()` | ✅ Yes | Standard pipes |
| `dup2()` | ✅ Yes | File descriptor ops |
| `open()` | ✅ Yes | Respects miniroot |
| `stat()` | ✅ Yes | iOS-compatible |
| `ptrace()` | ❌ No | errno=EPERM |
| `socket()` | ⚠️ Limited | Sandboxed |

## Session Model

ios_system uses thread-local storage for session state:

```
Thread 1: Session A (stdin/stdout for Window 1)
Thread 2: Session B (stdin/stdout for Window 2)
Thread 3: Session C (stdin/stdout for Window 3)
```

Each thread can have a different:
- Current directory
- Environment variables
- Standard I/O streams
- Miniroot (sandbox root)

## Best Practices

1. **Never use global stdout/stderr/stdin** - Always use ios_stdout(), ios_stderr(), ios_stdin()
2. **Don't assume fork() works** - Use threads for concurrency
3. **Respect miniroot** - Don't try to escape the sandbox
4. **Check return values** - ios_system() can fail
5. **Clean up resources** - fclose(), free(), etc. before returning

## Common Issues

### "Output goes to wrong window"
**Cause**: Using global stdout instead of ios_stdout()
**Fix**: Use fprintf(ios_stdout(), ...)

### "Environment variable not set"
**Cause**: Using setenv() instead of ios_setenv()
**Fix**: Include ios_error.h for macro redirection

### "Command not found after install"
**Cause**: Missing or incorrect commands.plist
**Fix**: Verify plist format and framework path

## Thread Safety Guarantees

### What is Thread-Safe

All ios_system APIs are thread-safe:
- `ios_system()` - Uses thread-local session
- `ios_getenv()/ios_setenv()` - Per-session storage
- `ios_stdout()/ios_stderr()/ios_stdin()` - Thread-local streams
- `ios_switchSession()` - Atomic session switch

### What is NOT Thread-Safe

Standard libc functions are NOT thread-safe across sessions:
- `getenv()` without ios_error.h - Uses global environment
- Global `stdout` - Goes to wrong session
- `chdir()` - Affects entire process

### Recommendations

1. **Always use ios_* functions** in multi-session contexts
2. **Include ios_error.h** to redirect standard calls
3. **Test with multiple windows** in a-Shell

## Error Handling

### Return Values

| Function | Success | Error |
|----------|---------|-------|
| `ios_system()` | Exit status 0 | Non-zero exit status |
| `ios_fork()` | - | Returns -1, errno=ENOSYS |
| `ios_getenv()` | Pointer to value | NULL |
| `ios_setenv()` | 0 | -1 |

### errno Values

- `ENOSYS` - Function not implemented (fork, vfork, etc.)
- `EPERM` - Operation not permitted (sandbox violation)
- `ENOENT` - No such file or directory
- `ENOMEM` - Out of memory

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-03 | Initial contract definition |
| 1.1 | 2026-03 | Added thread safety section, error handling |

## See Also

- [Porting Guide](../guides/porting_guide.md) - Step-by-step porting instructions
- [Package Manifest Schema](../specs/package-manifest-schema.md) - Build configuration
- [iOS Build on Docker](../development/ios-build-on-docker.md) - CI/CD setup

## References

- ios_system source: https://github.com/holzschu/ios_system
- a-Shell: https://github.com/holzschu/a-shell
- Termux packages (inspiration): https://github.com/termux/termux-packages
- Apple iOS File System Programming Guide: https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/

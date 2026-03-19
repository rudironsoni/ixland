# Hardening Tier A Commands Guide (M3-I1)

## Overview

This guide documents how to harden BSD commands for reliable operation under ios_system with thread-local I/O and session management.

## Current Status

**Phase:** Implementation Ready
**Priority:** P2
**Blocked by:** None
**Blocks:** M3-I2 (conformance test suite)

## Background

The BSD commands in ios_system (file_cmds, text_cmds, shell_cmds) already use ios_error.h for I/O redirection. This guide documents verification and improvements needed.

## I/O Redirection Mechanism

Commands include `ios_error.h` which provides:

```c
// Automatic redirection via macros
#define putchar(a) fputc(a, thread_stdout)
#define getchar() fgetc(thread_stdin)
#define write ios_write
#define fwrite ios_fwrite
#define puts ios_puts
#define fputs ios_fputs

// Thread-local streams
extern __thread FILE* thread_stdout;
extern __thread FILE* thread_stderr;
```

This ensures output goes to the correct terminal session.

## Verification Checklist

### File Commands (file_cmds/)
- [ ] **ls** - Uses printf/puts, should work via macros
- [ ] **cp** - File operations + progress output
- [ ] **mv** - File operations + error messages
- [ ] **rm** - Recursive delete, confirmation prompts
- [ ] **mkdir** - Directory creation
- [ ] **touch** - File timestamp modification
- [ ] **ln** - Symbolic/hard links
- [ ] **chmod** - Permission changes
- [ ] **chown** - Ownership changes (sideload only)

### Text Commands (text_cmds/)
- [ ] **cat** - File concatenation
- [ ] **grep** - Pattern matching
- [ ] **sed** - Stream editing
- [ ] **awk** - Pattern scanning (via awk framework)
- [ ] **sort** - Line sorting
- [ ] **head** - First N lines
- [ ] **tail** - Last N lines
- [ ] **wc** - Word/line/char count
- [ ] **diff** - File comparison

### Shell Commands (shell_cmds/)
- [ ] **echo** - String output
- [ ] **pwd** - Print working directory
- [ ] **basename** - Filename extraction
- [ ] **dirname** - Directory extraction
- [ ] **date** - Date/time display
- [ ] **env** - Environment display
- [ ] **kill** - Signal sending
- [ ] **ps** - Process status (needs M4-I1)
- [ ] **uname** - System info (needs M4-I1)

### Archive Commands
- [ ] **tar** - Archive operations (libarchive-based)
- [ ] **gzip** - Compression

## Commands Needing Special Attention

### 1. Interactive Commands
Commands that prompt for input need special handling:

```c
// For rm -i, mv -i, cp -i
if (isatty(STDIN_FILENO)) {
    // Prompt user: fputs("remove file? ", thread_stdout);
    // Read response: fgets(buffer, sizeof(buffer), thread_stdin);
}
```

### 2. Progress Indicators
Commands like cp with progress need to use thread_stdout:

```c
// Instead of: fprintf(stderr, "copied %d%%\n", percent);
// Use:        fprintf(thread_stderr, "copied %d%%\n", percent);
```

### 3. Error Messages
All error output should go to thread_stderr:

```c
// Already handled by err.h macros if using warn/err functions
warn("file not found");  // Goes to thread_stderr via ios_error.h
```

## Testing Protocol

### Test 1: Basic Output
```bash
# In a-Shell
ls -la > /tmp/output.txt
# Verify output captured in file (not printed to terminal)
```

### Test 2: Session Isolation
```bash
# Session A
export TEST_VAR=sessionA

# Session B
export TEST_VAR=sessionB
echo $TEST_VAR  # Should print "sessionB"

# Session A
echo $TEST_VAR  # Should print "sessionA"
```

### Test 3: Pipeline
```bash
ls | grep "txt" | wc -l
# Should work correctly with thread-local I/O
```

### Test 4: Error Output
```bash
cp nonexistent/file.txt dest/ 2>&1 | grep "No such file"
# Error should be captured
```

## Known Issues

1. **ps command** - Needs M4-I1 (ios_sysinfo.c) for process enumeration
2. **uname** - Needs M4-I1 for system info
3. **df** - Sideload only (requires root)
4. **w** - Sideload only (requires access to other users' processes)

## Implementation Priority

1. **P0 (Critical):** ls, cat, cp, mv, rm, echo, pwd
2. **P1 (Important):** grep, sed, mkdir, touch, head, tail
3. **P2 (Standard):** sort, wc, diff, ln, chmod
4. **P3 (Optional):** chown, df, w (sideload only)

## Files to Modify

No modifications needed if commands already include ios_error.h. Verification only:

```bash
# Check if command includes ios_error.h
grep -l "ios_error.h" ios_system/*/cmd_ios/*.c

# Check for direct stdout/stderr usage
grep -n "fprintf(stdout" ios_system/*/cmd_ios/*.c
grep -n "fprintf(stderr" ios_system/*/cmd_ios/*.c
```

## Verification Steps

1. Build each command
2. Test in single session
3. Test with multiple sessions
4. Test in pipelines
5. Verify error codes match BSD

## Agent Notes

- Most commands already work via ios_error.h macros
- Focus on interactive commands (rm -i, cp -i, mv -i)
- Test ps/uname after M4-I1 is complete
- Don't modify commands unless they fail tests
- Keep BSD behavior exactly (no feature changes)

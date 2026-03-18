# Task Specification Template

Every a-Shell Next task MUST include these sections with this level of detail.

## Title Format

```
{Milestone}-I{Number}: {Action verb} {what} {for whom/what}
```

**Examples:**
- `M2-I1: Create ashell_package.sh build system library`
- `M2-I3: Create hello reference package demonstrating full pattern`
- `M3-I1: Harden ls command to ios_system contract`

## Required Sections

### 1. Goal (1 sentence)

What to accomplish. Must be testable.

**Good:** "Create ashell_package.sh with all step functions for building iOS packages"
**Bad:** "Work on build system" (not testable)

### 2. Success Criteria (checklist)

- [ ] Specific, measurable outcomes
- [ ] Each item has verification method
- [ ] No subjective criteria

**Example:**
```markdown
- [ ] ashell_step_extract_package downloads and verifies SHA256
- [ ] ashell_step_patch_package applies patches in order
- [ ] ashell_step_configure sets iOS cross-compilation flags
- [ ] All functions have error handling
- [ ] Unit tests pass (run ./test_package.sh)
```

### 3. Context (2-3 paragraphs)

- Why this matters
- What depends on it
- Technical background
- iOS-specific constraints

**Example:**
```markdown
This is the foundation of the package system. Every package needs these
step functions to build correctly. Without them, we cannot compile any
Unix tools for iOS.

The build system mirrors Termux's approach but adapts for iOS constraints:
- Uses XCFrameworks instead of ELF binaries
- No real fork/exec (simulated via pthreads)
- Sandboxed file system access

This work blocks all M2 package creation tasks. The hello package
demonstrates the complete pattern.
```

### 4. Dependencies

```markdown
- **Blocked by:** Exact issue IDs (none for root tasks)
- **Blocks:** Exact issue IDs that depend on this
- **Related:** Reference issues for context
```

**Use exact IDs:**
- `M2-I4` (not "cross-compilation task")
- `a-shell-next-fk8y` (beads ID if known)

### 5. Code Snippets

Working code examples:
- Function signatures
- Implementation patterns
- Error handling
- Integration points

**Must include:**
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

### 6. Exact File Paths

Absolute paths from repo root:
- Files to create
- Files to modify (with line numbers if applicable)
- Directories to create

**Format:**
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

### 7. Copy-Paste Commands

Exact commands for:
- Setup
- Implementation
- Testing
- Verification

**Format:**
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

### 8. Decision Records

| Decision | Rationale | Alternatives Rejected |
|----------|-----------|----------------------|
| Bash over Python | Matches Termux, easier for Unix devs | Python (more deps) |
| XCFramework | Apple's modern format | .a files (deprecated) |

### 9. Do's and Don'ts

Specific guidelines based on past experience:

```markdown
## Do's and Don'ts

### Do
- Test on physical device before claiming done
- Include error handling for every network call
- Use ASHELL_ prefix for all environment variables
- Validate file paths before use

### Don't
- Assume macOS/Xcode is available (lazy load SDK path)
- Skip error handling for commands that can fail
- Use global stdout - use ios_stdout() in C/Swift
- Hardcode paths - use $ASHELL_PREFIX
```

### 10. When to Stop

Explicit conditions:

```markdown
## When to Stop

### Complete When
- [ ] All success criteria pass
- [ ] Tests run green
- [ ] Documentation updated
- [ ] No TODOs or placeholders remain

### Escalate If
- Blocked by external dependency > 24 hours
- Design decision needed (security, architecture)
- Scope creep beyond original task

### Ask for Help When
- Stuck for > 2 hours on same problem
- Unclear how to implement iOS-specific behavior
- Tests fail with mysterious errors
```

### 11. Verification Steps

Numbered, specific, copy-pasteable:

```markdown
## Verification Steps

1. **Run:** `./build.sh hello`
2. **Expect:** Build completes without errors
3. **Check:** `ls -la .build/hello/hello.xcframework`
   - Should exist
   - Should contain ios-arm64/
   - Should have Info.plist
4. **Validate:** `plutil -lint .build/hello/commands.plist`
   - Should output "OK"
5. **Test:** Load framework in a-Shell app
   - Add to Xcode project
   - Build and run
   - Type `hello` in terminal
   - Should output "Hello from a-Shell!"
```

### 12. Agent Notes (NEW)

Special instructions for AI agents:

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

## Complete Example Task

See any issue in the beads database for a complete example:
```bash
bd show <issue-id>
```

## Creating New Issues

When creating new issues, always include:
1. All 12 sections above
2. Real code snippets (not placeholders)
3. Exact file paths with line numbers
4. Copy-pasteable verification commands
5. Agent notes with specific guidance

**Shortcut:**
```bash
# Create issue with all fields
bd create --title="..." --description="..." --type=task --priority=2

# Then update with full description including all sections
bd update <id> --notes="..."
```

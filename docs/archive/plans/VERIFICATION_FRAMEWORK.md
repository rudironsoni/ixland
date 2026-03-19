# Verification Framework

**Rule**: No component is "done" until it passes all verification steps.

## Verification Levels

### Level 1: Syntax/Structure
- File exists at correct path
- Basic syntax is valid (bash -n, swift build --dry-run)
- Required variables/functions are defined

### Level 2: Static Analysis
- Code follows project conventions
- No obvious bugs or security issues
- Documentation/comments are present

### Level 3: Unit Test
- Component works in isolation
- Test with mock inputs
- Edge cases handled

### Level 4: Integration Test
- Component works with other parts
- End-to-end workflow passes
- Error handling verified

### Level 5: Production Ready
- Works on target platform (iOS/macOS)
- Performance acceptable
- Security review passed

---

## Component Verification Checklist

### Build System Components

#### ashell_package.sh
- [ ] L1: File exists, syntax valid (`bash -n`)
- [ ] L2: All functions documented
- [ ] L3: Test each function with mocks
- [ ] L4: Test with hello package
- [ ] L5: Build on macOS with Xcode

#### build.sh
- [ ] L1: File exists, syntax valid
- [ ] L2: Commands documented (build, clean, list)
- [ ] L3: Test `list` command
- [ ] L4: Test `build hello` (may fail on Linux, that's OK)
- [ ] L5: Full build on macOS

#### Individual Step Scripts (if created)
- [ ] L1: Each script exists in scripts/build/
- [ ] L2: Scripts source ashell_package.sh
- [ ] L3: Test each step independently
- [ ] L4: Integration with main build
- [ ] L5: Production builds

### ashell-system Components

#### ashell_error.h
- [ ] L1: File exists, valid C syntax
- [ ] L2: All required macros defined
- [ ] L3: Test compilation with sample code
- [ ] L4: Test with package that uses fork()
- [ ] L5: Production package builds

#### ios_fork(), ios_vfork()
- [ ] L1: Functions defined
- [ ] L2: Return values documented
- [ ] L3: Unit test returns ENOSYS
- [ ] L4: Test with shell code
- [ ] L5: Real shell works

#### ios_execv*() family
- [ ] L1: All variants defined
- [ ] L2: Documentation complete
- [ ] L3: Unit test dispatches correctly
- [ ] L4: Test with command execution
- [ ] L5: Real commands work

#### Process Table
- [ ] L1: Data structures defined
- [ ] L2: Thread-safe implementation
- [ ] L3: Unit test add/remove processes
- [ ] L4: Test with multiple commands
- [ ] L5: Production use stable

### apt Package

#### Build Recipe
- [ ] L1: build.sh exists, syntax valid
- [ ] L2: Version 3.1.16 specified
- [ ] L3: Patches apply cleanly (test)
- [ ] L4: Configure step works
- [ ] L5: Full build produces XCFramework

#### Patches
- [ ] L1: All patches exist
- [ ] L2: Patches documented
- [ ] L3: Test apply to 3.1.16 source
- [ ] L4: Build with patches succeeds
- [ ] L5: Runtime behavior correct

### Swift Components

#### Remove apt.swift
- [ ] L1: File identified
- [ ] L2: Dependencies checked
- [ ] L3: Remove file
- [ ] L4: Verify build still works
- [ ] L5: No references remain

---

## Current Component Status

| Component | L1 | L2 | L3 | L4 | L5 | Status |
|-----------|----|----|----|----|----|--------|
| ashell_package.sh | ? | ? | ? | ? | ? | UNTESTED |
| build.sh | ? | ? | ? | ? | ? | UNTESTED |
| ashell_error.h | ❌ | ❌ | ❌ | ❌ | ❌ | MISSING |
| ios_* syscalls | ❌ | ❌ | ❌ | ❌ | ❌ | MISSING |
| apt 3.1.16 | ❌ | ❌ | ❌ | ❌ | ❌ | WRONG VERSION |
| Remove apt.swift | ❌ | ❌ | ❌ | ❌ | ❌ | NOT DONE |

---

## Verification Commands

### Bash Syntax Check
```bash
bash -n ashell_package.sh
echo $?  # 0 = success
```

### Test Function Exists
```bash
grep -q "ashell_step_extract_package()" ashell_package.sh
echo $?  # 0 = success
```

### Test Build List
```bash
./build.sh list
# Should show packages without errors
```

### Test Build (Linux will fail, that's expected)
```bash
./build.sh hello 2>&1
# Expected: ERROR (no Xcode) on Linux
# Verify: Error message is clear
```

---

## Honest Reporting Template

When reporting status, use this format:

```
Component: <name>
Claimed Status: <what was claimed>
Actual Status: <what testing revealed>
Verification Level: <L1-L5>
Test Evidence: <command output>
Blockers: <what's preventing completion>
Next Step: <what needs to be done>
```

---

## Rules

1. **Never claim "done" without L4 or L5 verification**
2. **Document all test commands and outputs**
3. **Be explicit about blockers**
4. **Update status honestly when tests fail**
5. **Linux build failures are OK** - just document them


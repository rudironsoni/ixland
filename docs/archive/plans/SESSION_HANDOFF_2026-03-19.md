# Session Handoff - a-Shell Next

**Date**: 2026-03-19
**Platform**: Linux (development environment)
**Next Session**: macOS (for iOS builds)

---

## Session Summary

### What Was Done
1. **Analyzed current codebase state** - Discovered significant gaps vs plan
2. **Created verification framework** - Proper testing methodology
3. **Tested build system** - Works for listing, fails on Linux (expected)
4. **Documented gaps** - Critical missing components identified

### Key Discovery
**The build system infrastructure exists but CANNOT fully test on Linux.**
- `build.sh list` works ✓
- `build.sh hello` fails (no Xcode/clang) ✗
- Requires macOS for actual builds

---

## Critical Path - Must Do First

### 1. Create ashell_error.h (CRITICAL)
**Location**: `ashell-system/Headers/ashell_error.h`
**Purpose**: Drop-in syscall replacements via macros

```c
#ifndef ASHELL_ERROR_H
#define ASHELL_ERROR_H

#define fork    ios_fork
#define vfork   ios_vfork
#define execv   ios_execv
#define execvp  ios_execvp
#define execve  ios_execve
#define waitpid ios_waitpid
#define wait    ios_wait
#define exit    ios_exit

#define getenv  ios_getenv
#define setenv  ios_setenv

#define signal  ios_signal
#define sigaction ios_sigaction

#endif
```

**Why Critical**: Required for bash/zsh compilation. Without this, shells can't be built.

### 2. Implement ios_* Syscalls (CRITICAL)
**Location**: `ashell-system/Sources/ashell_syscalls.c`

```c
pid_t ios_fork(void) {
    errno = ENOSYS;
    return -1;
}

pid_t ios_vfork(void) {
    // Allocate virtual PID
    static pid_t next = 1000;
    return __atomic_fetch_add(&next, 1, __ATOMIC_SEQ_CST);
}

int ios_execv(const char *path, char *const argv[]) {
    // Build command string
    // Call ios_system()
    // Never return
}
```

**Why Critical**: Packages call fork/exec. Must have iOS-compatible implementations.

### 3. Update apt to 3.1.16 (HIGH)
**Location**: `ashell-packages/root-packages/apt/build.sh`

Change:
```bash
ASHELL_PKG_VERSION="2.8.1"  # OLD
ASHELL_PKG_VERSION="3.1.16" # NEW
```

Then update SHA256 and patches for 3.1.16.

### 4. Remove Swift apt.swift (HIGH)
**Location**: `ashell-core/Sources/Commands/apt.swift`

Action: Delete or move to deprecated/. Update any imports.

---

## Test Commands (Run on macOS)

```bash
# 1. Navigate to project
cd ~/src/github/rudironsoni/a-shell-next/ashell-packages

# 2. List packages (should work)
./build.sh list

# 3. Build hello package (should work on macOS with Xcode)
./build.sh hello

# 4. Check output
ls -la .build/hello/*.xcframework

# 5. Build apt
./build.sh apt  # After updating to 3.1.16

# 6. Verify ashell_error.h works
# Create test program that includes it and calls fork()
# Should compile and link against ashell-system
```

---

## Files to Review

| File | Purpose | Status |
|------|---------|--------|
| `docs/plans/a-shell-next-master-plan.md` | Full master plan | Read first |
| `docs/plans/IMPLEMENTATION_GAPS.md` | What's missing | Critical read |
| `docs/plans/VERIFICATION_RESULTS.md` | Test results | Context |
| `ashell_package.sh` | Build system | Review |
| `build.sh` | Main build script | Review |
| `root-packages/apt/build.sh` | apt build | Update version |
| `ashell-core/Sources/Commands/apt.swift` | Swift apt | Remove |

---

## Beads Issues to Continue

### Open Issues (Ready to Work)
```bash
bd ready                    # See what's ready
bd show a-shell-next-cpfy   # M3-I1: ashell_error.h
bd show a-shell-next-mdcn   # M3-I2: ios_fork()
```

### Reopen These When Starting
- a-shell-next-cpfy (M3-I1)
- a-shell-next-mdcn (M3-I2)
- a-shell-next-nvq5 (M3-I3)

---

## Environment Setup (macOS)

```bash
# 1. Verify Xcode installed
xcode-select --install  # If not installed
xcrun --version

# 2. Verify iOS SDK
xcrun --sdk iphoneos --show-sdk-path

# 3. Test build
./build.sh hello

# 4. Install osxcross (optional, for Linux builds later)
# https://github.com/tpoechtrager/osxcross
```

---

## Unresolved Questions

1. **scripts/build/ directory** - Empty. Design choice or gap?
   - Current: Functions in ashell_package.sh
   - Plan: Separate scripts
   - Decision needed: Keep as-is or refactor?

2. **Linux cross-compilation** - Required or nice-to-have?
   - Current: macOS-only
   - Option: Add osxcross for Linux builds
   - Decision needed: Support Linux developers?

3. **Swift apt.swift** - Remove or keep as fallback?
   - Current: Still exists
   - Plan: Remove
   - Decision needed: Clean removal or gradual deprecation?

---

## Success Criteria for Next Session

Before claiming anything "done":

1. **ashell_error.h** - Must compile with test program
2. **ios_fork()** - Must return ENOSYS when tested
3. **apt 3.1.16** - Must build on macOS
4. **build.sh hello** - Must produce working XCFramework

---

## Contact

**Rudi** - Project owner
**Tested on**: Linux (Debian)
**Next test on**: macOS (your machine)

---

## Quick Start Checklist

When you continue on macOS:

- [ ] Verify Xcode installed
- [ ] Run `./build.sh list` (should work)
- [ ] Run `./build.sh hello` (should build)
- [ ] Check `.build/hello/*.xcframework` exists
- [ ] Create `ashell-system/Headers/ashell_error.h`
- [ ] Implement ios_* syscalls
- [ ] Update apt to 3.1.16
- [ ] Remove apt.swift
- [ ] Re-run verification tests
- [ ] Update beads issues with actual results

---

**End of Session Handoff**

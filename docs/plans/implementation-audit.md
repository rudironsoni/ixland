# a-Shell Next: Implementation Audit

**Date**: 2026-03-19
**Purpose**: Compare current implementation vs decided architecture, identify gaps

---

## Executive Summary

| Component | Status | Gap | Priority |
|-----------|--------|-----|----------|
| Clean plan exists | ✅ Done | - | - |
| apt directories merged | ❌ Not done | Duplicate apt/ exists | P0 |
| apt updated to 3.1.16 | ❌ Not done | Still at 2.8.1 | P0 |
| Swift apt removed | ❌ Not done | Still in ashell-core/ | P0 |
| ashell-system extended | ❌ Not done | Missing syscall replacements | P1 |
| WASM infrastructure | ⚠️ Partial | Uses wasm3.js, needs wasi-libc | P1 |
| Python runtime | ❌ Not done | No BeeWare integration | P2 |
| Build system | ✅ Done | ashell_package.sh exists | - |
| Package recipes | ✅ Done | hello, coreutils-minimal exist | - |

---

## Critical Issues Found

### Issue 1: Duplicate apt Directories

**Problem**: apt exists in two locations:
- `/ashell-packages/apt/` (WRONG - should be removed)
- `/ashell-packages/root-packages/apt/` (CORRECT)

**Evidence**:
```bash
# Current structure:
ashell-packages/
├── apt/                    # ❌ WRONG - duplicate
│   ├── build.sh
│   └── patches/
└── root-packages/
    └── apt/                # ✅ CORRECT
        ├── build.sh
        └── patches/
```

**Impact**: Confusion about which is canonical. Build scripts may reference wrong path.

**Fix**:
1. Compare contents of both directories
2. Merge any unique content from `ashell-packages/apt/` → `root-packages/apt/`
3. Delete `ashell-packages/apt/`
4. Update all references

---

### Issue 2: Swift apt Implementation (WRONG)

**Problem**: Current implementation is a Swift package manager, NOT Debian apt.

**Evidence** (ashell-core/Sources/Commands/apt.swift, lines 16-260):
```swift
@_cdecl("apt")
public func apt(argc: Int32, argv: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?) -> Int32 {
    // ...
    Task {
        let manager = PackageManager.shared
        let installResult = await manager.install(package: packageName)
        // Downloads XCFrameworks via URLSession - NOT apt!
    }
}
```

**Expected**: Real Debian apt 3.1.16 binary compiled for iOS.

**Gap**:
- No apt XCFramework built from Debian source
- No dpkg integration
- No .deb package handling
- No dependency resolution

**Fix**:
1. Remove `ashell-core/Sources/Commands/apt.swift`
2. Remove `ashell-core/Sources/PackageManager/PackageManager.swift`
3. Build real apt via `root-packages/apt/build.sh`
4. Integrate apt binary into app

---

### Issue 3: apt Version Outdated

**Problem**: apt build.sh references version 2.8.1, should be 3.1.16.

**Evidence** (ashell-packages/root-packages/apt/build.sh, line 6-7):
```bash
ASHELL_PKG_NAME="apt"
ASHELL_PKG_VERSION="2.8.1"  # ❌ OLD - should be 3.1.16
```

**Impact**: Missing features and bug fixes from latest apt.

**Fix**:
1. Update version to 3.1.16
2. Update download URL
3. Update SHA256
4. Review patches for compatibility with 3.1.16

---

### Issue 4: Missing ashell-system Extensions

**Problem**: ios_system only provides `#define system ios_system`. Missing replacements for fork, exec, getenv.

**Current** (ios_system/ios_system.h):
```c
#define system ios_system
```

**Expected** (ashell-system/ashell_error.h):
```c
#define system ios_system
#define fork ios_fork
#define execv ios_execv
#define execvp ios_execvp
#define getenv ios_getenv
#define setenv ios_setenv
```

**Gap**:
- No `ashell_error.h` header exists
- No `ios_fork()`, `ios_execv()`, etc. implementations
- No session-based PID simulation
- Packages can't compile if they use fork/exec

**Fix**:
1. Create `ios_system/ashell_error.h` with all syscall replacements
2. Implement `ios_fork()` (returns ENOSYS)
3. Implement `ios_execv()` (redirects to ios_system)
4. Implement `ios_getenv()` (session-local)
5. Update ios_system.h to include ashell_error.h

---

### Issue 5: WASM Infrastructure Incomplete

**Problem**: Current WASM uses wasm3.js JavaScript bridge. Needs wasi-libc + POSIX layer.

**Current** (a-shell/wasm.js):
- Uses @wasmer/wasi JavaScript library
- Requires WebView JavaScript bridge
- Performance: 10-50x slower than native

**Expected**:
- wasi-libc as base
- POSIX layer on top
- Direct ios_system integration (no JS)
- AOT compilation for hot paths

**Gap**:
- No wasi-libc integration
- No custom WASI syscalls (ios_open_tty, etc.)
- No POSIX signal emulation
- No zero-copy pipes

**Fix**:
1. Integrate wasi-libc into build system
2. Create POSIX layer (signals, process model)
3. Add custom WASI syscalls
4. Bridge directly to ios_system (no JavaScript)

---

### Issue 6: Python Runtime Not Integrated

**Problem**: No BeeWare Python-Apple-support integration.

**Gap**:
- No Python XCFramework build recipe
- No cibuildwheel integration for iOS wheels
- No custom pip index
- No wheel post-processor

**Fix**:
1. Create `ashell-runtimes/python-runtime/` or `root-packages/python/`
2. Integrate BeeWare Python-Apple-support
3. Set up cibuildwheel for iOS wheels
4. Create wheel post-processor (so → framework)
5. Set up pip index infrastructure

---

## Gap Analysis by Component

### ashell-system (ios_system)

| Feature | Current | Required | Gap |
|---------|---------|----------|-----|
| system() replacement | ✅ | ✅ | None |
| fork() replacement | ❌ | ✅ | Add ios_fork() |
| execv() replacement | ❌ | ✅ | Add ios_execv() |
| getenv() replacement | ❌ | ✅ | Add ios_getenv() |
| Session management | ⚠️ Basic | ⚠️ Full | Extend PID simulation |
| Thread-local I/O | ✅ | ✅ | None |
| ashell_error.h | ❌ | ✅ | Create header |

### Package Manager

| Feature | Current | Required | Gap |
|---------|---------|----------|-----|
| Debian apt 3.1.16 | ❌ | ✅ | Build real apt |
| dpkg integration | ❌ | ✅ | Build dpkg |
| .deb handling | ❌ | ✅ | Add to apt |
| WASM packages | ⚠️ | ✅ | Complete WASI infra |
| Swift pkg command | ✅ | ❌ | Remove it |

### Build System

| Feature | Current | Required | Gap |
|---------|---------|----------|-----|
| ashell_package.sh | ✅ | ✅ | None |
| Termux-style recipes | ✅ | ✅ | None |
| Git diff patches | ✅ | ✅ | None |
| XCFramework gen | ✅ | ✅ | None |
| apt recipe | ⚠️ | ⚠️ | Update to 3.1.16 |

### WASM Runtime

| Feature | Current | Required | Gap |
|---------|---------|----------|-----|
| wasm3 engine | ✅ | ✅ | None |
| wasi-libc | ❌ | ✅ | Integrate |
| POSIX layer | ❌ | ✅ | Build on wasi-libc |
| Custom WASI syscalls | ❌ | ✅ | Add for TTY |
| AOT compilation | ❌ | ⚠️ | Future enhancement |
| No JS bridge | ❌ | ✅ | Direct integration |

---

## Priority Order for Implementation

### P0: Critical (Blocks all other work)
1. Merge apt directories (remove duplicate)
2. Remove Swift apt implementation
3. Update apt to 3.1.16
4. Build real apt XCFramework

### P1: High (Enables package ecosystem)
5. Create ashell_error.h with syscall replacements
6. Implement ios_fork, ios_execv, ios_getenv
7. Create wasi-libc integration
8. Build POSIX layer on WASI

### P2: Medium (Feature completeness)
9. Integrate BeeWare Python runtime
10. Set up cibuildwheel for iOS wheels
11. Create custom pip index
12. Add WASM AOT compilation

### P3: Low (Polish)
13. Performance optimization
14. Documentation completion
15. CI/CD for package builds

---

## Migration Steps

### Step 1: Clean up apt (Week 1)

```bash
# 1. Merge directories
cd /home/rrj/src/github/rudironsoni/a-shell-next/ashell-packages

# Compare contents
diff -r apt/ root-packages/apt/

# Merge if needed
cp -r apt/patches/* root-packages/apt/patches/ 2>/dev/null || true

# Remove duplicate
rm -rf apt/

# 2. Update version
# Edit root-packages/apt/build.sh: change 2.8.1 → 3.1.16

# 3. Remove Swift implementation
git rm ashell-core/Sources/Commands/apt.swift
git rm ashell-core/Sources/PackageManager/PackageManager.swift

# Keep ArchiveExtractor.swift (reusable)
```

### Step 2: Extend ashell-system (Week 2-3)

```bash
# Create ashell_error.h
cat > ios_system/ashell_error.h << 'EOF'
#ifndef ASHELL_ERROR_H
#define ASHELL_ERROR_H

#define fork ios_fork
#define execv ios_execv
#define execvp ios_execvp
#define execve ios_execve
#define getenv ios_getenv
#define setenv ios_setenv
#define putenv ios_putenv
#define exit ios_exit
#define signal ios_signal
#define raise ios_raise

#endif
EOF

# Implement in ios_system.c
# Add ios_fork(), ios_execv(), ios_getenv()
```

### Step 3: Build Real apt (Week 3-4)

```bash
cd ashell-packages
./build.sh apt

# Test: should produce apt.framework/
ls -la .build/apt/apt.framework/
```

### Step 4: WASM Infrastructure (Week 5-8)

- Integrate wasi-libc
- Build POSIX layer
- Add custom WASI syscalls
- Test vim.wasm, git.wasm

---

## Questions for User

1. **apt patches**: Do you want to keep the existing patches for apt 2.8.1 and adapt them for 3.1.16, or start fresh with 3.1.16 patches?

2. **Swift PackageManager**: The ArchiveExtractor.swift has libarchive integration - should we keep this as a utility even after removing the Swift apt command?

3. **ashell-system location**: Should the new ashell_error.h go in `ios_system/` (current) or should we create `ashell-system/` directory now?

4. **WASM engine**: Do you want to stick with wasm3 or migrate to WAMR for better WASI support?

5. **Python runtime**: Should this be in `root-packages/python/` (distro essential) or `ashell-runtimes/python-runtime/` (separate repo)?

---

## Success Criteria by Phase

### Phase 1 Complete When:
- [ ] No duplicate apt directories
- [ ] Swift apt.swift removed
- [ ] apt 3.1.16 builds successfully
- [ ] `./build.sh apt` produces apt.framework

### Phase 2 Complete When:
- [ ] ashell_error.h exists with all syscall replacements
- [ ] ios_fork(), ios_execv(), ios_getenv() implemented
- [ ] Test package using fork() compiles and runs

### Phase 3 Complete When:
- [ ] wasi-libc integrated
- [ ] POSIX layer provides signal emulation
- [ ] vim.wasm runs without JavaScript bridge
- [ ] WASM packages install via apt

### Phase 4 Complete When:
- [ ] Python runtime builds from BeeWare
- [ ] pip install works for pure Python packages
- [ ] Custom wheel index serves iOS wheels
- [ ] numpy installs via pip

---

## References

- Original plan: `docs/plans/20260318-a-shell-next.md`
- Clean plan: `/home/rrj/.claude/plans/a-shell-next-clean.md`
- Build system: `ashell-packages/ashell_package.sh`
- Current apt: `ashell-packages/root-packages/apt/build.sh`
- Swift apt (to remove): `ashell-core/Sources/Commands/apt.swift`

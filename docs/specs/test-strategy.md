# Test Strategy and Behavior Gates

## Overview

This document defines the testing strategy for a-Shell Next using **behavior-based gates** rather than coverage metrics. Tests validate that the system behaves correctly under real-world conditions.

## Philosophy

> "A test that doesn't catch real bugs is waste. A gate that doesn't protect users is theater."

### Principles

1. **Behavior over coverage** - 80% coverage of useless code is worthless
2. **Real conditions over mocks** - Test on actual iOS devices when possible
3. **Fail fast** - Catch errors at the right level
4. **Automate gates** - Manual testing doesn't scale

## Test Pyramid for a-Shell

```
                    ┌─────────────────┐
                    │   E2E Tests   │  ← 5% - Full integration
                    │  (iOS Device) │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │  Package Tests  │  ← 15% - XCFramework load
                    │  (Simulator)  │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │  Build System   │  ← 30% - Script validation
                    │    (Docker)     │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │  Unit Tests     │  ← 50% - Pure logic
                    │   (Linux/Mac)   │
                    └─────────────────┘
```

## Behavior-Based Gates

### Gate Categories

| Gate | Phase | What It Validates | How It Runs |
|------|-------|-------------------|-------------|
| **Syntax Gate** | M0 | Scripts have no syntax errors | Docker |
| **Contract Gate** | M0 | APIs match documented contract | Docker |
| **Build Gate** | M2 | Packages build without errors | macOS |
| **XCFramework Gate** | M2 | Valid XCFramework structure | macOS |
| **Integration Gate** | M3 | Commands run in ios_system | Simulator |
| **Session Gate** | M3 | Sessions don't leak state | Simulator |
| **Safety Gate** | M3 | No file system escapes | Simulator |
| **Performance Gate** | M6 | Acceptable load times | Device |

## Phase-Specific Gates

### M0: Contract & Documentation

**Gate: Contract Compliance**
- **Validates**: All public APIs have tests
- **Pass Criteria**: 100% of documented APIs have smoke tests
- **Runs On**: Docker (Ubuntu)
- **Time**: < 1 minute

**Gate: Syntax Validation**
- **Validates**: All bash scripts parse
- **Pass Criteria**: `bash -n` passes on all *.sh
- **Runs On**: Docker (Ubuntu)
- **Time**: < 30 seconds

### M1: Platform Hardening

**Gate: Thread Safety**
- **Validates**: Concurrent sessions don't corrupt state
- **Pass Criteria**: 1000 rapid session switches without crash
- **Runs On**: macOS
- **Time**: < 5 minutes

**Gate: Environment Isolation**
- **Validates**: Sessions have isolated environments
- **Pass Criteria**: Session A env doesn't leak to Session B
- **Runs On**: Simulator
- **Time**: < 2 minutes

### M2: Forge Alpha

**Gate: Bootstrap Success**
- **Validates**: hello package builds clean
- **Pass Criteria**: XCFramework created, plist valid
- **Runs On**: macOS
- **Time**: < 3 minutes

**Gate: Package Install**
- **Validates**: pkg install works end-to-end
- **Pass Criteria**: Install, run, uninstall cycle works
- **Runs On**: Simulator
- **Time**: < 5 minutes

**Gate: Reproducible Build**
- **Validates**: Same source produces same XCFramework
- **Pass Criteria**: SHA256 of XCFramework matches across runs
- **Runs On**: macOS
- **Time**: < 10 minutes

### M3: Core Shell Beta

**Gate: Tier A Command Conformance**
- **Validates**: Essential commands behave like BSD
- **Commands**: ls, cat, cp, mv, rm, mkdir, touch
- **Pass Criteria**: 90%+ match BSD behavior in test suite
- **Runs On**: Simulator
- **Time**: < 10 minutes

**Gate: File Safety**
- **Validates**: No data loss operations
- **Tests**: cp -r, mv across devices, rm -rf
- **Pass Criteria**: All operations safe, recoverable, or confirmed
- **Runs On**: Simulator
- **Time**: < 5 minutes

**Gate: Pipeline Stability**
- **Validates**: Command chains work: `ls | grep | wc`
- **Pass Criteria**: 100+ pipeline combinations
- **Runs On**: Simulator
- **Time**: < 3 minutes

**Gate: Session Validity**
- **Validates**: Sessions survive app backgrounding
- **Pass Criteria**: 100 switch cycles without crash
- **Runs On**: Device (manual)
- **Time**: < 15 minutes

### M4: Interactive UX

**Gate: LineEditor Integration**
- **Validates**: History, completion work
- **Pass Criteria**: Interactive commands have history
- **Runs On**: Device
- **Time**: Manual

### M6: Release Candidate

**Gate: Performance Baseline**
- **Validates**: Cold start < 2 seconds
- **Pass Criteria**: Time from launch to prompt
- **Runs On**: Device
- **Time**: Automated

**Gate: Memory Stability**
- **Validates**: No leaks over extended use
- **Pass Criteria**: Memory delta < 10MB after 100 commands
- **Runs On**: Device
- **Time**: < 10 minutes

## Test Implementation

### Level 1: Static Analysis (Docker)

```bash
# syntax-check.sh - Fast, runs on every commit
#!/bin/bash
FAILED=0

for script in $(find . -name '*.sh'); do
    if ! bash -n "$script"; then
        echo "Syntax error: $script"
        ((FAILED++))
    fi
done

exit $FAILED
```

### Level 2: Build System Tests (Docker)

```bash
# test-build-system.sh
#!/bin/bash

# Test ashell_package.sh functions
source ashell_packages/ashell_package.sh

# Test: Lazy SDK loading
[[ -z "$ASHELL_SDK_PATH" ]] && echo "SDK not loaded (correct)"

# Test: Function exports
type ashell_step_extract_package >/dev/null || exit 1

# Test: Download and verify (mock)
# ...
```

### Level 3: Package Build Tests (macOS)

```bash
# build-hello.sh
#!/bin/bash
set -e

cd ashell-packages
./build.sh hello

# Verify output
[[ -d ".build/hello/hello.xcframework" ]] || exit 1
plutil -lint .build/hello/hello.framework/commands.plist
```

### Level 4: Integration Tests (iOS)

```swift
// CommandTest.swift
import XCTest

class CommandTests: XCTestCase {
    func testLs() {
        let runner = CommandRunner()
        let (output, exitCode) = runner.run("ls -la")
        XCTAssertEqual(exitCode, 0)
        XCTAssertTrue(output.contains("total"))
    }

    func testPipeline() {
        let runner = CommandRunner()
        let (output, _) = runner.run("echo 'hello world' | wc -w")
        XCTAssertEqual(output.trimmingCharacters(in: .whitespaces), "2")
    }
}
```

## CI/CD Integration

### GitHub Actions Workflow

```yaml
# Test gates in order
jobs:
  syntax-gate:      # Fast, always runs
    runs-on: ubuntu-latest
    steps:
      - run: ./scripts/syntax-check.sh

  contract-gate:    # Runs if syntax passes
    needs: syntax-gate
    runs-on: ubuntu-latest
    steps:
      - run: ./scripts/test-contract.sh

  build-gate:       # Expensive, only on PRs
    needs: contract-gate
    if: github.event_name == 'pull_request'
    runs-on: macos-latest
    steps:
      - run: ./scripts/test-build.sh

  integration-gate: # Most expensive, on demand
    needs: build-gate
    runs-on: macos-latest
    steps:
      - run: ./scripts/test-integration.sh
```

## Coverage Targets (Secondary)

These are **not** primary goals but tracked:

| Layer | Target | Rationale |
|-------|--------|-----------|
| Build system | 80% | Pure bash, easy to test |
| Package recipes | 60% | Boilerplate heavy |
| ios_system | 40% | Legacy C, hard to mock |
| Commands | 50% | Varies by complexity |

## Failure Handling

### Gate Failure Response

| Gate | Fails Means | Response |
|------|-------------|----------|
| Syntax | Broken build | Block PR, fix immediately |
| Contract | API drift | Block PR, update docs |
| Build | Package broken | Block PR, debug locally |
| XCFramework | Invalid output | Block release, investigate |
| Integration | Runtime error | Block PR, device testing |
| Session | State corruption | Priority bug, immediate fix |

## Manual Testing

Some tests require human judgment:

| Test | Why Manual | Frequency |
|------|------------|-----------|
| UI feel | Responsiveness | Each release |
| Real device | Hardware differences | Each release |
| App Store review | Policy changes | Each submission |
| Performance | Real-world conditions | Each milestone |

## Test Data

### Mock Packages

- `hello` - Minimal working package
- `broken` - Intentionally broken for testing
- `stress` - Large package for performance

### Test Fixtures

```
tests/fixtures/
├── archives/
│   ├── hello-1.0.0.tar.gz
│   └── corrupt.tar.gz
├── patches/
│   ├── valid.patch
│   └── invalid.patch
└── plists/
    ├── valid.plist
    └── invalid.plist
```

## Metrics Dashboard

Track these over time:

- Gate pass rate (%)
- Flaky test count
- Mean time to detect (MTTD)
- Test suite duration
- Device test coverage

## Evolution

This strategy evolves:

1. **Start simple** - Syntax + build gates only
2. **Add integration** - When core is stable
3. **Automate device tests** - When CI supports it
4. **Measure behavior** - Real user metrics

## References

- "Testing in Production" - Netflix Tech Blog
- "Behavior-Driven Development" - Dan North
- "The Practical Test Pyramid" - Martin Fowler
- Termux testing approach

---

**Remember**: Tests exist to catch bugs that matter to users. Every test must justify its cost in time and maintenance.

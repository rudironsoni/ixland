# M0 Tasks

## M0-I1: Create ios_system contract document

**Priority:** P1
**Parent:** M0: Contract & Documentation Foundation

Create comprehensive ios_system contract document that defines the platform API, behaviors, and guarantees.

Success Criteria: Document all public APIs with examples, define thread-local I/O guarantees, document session management, specify command registration, define miniroot/sandbox behavior, document environment semantics.

Files: docs/api/ios_system_contract.md, ios_system/ios_system.h

---

## M0-I2: Write porting guide for package authors

**Priority:** P1
**Parent:** M0: Contract & Documentation Foundation

Create step-by-step porting guide enabling developers to port Unix commands to a-Shell.

Success Criteria: Step-by-step workflow, common porting patterns, troubleshooting section, 2 complete walkthrough examples, template build.sh.

Files: docs/guides/porting_guide.md, ashell-packages/TEMPLATE/build.sh

---

## M0-I3: Define package manifest schema

**Priority:** P1
**Parent:** M0: Contract & Documentation Foundation

Define formal schema for package manifests (build.sh variables) and metadata format.

Success Criteria: Schema defines all ASHELL_PKG_* variables, JSON Schema for validation, validation rules, versioning strategy.

Files: docs/specs/package-manifest-schema.md, ashell-packages/schemas/manifest.json

---

## M0-I4: Establish test strategy and behavior gates

**Priority:** P1
**Parent:** M0: Contract & Documentation Foundation

Define testing strategy with behavior-based gates for each phase.

Success Criteria: Testing philosophy documented, behavior gates for M0-M6, test infrastructure plan, CI/CD test requirements.

Files: docs/testing/test-strategy.md

---

# M1 Tasks

## M1-I1: Add structured trace hooks

**Priority:** P2
**Parent:** M1: Platform Hardening

Add structured trace hooks throughout ios_system for debugging.

Success Criteria: Trace points at major functions, configurable levels, low overhead when disabled.

Files: ios_system/trace.h, ios_system/trace.c, ios_system/ios_system.c

---

## M1-I2: Standardize environment/path initialization

**Priority:** P2
**Parent:** M1: Platform Hardening

Standardize ios_system initialization for environment variables and PATH.

Success Criteria: Consistent PREFIX handling, PATH includes PREFIX/bin, no hardcoded paths.

Files: ios_system/environment.c, ios_system/ios_system.h

---

## M1-I3: Add runtime command registration API

**Priority:** P2
**Parent:** M1: Platform Hardening

Add runtime API for registering/unregistering commands dynamically.

Success Criteria: ios_register_command() API, ios_unregister_command() API, thread-safe, documented.

Files: ios_system/ios_system.h, ios_system/command.c

---

## M1-I4: Harden thread-safety

**Priority:** P2
**Parent:** M1: Platform Hardening

Audit and harden thread-safety of session management.

Success Criteria: All shared state protected, no race conditions, thread-local storage validated, stress tests pass.

Files: ios_system/session.c, ios_system/thread_ios.c

---

# M3 Tasks

## M3-I1: Harden Tier A commands

**Priority:** P2
**Parent:** M3: Core Shell Beta

Harden essential commands (ls, cat, cp, mv, grep, etc.).

Success Criteria: All Tier A commands pass conformance tests, consistent error handling, proper ios_system integration, thread-safe output.

Files: ios_system/file_cmds/*, ios_system/text_cmds/*, ios_system/shell_cmds/*

---

## M3-I2: Build conformance test suite

**Priority:** P2
**Parent:** M3: Core Shell Beta

Create conformance test suite for Tier A commands.

Success Criteria: Test cases for each command, edge case coverage, automated execution, clear pass/fail.

Files: ashell-core/Tests/

---

## M3-I3: Implement file-safety testing

**Priority:** P2
**Parent:** M3: Core Shell Beta

Implement tests ensuring file operations don't corrupt data.

Success Criteria: Copy preserves data, move is atomic, remove protects, no data loss.

Files: ashell-core/Tests/FileSafetyTests.swift

---

## M3-I4: Validate session behavior

**Priority:** P2
**Parent:** M3: Core Shell Beta

Validate session management under various conditions.

Success Criteria: Session isolation verified, environment isolation, concurrent handling, session cleanup.

Files: ashell-core/Tests/SessionTests.swift

---

# M4 Tasks

## M4-I1: Add Library APIs for system info

**Priority:** P2
**Parent:** M4: Interactive UX & System Info

Add Library APIs to replace /proc filesystem reads.

Success Criteria: ios_getsys_info(), ios_getproc_info(), memory info, CPU info, process list.

Files: ios_system/ios_system.h, ios_system/sysinfo.c

---

## M4-I2: Integrate LineEditor

**Priority:** P2
**Parent:** M4: Interactive UX & System Info

Integrate LineEditor for interactive shell input.

Success Criteria: LineEditor integrated, history persists, tab completion works, only used interactively.

Files: ashell-app/Package.swift, ashell-app/Sources/Terminal/LineEditor.swift

---

## M4-I3: Implement history and completion

**Priority:** P2
**Parent:** M4: Interactive UX & System Info

Implement command history and tab completion.

Success Criteria: Up/down for history, tab completes commands and filenames, history persisted.

Files: ashell-app/Sources/Terminal/Completion.swift, ashell-app/Sources/Terminal/History.swift

---

# M5 Tasks

## M5-I1: Create Python runtime package

**Priority:** P2
**Parent:** M5: Heavy Packages

Create Python runtime package based on BeeWare Python-Apple-support.

Success Criteria: Installs via apt install python, pip works, XCFramework builds.

Files: ashell-packages/python/build.sh

---

## M5-I2: Create wheel forge

**Priority:** P2
**Parent:** M5: Heavy Packages

Create system for building iOS Python wheels.

Success Criteria: cibuildwheel config, post-processor (.so to framework), wheel index.

Files: ashell-packages/python/scripts/post-process-wheel.sh, .github/workflows/wheels.yml

---

## M5-I3: Create SSH tools package

**Priority:** P2
**Parent:** M5: Heavy Packages

Create SSH tools package with libssh2.

Success Criteria: ssh, scp, sftp commands work, key management.

Files: ashell-packages/libssh2/build.sh, ashell-packages/openssh/build.sh

---

## M5-I4: Create curated Python packages

**Priority:** P2
**Parent:** M5: Heavy Packages

Create packages for common Python libraries.

Success Criteria: numpy, pillow, requests packages available.

Files: ashell-packages/python-numpy/build.sh, ashell-packages/python-pillow/build.sh

---

# M6 Tasks

## M6-I1: Performance benchmarks

**Priority:** P3
**Parent:** M6: Release Candidate

Create and run performance benchmarks.

Success Criteria: Command startup benchmark, package install benchmark, session switching benchmark, memory usage benchmark.

Files: ashell-core/Benchmarks/

---

## M6-I2: Crash rate validation

**Priority:** P3
**Parent:** M6: Release Candidate

Validate crash rate meets targets.

Success Criteria: Crash rate < 0.1%, no known crashers, fuzz testing complete, stress testing complete.

Files: Testing infrastructure

---

## M6-I3: Documentation completion

**Priority:** P3
**Parent:** M6: Release Candidate

Complete all documentation for release.

Success Criteria: API docs complete, user guide complete, porting guide complete, release notes.

Files: docs/

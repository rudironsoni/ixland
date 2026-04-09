# iXland User Testing Documentation

## Overview

iXland is a kernel library project, not a user-facing application with a UI. User testing validation is performed through **unit test execution** rather than browser-based or TUI-based flow validation.

## Testing Approach

Since this is a kernel library (ixland-system, ixland-libc), validation is performed via:

1. **Unit Tests** - Located in `ixland-system/Tests/unit/`
2. **CTest Integration** - CMake test runner
3. **Direct Test Execution** - `ixland-core-tests` binary

## Test Organization

### Test Categories

| Category | Test Files | Assertions Covered |
|----------|------------|-------------------|
| Process Management | `test_fork.c`, `test_wait.c`, `test_task_lifecycle.c`, `test_pid.c` | VAL-SYSCALL-001 to 010, VAL-PROCESS-* |
| File Operations | `test_file_syscalls.c`, `test_fdtable.c`, `test_vfs_path.c` | VAL-SYSCALL-018 to 029 |
| Directory Operations | `test_directory_syscalls.c` | VAL-SYSCALL-030 to 037 |
| Signal Handling | `test_signal.c`, `test_signal_pending.c`, `test_signal_process.c` | VAL-SYSCALL-038 to 045 |
| Identity | `test_identity_syscalls.c` | VAL-SYSCALL-046 to 050 |
| I/O Multiplexing | `test_poll_syscalls.c` | VAL-SYSCALL-051 to 055 |
| Process Groups/Sessions | `test_pgrp_session.c` | VAL-SYSCALL-013 to 017 |
| Exec | `test_exec.c` | VAL-SYSCALL-004 to 006 |

## Running Tests

### Build Tests
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_SYSTEM_NAME=Darwin  # macOS for testing
cmake --build . --target ixland-core-tests
```

### Run All Tests
```bash
cd build/ixland-system
./ixland-core-tests
```

### Run Specific Test Category
```bash
./ixland-core-tests fork      # Run fork-related tests
./ixland-core-tests signal    # Run signal-related tests
./ixland-core-tests file      # Run file-related tests
```

### Via CTest
```bash
cd build
ctest --output-on-failure
```

## Validation Concurrency

**Max Concurrent Validators: 1**

Since this is a unit test-based validation, tests run in a single process and must be executed sequentially. There is no isolation boundary for concurrent execution.

## Flow Validator Guidance: Kernel Library Testing

For flow validators testing this milestone:

1. **No browser/TUI automation needed** - Tests are C unit tests
2. **Test execution is the validation method** - Run `ixland-core-tests`
3. **Test pass/fail maps to assertion pass/fail**
4. **Evidence is test output** - Capture full test output

### Test-to-Assertion Mapping

Each test case validates specific assertions from the validation contract:

- `fork_creates_new_pid` → VAL-SYSCALL-001
- `fork_copies_fd_table` → VAL-SYSCALL-001
- `fork_respects_nproc_limit` → VAL-SYSCALL-002
- `vfork_child_marked_correctly` → VAL-SYSCALL-003
- `execve_validates_pathname` → VAL-SYSCALL-004
- `exit_sets_exit_status` → VAL-SYSCALL-007
- `wait_parent_observes_child_exit` → VAL-SYSCALL-008
- `wnohang_live_child_returns_zero` → VAL-SYSCALL-009
- `pgrp_initial_task_leader_invariants` → VAL-SYSCALL-013 to 017
- And so on...

## Service Configuration

This project has no long-running services. The `services.yaml` uses commands for build operations.

## Known Test Limitations

Some tests may be skipped or fail due to:

1. **VFS stub limitations** - File operations use stub VFS
2. **iOS-specific constraints** - Some syscalls return ENOSYS on iOS
3. **Test environment** - macOS build for testing may differ from iOS target

## Evidence Collection

When running tests, collect:

1. **Test output** - Full pass/fail results
2. **Test count** - Total tests run
3. **Failure details** - Specific test names and failure reasons

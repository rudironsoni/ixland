# a-Shell Next Project Issues

## Epic M0: Contract & Documentation Foundation

### M0-I1: Create ios_system contract document

**Type:** epic
**Priority:** P1
**Labels:** documentation,contract,M0

#### Description

## Goal
Create a comprehensive ios_system contract document that defines the platform API, behaviors, and guarantees that package authors can rely on.

## Success Criteria
- [ ] Document all public APIs in ios_system.h with examples
- [ ] Define thread-local I/O behavior guarantees
- [ ] Document session management contract
- [ ] Specify command registration mechanism
- [ ] Define miniroot/sandbox behavior
- [ ] Document environment variable semantics

## Context
The ios_system library is the foundation of a-Shell. It provides Unix-like syscall replacements for iOS. Without a clear contract, package authors cannot know what behaviors they can rely on. This document becomes the API reference for all package porting work.

## Dependencies
- **Blocked by:** None (foundational)
- **Blocks:** All M1-M6 package work

## Implementation Details
Create `docs/api/ios_system_contract.md` with sections:
1. Architecture overview
2. Public API reference (all functions)
3. Thread-local I/O guarantees
4. Session management
5. Environment semantics
6. Sandbox/miniroot behavior
7. Command registration
8. Porting examples

Reference existing work in `docs/api/ios_system_contract.md` (partially complete)

## Files to Modify
- `docs/api/ios_system_contract.md` - Create/extend
- `ios_system/ios_system.h` - Add doc comments if missing

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Use markdown format | Easy to read in GitHub, version controlled |
| Include code examples | Shows actual usage patterns |
| Document edge cases | Prevents confusion about iOS limitations |

## Do's and Don'ts
### Do
- Document every public function
- Include return value semantics
- Show error handling patterns
- Note iOS-specific limitations

### Don't
- Document internal/private APIs
- Make promises about future behavior
- Omit error conditions
- Assume POSIX compliance where iOS differs

## When to Stop
- All public APIs documented
- At least 3 complete porting examples
- Reviewed by someone familiar with ios_system

## Verification Steps
1. Read docs/api/ios_system_contract.md - should be comprehensive
2. Check all functions from ios_system.h are documented
3. Verify code examples compile
4. Test with a new package author - can they port a command?

## References
- docs/plans/20260318-a-shell-next.md - Phase 2 section
- ios_system/ios_system.h - Source of truth for APIs

---

### M0-I2: Write porting guide for package authors

**Type:** task
**Priority:** P1
**Labels:** documentation,guide,M0

#### Description

## Goal
Create a step-by-step porting guide that enables developers to port Unix commands to a-Shell without prior iOS knowledge.

## Success Criteria
- [ ] Step-by-step workflow from source to XCFramework
- [ ] Common porting patterns documented
- [ ] Troubleshooting section for common errors
- [ ] At least 2 complete walkthrough examples
- [ ] Template build.sh for new packages

## Context
Package authors need practical guidance beyond API documentation. This guide bridges the gap between "I have a Unix command" and "It works in a-Shell."

## Dependencies
- **Blocked by:** M0-I1 (ios_system contract)
- **Blocks:** M2 package creation work

## Implementation Details
Create `docs/guides/porting_guide.md`:
1. Prerequisites (macOS, Xcode, etc.)
2. Getting the source
3. Identifying iOS incompatibilities
4. Creating patches
5. Writing build.sh
6. Testing locally
7. Submitting package

## Files to Modify
- `docs/guides/porting_guide.md` - Create
- `ashell-packages/TEMPLATE/build.sh` - Create template

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Include patch examples | Authors learn by example |
| Template build.sh | Reduces boilerplate errors |

## Do's and Don'ts
### Do
- Assume reader knows Unix, not iOS
- Show actual patch files
- Explain WHY not just HOW
- Include error messages and fixes

### Don't
- Assume iOS development knowledge
- Skip "obvious" steps
- Use a-Shell jargon without explanation

## When to Stop
- Guide is complete enough for external contributor to port a package
- Tested by someone new to the project

## Verification Steps
1. Give guide to developer unfamiliar with project
2. They should successfully port a simple package
3. Document any confusion points

---

### M0-I3: Define package manifest schema

**Type:** task
**Priority:** P1
**Labels:** schema,specification,M0

#### Description

## Goal
Define the formal schema for package manifests (build.sh variables) and metadata format.

## Success Criteria
- [ ] Schema document defines all ASHELL_PKG_* variables
- [ ] JSON Schema for package metadata
- [ ] Validation rules documented
- [ ] Versioning strategy defined

## Context
Package manifests are bash scripts with ASHELL_PKG_* variables. We need a schema to validate these and enable tooling.

## Dependencies
- **Blocked by:** None
- **Blocks:** M2-I1 (build system library needs schema)

## Implementation Details
Create `docs/specs/package-manifest-schema.md`:
- Required vs optional variables
- Type constraints (string, array, etc.)
- Validation rules
- Version compatibility

## Files to Modify
- `docs/specs/package-manifest-schema.md` - Create
- `ashell-packages/schemas/manifest.json` - JSON Schema

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Bash variables not JSON | Matches Termux, easier for Unix devs |
| JSON Schema for tooling | Enables validation, IDE support |

## Do's and Don'ts
### Do
- Keep variable names consistent
- Document default values
- Version the schema

### Don't
- Break backward compatibility without versioning
- Make required fields that could be inferred

## When to Stop
- Schema validates all existing packages
- Can generate from schema

## Verification Steps
1. Schema validates hello/build.sh
2. Schema validates coreutils-minimal/build.sh
3. Invalid manifests are rejected

---

### M0-I4: Establish test strategy and behavior gates

**Type:** task
**Priority:** P1
**Labels:** testing,strategy,M0

#### Description

## Goal
Define the testing strategy with behavior-based gates for each phase.

## Success Criteria
- [ ] Testing philosophy documented
- [ ] Behavior gates defined for M0-M6
- [ ] Test infrastructure plan created
- [ ] CI/CD test requirements specified

## Context
Quality is critical. We need clear gates to know when each phase is "done."

## Dependencies
- **Blocked by:** None
- **Blocks:** All implementation phases

## Implementation Details
Create `docs/testing/test-strategy.md`:
- Behavior-based vs coverage-based
- Gates per milestone
- Test types (unit, integration, conformance)
- CI requirements

## Files to Modify
- `docs/testing/test-strategy.md` - Create

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Behavior gates over coverage | Quality over quantity |
| Per-phase gates | Natural checkpoints |

## Do's and Don'ts
### Do
- Make gates measurable
- Include negative test cases
- Test error paths

### Don't
- Rely solely on code coverage
- Skip integration testing

## When to Stop
- All phases have defined gates
- Gates are measurable

## Verification Steps
1. Review gates with team
2. Verify gates can be automated

---

## Epic M1: Platform Hardening

### M1-I1: Add structured trace hooks to ios_system

**Type:** task
**Priority:** P2
**Labels:** ios_system,tracing,M1

#### Description

## Goal
Add structured trace hooks throughout ios_system for debugging and observability.

## Success Criteria
- [ ] Trace points at entry/exit of major functions
- [ ] Configurable trace levels
- [ ] Low overhead when disabled
- [ ] Documentation for trace format

## Context
Debugging ios_system issues is difficult without visibility into internal operations. Trace hooks enable systematic debugging.

## Dependencies
- **Blocked by:** M0-I1 (contract defines trace boundaries)
- **Blocks:** M3 debugging work

## Implementation Details
Add to `ios_system/`:
- Trace macros (ASHELL_TRACE_ENTER, ASHELL_TRACE_EXIT)
- Configurable via environment variable
- Output to stderr or file

## Files to Modify
- `ios_system/trace.h` - Create
- `ios_system/trace.c` - Create
- `ios_system/ios_system.c` - Add trace points

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Macros not functions | Zero overhead when disabled |
| Environment config | No recompilation needed |

## Do's and Don'ts
### Do
- Use macros for zero-cost
- Include timestamps
- Allow filtering by subsystem

### Don't
- Add traces in hot paths without benchmarking
- Use printf directly

## When to Stop
- All major functions have entry/exit traces
- Performance impact < 1% when disabled

## Verification Steps
1. Build with tracing enabled
2. Run commands, verify trace output
3. Benchmark with/without tracing

---

### M1-I2: Standardize environment/path initialization

**Type:** task
**Priority:** P2
**Labels:** ios_system,environment,M1

#### Description

## Goal
Standardize how ios_system initializes environment variables and PATH.

## Success Criteria
- [ ] Consistent PREFIX handling
- [ ] PATH includes PREFIX/bin automatically
- [ ] Environment variables documented
- [ ] No hardcoded paths in code

## Context
Different parts of ios_system handle paths inconsistently. This causes confusion and bugs.

## Dependencies
- **Blocked by:** M0-I1
- **Blocks:** M2 package installation

## Implementation Details
- Centralize path initialization
- Use ASHELL_PREFIX consistently
- Document all environment variables

## Files to Modify
- `ios_system/environment.c` - Create/refactor
- `ios_system/ios_system.h` - Add env docs

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Centralized init | Single source of truth |
| PREFIX override support | Testing flexibility |

## Do's and Don'ts
### Do
- Fail gracefully if PREFIX undefined
- Log initialization at trace level
- Support environment overrides

### Don't
- Hardcode paths
- Assume specific directory structure

## When to Stop
- All path references use PREFIX
- Environment documented
- Tests verify initialization

## Verification Steps
1. Check no hardcoded /usr or /bin paths
2. Test with custom PREFIX
3. Verify PATH includes PREFIX/bin

---

### M1-I3: Add runtime command registration API

**Type:** task
**Priority:** P2
**Labels:** ios_system,api,M1

#### Description

## Goal
Add runtime API for registering and unregistering commands dynamically.

## Success Criteria
- [ ] `ios_register_command()` API
- [ ] `ios_unregister_command()` API
- [ ] Thread-safe implementation
- [ ] Documentation with examples

## Context
Currently commands are registered via plist files. Runtime registration enables dynamic package loading.

## Dependencies
- **Blocked by:** M0-I1
- **Blocks:** M2 package manager integration

## Implementation Details
Add to `ios_system/ios_system.h`:
```c
int ios_register_command(const char* name, command_func_t func);
int ios_unregister_command(const char* name);
```

## Files to Modify
- `ios_system/ios_system.h` - Add API
- `ios_system/command.c` - Implement

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Runtime registration | Enables dynamic packages |
| Thread-safe | Multiple sessions may register |

## Do's and Don'ts
### Do
- Make thread-safe
- Validate inputs
- Return error codes

### Don't
- Allow duplicate registrations
- Leak memory on unregister

## When to Stop
- API implemented and tested
- Works with existing command dict

## Verification Steps
1. Register test command at runtime
2. Verify it executes
3. Unregister, verify removal

---

### M1-I4: Harden thread-safety of session management

**Type:** task
**Priority:** P2
**Labels:** ios_system,threading,M1

#### Description

## Goal
Audit and harden thread-safety of session management code.

## Success Criteria
- [ ] All shared state protected
- [ ] No race conditions in session switching
- [ ] Thread-local storage validated
- [ ] Stress tests pass

## Context
ios_system uses thread-local storage for sessions. Race conditions could cause output to wrong terminal.

## Dependencies
- **Blocked by:** M0-I1
- **Blocks:** M3 multi-window stability

## Implementation Details
- Audit all shared mutable state
- Add locks where needed
- Validate thread-local usage

## Files to Modify
- `ios_system/session.c` - Review/rewrite
- `ios_system/thread_ios.c` - Review

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Auditing first | Find issues before fixing |
| Stress testing | Race conditions are timing-dependent |

## Do's and Don'ts
### Do
- Use atomic operations where possible
- Minimize lock scope
- Document invariants

### Don't
- Hold locks during I/O
- Assume single-threaded

## When to Stop
- No warnings from thread sanitizer
- Stress tests pass
- Code review complete

## Verification Steps
1. Run with ThreadSanitizer
2. Multi-window stress test
3. Rapid session switching test

---

## Epic M2: Forge Alpha (HIGHEST PRIORITY)

### M2-I1: Create ashell_package.sh build system library

**Type:** task
**Priority:** P0
**Labels:** build-system,core,M2

#### Description

## Goal
Create the core build system library (ashell_package.sh) with all ashell_step_* functions.

## Success Criteria
- [ ] All ashell_step_* functions implemented
- [ ] Lazy SDK loading for cross-platform builds
- [ ] SHA256 verification
- [ ] Patch application in order
- [ ] XCFramework generation
- [ ] Comprehensive error handling

## Context
This is the foundation of the package build system. Every package will use this library.

## Dependencies
- **Blocked by:** M0-I3 (manifest schema)
- **Blocks:** All other M2 issues

## Implementation Details
Create `ashell-packages/ashell_package.sh` with:
- Configuration (ASHELL_PREFIX, etc.)
- ashell_step_extract_package()
- ashell_step_patch_package()
- ashell_step_configure()
- ashell_step_make()
- ashell_step_make_install()
- ashell_step_create_xcframework()
- ashell_step_generate_plist()
- ashell_step_codesign()
- Utility functions (download, verify, etc.)

## Files to Modify
- `ashell-packages/ashell_package.sh` - Create

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Bash not Make | Matches Termux, easier for Unix devs |
| Lazy SDK loading | Supports Linux CI without macOS |
| Step functions | Overrideable per-package |

## Do's and Don'ts
### Do
- Validate all inputs
- Provide clear error messages
- Support verbose mode
- Clean up on failure

### Don't
- Assume macOS/Xcode always present
- Hardcode paths
- Skip error checking

## When to Stop
- All step functions implemented
- hello package builds successfully
- Linux CI works (SDK loading lazy)

## Verification Steps
1. bash -n ashell_package.sh (syntax check)
2. Source from test script
3. Build hello package end-to-end
4. Test on Linux (should list packages without error)

---

### M2-I2: Create build.sh main orchestration script

**Type:** task
**Priority:** P0
**Labels:** build-system,cli,M2

#### Description

## Goal
Create the main build.sh orchestration script with build/clean/install/package/list commands.

## Success Criteria
- [ ] build command works
- [ ] clean command removes artifacts
- [ ] install command (for testing)
- [ ] package command creates distributable
- [ ] list command shows packages
- [ ] Help/usage display

## Context
This is the CLI developers use to interact with the build system.

## Dependencies
- **Blocked by:** M2-I1
- **Blocks:** None (parallel to other M2 work)

## Implementation Details
Create `ashell-packages/build.sh`:
- Parse command line arguments
- Dispatch to package build.sh
- Handle global options

## Files to Modify
- `ashell-packages/build.sh` - Create

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Single entry point | Consistent UX |
| Per-package build.sh | Flexibility for complex packages |

## Do's and Don'ts
### Do
- Validate arguments
- Show progress
- Return proper exit codes

### Don't
- Hardcode package list
- Require unnecessary options

## When to Stop
- All commands work
- Tab completion friendly
- Well documented

## Verification Steps
1. ./build.sh list (shows packages)
2. ./build.sh hello (builds hello)
3. ./build.sh hello clean (cleans)
4. ./build.sh --help (shows usage)

---

### M2-I3: Create hello reference package

**Type:** task
**Priority:** P0
**Labels:** package,reference,M2

#### Description

## Goal
Create the hello reference package demonstrating the full build pattern.

## Success Criteria
- [ ] Minimal C program
- [ ] Complete build.sh
- [ ] Uses ios_system.h
- [ ] Builds to XCFramework
- [ ] Installs via apt

## Context
This is the canonical example all future packages will reference.

## Dependencies
- **Blocked by:** M2-I1, M2-I2
- **Blocks:** None (reference implementation)

## Implementation Details
Create `ashell-packages/hello/`:
- build.sh with all standard variables
- Minimal hello.c using ios_stdout()
- README.md explaining the pattern

## Files to Modify
- `ashell-packages/hello/build.sh` - Create
- `ashell-packages/hello/README.md` - Create

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Simple C | Demonstrates minimum viable package |
| Well commented | Serves as documentation |

## Do's and Don'ts
### Do
- Use ios_system APIs
- Document every section
- Keep minimal but complete

### Don't
- Add unnecessary complexity
- Skip error handling

## When to Stop
- Builds successfully
- Can be installed/uninstalled
- Serves as template

## Verification Steps
1. ./build.sh hello
2. Check .build/hello/ contains XCFramework
3. Manually install and test

---

### M2-I4: Add iOS cross-compilation toolchain

**Type:** task
**Priority:** P0
**Labels:** toolchain,ios,M2

#### Description

## Goal
Create iOS cross-compilation toolchain configuration.

## Success Criteria
- [ ] CC/CXX configured for iOS targets
- [ ] Supports arm64 and x86_64 (simulator)
- [ ] cmake toolchain file
- [ ] autotools configuration

## Context
Building for iOS requires specific compiler flags and SDK paths.

## Dependencies
- **Blocked by:** M2-I1
- **Blocks:** Complex package builds

## Implementation Details
Create:
- `ashell-packages/config/ios-toolchain.cmake`
- `ashell-packages/config/ios-configure.sh`
- Integration in ashell_package.sh

## Files to Modify
- `ashell-packages/config/` - Create directory with toolchain files

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Lazy SDK loading | Allows listing on non-macOS |
| Support simulator | Testing without device |

## Do's and Don'ts
### Do
- Cache SDK path lookup
- Support both device and simulator
- Document requirements

### Don't
- Require Xcode for all operations
- Hardcode SDK versions

## When to Stop
- Builds work on macOS with Xcode
- List works on Linux
- Both archs supported

## Verification Steps
1. Build hello for arm64
2. Build hello for x86_64 simulator
3. Verify on Linux: ./build.sh list works

---

### M2-I5: Implement XCFramework generation

**Type:** task
**Priority:** P0
**Labels:** build-system,xcframework,M2

#### Description

## Goal
Implement XCFramework creation from build artifacts.

## Success Criteria
- [ ] Combines device and simulator builds
- [ ] Includes headers
- [ ] Includes Info.plist
- [ ] Code signing optional but supported

## Context
XCFramework is Apple's format for binary frameworks supporting multiple platforms.

## Dependencies
- **Blocked by:** M2-I4
- **Blocks:** Package distribution

## Implementation Details
Add to ashell_package.sh:
- ashell_step_create_xcframework()
- Use xcodebuild -create-xcframework
- Combine archs

## Files to Modify
- `ashell-packages/ashell_package.sh` - Add XCFramework step

## Decision Records
| Decision | Rationale |
|----------|-----------|
| XCFramework not .a | Apple's modern format |
| Multi-arch | Single artifact for all platforms |

## Do's and Don'ts
### Do
- Include all architectures
- Sign if identity available
- Validate output

### Don't
- Ship unsigned for distribution
- Forget headers

## When to Stop
- Creates valid XCFramework
- Loads in a-Shell
- All archs present

## Verification Steps
1. Build hello
2. Check XCFramework contents
3. Verify with lipo -info
4. Test loading in a-Shell

---

### M2-I6: Replace pkg extraction with libarchive

**Type:** task
**Priority:** P0
**Labels:** extraction,libarchive,M2

#### Description

## Goal
Replace URLSession-based extraction with libarchive integration.

## Success Criteria
- [ ] ArchiveExtractor.swift uses libarchive
- [ ] Supports .tar.gz, .tar.bz2, .tar.xz, .zip
- [ ] Progress callbacks
- [ ] Error handling

## Context
libarchive is already in ios_system. Using it provides better format support.

## Dependencies
- **Blocked by:** M2-I1
- **Blocks:** apt install functionality

## Implementation Details
Modify `ashell-core/Sources/PackageManager/ArchiveExtractor.swift`:
- Bridge to libarchive C API
- Support multiple formats
- Progress reporting

## Files to Modify
- `ashell-core/Sources/PackageManager/ArchiveExtractor.swift`

## Decision Records
| Decision | Rationale |
|----------|-----------|
| libarchive not tar command | Consistent, better format support |
| Swift wrapper | Type safety, async/await |

## Do's and Don'ts
### Do
- Handle all archive formats
- Report progress
- Clean up partial extractions

### Don't
- Shell out to tar
- Ignore errors

## When to Stop
- All test archives extract
- Progress reported
- Errors handled

## Verification Steps
1. Test with .tar.gz
2. Test with .zip
3. Test with .tar.bz2
4. Verify progress callbacks

---

### M2-I7: Add plist generation from build metadata

**Type:** task
**Priority:** P0
**Labels:** build-system,plist,M2

#### Description

## Goal
Generate commands.plist from ASHELL_PKG_COMMANDS metadata.

## Success Criteria
- [ ] Parses ASHELL_PKG_COMMANDS array
- [ ] Generates valid plist XML
- [ ] Includes framework path
- [ ] Includes entry point
- [ ] Includes auth string

## Context
Commands.plist tells ios_system how to find and run commands.

## Dependencies
- **Blocked by:** M2-I1
- **Blocks:** Command registration

## Implementation Details
Add to ashell_package.sh:
- ashell_step_generate_plist()
- Generate XML from ASHELL_PKG_COMMANDS
- Place in staging dir

## Files to Modify
- `ashell-packages/ashell_package.sh` - Add plist generation

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Generate not hand-write | Consistent, less error-prone |
| XML format | Matches ios_system expectation |

## Do's and Don'ts
### Do
- Validate generated plist
- Escape special characters
- Include all command metadata

### Don't
- Hand-edit generated files
- Skip validation

## When to Stop
- Generated plist loads in ios_system
- All commands registered
- Valid XML

## Verification Steps
1. Build hello
2. Check generated plist
3. Load in ios_system
4. Verify command runs

---

### M2-I8: Create CI pipeline for automated builds

**Type:** task
**Priority:** P0
**Labels:** ci,automation,M2

#### Description

## Goal
Create GitHub Actions CI pipeline for automated package builds.

## Success Criteria
- [ ] Builds on macOS runner
- [ ] Matrix build for packages
- [ ] Artifact upload
- [ ] Status badges

## Context
CI ensures packages build consistently and provides artifacts for distribution.

## Dependencies
- **Blocked by:** All other M2 issues
- **Blocks:** None (final M2 task)

## Implementation Details
Create `.github/workflows/packages.yml`:
- macOS runner
- Build matrix
- Artifact upload
- Status reporting

## Files to Modify
- `.github/workflows/packages.yml` - Create

## Decision Records
| Decision | Rationale |
|----------|-----------|
| GitHub Actions | Native integration |
| Matrix builds | Parallel package builds |

## Do's and Don'ts
### Do
- Cache dependencies
- Parallel builds
- Clear error reporting

### Don't
- Build all packages sequentially
- Skip error details

## When to Stop
- CI passes for all packages
- Artifacts available
- Badges green

## Verification Steps
1. Push to trigger CI
2. Verify all packages build
3. Download and verify artifacts

---

## Epic M3: Core Shell Beta

### M3-I1: Harden Tier A commands

**Type:** task
**Priority:** P2
**Labels:** commands,hardening,M3

#### Description

## Goal
Harden essential Tier A commands (ls, cat, cp, mv, rm, grep, etc.) for consistent behavior.

## Success Criteria
- [ ] All Tier A commands pass conformance tests
- [ ] Consistent error handling
- [ ] Proper ios_system integration
- [ ] Thread-safe output

## Context
These commands are used daily and must be rock solid.

## Dependencies
- **Blocked by:** M1 work
- **Blocks:** None (can parallelize)

## Implementation Details
Audit and harden commands in ios_system:
- file_cmds/ (cp, mv, rm)
- text_cmds/ (cat, grep, sed)
- shell_cmds/ (ls, pwd)

## Files to Modify
- Various in ios_system/

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Audit then fix | Systematic approach |
| Conformance tests | Verify behavior matches expectations |

## Do's and Don'ts
### Do
- Use ios_stdout() not stdout
- Check return values
- Handle edge cases

### Don't
- Assume POSIX behavior
- Use global state

## When to Stop
- All Tier A commands pass tests
- No known crashers

## Verification Steps
1. Run conformance suite
2. Fuzz test
3. Multi-window stress test

---

### M3-I2: Build conformance test suite

**Type:** task
**Priority:** P2
**Labels:** testing,conformance,M3

#### Description

## Goal
Create conformance test suite for Tier A commands.

## Success Criteria
- [ ] Test cases for each Tier A command
- [ ] Edge case coverage
- [ ] Automated execution
- [ ] Clear pass/fail reporting

## Context
Tests ensure commands behave consistently and don't regress.

## Dependencies
- **Blocked by:** M0-I4
- **Blocks:** M3-I1

## Implementation Details
Create test suite:
- Swift Testing framework
- Command invocations
- Output verification
- Exit code checks

## Files to Modify
- `ashell-core/Tests/` - Create test files

## Decision Records
| Decision | Rationale |
|----------|-----------|
| Swift Testing | Modern, async/await support |
| Snapshot testing | Verify output format |

## Do's and Don'ts
### Do
- Test edge cases
- Include error conditions
- Make deterministic

### Don't
- Skip flaky tests
- Depend on environment

## When to Stop
- All Tier A covered
- CI runs tests

## Verification Steps
1. Run test suite
2. Verify coverage report
3. Check CI integration

---

### M3-I3: Implement file-safety testing

**Type:** task
**Priority:** P2
**Labels:** testing,safety,M3

#### Description

## Goal
Implement tests ensuring file operations don't corrupt or lose data.

## Success Criteria
- [ ] Copy preserves data
- [ ] Move is atomic
- [ ] Remove confirms or protects
- [ ] No data loss scenarios

## Context
File operations must be trustworthy.

## Dependencies
- **Blocked by:** M3-I2
- **Blocks:** None

## Implementation Details
Create safety tests:
- Checksum verification
- Atomic operation tests
- Permission preservation

## Files to Modify
- `ashell-core/Tests/FileSafetyTests.swift`

## Do's and Don'ts
### Do
- Test with large files
- Test edge cases (full disk, etc.)
- Verify permissions

### Don't
- Skip error conditions
- Assume success

## When to Stop
- All file ops tested
- No data loss in tests

## Verification Steps
1. Run file safety tests
2. Verify checksums match
3. Check permission preservation

---

### M3-I4: Validate session behavior

**Type:** task
**Priority:** P2
**Labels:** testing,session,M3

#### Description

## Goal
Validate session management behavior under various conditions.

## Success Criteria
- [ ] Session isolation verified
- [ ] Environment isolation
- [ ] Concurrent session handling
- [ ] Session cleanup

## Context
Sessions must be isolated to prevent cross-contamination.

## Dependencies
- **Blocked by:** M1-I4
- **Blocks:** None

## Implementation Details
Create session tests:
- Multi-window scenarios
- Environment variable isolation
- Concurrent access

## Files to Modify
- `ashell-core/Tests/SessionTests.swift`

## Do's and Don'ts
### Do
- Test concurrent sessions
- Verify cleanup
- Check resource limits

### Don't
- Skip cleanup tests
- Ignore race conditions

## When to Stop
- All session tests pass
- Multi-window stable

## Verification Steps
1. Run session tests
2. Multi-window stress test
3. Verify no cross-session leaks

---

## Epic M4: Interactive UX & System Info

### M4-I1: Add Library APIs for system info

**Type:** task
**Priority:** P2
**Labels:** api,system-info,M4

#### Description

## Goal
Add Library APIs to replace /proc filesystem reads.

## Success Criteria
- [ ] ios_getsys_info() API
- [ ] ios_getproc_info() API
- [ ] Memory info available
- [ ] CPU info available
- [ ] Process list available

## Context
/proc doesn't exist on iOS. We need APIs to provide this info.

## Dependencies
- **Blocked by:** M0-I1
- **Blocks:** M4-I2, M3 commands

## Implementation Details
Add to ios_system:
```c
ios_sys_info_t ios_getsys_info(void);
ios_proc_info_t* ios_getproc_info(pid_t pid);
```

## Files to Modify
- `ios_system/ios_system.h`
- `ios_system/sysinfo.c` - Create

## Decision Records
| Decision | Rationale |
|----------|-----------|
| API not filesystem | iOS doesn't support /proc |
| Structured data | Easier to consume than parsing |

## Do's and Don'ts
### Do
- Use iOS APIs (NSProcessInfo, etc.)
- Cache where appropriate
- Document units

### Don't
- Fake data
- Block on slow operations

## When to Stop
- APIs implemented
- Commands (ps, top) can use them

## Verification Steps
1. Call APIs from test
2. Verify data accuracy
3. Check commands use them

---

### M4-I2: Integrate LineEditor for interactive input

**Type:** task
**Priority:** P2
**Labels:** ux,interactive,M4

#### Description

## Goal
Integrate LineEditor for interactive shell input with history and completion.

## Success Criteria
- [ ] LineEditor integrated
- [ ] History persists
- [ ] Tab completion works
- [ ] Only used for interactive sessions

## Context
Current input is basic. LineEditor provides proper line editing.

## Dependencies
- **Blocked by:** M4-I1
- **Blocks:** M4-I3

## Implementation Details
- Add LineEditor dependency
- Hook into interactive commands
- Detect interactive vs piped

## Files to Modify
- `ashell-app/Package.swift` - Add dependency
- `ashell-app/Sources/Terminal/LineEditor.swift` - Create

## Decision Records
| Decision | Rationale |
|----------|-----------|
| LineEditor | Established library, libedit wrapper |
| Interactive detection | Don't use for scripts |

## Do's and Don'ts
### Do
- Detect interactive mode
- Save history
- Support completion

### Don't
- Use in non-interactive mode
- Block signals

## When to Stop
- Interactive commands use it
- History works
- Completion works

## Verification Steps
1. Test interactive shell
2. Verify history persists
3. Test tab completion
4. Verify pipes skip LineEditor

---

### M4-I3: Implement history and completion

**Type:** task
**Priority:** P2
**Labels:** ux,interactive,M4

#### Description

## Goal
Implement command history and tab completion using LineEditor.

## Success Criteria
- [ ] Up/down arrow for history
- [ ] Tab completes commands
- [ ] Tab completes filenames
- [ ] History persisted to file

## Context
Essential shell features for usability.

## Dependencies
- **Blocked by:** M4-I2
- **Blocks:** None

## Implementation Details
- History file in ASHELL_CONFIG
- Completion hooks for commands
- Filename completion

## Files to Modify
- `ashell-app/Sources/Terminal/Completion.swift`
- `ashell-app/Sources/Terminal/History.swift`

## Do's and Don'ts
### Do
- Persist history
- Handle spaces in filenames
- Limit history size

### Don't
- Leak memory
- Corrupt history file

## When to Stop
- All features work
- Performance acceptable

## Verification Steps
1. Test history navigation
2. Test command completion
3. Test filename completion
4. Verify persistence

---

## Epic M5: Heavy Packages

### M5-I1: Create Python runtime package

**Type:** task
**Priority:** P2
**Labels:** runtime,python,M5

#### Description

## Goal
Create Python runtime package based on BeeWare Python-Apple-support.

## Success Criteria
- [ ] Downloads/uses BeeWare Python
- [ ] Builds as XCFramework
- [ ] Installs via apt
- [ ] pip works

## Context
Python is a heavyweight package but essential.

## Dependencies
- **Blocked by:** M2 completion
- **Blocks:** M5-I4

## Implementation Details
Create `ashell-packages/python/`:
- Uses BeeWare Python-Apple-support
- build.sh downloads pre-built or builds
- Registers python, pip commands

## Files to Modify
- `ashell-packages/python/build.sh`

## Decision Records
| Decision | Rationale |
|----------|-----------|
| BeeWare not custom | Official PEP 730 support |
| Optional install | Not all users need Python |

## Do's and Don'ts
### Do
- Use official Python
- Support pip
- Handle C extensions

### Don't
- Fork Python
- Break pip

## When to Stop
- python runs
- pip install works
- Basic packages install

## Verification Steps
1. apt install python
2. python --version
3. pip install requests

---

### M5-I2: Create wheel forge and post-processor

**Type:** task
**Priority:** P2
**Labels:** python,wheel,M5

#### Description

## Goal
Create system for building and processing iOS Python wheels.

## Success Criteria
- [ ] cibuildwheel configuration
- [ ] Wheel post-processor (.so → framework)
- [ ] Wheel index for distribution

## Context
Python packages need wheels built for iOS.

## Dependencies
- **Blocked by:** M5-I1
- **Blocks:** M5-I4

## Implementation Details
- CI for wheel building
- Post-process script
- Index generation

## Files to Modify
- `ashell-packages/python/scripts/post-process-wheel.sh`
- `.github/workflows/wheels.yml`

## Do's and Don'ts
### Do
- Support pure Python wheels
- Convert extensions to frameworks
- Version compatibility check

### Don't
- Skip extension conversion
- Ignore ABI tags

## When to Stop
- Wheels build
- Post-processor works
- Index serves wheels

## Verification Steps
1. Build a wheel
2. Post-process
3. Install in a-Shell

---

### M5-I3: Create SSH tools package

**Type:** task
**Priority:** P2
**Labels:** network,ssh,M5

#### Description

## Goal
Create SSH tools package with libssh2.

## Success Criteria
- [ ] ssh command works
- [ ] scp works
- [ ] sftp works
- [ ] Key management

## Context
SSH is essential for remote work.

## Dependencies
- **Blocked by:** M2 completion
- **Blocks:** None

## Implementation Details
Create packages:
- libssh2 dependency
- openssh client

## Files to Modify
- `ashell-packages/libssh2/build.sh`
- `ashell-packages/openssh/build.sh`

## Do's and Don'ts
### Do
- Support key auth
- Handle sandbox restrictions
- Include known_hosts

### Don't
- Skip host key verification
- Ignore iOS network limits

## When to Stop
- ssh user@host works
- scp file user@host: works

## Verification Steps
1. apt install openssh
2. ssh to test server
3. scp test file

---

### M5-I4: Create curated Python packages

**Type:** task
**Priority:** P2
**Labels:** python,packages,M5

#### Description

## Goal
Create packages for common Python libraries (numpy, pillow, etc.).

## Success Criteria
- [ ] numpy package
- [ ] pillow package
- [ ] requests package
- [ ] Documentation

## Context
Users expect common packages to be available.

## Dependencies
- **Blocked by:** M5-I1, M5-I2
- **Blocks:** None

## Implementation Details
Create package recipes for popular libraries.

## Files to Modify
- `ashell-packages/python-numpy/build.sh`
- `ashell-packages/python-pillow/build.sh`

## Do's and Don'ts
### Do
- Test imports
- Document limitations
- Handle dependencies

### Don't
- Skip tests
- Ignore platform tags

## When to Stop
- Packages install
- Imports work
- Basic functionality tested

## Verification Steps
1. apt install python-numpy
2. python -c "import numpy"
3. Run basic operations

---

## Epic M6: Release Candidate

### M6-I1: Performance benchmarks

**Type:** task
**Priority:** P3
**Labels:** performance,benchmarks,M6

#### Description

## Goal
Create and run performance benchmarks.

## Success Criteria
- [ ] Command startup benchmark
- [ ] Package install benchmark
- [ ] Session switching benchmark
- [ ] Memory usage benchmark

## Context
Ensure performance meets targets before release.

## Dependencies
- **Blocked by:** All previous work
- **Blocks:** None

## Implementation Details
Create benchmark suite using swift-benchmark.

## Files to Modify
- `ashell-core/Benchmarks/` - Create

## Do's and Don'ts
### Do
- Baseline comparisons
- Automated runs
- Track trends

### Don't
- Benchmark debug builds
- Ignore variance

## When to Stop
- Benchmarks automated
- Targets documented

## Verification Steps
1. Run benchmarks
2. Compare to targets
3. Document results

---

### M6-I2: Crash rate validation

**Type:** task
**Priority:** P3
**Labels:** stability,validation,M6

#### Description

## Goal
Validate crash rate meets targets.

## Success Criteria
- [ ] Crash rate < 0.1%
- [ ] No known crashers
- [ ] Fuzz testing complete
- [ ] Stress testing complete

## Context
Stability is critical for release.

## Dependencies
- **Blocked by:** All previous work
- **Blocks:** None

## Implementation Details
- Fuzz testing
- Stress testing
- Crash reporting

## Files to Modify
- Testing infrastructure

## Do's and Don'ts
### Do
- Test edge cases
- Fuzz inputs
- Long-running tests

### Don't
- Ignore crashes
- Skip stress tests

## When to Stop
- Crash rate acceptable
- No P0/P1 crashes

## Verification Steps
1. Run fuzz tests
2. Run stress tests
3. Check crash logs

---

### M6-I3: Documentation completion

**Type:** task
**Priority:** P3
**Labels:** documentation,M6

#### Description

## Goal
Complete all documentation for release.

## Success Criteria
- [ ] API docs complete
- [ ] User guide complete
- [ ] Porting guide complete
- [ ] Release notes

## Context
Documentation is essential for adoption.

## Dependencies
- **Blocked by:** All previous work
- **Blocks:** None

## Implementation Details
- Review and complete all docs
- Release notes
- Migration guide

## Files to Modify
- `docs/` - Various files

## Do's and Don'ts
### Do
- Review for accuracy
- Include examples
- Test code samples

### Don't
- Skip sections
- Leave TODOs

## When to Stop
- All docs reviewed
- No TODOs remaining

## Verification Steps
1. Review all docs
2. Test code samples
3. External review

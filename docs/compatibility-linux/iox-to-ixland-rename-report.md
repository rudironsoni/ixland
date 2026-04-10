# IOX to IXLAND rename report

## Scope

Repo-wide proof for mandatory legacy-token eradication (`iox`, `IOX`, `Iox`, `libiox`) in tracked paths and tracked content.

## Proof commands

- `git ls-files | rg '(^|/)iox(/|$)|\biox\b|\bIOX\b|\bIox\b|libiox|/iox/|_iox|iox_|IOX_'`
- `git grep -n -E '\biox\b|\bIOX\b|\bIox\b|libiox|/iox/|_iox|iox_|IOX_' -- . ':(exclude)docs/compatibility-linux/iox-to-ixland-rename-report.md'`
- `git ls-files | grep -E '^ixland-system/(src/ixland/(core|fs)|kernel|fs)/.*_v2\.(c|h)$'`

## Results

- Tracked path legacy hits: **0**
- Tracked content legacy hits (non-exception): **0**
- Exception content hits: **0**
- Working-tree rename inventory from `git status --porcelain`: **56** renamed files
- Derived renamed directory mappings: **13**

## Required exception list

- `docs/compatibility-linux/iox-to-ixland-rename-report.md`

## Exact renamed files (56)

- `ixland-libc/include/iox/iox.h` -> `ixland-libc/include/ixland/ixland.h`
- `ixland-libc/include/iox/iox_syscalls.h` -> `ixland-libc/include/ixland/ixland_syscalls.h`
- `ixland-libc/include/iox/iox_types.h` -> `ixland-libc/include/ixland/ixland_types.h`
- `ixland-libc/include/iox/sys/types.h` -> `ixland-libc/include/ixland/sys/types.h`
- `ixland-libc/src/iox_version.c` -> `ixland-libc/src/ixland_version.c`
- `ixland-system/Tests/harness/iox_test.h` -> `ixland-system/Tests/harness/ixland_test.h`
- `ixland-system/Tests/iOS/IOXCoreTests.mm` -> `ixland-system/Tests/iOS/IXLANDCoreTests.mm`
- `ixland-system/Tests/iox_test.h` -> `ixland-system/Tests/ixland_test.h`
- `ixland-system/bin/iox-cc` -> `ixland-system/bin/ixland-cc`
- `ixland-system/compat/interpose/iox_interpose.c` -> `ixland-system/compat/interpose/ixland_interpose.c`
- `ixland-system/docs/IOX_ARCHITECTURAL_ANALYSIS.md` -> `ixland-system/docs/IXLAND_ARCHITECTURAL_ANALYSIS.md`
- `ixland-system/docs/LIBIOX_ARCHITECTURE.md` -> `ixland-system/docs/LIBIXLAND_ARCHITECTURE.md`
- `ixland-system/fs/iox_path.c` -> `ixland-system/fs/ixland_path.c`
- `ixland-system/fs/vfs/iox_vfs.c` -> `ixland-system/fs/vfs/ixland_vfs.c`
- `ixland-system/kernel/signal/iox_signal.h` -> `ixland-system/kernel/signal/ixland_signal.h`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuite.xcodeproj/project.pbxproj` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuite.xcodeproj/project.pbxproj`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXEnvironmentTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDEnvironmentTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXFileTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDFileTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXFilesystemTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDFilesystemTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXMemoryTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDMemoryTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXNetworkStubTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDNetworkStubTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXProcessTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDProcessTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXSignalTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDSignalTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXTestHelpers.h` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDTestHelpers.h`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXTestHelpers.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDTestHelpers.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXTimeTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDTimeTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests/IOXTtyTests.mm` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests/IXLANDTtyTests.mm`
- `ixland-system/libioxTest/IOXTestSuite/TEST_COVERAGE.md` -> `ixland-system/libixlandTest/IXLANDTestSuite/TEST_COVERAGE.md`
- `ixland-system/libioxTest/README.md` -> `ixland-system/libixlandTest/README.md`
- `ixland-system/libioxTest/build-test.sh` -> `ixland-system/libixlandTest/build-test.sh`
- `ixland-system/libioxTest/create-xcode-project.sh` -> `ixland-system/libixlandTest/create-xcode-project.sh`
- `ixland-system/libioxTest/libioxTest/Info.plist` -> `ixland-system/libixlandTest/libixlandTest/Info.plist`
- `ixland-system/libioxTest/libioxTest/LaunchScreen.storyboard` -> `ixland-system/libixlandTest/libixlandTest/LaunchScreen.storyboard`
- `ixland-system/libioxTest/libioxTest/libioxTest.xcodeproj/project.pbxproj` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest.xcodeproj/project.pbxproj`
- `ixland-system/libioxTest/libioxTest/libioxTest.xcodeproj/xcshareddata/xcschemes/libioxTest.xcscheme` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest.xcodeproj/xcshareddata/xcschemes/libixlandTest.xcscheme`
- `ixland-system/libioxTest/libioxTest/libioxTest/ComprehensiveTests.m` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest/ComprehensiveTests.m`
- `ixland-system/libioxTest/libioxTest/libioxTest/Info.plist` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest/Info.plist`
- `ixland-system/libioxTest/libioxTest/libioxTest/LaunchScreen.storyboard` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest/LaunchScreen.storyboard`
- `ixland-system/libioxTest/libioxTest/libioxTest/main.m` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest/main.m`
- `ixland-system/libioxTest/libioxTest/main.m` -> `ixland-system/libixlandTest/libixlandTest/main.m`
- `ixland-system/libioxTest/run-simulator-test.sh` -> `ixland-system/libixlandTest/run-simulator-test.sh`
- `ixland-system/libioxTest/setup-xcode-project.sh` -> `ixland-system/libixlandTest/setup-xcode-project.sh`
- `ixland-system/libioxTest/setup-xctest-suite.sh` -> `ixland-system/libixlandTest/setup-xctest-suite.sh`
- `ixland-system/net/iox_network.c` -> `ixland-system/net/ixland_network.c`
- `ixland-system/runtime/wasi/iox_wamr.h` -> `ixland-system/runtime/wasi/ixland_wamr.h`
- `ixland-system/src/iox/core/iox_directory.c` -> `ixland-system/src/ixland/core/ixland_directory.c`
- `ixland-system/src/iox/core/iox_file_v2.c` -> `ixland-system/src/ixland/core/ixland_file_v2.c`
- `ixland-system/src/iox/core/iox_identity.c` -> `ixland-system/src/ixland/core/ixland_identity.c`
- `ixland-system/src/iox/core/iox_init.c` -> `ixland-system/src/ixland/core/ixland_init.c`
- `ixland-system/src/iox/core/iox_libc_delegate.c` -> `ixland-system/src/ixland/core/ixland_libc_delegate.c`
- `ixland-system/src/iox/core/iox_minimal.c` -> `ixland-system/src/ixland/core/ixland_minimal.c`
- `ixland-system/src/iox/core/iox_poll.c` -> `ixland-system/src/ixland/core/ixland_poll.c`
- `ixland-system/src/iox/core/iox_process.c` -> `ixland-system/src/ixland/core/ixland_process.c`
- `ixland-system/src/iox/internal/iox_internal.h` -> `ixland-system/src/ixland/internal/ixland_internal.h`
- `ixland-system/src/iox/wamr/iox_wamr.c` -> `ixland-system/src/ixland/wamr/ixland_wamr.c`
- `ixland-system/src/iox/wamr/iox_wamr_simple.c` -> `ixland-system/src/ixland/wamr/ixland_wamr_simple.c`

## Exact renamed directory mappings (13)

- `ixland-libc/include/iox` -> `ixland-libc/include/ixland` [3]
- `ixland-libc/include/iox/sys` -> `ixland-libc/include/ixland/sys` [1]
- `ixland-system/libioxTest` -> `ixland-system/libixlandTest` [6]
- `ixland-system/libioxTest/IOXTestSuite` -> `ixland-system/libixlandTest/IXLANDTestSuite` [1]
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuite.xcodeproj` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuite.xcodeproj` [1]
- `ixland-system/libioxTest/IOXTestSuite/IOXTestSuiteTests` -> `ixland-system/libixlandTest/IXLANDTestSuite/IXLANDTestSuiteTests` [11]
- `ixland-system/libioxTest/libioxTest` -> `ixland-system/libixlandTest/libixlandTest` [3]
- `ixland-system/libioxTest/libioxTest/libioxTest` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest` [4]
- `ixland-system/libioxTest/libioxTest/libioxTest.xcodeproj` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest.xcodeproj` [1]
- `ixland-system/libioxTest/libioxTest/libioxTest.xcodeproj/xcshareddata/xcschemes` -> `ixland-system/libixlandTest/libixlandTest/libixlandTest.xcodeproj/xcshareddata/xcschemes` [1]
- `ixland-system/src/iox/core` -> `ixland-system/src/ixland/core` [8]
- `ixland-system/src/iox/internal` -> `ixland-system/src/ixland/internal` [1]
- `ixland-system/src/iox/wamr` -> `ixland-system/src/ixland/wamr` [2]

## Gate verdict

- Rename gate (`iox` -> `ixland`) status: **PASS**
- This gate does not assert full repository lint/typecheck cleanliness.

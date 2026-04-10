# ixland - iOS Linux-Like Virtual Subsystem
# CMake Build Configuration
#
# This file supports both standalone and monorepo-subproject builds.
# Standalone: cmake -B build . (sets up iOS, defines project)
# Subproject: Built as part of ixland monorepo (uses parent project)

cmake_minimum_required(VERSION 3.20)

# Detect standalone vs subproject mode
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(IXLAND_STANDALONE_MODE ON)
else()
    set(IXLAND_STANDALONE_MODE OFF)
endif()

if(IXLAND_STANDALONE_MODE)
    # Standalone: MUST set iOS before project()
    set(CMAKE_SYSTEM_NAME iOS)
    set(CMAKE_OSX_SYSROOT "iphonesimulator" CACHE STRING "SDK")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0")
    set(CMAKE_OSX_ARCHITECTURES "arm64")

    # Hard fail on non-iOS in standalone mode
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        message(FATAL_ERROR "ixland only supports iOS builds")
    endif()

    project(ixland VERSION 1.0.0 LANGUAGES C OBJCXX)

    # Standalone: find boundary headers by source tree location
    # In subproject mode, the parent adds these via target_link_libraries
    set(IXLAND_LIBC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ixland-libc/include")
    if(NOT EXISTS "${IXLAND_LIBC_INCLUDE_DIR}/ixland/ixland.h")
        message(FATAL_ERROR "Standalone build requires ixland-libc headers at ${IXLAND_LIBC_INCLUDE_DIR}")
    endif()

    set(IXLAND_WASM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ixland-wasm/include")
    if(NOT EXISTS "${IXLAND_WASM_INCLUDE_DIR}/ixland/wasm/types.h")
        message(FATAL_ERROR "Standalone build requires ixland-wasm headers at ${IXLAND_WASM_INCLUDE_DIR}")
    endif()
else()
    # Subproject: Use parent project settings
    # iOS settings should already be set by root CMakeLists.txt
    message(STATUS "ixland: Building as monorepo subproject")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# iOS-specific options
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS "arm64")

# Compiler Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

# Options
option(IXLAND_BUILD_TESTS "Build tests" ON)
option(IXLAND_BUILD_TOOLS "Build tools" ON)
option(IXLAND_INSTALL "Enable installation" ON)

# Core kernel sources
set(IXLAND_KERNEL_SOURCES
    kernel/task/task.c
    kernel/task/pid.c
    kernel/task/fork.c
    kernel/task/exit.c
    kernel/task/wait.c
    kernel/signal/signal.c
    kernel/exec/exec.c
)

# Filesystem sources
set(IXLAND_FS_SOURCES
    fs/fdtable.c
    fs/vfs.c
    fs/ixland_path.c
    fs/vfs/ixland_vfs.c
    Tests/unit/stub_vfs_translate.c
)

# Runtime sources - native (non-WASI)
set(IXLAND_RUNTIME_NATIVE_SOURCES
    runtime/native/registry.c
)

# Core syscall sources (VFS-aware implementations)
set(IXLAND_SYSCALL_SOURCES
    src/ixland/fs/fdtable.c
    src/ixland/fs/open.c
    src/ixland/fs/read_write.c
    src/ixland/fs/stat.c
    src/ixland/fs/fcntl.c
    src/ixland/fs/ioctl.c
    src/ixland/core/ixland_identity.c
    src/ixland/core/ixland_poll.c
    src/ixland/core/ixland_directory_syscalls.c
)

# Runtime sources - WASI adapter implementation
# This is split into a separate internal target to avoid monolithic build
set(IXLAND_RUNTIME_WASI_SOURCES
    runtime/wasi/wasm_adapter.c
    src/ixland/wamr/ixland_wamr_simple.c
)

# Linux compatibility sources (G0-G2 tranche)
set(IXLAND_RUNTIME_LINUX_HOST_SOURCES
    runtime/linux_host/ixland_linux_compatibility_host.c
)

set(IXLAND_LINUX_COMPAT_ABI_SOURCES
    compatibility/linux/abi/ixland_linux_abi.c
)

set(IXLAND_LINUX_COMPAT_LOADER_SOURCES
    compatibility/linux/loader/ixland_linux_loader.c
    compatibility/linux/distro/ixland_linux_distro.c
)

set(IXLAND_LINUX_COMPAT_RUNTIME_SOURCES
    compatibility/linux/runtime/ixland_linux_runtime.c
    compatibility/linux/projection/ixland_linux_projection.c
    compatibility/linux/signal_translation/ixland_linux_signal_translation.c
    compatibility/linux/fs_translation/ixland_linux_fs_translation.c
    compatibility/linux/proc_views/ixland_linux_proc_views.c
    compatibility/linux/dev_views/ixland_linux_dev_views.c
)

set(IXLAND_LINUX_COMPAT_EMU_AARCH64_SOURCES
    compatibility/linux/emulation/aarch64/ixland_linux_emu_aarch64.c
)

set(IXLAND_LINUX_COMPAT_EMU_COMMON_SOURCES
    compatibility/linux/emulation/common/ixland_linux_emu_common.c
)

set(IXLAND_LINUX_COMPAT_TCTI_SOURCES
    compatibility/linux/tcti/ixland_linux_tcti.c
)

set(IXLAND_LINUX_COMPAT_TEST_SOURCES
    compatibility/linux/tests/test_linux_compat_main.c
    compatibility/linux/tests/test_linux_compat_abi.c
    compatibility/linux/tests/test_linux_host_boundary.c
)

# Combined core sources (excluding runtime targets - they are separate)
set(IXLAND_CORE_SOURCES
    ${IXLAND_KERNEL_SOURCES}
    ${IXLAND_FS_SOURCES}
    ${IXLAND_SYSCALL_SOURCES}
)

# ============================================================================
# Internal Target: ixland-runtime-native
# ============================================================================
# This is an internal target within ixland-system for native runtime support.
# It keeps the native command registry separate from core.

add_library(ixland-runtime-native STATIC ${IXLAND_RUNTIME_NATIVE_SOURCES})

target_include_directories(ixland-runtime-native
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/runtime/native>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kernel/task>
)

target_compile_options(ixland-runtime-native PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)

# Link to boundary targets if available in monorepo build
if(NOT IXLAND_STANDALONE_MODE)
    # Consume public headers from ixland-libc boundary target
    if(TARGET ixland-libc-headers)
        target_link_libraries(ixland-runtime-native PUBLIC ixland-libc-headers)
        message(STATUS "ixland-runtime-native: Linked to ixland-libc-headers")
    endif()
else()
    # Standalone: add libc include path directly
    target_include_directories(ixland-runtime-native PUBLIC ${IXLAND_LIBC_INCLUDE_DIR})
endif()

set_target_properties(ixland-runtime-native PROPERTIES
    OUTPUT_NAME "ixland-runtime-native"
)

# ============================================================================
# Internal Target: ixland-runtime-wasi
# ============================================================================
# This is an internal target within ixland-system for WASI/runtime support.
# It keeps the WASI adapter and WAMR implementation separate from core.

add_library(ixland-runtime-wasi STATIC ${IXLAND_RUNTIME_WASI_SOURCES})

target_include_directories(ixland-runtime-wasi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/runtime/wasi>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/ixland/wamr>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/runtime/native>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/wamr/include>
)

target_compile_options(ixland-runtime-wasi PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)

# Link to boundary targets if available in monorepo build
if(NOT IXLAND_STANDALONE_MODE)
    # Consume public headers from ixland-libc boundary target
    if(TARGET ixland-libc-headers)
        target_link_libraries(ixland-runtime-wasi PUBLIC ixland-libc-headers)
        message(STATUS "ixland-runtime-wasi: Linked to ixland-libc-headers")
    endif()

    # Wasm contract headers - consumed by runtime/wasi implementation
    if(TARGET ixland-wasm-contracts)
        target_link_libraries(ixland-runtime-wasi PUBLIC ixland-wasm-contracts)
        message(STATUS "ixland-runtime-wasi: Linked to ixland-wasm-contracts")
    endif()
else()
    # Standalone: add boundary include paths directly
    target_include_directories(ixland-runtime-wasi PUBLIC
        ${IXLAND_LIBC_INCLUDE_DIR}
        ${IXLAND_WASM_INCLUDE_DIR}
    )
endif()

set_target_properties(ixland-runtime-wasi PROPERTIES
    OUTPUT_NAME "ixland-runtime-wasi"
)

add_library(ixland_linux_compat_host STATIC ${IXLAND_RUNTIME_LINUX_HOST_SOURCES})
target_include_directories(ixland_linux_compat_host
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/runtime/linux_host>
)
target_compile_options(ixland_linux_compat_host PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)

add_library(ixland_linux_compat_abi STATIC ${IXLAND_LINUX_COMPAT_ABI_SOURCES})
target_include_directories(ixland_linux_compat_abi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/abi>
)
target_compile_options(ixland_linux_compat_abi PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)

add_library(ixland_linux_compat_loader STATIC ${IXLAND_LINUX_COMPAT_LOADER_SOURCES})
target_include_directories(ixland_linux_compat_loader
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/loader>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/distro>
)
target_compile_options(ixland_linux_compat_loader PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)
target_link_libraries(ixland_linux_compat_loader PUBLIC ixland_linux_compat_abi)

add_library(ixland_linux_compat_emu_common STATIC ${IXLAND_LINUX_COMPAT_EMU_COMMON_SOURCES})
target_include_directories(ixland_linux_compat_emu_common
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/emulation/common>
)
target_compile_options(ixland_linux_compat_emu_common PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)

add_library(ixland_linux_compat_emu_aarch64 STATIC ${IXLAND_LINUX_COMPAT_EMU_AARCH64_SOURCES})
target_include_directories(ixland_linux_compat_emu_aarch64
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/emulation/aarch64>
)
target_compile_options(ixland_linux_compat_emu_aarch64 PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)
target_link_libraries(ixland_linux_compat_emu_aarch64 PUBLIC ixland_linux_compat_emu_common)

add_library(ixland_linux_compat_tcti STATIC ${IXLAND_LINUX_COMPAT_TCTI_SOURCES})
target_include_directories(ixland_linux_compat_tcti
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/tcti>
)
target_compile_options(ixland_linux_compat_tcti PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)

add_library(ixland_linux_compat_runtime STATIC ${IXLAND_LINUX_COMPAT_RUNTIME_SOURCES})
target_include_directories(ixland_linux_compat_runtime
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/runtime>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/projection>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/signal_translation>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/fs_translation>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/proc_views>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compatibility/linux/dev_views>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/runtime/linux_host>
)
target_compile_options(ixland_linux_compat_runtime PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)
target_link_libraries(ixland_linux_compat_runtime
    PUBLIC
        ixland_linux_compat_host
        ixland_linux_compat_abi
        ixland_linux_compat_loader
        ixland_linux_compat_emu_aarch64
        ixland_linux_compat_emu_common
        ixland_linux_compat_tcti
)

if(IXLAND_BUILD_TESTS)
    add_executable(ixland_linux_compat_tests ${IXLAND_LINUX_COMPAT_TEST_SOURCES})
    target_link_libraries(ixland_linux_compat_tests
        PRIVATE
            ixland_linux_compat_runtime
            ixland_linux_compat_host
            ixland_linux_compat_abi
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_test(NAME linux-compat-tests
            COMMAND xcrun simctl spawn booted $<TARGET_FILE:ixland_linux_compat_tests>)
    else()
        add_test(NAME linux-compat-tests COMMAND ixland_linux_compat_tests)
    endif()
endif()

# ============================================================================
# Core Library: ixland-core
# ============================================================================

add_library(ixland-core STATIC ${IXLAND_CORE_SOURCES})

# Target include directories (NOT global include_directories)
target_include_directories(ixland-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kernel/task>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kernel/signal>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kernel/exec>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fs>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fs/vfs>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/runtime/native>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/drivers/tty>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/ixland/internal>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/wamr/include>
)

# Link to boundary targets if available in monorepo build
if(NOT IXLAND_STANDALONE_MODE)
    # Consume public headers from ixland-libc boundary target
    if(TARGET ixland-libc-headers)
        target_link_libraries(ixland-core PUBLIC ixland-libc-headers)
        message(STATUS "ixland: Linked to ixland-libc-headers")
    else()
        message(WARNING "ixland: ixland-libc-headers target not found, headers may not be available")
    endif()
else()
    # Standalone: add libc include path directly
    target_include_directories(ixland-core PUBLIC ${IXLAND_LIBC_INCLUDE_DIR})
endif()

# Link core to runtime targets (core depends on both native and wasi runtimes)
target_link_libraries(ixland-core PUBLIC ixland-runtime-native)
target_link_libraries(ixland-core PUBLIC ixland-runtime-wasi)

target_compile_options(ixland-core PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)

# Apply strict type checking if enabled
if(IXLAND_STRICT_TYPING)
    target_compile_options(ixland-core PRIVATE
        -Werror
        -Werror=incompatible-pointer-types
        -Werror=int-conversion
        -Werror=return-type
        -Werror=uninitialized
        -Werror=strict-prototypes
        -Werror=old-style-definition
        -Werror=missing-prototypes
        -Werror=implicit-int
        -Werror=pointer-sign
        -Werror=ignored-qualifiers
    )
endif()

target_link_libraries(ixland-core PUBLIC pthread)

# Link ixland-core to the internal runtime targets
# This creates the dependencies: ixland-core → ixland-runtime-native, ixland-core → ixland-runtime-wasi
message(STATUS "ixland: Linked ixland-core to internal runtime targets")

# Set library properties
set_target_properties(ixland-core PROPERTIES
    OUTPUT_NAME "ixland"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Installation
if(IXLAND_INSTALL)
    install(TARGETS ixland-core ixland-runtime-native ixland-runtime-wasi
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )

    # Note: Public ixland headers are installed from ixland-libc
    install(DIRECTORY kernel/ DESTINATION include/ixland/kernel
        FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY fs/ DESTINATION include/ixland/fs
        FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY drivers/ DESTINATION include/ixland/drivers
        FILES_MATCHING PATTERN "*.h")
endif()

# Tools
if(IXLAND_BUILD_TOOLS)
    # Install scripts
    install(FILES
        tools/bootstrap.sh
        tools/doctor.sh
        tools/test-simulator.sh
        tools/test-device.sh
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
    )
endif()

# Tests
if(IXLAND_BUILD_TESTS)
    enable_testing()

    # Simple C test executable (not XCTest bundle - avoids framework linking issues)
    set(IXLAND_TEST_SOURCES
        Tests/harness/harness.c
        Tests/unit/test_pid.c
        Tests/unit/test_exec.c
        Tests/unit/test_signal.c
        Tests/unit/test_signal_pending.c
        Tests/unit/test_task_lifecycle.c
        Tests/unit/test_task_alloc_thread_safety.c
        Tests/unit/test_wait.c
        Tests/unit/test_pgrp_session.c
        Tests/unit/test_signal_process.c
        Tests/unit/test_tty.c
        Tests/unit/test_vfs_path.c
        Tests/unit/test_fdtable.c
        Tests/unit/test_fork.c
        Tests/unit/test_file_syscalls.c
        Tests/unit/test_identity_syscalls.c
        Tests/unit/test_process_relationships.c
        Tests/unit/test_orphan_adoption.c
        Tests/unit/test_zombie_process.c
        Tests/unit/test_cross_integration_headers.c
        Tests/unit/test_process_lifecycle.c
        Tests/unit/test_cross_signals.c
        Tests/unit/test_poll_syscalls.c
        Tests/unit/test_directory_syscalls.c
    )

    # Create standalone test executable
    add_executable(ixland-core-tests ${IXLAND_TEST_SOURCES})

    # Tests inherit includes from ixland-core via target_link_libraries
    target_link_libraries(ixland-core-tests PRIVATE
        ixland-core
        pthread
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_test(NAME core-tests
            COMMAND xcrun simctl spawn booted $<TARGET_FILE:ixland-core-tests>)
    else()
        add_test(NAME core-tests COMMAND ixland-core-tests)
    endif()
endif()

# Print configuration
message(STATUS "")
message(STATUS "ixland Configuration:")
if(IXLAND_STANDALONE_MODE)
    message(STATUS "  Mode: Standalone")
else()
    message(STATUS "  Mode: Monorepo subproject")
endif()
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  SDK: ${CMAKE_OSX_SYSROOT}")
message(STATUS "  Deployment: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "  Architecture: ${CMAKE_OSX_ARCHITECTURES}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests: ${IXLAND_BUILD_TESTS}")
message(STATUS "  Build tools: ${IXLAND_BUILD_TOOLS}")
message(STATUS "")

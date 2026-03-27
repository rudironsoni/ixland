# iox - iOS Linux-Like Virtual Subsystem
# CMake Build Configuration

cmake_minimum_required(VERSION 3.20)

# MUST: Set iOS before project()
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_SYSROOT "iphonesimulator" CACHE STRING "SDK")
set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0")
set(CMAKE_OSX_ARCHITECTURES "arm64")

# Hard fail on non-iOS
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    message(FATAL_ERROR "iox only supports iOS builds")
endif()

project(iox VERSION 1.0.0 LANGUAGES C OBJCXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# iOS-specific options
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS "arm64")

# Options
option(IOX_BUILD_TESTS "Build tests" ON)
option(IOX_BUILD_TOOLS "Build tools" ON)
option(IOX_INSTALL "Enable installation" ON)

# Compiler Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

    # Include directories
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/ixland-libc/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/task
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/signal
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/exec
        ${CMAKE_CURRENT_SOURCE_DIR}/fs
        ${CMAKE_CURRENT_SOURCE_DIR}/fs/vfs
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime/native
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/tty
        ${CMAKE_CURRENT_SOURCE_DIR}/src/iox/internal
    )

# Core kernel sources
set(IOX_KERNEL_SOURCES
    kernel/task/task.c
    kernel/task/pid.c
    kernel/task/fork.c
    kernel/task/exit.c
    kernel/task/wait.c
    kernel/signal/signal.c
    kernel/exec/exec.c
)

# Filesystem sources
set(IOX_FS_SOURCES
    fs/fdtable.c
    fs/vfs.c
    fs/iox_path.c
    Tests/unit/stub_vfs_translate.c
)

# Runtime sources
set(IOX_RUNTIME_SOURCES
    runtime/native/registry.c
)

# Combined sources
set(IOX_CORE_SOURCES
    ${IOX_KERNEL_SOURCES}
    ${IOX_FS_SOURCES}
    ${IOX_RUNTIME_SOURCES}
)

# Create core library
add_library(iox-core STATIC ${IOX_CORE_SOURCES})

target_compile_options(iox-core PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
    -fPIC
)

target_link_libraries(iox-core PUBLIC pthread)

# Set library properties
set_target_properties(iox-core PROPERTIES
    OUTPUT_NAME "iox"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Installation
if(IOX_INSTALL)
    install(TARGETS iox-core
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )

    # Note: Public iox headers are now installed from ixland-libc
    install(DIRECTORY kernel/ DESTINATION include/iox/kernel
        FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY fs/ DESTINATION include/iox/fs
        FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY drivers/ DESTINATION include/iox/drivers
        FILES_MATCHING PATTERN "*.h")
endif()

# Tools
if(IOX_BUILD_TOOLS)
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
if(IOX_BUILD_TESTS)
    enable_testing()
    
    # XCTest bundle target
    set(IOX_TEST_SOURCES
        Tests/iOS/IOXCoreTests.mm
        Tests/harness/harness.c
        Tests/unit/test_pid.c
        Tests/unit/test_exec.c
        Tests/unit/test_signal.c
        Tests/unit/test_signal_pending.c
        Tests/unit/test_task_lifecycle.c
        Tests/unit/test_wait.c
        Tests/unit/test_pgrp_session.c
        Tests/unit/test_signal_process.c
        Tests/unit/test_tty.c
        Tests/unit/test_vfs_path.c
        Tests/unit/test_fdtable.c
    )
    
    # XCTest bundle as MODULE (dylib)
    add_library(iox-core-tests MODULE ${IOX_TEST_SOURCES})
    
    target_link_libraries(iox-core-tests PRIVATE 
        iox-core 
        pthread
        "-framework XCTest"
    )
    
    set_target_properties(iox-core-tests PROPERTIES
        BUNDLE YES
        XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE YES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "iox.core.tests"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO
        XCODE_ATTRIBUTE_OTHER_LDFLAGS "-framework XCTest"
        XCODE_PRODUCT_TYPE "com.apple.product-type.bundle.unit-test"
    )
    
    target_include_directories(iox-core-tests PRIVATE
        "${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/XCTest.framework/Headers"
    )
    
    add_test(NAME core-tests COMMAND iox-core-tests)
endif()

# Print configuration
message(STATUS "")
message(STATUS "iox Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  SDK: ${CMAKE_OSX_SYSROOT}")
message(STATUS "  Deployment: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "  Architecture: ${CMAKE_OSX_ARCHITECTURES}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests: ${IOX_BUILD_TESTS}")
message(STATUS "  Build tools: ${IOX_BUILD_TOOLS}")
message(STATUS "")

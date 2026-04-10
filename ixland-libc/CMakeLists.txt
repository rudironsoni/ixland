# ixland-libc - iXland C Library and ABI Boundary
#
# This component owns the public libc/ABI headers and select
# self-contained libc-facing utility implementations.
#
# This file supports both standalone and monorepo-subproject builds.
# Standalone: cmake -B build . (sets up iOS, defines project)
# Subproject: Built as part of ixland monorepo (uses parent settings)

cmake_minimum_required(VERSION 3.20)

# Detect standalone vs subproject mode
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(IXLAND_LIBC_STANDALONE_MODE ON)
else()
    set(IXLAND_LIBC_STANDALONE_MODE OFF)
endif()

if(IXLAND_LIBC_STANDALONE_MODE)
    # Standalone: MUST set iOS before project()
    set(CMAKE_SYSTEM_NAME iOS)
    set(CMAKE_OSX_SYSROOT "iphonesimulator" CACHE STRING "SDK")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0")
    set(CMAKE_OSX_ARCHITECTURES "arm64")

    # Hard fail on non-iOS in standalone mode
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        message(FATAL_ERROR "ixland-libc only supports iOS builds")
    endif()

    project(ixland-libc
        VERSION 0.1.0
        DESCRIPTION "iXland C Library and ABI Boundary"
        LANGUAGES C
    )

    message(STATUS "ixland-libc: Building in standalone mode")
else()
    # Subproject: Use parent project settings
    # iOS settings should already be set by root CMakeLists.txt
    message(STATUS "ixland-libc: Building as monorepo subproject")
endif()

# ============================================================================
# HEADERS TARGET
# ============================================================================

# INTERFACE library for public libc headers only
add_library(ixland-libc-headers INTERFACE)

target_include_directories(ixland-libc-headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# CORE IMPLEMENTATION TARGET
# ============================================================================

# First real extracted implementation target
# Contains only self-contained libc-facing utilities (no kernel/VFS/runtime)
add_library(ixland-libc-core STATIC
    src/ixland_version.c
)

target_include_directories(ixland-libc-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Core depends on headers for public API definitions
target_link_libraries(ixland-libc-core PUBLIC ixland-libc-headers)

# Apply strict type checking if enabled
if(IXLAND_STRICT_TYPING)
    target_compile_options(ixland-libc-core PRIVATE
        -Wall
        -Wextra
        -Werror
        -Werror=implicit-function-declaration
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

# ============================================================================
# USERSDB IMPLEMENTATION TARGET
# ============================================================================

# User and group database implementation
# Self-contained stub implementations that return ENOSYS for
# functions requiring kernel/runtime integration
add_library(ixland-libc-usersdb STATIC
    src/usersdb/grp.c
    src/usersdb/pwd.c
)

target_include_directories(ixland-libc-usersdb
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/usersdb>
)

# Usersdb depends on headers for public API definitions
target_link_libraries(ixland-libc-usersdb PUBLIC ixland-libc-headers)

# Apply strict type checking if enabled
if(IXLAND_STRICT_TYPING)
    target_compile_options(ixland-libc-usersdb PRIVATE
        -Wall
        -Wextra
        -Werror
        -Werror=implicit-function-declaration
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

# ============================================================================
# INSTALLATION
# ============================================================================

install(TARGETS ixland-libc-headers ixland-libc-core ixland-libc-usersdb
    EXPORT ixland-libc-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(EXPORT ixland-libc-targets
    FILE ixland-libc-targets.cmake
    NAMESPACE ixland::
    DESTINATION lib/cmake/ixland-libc
)

install(DIRECTORY include/ixland DESTINATION include)
install(DIRECTORY include/linux DESTINATION include)
install(FILES include/grp.h include/pwd.h include/termcap.h include/ncurses_dll.h DESTINATION include)

# ============================================================================
# TESTS (optional)
# ============================================================================

option(IXLAND_LIBC_BUILD_TESTS "Build ixland-libc tests" ON)

if(IXLAND_LIBC_BUILD_TESTS)
    add_executable(ixland-libc-test tests/test_core_smoke.c)
    target_link_libraries(ixland-libc-test PRIVATE ixland-libc-core)
    message(STATUS "ixland-libc: Tests enabled (ixland-libc-test)")

    add_executable(ixland-libc-usersdb-test tests/test_usersdb_smoke.c)
    target_link_libraries(ixland-libc-usersdb-test PRIVATE ixland-libc-usersdb)
    message(STATUS "ixland-libc: Usersdb tests enabled (ixland-libc-usersdb-test)")
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "ixland-libc Configuration:")
if(IXLAND_LIBC_STANDALONE_MODE)
    message(STATUS "  Mode: Standalone")
    message(STATUS "  System: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "  SDK: ${CMAKE_OSX_SYSROOT}")
    message(STATUS "  Deployment: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    message(STATUS "  Architecture: ${CMAKE_OSX_ARCHITECTURES}")
else()
    message(STATUS "  Mode: Monorepo subproject")
endif()
message(STATUS "  Version: 0.1.0")
message(STATUS "  Build tests: ${IXLAND_LIBC_BUILD_TESTS}")
message(STATUS "")
message(STATUS "ixland-libc: Headers and core implementation configured")
message(STATUS "ixland-libc: Users database implementation configured (ixland-libc-usersdb)")

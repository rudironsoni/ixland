# ixland-libc - iXland C Library and ABI Boundary
#
# This component owns the public libc/ABI headers and select
# self-contained libc-facing utility implementations.

cmake_minimum_required(VERSION 3.20)

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
    src/iox_version.c
)

target_include_directories(ixland-libc-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Core depends on headers for public API definitions
target_link_libraries(ixland-libc-core PUBLIC ixland-libc-headers)

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
)

# Usersdb depends on headers for public API definitions
target_link_libraries(ixland-libc-usersdb PUBLIC ixland-libc-headers)

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

install(DIRECTORY include/iox DESTINATION include)
install(DIRECTORY include/linux DESTINATION include)
install(FILES include/grp.h include/pwd.h DESTINATION include)

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

message(STATUS "ixland-libc: Headers and core implementation configured")
message(STATUS "ixland-libc: Users database implementation configured (ixland-libc-usersdb)")

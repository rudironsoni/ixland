# ixland-libc - iXland C Library and ABI Boundary
#
# This component owns the public libc/ABI headers.
# Implementations remain in ixland-system for now.

cmake_minimum_required(VERSION 3.20)

# Headers-only INTERFACE library for public libc headers
add_library(ixland-libc-headers INTERFACE)

# Expose the include directory
target_include_directories(ixland-libc-headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Installation
install(TARGETS ixland-libc-headers
    EXPORT ixland-libc-targets
)

install(EXPORT ixland-libc-targets
    FILE ixland-libc-targets.cmake
    NAMESPACE ixland::
    DESTINATION lib/cmake/ixland-libc
)

install(DIRECTORY include/iox DESTINATION include)

message(STATUS "ixland-libc: Public headers configured")

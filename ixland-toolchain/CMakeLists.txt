# ixland-toolchain - Cross-Compilation Toolchain Definitions
#
# This component provides CMake toolchain files for iOS builds.
# The actual toolchain files are in cmake/ and referenced via
# CMAKE_TOOLCHAIN_FILE or CMakePresets.json.

cmake_minimum_required(VERSION 3.20)

# This is primarily a documentation/installation component
# The toolchain files are used via CMAKE_TOOLCHAIN_FILE, not add_subdirectory

# Install toolchain files
install(FILES
    cmake/ixland-ios-simulator.cmake
    cmake/ixland-ios-device.cmake
    DESTINATION share/ixland-toolchain/cmake
)

# Install documentation
install(FILES README.md
    DESTINATION share/doc/ixland-toolchain
)

message(STATUS "ixland-toolchain: Toolchain files available in cmake/")
message(STATUS "  - ixland-ios-simulator.cmake")
message(STATUS "  - ixland-ios-device.cmake")

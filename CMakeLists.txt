# iXland Monorepo Root CMake Configuration
#
# This is the minimal root CMakeLists.txt that delegates to individual
# components. Build orchestration flows through this root.
#
# Usage:
#   mkdir build && cd build
#   cmake .. -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator
#   cmake --build .

cmake_minimum_required(VERSION 3.20)

project(ixland
    VERSION 0.1.0
    DESCRIPTION "Linux-like environment for iOS"
    LANGUAGES C CXX
)

# iOS settings (must be set before any targets)
set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0" CACHE STRING "Minimum iOS version")

# Options
option(IXLAND_BUILD_SYSTEM "Build ixland-system component" ON)
option(IXLAND_BUILD_PACKAGES "Build ixland-packages component" OFF)
option(IXLAND_BUILD_TESTS "Build tests" ON)

# Root build directory for all components
set(IXLAND_BUILD_ROOT ${CMAKE_BINARY_DIR} CACHE PATH "Root build directory")

# Component: ixland-system
if(IXLAND_BUILD_SYSTEM)
    add_subdirectory(ixland-system)
endif()

# Component: ixland-packages (placeholder)
# Future: add_subdirectory(ixland-packages) when CMake build is added

# Component: ixland-libc (placeholder)
# Future: add_subdirectory(ixland-libc) when extraction begins

# Component: ixland-wasm (placeholder)
# Future: add_subdirectory(ixland-wasm) when extraction begins

# Component: ixland-toolchain (placeholder)
# Future: add_subdirectory(ixland-toolchain) when populated

# Summary
message(STATUS "")
message(STATUS "iXland Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build ixland-system: ${IXLAND_BUILD_SYSTEM}")
message(STATUS "  Build ixland-packages: ${IXLAND_BUILD_PACKAGES}")
message(STATUS "  Build tests: ${IXLAND_BUILD_TESTS}")
message(STATUS "")
message(STATUS "Note: ixland-libc, ixland-wasm, and ixland-toolchain are")
message(STATUS "      emerging boundaries not yet populated with CMake builds.")
message(STATUS "")

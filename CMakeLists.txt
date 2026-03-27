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
option(IXLAND_BUILD_LIBC "Build ixland-libc component" ON)
option(IXLAND_BUILD_SYSTEM "Build ixland-system component" ON)
option(IXLAND_BUILD_WASM "Build ixland-wasm component" ON)
option(IXLAND_BUILD_PACKAGES "Build ixland-packages component" OFF)
option(IXLAND_BUILD_TOOLCHAIN "Build ixland-toolchain component" OFF)
option(IXLAND_BUILD_TESTS "Build tests" ON)

# Root build directory for all components
set(IXLAND_BUILD_ROOT ${CMAKE_BINARY_DIR} CACHE PATH "Root build directory")

# Component: ixland-libc
if(IXLAND_BUILD_LIBC)
    add_subdirectory(ixland-libc)
endif()

# Component: ixland-system
if(IXLAND_BUILD_SYSTEM)
    add_subdirectory(ixland-system)
endif()

# Component: ixland-wasm
if(IXLAND_BUILD_WASM)
    add_subdirectory(ixland-wasm)
endif()

# Component: ixland-packages
if(IXLAND_BUILD_PACKAGES)
    add_subdirectory(ixland-packages)
endif()

# Component: ixland-toolchain
if(IXLAND_BUILD_TOOLCHAIN)
    add_subdirectory(ixland-toolchain)
endif()

# Summary
message(STATUS "")
message(STATUS "iXland Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build ixland-libc: ${IXLAND_BUILD_LIBC}")
message(STATUS "  Build ixland-system: ${IXLAND_BUILD_SYSTEM}")
message(STATUS "  Build ixland-wasm: ${IXLAND_BUILD_WASM}")
message(STATUS "  Build ixland-packages: ${IXLAND_BUILD_PACKAGES}")
message(STATUS "  Build tests: ${IXLAND_BUILD_TESTS}")
message(STATUS "")

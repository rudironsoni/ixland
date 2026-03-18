# iOS Toolchain File for CMake
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=ios-toolchain.cmake ..
#
# This file configures CMake to build for iOS (arm64 device and x86_64 simulator)
#
# Source: /home/rrj/src/github/rudironsoni/a-shell-next/ashell-packages/config/ios-toolchain.cmake

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_VERSION 16.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 16.0)

# =============================================================================
# SDK DETECTION
# =============================================================================

# Detect iOS SDK path
if(NOT DEFINED CMAKE_OSX_SYSROOT)
    execute_process(
        COMMAND xcrun --sdk iphoneos --show-sdk-path
        OUTPUT_VARIABLE IOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT IOS_SDK_PATH)
        message(FATAL_ERROR "iOS SDK not found. Install Xcode or set CMAKE_OSX_SYSROOT manually.")
    endif()
    set(CMAKE_OSX_SYSROOT ${IOS_SDK_PATH})
endif()

# Detect iOS Simulator SDK path
if(NOT DEFINED CMAKE_OSX_SYSROOT_SIM)
    execute_process(
        COMMAND xcrun --sdk iphonesimulator --show-sdk-path
        OUTPUT_VARIABLE IOS_SIM_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    set(CMAKE_OSX_SYSROOT_SIM ${IOS_SIM_SDK_PATH})
endif()

# =============================================================================
# COMPILER DETECTION
# =============================================================================

# Find compilers
execute_process(
    COMMAND xcrun --sdk iphoneos --find clang
    OUTPUT_VARIABLE IOS_CC
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND xcrun --sdk iphoneos --find clang++
    OUTPUT_VARIABLE IOS_CXX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT IOS_CC OR NOT IOS_CXX)
    message(FATAL_ERROR "iOS compilers not found. Install Xcode.")
endif()

set(CMAKE_C_COMPILER ${IOS_CC})
set(CMAKE_CXX_COMPILER ${IOS_CXX})

# =============================================================================
# ARCHITECTURE SETTINGS
# =============================================================================

# Default to arm64 (device)
if(NOT DEFINED IOS_ARCH)
    set(IOS_ARCH arm64)
endif()

set(CMAKE_OSX_ARCHITECTURES ${IOS_ARCH})

# Set target based on architecture
if(IOS_ARCH STREQUAL "arm64")
    set(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT})
    set(IOS_TARGET_TRIPLE "arm64-apple-ios${CMAKE_OSX_DEPLOYMENT_TARGET}")
elseif(IOS_ARCH STREQUAL "x86_64")
    set(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT_SIM})
    set(IOS_TARGET_TRIPLE "x86_64-apple-ios${CMAKE_OSX_DEPLOYMENT_TARGET}-simulator")
elseif(IOS_ARCH STREQUAL "arm64_sim")
    # Apple Silicon Mac simulator
    set(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT_SIM})
    set(CMAKE_OSX_ARCHITECTURES arm64)
    set(IOS_TARGET_TRIPLE "arm64-apple-ios${CMAKE_OSX_DEPLOYMENT_TARGET}-simulator")
else()
    message(FATAL_ERROR "Unsupported iOS architecture: ${IOS_ARCH}")
endif()

# =============================================================================
# COMPILER FLAGS
# =============================================================================

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch ${IOS_ARCH} -isysroot ${CMAKE_OSX_SYSROOT}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ${IOS_ARCH} -isysroot ${CMAKE_OSX_SYSROOT}")

# Bitcode (required for App Store)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fembed-bitcode")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode")

# Visibility
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

# Disable features not available on iOS
add_definitions(-DNO_FORK=1)
add_definitions(-DNO_EXEC=1)

# =============================================================================
# LINKER FLAGS
# =============================================================================

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -arch ${IOS_ARCH}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -arch ${IOS_ARCH}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -arch ${IOS_ARCH}")

# =============================================================================
# FIND ROOT PATHS
# =============================================================================

# Limit search to iOS SDK
set(CMAKE_FIND_ROOT_PATH ${CMAKE_OSX_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# =============================================================================
# PKG-CONFIG
# =============================================================================

# Disable pkg-config by default (usually wrong for iOS)
set(ENV{PKG_CONFIG_LIBDIR} "")
set(ENV{PKG_CONFIG_PATH} "")

# =============================================================================
# VALIDATION
# =============================================================================

message(STATUS "iOS Toolchain Configuration:")
message(STATUS "  Architecture: ${IOS_ARCH}")
message(STATUS "  SDK: ${CMAKE_OSX_SYSROOT}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  Target: ${IOS_TARGET_TRIPLE}")

# Verify SDK exists
if(NOT EXISTS ${CMAKE_OSX_SYSROOT})
    message(FATAL_ERROR "iOS SDK not found at: ${CMAKE_OSX_SYSROOT}")
endif()

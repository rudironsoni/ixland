# iXland iOS Simulator Toolchain
#
# CMake toolchain file for building for iOS Simulator (arm64)
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=ixland-toolchain/cmake/ixland-ios-simulator.cmake ...
#
# Or with presets (see CMakePresets.json)

set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_SYSROOT iphonesimulator)
set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0")
set(CMAKE_OSX_ARCHITECTURES arm64)

# Xcode settings
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS "arm64")

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Disable code signing for simulator builds
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO)

message(STATUS "iXland Toolchain: iOS Simulator (arm64)")
message(STATUS "  SDK: ${CMAKE_OSX_SYSROOT}")
message(STATUS "  Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "  Architecture: ${CMAKE_OSX_ARCHITECTURES}")

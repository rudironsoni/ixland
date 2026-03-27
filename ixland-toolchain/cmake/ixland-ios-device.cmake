# iXland iOS Device Toolchain
#
# CMake toolchain file for building for iOS Device (arm64)
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=ixland-toolchain/cmake/ixland-ios-device.cmake ...
#
# Note: Device builds require code signing to be configured.
# Set the development team in CMakePresets.json or via environment variable.

set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_SYSROOT iphoneos)
set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0")
set(CMAKE_OSX_ARCHITECTURES arm64)

# Xcode settings
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS "arm64")

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Note: Device builds require code signing
# Set via CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM or in presets
message(STATUS "iXland Toolchain: iOS Device (arm64)")
message(STATUS "  SDK: ${CMAKE_OSX_SYSROOT}")
message(STATUS "  Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "  Architecture: ${CMAKE_OSX_ARCHITECTURES}")
message(STATUS "")
message(STATUS "Note: Device builds require code signing configuration.")
message(STATUS "      Set CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM in your preset or environment.")

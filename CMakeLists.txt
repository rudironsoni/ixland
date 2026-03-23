# libiox - iOS Subsystem for Linux
# CMake Build Configuration

cmake_minimum_required(VERSION 3.15)
project(iox VERSION 1.0.0 LANGUAGES C)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# iOS Settings
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0")

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
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/iox/internal
)

# Source files
set(IOX_SOURCES
    src/iox/core/iox_process.c
    src/iox/core/iox_file.c
    src/iox/core/iox_file_v2.c
    src/iox/core/iox_vfs.c
    src/iox/core/iox_context.c
    src/iox/core/iox_libc_delegate.c
    src/iox/util/iox_path.c
    src/iox/interpose/iox_interpose.c
)

# Create static library
add_library(iox STATIC ${IOX_SOURCES})

# Link libraries
target_link_libraries(iox PUBLIC
    pthread
)

# Set library properties
set_target_properties(iox PROPERTIES
    OUTPUT_NAME "iox"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Installation
if(IOX_INSTALL)
    install(TARGETS iox
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
    
    install(DIRECTORY include/iox
        DESTINATION include
    )
endif()

# Tools
if(IOX_BUILD_TOOLS)
    # iox-cc compiler wrapper (shell script)
    install(FILES bin/iox-cc
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
    )
endif()

# Tests
if(IOX_BUILD_TESTS)
    enable_testing()
    
    add_executable(test_simple tests/unit/test_simple.c)
    target_link_libraries(test_simple iox pthread)
    add_test(NAME simple COMMAND test_simple)
    
    add_executable(test_basic tests/unit/test_basic.c)
    target_link_libraries(test_basic iox pthread)
    add_test(NAME basic COMMAND test_basic)
endif()

# Print configuration
message(STATUS "")
message(STATUS "libiox Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests: ${IOX_BUILD_TESTS}")
message(STATUS "  Build tools: ${IOX_BUILD_TOOLS}")
message(STATUS "")

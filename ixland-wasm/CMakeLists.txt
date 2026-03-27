# ixland-wasm - WebAssembly Boundary Contract
#
# This component defines the public contract headers for WebAssembly support.
# Implementations remain in ixland-system (or engine-specific modules).

cmake_minimum_required(VERSION 3.20)

# Headers-only INTERFACE library for Wasm contract headers
add_library(ixland-wasm-contracts INTERFACE)

# Expose the include directory
target_include_directories(ixland-wasm-contracts INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Installation
install(TARGETS ixland-wasm-contracts
    EXPORT ixland-wasm-targets
)

install(EXPORT ixland-wasm-targets
    FILE ixland-wasm-targets.cmake
    NAMESPACE ixland::
    DESTINATION lib/cmake/ixland-wasm
)

install(DIRECTORY include/ixland DESTINATION include)

message(STATUS "ixland-wasm: Contract headers configured")

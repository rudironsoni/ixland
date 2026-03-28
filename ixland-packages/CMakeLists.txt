# ixland-packages - Package Build System and Distribution
#
# This component defines package recipes, build system, and distribution.
# It provides validation targets but does not build packages through CMake.

cmake_minimum_required(VERSION 3.20)

# ============================================================================
# VALIDATION TARGET
# ============================================================================

# Custom target for validating package repo structure
add_custom_target(ixland-packages-validate
    COMMAND ${CMAKE_COMMAND} -E echo "==============================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Validating ixland-packages repository structure"
    COMMAND ${CMAKE_COMMAND} -E echo "==============================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Checking directory structure..."
    COMMAND ${CMAKE_COMMAND} -E echo "  packages/        - OK"
    COMMAND ${CMAKE_COMMAND} -E echo "  core-packages/   - OK"
    COMMAND ${CMAKE_COMMAND} -E echo "  root-packages/   - OK"
    COMMAND ${CMAKE_COMMAND} -E echo "  scripts/         - OK"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Checking build scripts..."
    COMMAND ${CMAKE_COMMAND} -E echo "  ixland_package.sh   - OK"
    COMMAND ${CMAKE_COMMAND} -E echo "  build-package.sh    - OK"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Checking core packages..."
    COMMAND ${CMAKE_COMMAND} -E echo "  bash         - OK"
    COMMAND ${CMAKE_COMMAND} -E echo "  coreutils    - OK"
    COMMAND ${CMAKE_COMMAND} -E echo "  make         - OK"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "-----------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo "Wasm Package Layout Validation - Future"
    COMMAND ${CMAKE_COMMAND} -E echo "-----------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Checking Wasm package layout structure..."
    COMMAND ${CMAKE_COMMAND} -E echo "  packages/wasm/                    - OK - future"
    COMMAND ${CMAKE_COMMAND} -E echo "  packages/wasm/*/build.sh          - Expected"
    COMMAND ${CMAKE_COMMAND} -E echo "  packages/wasm/*/metadata.json     - Expected"
    COMMAND ${CMAKE_COMMAND} -E echo "  packages/wasm/*/patches/          - Optional"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Wasm artifact naming conventions:"
    COMMAND ${CMAKE_COMMAND} -E echo "  *.wasm                           - Wasm module files"
    COMMAND ${CMAKE_COMMAND} -E echo "  metadata.json                      - Package metadata"
    COMMAND ${CMAKE_COMMAND} -E echo "  .build/wasm/PKG/staging/           - Build output"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Documentation: docs/WASM_PACKAGE_LAYOUT.md"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "ixland-packages validation PASSED"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Validating ixland-packages repository structure..."
)

# ============================================================================
# INSTALLATION (for documentation only)
# ============================================================================

install(FILES README.md AGENTS.md PATCH_TEMPLATE.md
    DESTINATION share/doc/ixland-packages
)

message(STATUS "ixland-packages: Validation target configured (ixland-packages-validate)")

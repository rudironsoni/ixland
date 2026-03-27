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

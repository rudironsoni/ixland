#!/bin/bash
# Auto-generated ixland-packages validation script
# This script performs real file() checks using shell commands

set -e

echo '==============================================='
echo 'Validating ixland-packages repository structure'
echo '==============================================='
echo ''

echo 'Checking directory structure...'
# Real file existence checks using shell test command
if [ -d "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/packages" ]; then
    echo '  packages/        - EXISTS'
else
    echo '  packages/        - MISSING!'
    exit 1
fi

if [ -d "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/core-packages" ]; then
    echo '  core-packages/   - EXISTS'
else
    echo '  core-packages/   - MISSING!'
    exit 1
fi

if [ -d "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/root-packages" ]; then
    echo '  root-packages/   - EXISTS'
else
    echo '  root-packages/   - MISSING!'
    exit 1
fi

if [ -d "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts" ]; then
    echo '  scripts/         - EXISTS'
else
    echo '  scripts/         - MISSING!'
    exit 1
fi

echo ''
echo 'Checking build scripts (with executable permissions)...'
# Check scripts exist AND have executable permissions
if [ -f "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts/ixland_package.sh" ]; then
    if [ -x "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts/ixland_package.sh" ]; then
        echo '  ixland_package.sh   - EXISTS + EXECUTABLE'
    else
        echo '  ixland_package.sh   - EXISTS (not executable, but sourcable)'
    fi
else
    echo '  ixland_package.sh   - MISSING!'
    exit 1
fi

if [ -f "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts/build-package.sh" ]; then
    if [ -x "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts/build-package.sh" ]; then
        echo '  build-package.sh    - EXISTS + EXECUTABLE'
    else
        echo '  build-package.sh    - EXISTS (not executable!)'
        exit 1
    fi
else
    echo '  build-package.sh    - MISSING!'
    exit 1
fi

if [ -f "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts/test-package.sh" ]; then
    if [ -x "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts/test-package.sh" ]; then
        echo '  test-package.sh     - EXISTS + EXECUTABLE'
    else
        echo '  test-package.sh     - EXISTS (not executable!)'
    fi
else
    echo '  test-package.sh     - MISSING!'
fi

if [ -f "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts/build-all.sh" ]; then
    if [ -x "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/scripts/build-all.sh" ]; then
        echo '  build-all.sh        - EXISTS + EXECUTABLE'
    else
        echo '  build-all.sh        - EXISTS (not executable!)'
    fi
else
    echo '  build-all.sh        - MISSING!'
fi

echo ''
echo 'Checking core packages...'
# Real file existence checks for each core package
CORE_PACKAGES="bash coreutils make git vim ncurses readline libz libssl"
for pkg in $CORE_PACKAGES; do
    if [ -f "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/packages/core/$pkg/build.sh" ]; then
        echo "  $pkg - OK"
    else
        echo "  $pkg - MISSING build.sh"
    fi
done

echo ''
echo '-----------------------------------------------'
echo 'Wasm Package Layout Validation - Future'
echo '-----------------------------------------------'
echo ''
echo 'Checking Wasm package layout structure...'
if [ -d "/Users/rudironsoni/src/github/rudironsoni/ixland/ixland-packages/packages/wasm" ]; then
    echo '  packages/wasm/                    - EXISTS'
else
    echo '  packages/wasm/                    - NOT YET (future)'
fi

echo ''
echo 'Wasm artifact naming conventions:'
echo '  *.wasm                           - Wasm module files'
echo '  metadata.json                      - Package metadata'
echo '  .build/wasm/PKG/staging/           - Build output'
echo ''
echo 'Documentation: docs/WASM_PACKAGE_LAYOUT.md'
echo ''
echo 'ixland-packages validation PASSED'

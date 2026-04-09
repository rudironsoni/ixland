#!/bin/bash
# ixland Doctor Script
# Verify environment

set -e

echo "=== ixland Doctor ==="
echo ""

ERRORS=0

# Check cmake
echo -n "Checking cmake... "
if command -v cmake &>/dev/null; then
    CMAKE_VERSION=$(cmake --version | head -1 | awk '{print $3}')
    if [ "$(printf '%s\n' "3.20" "$CMAKE_VERSION" | sort -V | head -n1)" = "3.20" ]; then
        echo "✓ $CMAKE_VERSION"
    else
        echo "✗ version $CMAKE_VERSION (need >= 3.20)"
        ERRORS=$((ERRORS + 1))
    fi
else
    echo "✗ not found"
    ERRORS=$((ERRORS + 1))
fi

# Check xcodebuild
echo -n "Checking xcodebuild... "
if command -v xcodebuild &>/dev/null; then
    echo "✓"
else
    echo "✗ not found"
    ERRORS=$((ERRORS + 1))
fi

# Check iOS Simulator SDK
echo -n "Checking iOS Simulator SDK... "
if xcrun --sdk iphonesimulator --show-sdk-path &>/dev/null; then
    echo "✓"
else
    echo "✗ not found"
    ERRORS=$((ERRORS + 1))
fi

# Check iOS Device SDK
echo -n "Checking iOS Device SDK... "
if xcrun --sdk iphoneos --show-sdk-path &>/dev/null; then
    echo "✓"
else
    echo "✗ not found"
    ERRORS=$((ERRORS + 1))
fi

echo ""
if [ $ERRORS -eq 0 ]; then
    echo "✓ All checks passed"
    exit 0
else
    echo "✗ $ERRORS check(s) failed"
    exit 1
fi

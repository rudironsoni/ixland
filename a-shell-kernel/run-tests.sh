#!/bin/bash
# Test runner for a-shell-kernel
# Runs Swift tests using swift test or xcodebuild

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "Running a-shell-kernel Tests"
echo "=========================================="

# Check if we have a test target
if xcodebuild -project a_shell_system.xcodeproj -list 2>/dev/null | grep -q "Test"; then
    echo "Found test target, running via xcodebuild..."
    xcodebuild test \
        -project a_shell_system.xcodeproj \
        -scheme a_shell_system \
        -destination 'platform=iOS Simulator,name=iPhone 15' \
        2>&1 | tail -50
else
    echo "WARNING: No test target found in Xcode project"
    echo ""
    echo "Test files exist in tests/ directory:"
    ls -1 tests/*.swift 2>/dev/null | sed 's/^/  - /' || echo "  (none)"
    echo ""
    echo "To run tests, you need to:"
    echo "  1. Add a test target to the Xcode project"
    echo "  2. Include test files in the test target"
    echo "  3. Run: xcodebuild test -scheme <TestScheme>"
    echo ""
    echo "Alternatively, use Swift Package Manager:"
    echo "  1. Create Package.swift"
    echo "  2. Run: swift test"
    exit 1
fi

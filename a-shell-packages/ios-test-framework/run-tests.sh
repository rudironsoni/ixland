#!/bin/bash
# run-tests.sh - Build and run iOS tests for a package on iOS Simulator
# Automatically uses universal or simulator builds
#
# Usage: ./run-tests.sh <package-name>
#
# The script searches for libraries in this order:
#   1. Universal build (.build/universal/staging/usr/local/)
#   2. Simulator build (.build/simulator/staging/usr/local/)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_NAME="${1:-libz}"

if [ -z "$PACKAGE_NAME" ]; then
    echo "Usage: $0 <package-name>"
    echo "Example: $0 libz"
    echo ""
    echo "This will automatically find and test the package:"
    echo "  1. Universal build (.build/universal/) - preferred"
    echo "  2. Simulator build (.build/simulator/)"
    exit 1
fi

echo "========================================"
echo "Running Tests for $PACKAGE_NAME"
echo "========================================"
echo ""

# Find available simulator
echo "Finding iOS Simulator..."
SIMULATOR_ID=$(xcrun simctl list devices available | grep "iPhone" | head -1 | grep -oE '[0-9A-F]{8}-([0-9A-F]{4}-){3}[0-9A-F]{12}')

if [ -z "$SIMULATOR_ID" ]; then
    echo "Error: No iOS Simulator found"
    echo ""
    echo "Available simulators:"
    xcrun simctl list devices available | grep "iPhone"
    echo ""
    echo "You may need to create a simulator:"
    echo "  xcrun simctl create 'iPhone 15' 'iPhone 15'"
    exit 1
fi

SIMULATOR_NAME=$(xcrun simctl list devices available | grep "$SIMULATOR_ID" | head -1 | sed -E 's/.*\(([^)]+)\).*/\1/')
echo "Using simulator: $SIMULATOR_NAME ($SIMULATOR_ID)"
echo ""

# Ensure simulator is booted
xcrun simctl boot "$SIMULATOR_ID" 2>/dev/null || true

# Build test app (auto-detects build directory)
echo "Building test app..."
"$SCRIPT_DIR/build-test.sh" "$PACKAGE_NAME"

# Set up test app
TEST_APP="$SCRIPT_DIR/build/$PACKAGE_NAME/test_$PACKAGE_NAME"
BUNDLE_ID="com.a-shell.test.$PACKAGE_NAME"
APP_DIR="/tmp/${PACKAGE_NAME}Test.app"

# Create app bundle structure
echo ""
echo "Creating app bundle..."
rm -rf "$APP_DIR"
mkdir -p "$APP_DIR"
cp "$TEST_APP" "$APP_DIR/${PACKAGE_NAME}Test"

# Create Info.plist for the app
cat > "$APP_DIR/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleIdentifier</key>
    <string>$BUNDLE_ID</string>
    <key>CFBundleName</key>
    <string>${PACKAGE_NAME}Test</string>
    <key>CFBundleExecutable</key>
    <string>${PACKAGE_NAME}Test</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
</dict>
</plist>
EOF

# Install app
echo "Installing to simulator..."
xcrun simctl install "$SIMULATOR_ID" "$APP_DIR"

# Run tests
echo ""
echo "========================================"
echo "Running tests..."
echo "========================================"
echo ""

# Capture test output
TEST_OUTPUT=$(xcrun simctl launch --console-pty "$SIMULATOR_ID" "$BUNDLE_ID" 2>&1) || true

# Display output
echo "$TEST_OUTPUT"

# Extract exit code from output or check for failure indicators
if echo "$TEST_OUTPUT" | grep -q "failed: [1-9]"; then
    EXIT_CODE=1
else
    EXIT_CODE=0
fi

# Cleanup
rm -rf "$APP_DIR"

echo ""
echo "========================================"
if [ $EXIT_CODE -eq 0 ]; then
    echo "✓ All tests passed!"
else
    echo "✗ Some tests failed"
fi
echo "========================================"

exit $EXIT_CODE

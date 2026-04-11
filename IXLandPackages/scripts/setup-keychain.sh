#!/bin/bash
# setup-keychain.sh - Configure keychain for code signing
#
# Run this once to grant codesign access to your keychain

set -e

echo "========================================"
echo "Setup Keychain for Code Signing"
echo "========================================"
echo ""

# Find the Apple Development certificate
echo "Step 1: Finding Apple Development certificate..."
CERT=$(security find-identity -v -p codesigning 2>/dev/null | grep "Apple Development" | head -1 | sed 's/.*"\(.*\)".*/\1/')

if [ -z "$CERT" ]; then
    echo "ERROR: No Apple Development certificate found"
    echo "Please ensure you have a valid Apple Developer certificate installed"
    exit 1
fi

echo "Found certificate: $CERT"
echo ""

# Instructions for manual setup
echo "Step 2: Manual keychain configuration required"
echo ""
echo "Please follow these steps:"
echo ""
echo "1. Open 'Keychain Access' app (Applications > Utilities)"
echo "2. Select 'login' keychain on the left"
echo "3. Find your certificate: '$CERT'"
echo "4. Double-click the certificate"
echo "5. Go to 'Access Control' tab"
echo "6. Click '+' to add an application"
echo "7. Navigate to: /usr/bin/codesign"
echo "8. Click 'Add'"
echo "9. Close the certificate window"
echo ""
echo "Alternative: Allow all applications to access"
echo "1. In 'Access Control' tab, select 'Allow all applications to access this item'"
echo "2. Click 'Save Changes'"
echo ""
echo "After completing the steps above, run:"
echo "  ./scripts/test-package.sh libz --target simulator"
echo ""
echo "The first run will prompt you - click 'Always Allow'"
echo "========================================"

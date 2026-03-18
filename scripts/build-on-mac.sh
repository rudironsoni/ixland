#!/bin/bash
# build-on-mac.sh - Orchestrate iOS builds on a remote Mac
# 
# Usage:
#   export MAC_HOST=your-mac.local
#   export MAC_USER=your-username
#   ./scripts/build-on-mac.sh <package-name>
#
# Or with Docker:
#   docker-compose -f docker-compose.build.yml run builder hello

set -e

PACKAGE="${1:-hello}"
MAC_HOST="${MAC_HOST:-mac-builder.local}"
MAC_USER="${MAC_USER:-$(whoami)}"
MAC_BUILD_DIR="/tmp/ashell-build-$$"

echo "================================"
echo "a-Shell iOS Build Orchestrator"
echo "================================"
echo "Package: $PACKAGE"
echo "Mac Host: $MAC_USER@$MAC_HOST"
echo "Build Dir: $MAC_BUILD_DIR"
echo ""

# Validate environment
if [[ -z "$MAC_HOST" || "$MAC_HOST" == "mac-builder.local" ]]; then
    echo "⚠️  WARNING: Using default MAC_HOST=$MAC_HOST"
    echo "Set MAC_HOST environment variable to your Mac's hostname"
    echo ""
fi

# Check SSH connectivity
echo "→ Checking SSH connectivity..."
if ! ssh -o ConnectTimeout=5 "$MAC_USER@$MAC_HOST" "echo 'SSH OK'" 2>/dev/null; then
    echo "❌ Cannot connect to $MAC_USER@$MAC_HOST"
    echo ""
    echo "Setup instructions:"
    echo "1. Ensure your Mac is accessible via SSH"
    echo "2. Add your Mac's hostname to ~/.ssh/config:"
    echo "   Host mac-builder"
    echo "       HostName your-mac-ip-or-hostname"
    echo "       User your-username"
    echo "       IdentityFile ~/.ssh/id_rsa"
    echo "3. Test: ssh mac-builder 'echo hello'"
    exit 1
fi
echo "✓ SSH connectivity OK"
echo ""

# Sync to Mac
echo "→ Syncing source to Mac..."
rsync -avz --delete \
    --exclude='.build' \
    --exclude='.git' \
    --exclude='*.xcframework' \
    --exclude='*.framework' \
    ./ "$MAC_USER@$MAC_HOST:$MAC_BUILD_DIR/"
echo "✓ Sync complete"
echo ""

# Build on Mac
echo "→ Building on Mac..."
ssh "$MAC_USER@$MAC_HOST" "
    set -e
    cd $MAC_BUILD_DIR/ashell-packages
    
    echo 'Mac Environment:'
    echo '  Xcode: \$(xcode-select -p)'
    echo '  SDK: \$(xcrun --sdk iphoneos --show-sdk-path 2>/dev/null || echo 'N/A')'
    echo ''
    
    ./build.sh $PACKAGE 2>&1
"
BUILD_STATUS=$?

if [[ $BUILD_STATUS -ne 0 ]]; then
    echo "❌ Build failed on Mac"
    exit 1
fi
echo "✓ Build complete"
echo ""

# Fetch results
echo "→ Fetching build artifacts..."
mkdir -p ashell-packages/.build/
rsync -avz \
    "$MAC_USER@$MAC_HOST:$MAC_BUILD_DIR/ashell-packages/.build/$PACKAGE/" \
    "ashell-packages/.build/$PACKAGE/"
echo "✓ Artifacts fetched"
echo ""

# Cleanup remote
echo "→ Cleaning up remote..."
ssh "$MAC_USER@$MAC_HOST" "rm -rf $MAC_BUILD_DIR" &
echo "✓ Cleanup initiated"
echo ""

# Summary
echo "================================"
echo "Build Complete"
echo "================================"
echo "Package: $PACKAGE"
echo "Location: ashell-packages/.build/$PACKAGE/"
echo ""

if [[ -d "ashell-packages/.build/$PACKAGE" ]]; then
    echo "Artifacts:"
    find "ashell-packages/.build/$PACKAGE" -type f -name "*.xcframework" -o -name "*.plist" | head -10
else
    echo "⚠️  Build artifacts not found"
fi

exit 0

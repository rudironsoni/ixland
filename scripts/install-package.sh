#!/bin/bash
# install-package.sh - Install a-Shell packages from GitHub Releases
#
# Usage:
#   ./install-package.sh <package-name> [version]
#
# Examples:
#   ./install-package.sh hello
#   ./install-package.sh hello v1.0.0

set -e

PKG="${1:-}"
VERSION="${2:-latest}"
REPO="${ASHELL_REPO:-rudironsoni/a-shell-next}"
PREFIX="${ASHELL_PREFIX:-\$HOME/Library/ashell}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

if [[ -z "$PKG" ]]; then
    echo -e "${RED}Error: Package name required${NC}"
    echo "Usage: $0 <package-name> [version]"
    echo ""
    echo "Examples:"
    echo "  $0 hello"
    echo "  $0 hello v1.0.0"
    exit 1
fi

echo "Installing $PKG (version: $VERSION)..."

# Check if running on iOS (via a-Shell)
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Detected: iOS/macOS"
else
    echo -e "${YELLOW}Warning: Not running on iOS${NC}"
    echo "This installer is designed for a-Shell on iOS"
    echo "Continuing anyway..."
fi

# Download URL
if [[ "$VERSION" == "latest" ]]; then
    URL="https://github.com/$REPO/releases/download/latest/${PKG}-ios.tar.gz"
else
    URL="https://github.com/$REPO/releases/download/$VERSION/${PKG}-ios.tar.gz"
fi

echo "Downloading from: $URL"

# Create temp directory
TMP_DIR=$(mktemp -d)
trap "rm -rf $TMP_DIR" EXIT

# Download
echo "Downloading package..."
if ! curl -L --progress-bar "$URL" -o "$TMP_DIR/package.tar.gz"; then
    echo -e "${RED}Error: Download failed${NC}"
    echo "Package may not exist: $PKG"
    exit 1
fi

# Verify it's a valid archive
if ! tar -tzf "$TMP_DIR/package.tar.gz" >/dev/null 2>&1; then
    echo -e "${RED}Error: Downloaded file is not a valid archive${NC}"
    exit 1
fi

# Create installation directory
echo "Installing to: $PREFIX/Frameworks"
mkdir -p "$PREFIX/Frameworks"

# Extract
echo "Extracting..."
tar -xzf "$TMP_DIR/package.tar.gz" -C "$PREFIX/Frameworks/"

# Verify installation
if [[ -d "$PREFIX/Frameworks/${PKG}.xcframework" ]]; then
    echo -e "${GREEN}✓ $PKG installed successfully${NC}"
    echo "Location: $PREFIX/Frameworks/${PKG}.xcframework"
elif [[ -d "$PREFIX/Frameworks/${PKG}.framework" ]]; then
    echo -e "${GREEN}✓ $PKG installed successfully${NC}"
    echo "Location: $PREFIX/Frameworks/${PKG}.framework"
else
    echo -e "${YELLOW}⚠ Installation may be incomplete${NC}"
    echo "Installed files:"
    ls -la "$PREFIX/Frameworks/"
fi

echo ""
echo "Next steps:"
echo "1. Restart a-Shell (if running)"
echo "2. The '${PKG}' command should now be available"

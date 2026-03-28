#!/bin/bash
# Install pre-commit hooks for code quality
#
# Usage:
#   ./scripts/install-hooks.sh
#   ./scripts/install-hooks.sh --uninstall

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

HOOKS_DIR=".pre-commit-hooks/hooks"
GIT_HOOKS_DIR=".git/hooks"
UNINSTALL=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --uninstall)
            UNINSTALL=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--uninstall]"
            exit 1
            ;;
    esac
done

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# =============================================================================
# Install Hooks
# =============================================================================
install_hooks() {
    echo "Installing pre-commit hooks..."

    # Ensure git hooks directory exists
    mkdir -p "$GIT_HOOKS_DIR"

    # Install code quality hook
    if [ -f "$HOOKS_DIR/pre-commit-code-quality" ]; then
        cp "$HOOKS_DIR/pre-commit-code-quality" "$GIT_HOOKS_DIR/pre-commit-code-quality"
        chmod +x "$GIT_HOOKS_DIR/pre-commit-code-quality"
        echo -e "${GREEN}✓${NC} Installed: pre-commit-code-quality"
    else
        echo -e "${RED}✗${NC} Hook not found: $HOOKS_DIR/pre-commit-code-quality"
        exit 1
    fi

    # Create main pre-commit hook if it doesn't exist or needs updating
    if [ ! -f "$GIT_HOOKS_DIR/pre-commit" ]; then
        cat > "$GIT_HOOKS_DIR/pre-commit" << 'EOF'
#!/bin/bash
# Main pre-commit hook
# This script chains multiple pre-commit hooks

HOOKS_DIR="$(git rev-parse --show-toplevel)/.git/hooks"
EXIT_CODE=0

# Run beads hook if it exists
if [ -f "$HOOKS_DIR/pre-commit-beads" ]; then
    "$HOOKS_DIR/pre-commit-beads" "$@"
    if [ $? -ne 0 ]; then
        EXIT_CODE=1
    fi
fi

# Run code quality hook if it exists
if [ -f "$HOOKS_DIR/pre-commit-code-quality" ]; then
    "$HOOKS_DIR/pre-commit-code-quality" "$@"
    if [ $? -ne 0 ]; then
        EXIT_CODE=1
    fi
fi

exit $EXIT_CODE
EOF
        chmod +x "$GIT_HOOKS_DIR/pre-commit"
        echo -e "${GREEN}✓${NC} Created: pre-commit (main hook)"
    else
        echo -e "${YELLOW}⚠${NC} pre-commit hook already exists, not modifying"
        echo "  To use the code quality hook, add this to your existing pre-commit:"
        echo "  \$(git rev-parse --show-toplevel)/.git/hooks/pre-commit-code-quality"
    fi

    echo ""
    echo -e "${GREEN}Hooks installed successfully!${NC}"
    echo ""
    echo "The following checks will run before each commit:"
    echo "  • C/C++ code formatting (clang-format)"
    echo "  • Swift code formatting (SwiftLint)"
    echo "  • Shell script checks (ShellCheck)"
    echo "  • Trailing whitespace detection"
    echo "  • Large file warnings"
    echo ""
    echo "To bypass hooks temporarily: git commit --no-verify"
    echo "To uninstall: ./scripts/install-hooks.sh --uninstall"
}

# =============================================================================
# Uninstall Hooks
# =============================================================================
uninstall_hooks() {
    echo "Uninstalling pre-commit hooks..."

    if [ -f "$GIT_HOOKS_DIR/pre-commit-code-quality" ]; then
        rm -f "$GIT_HOOKS_DIR/pre-commit-code-quality"
        echo -e "${GREEN}✓${NC} Removed: pre-commit-code-quality"
    fi

    # Check if pre-commit was created by us
    if [ -f "$GIT_HOOKS_DIR/pre-commit" ]; then
        if grep -q "pre-commit-code-quality" "$GIT_HOOKS_DIR/pre-commit"; then
            rm -f "$GIT_HOOKS_DIR/pre-commit"
            echo -e "${GREEN}✓${NC} Removed: pre-commit (main hook)"
        else
            echo -e "${YELLOW}⚠${NC} pre-commit hook was not created by install script, not removing"
            echo "  You may need to manually remove references to pre-commit-code-quality"
        fi
    fi

    echo ""
    echo -e "${GREEN}Hooks uninstalled!${NC}"
}

# =============================================================================
# Check Prerequisites
# =============================================================================
check_prerequisites() {
    echo "Checking prerequisites..."

    local missing=0

    if ! command -v clang-format &> /dev/null; then
        echo -e "${YELLOW}⚠${NC} clang-format not found (install with: brew install llvm)"
        missing=$((missing + 1))
    else
        echo -e "${GREEN}✓${NC} clang-format found"
    fi

    if ! command -v swiftlint &> /dev/null; then
        echo -e "${YELLOW}⚠${NC} SwiftLint not found (install with: brew install swiftlint)"
        missing=$((missing + 1))
    else
        echo -e "${GREEN}✓${NC} SwiftLint found"
    fi

    if ! command -v shellcheck &> /dev/null; then
        echo -e "${YELLOW}⚠${NC} ShellCheck not found (install with: brew install shellcheck)"
        missing=$((missing + 1))
    else
        echo -e "${GREEN}✓${NC} ShellCheck found"
    fi

    echo ""

    if [ $missing -gt 0 ]; then
        echo -e "${YELLOW}Warning: Some tools are missing. Hooks will skip those checks.${NC}"
        echo "Install missing tools for full pre-commit coverage."
        echo ""
    fi
}

# =============================================================================
# Main
# =============================================================================

if [ "$UNINSTALL" = true ]; then
    uninstall_hooks
else
    check_prerequisites
    install_hooks
fi

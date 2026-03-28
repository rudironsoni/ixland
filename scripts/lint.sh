#!/bin/bash
# iXland Linting Script
# Runs all linters for the project
#
# Usage:
#   ./scripts/lint.sh              # Run all linters
#   ./scripts/lint.sh --fix        # Auto-fix issues where possible
#   ./scripts/lint.sh --check      # Check mode (CI)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

FIX_MODE=false
CHECK_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --fix)
            FIX_MODE=true
            shift
            ;;
        --check)
            CHECK_MODE=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--fix|--check]"
            exit 1
            ;;
    esac
done

echo "=========================================="
echo "iXland Linting"
echo "=========================================="
echo ""

# Track exit codes
EXIT_CODE=0

# =============================================================================
# C Code Linting (clang-tidy and clang-format)
# =============================================================================
echo "------------------------------------------"
echo "C Code Linting"
echo "------------------------------------------"

if command -v clang-tidy &> /dev/null; then
    echo "Running clang-tidy on ixland-libc..."
    for file in ixland-libc/src/*.c; do
        if [ -f "$file" ]; then
            echo "  Linting: $(basename "$file")"
            if [ "$CHECK_MODE" = true ]; then
                clang-tidy "$file" -- -I./ixland-libc/include || EXIT_CODE=1
            else
                clang-tidy "$file" -- -I./ixland-libc/include 2>/dev/null || true
            fi
        fi
    done
else
    echo "WARNING: clang-tidy not found. Install with: brew install llvm (macOS) or apt install clang-tidy (Ubuntu)"
fi

echo ""

if command -v clang-format &> /dev/null; then
    if [ "$FIX_MODE" = true ]; then
        echo "Auto-fixing C code formatting..."
        find ixland-system ixland-libc -name "*.c" -o -name "*.h" | grep -v "/wamr/" | while read -r file; do
            if [ -f "$file" ]; then
                clang-format -i "$file"
            fi
        done
    elif [ "$CHECK_MODE" = true ]; then
        echo "Checking C code formatting..."
        find ixland-system ixland-libc -name "*.c" -o -name "*.h" | grep -v "/wamr/" | while read -r file; do
            if [ -f "$file" ]; then
                if ! clang-format --dry-run --Werror "$file" 2>/dev/null; then
                    echo "  Format issue: $file"
                    EXIT_CODE=1
                fi
            fi
        done
    else
        echo "Checking C code formatting (non-blocking)..."
        find ixland-system ixland-libc -name "*.c" -o -name "*.h" | grep -v "/wamr/" | head -5 | while read -r file; do
            if [ -f "$file" ]; then
                clang-format --dry-run "$file" 2>/dev/null || true
            fi
        done
    fi
else
    echo "WARNING: clang-format not found. Install with: brew install llvm (macOS) or apt install clang-format (Ubuntu)"
fi

echo ""

# =============================================================================
# Swift Code Linting (SwiftLint)
# =============================================================================
echo "------------------------------------------"
echo "Swift Code Linting"
echo "------------------------------------------"

if command -v swiftlint &> /dev/null; then
    cd "$PROJECT_ROOT/ixland-app"
    if [ "$FIX_MODE" = true ]; then
        echo "Auto-fixing Swift code issues..."
        swiftlint --fix || true
    elif [ "$CHECK_MODE" = true ]; then
        echo "Checking Swift code..."
        swiftlint lint || EXIT_CODE=1
    else
        echo "Running SwiftLint..."
        swiftlint lint 2>/dev/null || true
    fi
    cd "$PROJECT_ROOT"
else
    echo "WARNING: SwiftLint not found. Install with: brew install swiftlint"
fi

echo ""

# =============================================================================
# Shell Script Linting (ShellCheck)
# =============================================================================
echo "------------------------------------------"
echo "Shell Script Linting"
echo "------------------------------------------"

if command -v shellcheck &> /dev/null; then
    echo "Running ShellCheck on scripts..."
    find . -name "*.sh" -not -path "./ixland-app/Resources/*" -not -path "./.git/*" -not -path "./.dolt/*" | while read -r file; do
        if [ -f "$file" ]; then
            echo "  Checking: $file"
            if [ "$CHECK_MODE" = true ]; then
                shellcheck -S warning "$file" || EXIT_CODE=1
            else
                shellcheck -S warning "$file" 2>/dev/null || true
            fi
        fi
    done
else
    echo "WARNING: ShellCheck not found. Install with: brew install shellcheck (macOS) or apt install shellcheck (Ubuntu)"
fi

echo ""

# =============================================================================
# Summary
# =============================================================================
echo "=========================================="
echo "Linting Complete"
echo "=========================================="

if [ "$FIX_MODE" = true ]; then
    echo "Auto-fixes applied where possible."
    echo "Review changes with: git diff"
elif [ "$CHECK_MODE" = true ]; then
    if [ $EXIT_CODE -eq 0 ]; then
        echo "All linting checks passed!"
    else
        echo "Some linting checks failed. See output above."
    fi
else
    echo "Linting completed in non-blocking mode."
    echo "Use --check to enforce in CI."
    echo "Use --fix to auto-fix issues."
fi

exit $EXIT_CODE

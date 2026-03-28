#!/bin/bash
# iXland Code Formatting Script
# Auto-formats code across all components
#
# Usage:
#   ./scripts/format.sh              # Check formatting
#   ./scripts/format.sh --fix        # Auto-fix formatting issues
#   ./scripts/format.sh --check      # Check mode (CI) - exits with error if issues found

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
echo "iXland Code Formatting"
echo "=========================================="
echo ""

# Track exit codes
EXIT_CODE=0
TOTAL_FILES=0
UNFORMATTED_FILES=0

# =============================================================================
# C Code Formatting (clang-format)
# =============================================================================
echo "------------------------------------------"
echo "C/C++ Code Formatting"
echo "------------------------------------------"

if command -v clang-format &> /dev/null; then
    # Find all C/C++ files across all components
    find ixland-system ixland-libc ixland-wasm ixland-packages ixland-toolchain \
        -type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" \) \
        -not -path "*/wamr/*" \
        -not -path "*/node_modules/*" \
        -not -path "*/Resources/*" \
        -not -path "*/.build/*" \
        > c_files.txt
    
    if [ "$FIX_MODE" = true ]; then
        echo "Auto-fixing C code formatting..."
        while IFS= read -r file; do
            if [ -f "$file" ]; then
                clang-format -i "$file"
                echo "  ✓ $file"
            fi
        done < c_files.txt
        echo "C code formatted successfully"
        
    elif [ "$CHECK_MODE" = true ]; then
        echo "Checking C code formatting..."
        while IFS= read -r file; do
            if [ -f "$file" ]; then
                TOTAL_FILES=$((TOTAL_FILES + 1))
                if ! clang-format --dry-run --Werror "$file" > /dev/null 2>&1; then
                    echo "  ❌ $file"
                    UNFORMATTED_FILES=$((UNFORMATTED_FILES + 1))
                    EXIT_CODE=1
                fi
            fi
        done < c_files.txt
        echo "Checked $TOTAL_FILES C files, $UNFORMATTED_FILES need formatting"
        
    else
        echo "Checking C code formatting (non-blocking)..."
        while IFS= read -r file; do
            if [ -f "$file" ]; then
                TOTAL_FILES=$((TOTAL_FILES + 1))
                if ! clang-format --dry-run "$file" > /dev/null 2>&1; then
                    echo "  ⚠ $file"
                    UNFORMATTED_FILES=$((UNFORMATTED_FILES + 1))
                fi
            fi
        done < c_files.txt
        echo "Checked $TOTAL_FILES C files, $UNFORMATTED_FILES need formatting"
        echo "Run with --fix to auto-correct"
    fi
    
    rm -f c_files.txt
else
    echo "WARNING: clang-format not found"
    echo "  macOS: brew install llvm"
    echo "  Ubuntu: sudo apt install clang-format"
fi

echo ""

# =============================================================================
# Swift Code Formatting (SwiftLint --fix)
# =============================================================================
echo "------------------------------------------"
echo "Swift Code Formatting"
echo "------------------------------------------"

if command -v swiftlint &> /dev/null; then
    cd "$PROJECT_ROOT/ixland-app"
    
    if [ "$FIX_MODE" = true ]; then
        echo "Auto-fixing Swift code..."
        swiftlint --fix
        echo "Swift code formatted successfully"
        
    elif [ "$CHECK_MODE" = true ]; then
        echo "Checking Swift code formatting..."
        if ! swiftlint lint --strict; then
            EXIT_CODE=1
        fi
        
    else
        echo "Checking Swift code formatting (non-blocking)..."
        swiftlint lint || true
        echo "Run with --fix to auto-correct"
    fi
    
    cd "$PROJECT_ROOT"
else
    echo "WARNING: SwiftLint not found"
    echo "  macOS: brew install swiftlint"
fi

echo ""

# =============================================================================
# Shell Script Formatting (shfmt)
# =============================================================================
echo "------------------------------------------"
echo "Shell Script Formatting"
echo "------------------------------------------"

if command -v shfmt &> /dev/null; then
    if [ "$FIX_MODE" = true ]; then
        echo "Auto-fixing shell script formatting..."
        find . -name "*.sh" -not -path "./ixland-app/Resources/*" -not -path "./.git/*" -not -path "./.dolt/*" | while read -r file; do
            if [ -f "$file" ]; then
                shfmt -w -i 4 "$file"
                echo "  ✓ $file"
            fi
        done
        echo "Shell scripts formatted successfully"
        
    elif [ "$CHECK_MODE" = true ]; then
        echo "Checking shell script formatting..."
        find . -name "*.sh" -not -path "./ixland-app/Resources/*" -not -path "./.git/*" -not -path "./.dolt/*" | while read -r file; do
            if [ -f "$file" ]; then
                if ! shfmt -d -i 4 "$file" > /dev/null 2>&1; then
                    echo "  ❌ $file"
                    EXIT_CODE=1
                fi
            fi
        done
        
    else
        echo "Checking shell script formatting (non-blocking)..."
        find . -name "*.sh" -not -path "./ixland-app/Resources/*" -not -path "./.git/*" -not -path "./.dolt/*" | head -5 | while read -r file; do
            if [ -f "$file" ]; then
                if ! shfmt -d -i 4 "$file" > /dev/null 2>&1; then
                    echo "  ⚠ $file"
                fi
            fi
        done
        echo "Run with --fix to auto-correct"
    fi
else
    echo "INFO: shfmt not found (optional)"
    echo "  macOS: brew install shfmt"
    echo "  Ubuntu: sudo apt install shfmt"
fi

echo ""

# =============================================================================
# Summary
# =============================================================================
echo "=========================================="
echo "Formatting Complete"
echo "=========================================="

if [ "$FIX_MODE" = true ]; then
    echo "✅ All formatting fixes applied"
    echo "Review changes with: git diff"
elif [ "$CHECK_MODE" = true ]; then
    if [ $EXIT_CODE -eq 0 ]; then
        echo "✅ All formatting checks passed!"
    else
        echo "❌ Some files need formatting"
        echo "Run './scripts/format.sh --fix' to auto-correct"
    fi
else
    echo "Formatting check complete (non-blocking)"
    echo "Use --check for CI enforcement"
    echo "Use --fix to auto-correct issues"
fi

exit $EXIT_CODE

#!/bin/bash
# Dead Code Detection Script for iXland
# Detects unused/dead code across all components
#
# Usage:
#   ./scripts/check-dead-code.sh           # Check for dead code
#   ./scripts/check-dead-code.sh --ci      # CI mode (exit with error if issues found)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

CI_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --ci)
            CI_MODE=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--ci]"
            exit 1
            ;;
    esac
done

echo "=========================================="
echo "Dead Code Detection"
echo "=========================================="
echo ""

EXIT_CODE=0
TOTAL_ISSUES=0

# =============================================================================
# C Code Dead Code Detection (via clang-tidy)
# =============================================================================
echo "------------------------------------------"
echo "C Code Dead Code Detection"
echo "------------------------------------------"

if command -v clang-tidy &> /dev/null; then
    echo "Running clang-tidy with dead code checks..."

    # Find all C source files
    find ixland-libc ixland-system ixland-wasm ixland-packages ixland-toolchain \
        -type f \( -name "*.c" -o -name "*.h" \) \
        -not -path "*/wamr/*" \
        -not -path "*/node_modules/*" \
        -not -path "*/Resources/*" \
        -not -path "*/.build/*" \
        -not -path "*/build/*" \
        > /tmp/c_dead_code_files.txt

    ISSUES=0
    while IFS= read -r file; do
        if [ -f "$file" ]; then
            # Determine include paths based on file location
            INCLUDES=""
            if [[ "$file" == *"ixland-libc"* ]]; then
                INCLUDES="-I./ixland-libc/include"
            elif [[ "$file" == *"ixland-system"* ]]; then
                INCLUDES="-I./ixland-system/include -I./ixland-system/src/ixland/internal"
            elif [[ "$file" == *"ixland-wasm"* ]]; then
                INCLUDES="-I./ixland-wasm/include"
            fi

            # Run clang-tidy with dead code checks
            OUTPUT=$(clang-tidy "$file" \
                --checks='misc-unused-parameters,misc-unused-alias-decls,misc-unused-using-decls,readability-delete-null-pointer,readability-redundant-*,cppcoreguidelines-*-unused-parameters' \
                -- $INCLUDES 2>/dev/null) || true

            if echo "$OUTPUT" | grep -q "warning:"; then
                echo "  ⚠ Dead code issues in: $file"
                echo "$OUTPUT" | grep "warning:" | head -3
                ISSUES=$((ISSUES + 1))
            fi
        fi
    done < /tmp/c_dead_code_files.txt

    echo ""
    echo "C code dead code check: $(wc -l < /tmp/c_dead_code_files.txt) files checked, $ISSUES files with dead code"
    TOTAL_ISSUES=$((TOTAL_ISSUES + ISSUES))
    rm -f /tmp/c_dead_code_files.txt
else
    echo "WARNING: clang-tidy not found. Install with: brew install llvm (macOS) or apt install clang-tidy (Ubuntu)"
fi

echo ""

# =============================================================================
# Shell Script Dead Code Detection
# =============================================================================
echo "------------------------------------------"
echo "Shell Script Dead Code Detection"
echo "------------------------------------------"

if command -v shellcheck &> /dev/null; then
    echo "Checking for unused variables in shell scripts..."

    find . -name "*.sh" -not -path "./ixland-app/Resources/*" -not -path "./.git/*" -not -path "./.dolt/*" | while read -r file; do
        if [ -f "$file" ]; then
            OUTPUT=$(shellcheck -S warning "$file" 2>/dev/null) || true
            if echo "$OUTPUT" | grep -q "SC2034"; then
                echo "  ⚠ Unused variables in: $file"
            fi
        fi
    done
    echo "Shell script dead code check complete"
else
    echo "INFO: ShellCheck not found (optional for dead code detection)"
fi

echo ""

# =============================================================================
# Swift Dead Code Detection
# =============================================================================
echo "------------------------------------------"
echo "Swift Dead Code Detection"
echo "------------------------------------------"

if command -v swiftlint &> /dev/null; then
    cd "$PROJECT_ROOT/ixland-app"
    echo "Running SwiftLint for dead code detection..."
    swiftlint lint 2>/dev/null | grep -E "(unused|dead)" || echo "No obvious dead code detected"
    cd "$PROJECT_ROOT"
else
    echo "INFO: SwiftLint not found (optional for dead code detection)"
fi

echo ""

# =============================================================================
# Summary
# =============================================================================
echo "=========================================="
echo "Dead Code Detection Complete"
echo "=========================================="

if [ $TOTAL_ISSUES -eq 0 ]; then
    echo "✅ No dead code detected!"
else
    echo "⚠ Found potential dead code in $TOTAL_ISSUES files"
    if [ "$CI_MODE" = true ]; then
        EXIT_CODE=1
    fi
fi

if [ "$CI_MODE" = false ]; then
    echo ""
    echo "Use --ci flag to enforce in CI"
fi

exit $EXIT_CODE

#!/bin/bash
# iXland Naming Convention Checker
# Validates that code follows the project's naming conventions
#
# Usage:
#   ./scripts/check-naming.sh              # Check naming conventions
#   ./scripts/check-naming.sh --strict     # Exit with error on violations
#   ./scripts/check-naming.sh --fix        # Show fix suggestions

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

STRICT_MODE=false
FIX_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --strict)
            STRICT_MODE=true
            shift
            ;;
        --fix)
            FIX_MODE=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--strict|--fix]"
            exit 1
            ;;
    esac
done

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "iXland Naming Convention Checker"
echo "=========================================="
echo ""

EXIT_CODE=0
VIOLATIONS=0

# =============================================================================
# C Code Naming Conventions
# =============================================================================
check_c_naming() {
    echo "------------------------------------------"
    echo "C Code Naming Conventions"
    echo "------------------------------------------"

    if ! command -v clang-tidy &> /dev/null; then
        echo -e "${YELLOW}WARNING: clang-tidy not found${NC}"
        echo "Install with: brew install llvm"
        return 0
    fi

    echo "Checking iox_ prefix enforcement..."

    # Find all C source files
    find ixland-system ixland-libc ixland-wasm \
        -type f \( -name "*.c" -o -name "*.h" \) \
        -not -path "*/wamr/*" \
        -not -path "*/node_modules/*" \
        -not -path "*/Resources/*" \
        -not -path "*/.build/*" \
        > c_files.txt

    local checked=0
    local issues=0

    while IFS= read -r file; do
        if [ -f "$file" ]; then
            checked=$((checked + 1))

            # Run clang-tidy with only naming checks
            local output
            output=$(clang-tidy "$file" --checks='readability-identifier-naming' -- -I./ixland-libc/include 2>/dev/null) || true

            if echo "$output" | grep -q "warning:"; then
                if [ "$FIX_MODE" = true ]; then
                    echo ""
                    echo -e "${YELLOW}File: $file${NC}"
                    echo "$output" | grep "warning:" | head -5
                fi
                issues=$((issues + 1))
            fi
        fi
    done < c_files.txt

    rm -f c_files.txt

    if [ $issues -eq 0 ]; then
        echo -e "  ${GREEN}✓ All $checked files follow naming conventions${NC}"
    else
        echo -e "  ${YELLOW}⚠ Found naming issues in $issues files${NC}"
        VIOLATIONS=$((VIOLATIONS + issues))
        EXIT_CODE=1
    fi

    echo ""
    echo "C Naming Standards:"
    echo "  • Functions: iox_lowercase() (e.g., iox_task_alloc)"
    echo "  • Types/Structs: iox_lowercase_t (e.g., iox_task_t)"
    echo "  • Macros/Constants: IOX_UPPER_CASE (e.g., IOX_MAX_NAME)"
    echo "  • Enums: iox_enum_name with IOX_ENUM_VALUE"
    echo "  • Global variables: iox_lowercase"
}

# =============================================================================
# Swift Code Naming Conventions
# =============================================================================
check_swift_naming() {
    echo "------------------------------------------"
    echo "Swift Code Naming Conventions"
    echo "------------------------------------------"

    if ! command -v swiftlint &> /dev/null; then
        echo -e "${YELLOW}WARNING: SwiftLint not found${NC}"
        echo "Install with: brew install swiftlint"
        return 0
    fi

    cd "$PROJECT_ROOT/ixland-app"

    echo "Checking Swift naming conventions..."

    local output
    local issues=0

    # Run SwiftLint with identifier_name rule
    output=$(swiftlint lint --only-rule identifier_name 2>/dev/null) || true

    if echo "$output" | grep -q "warning\|error"; then
        issues=$(echo "$output" | grep -c "warning\|error" || echo 0)
        if [ "$FIX_MODE" = true ]; then
            echo "$output" | grep "warning\|error" | head -10
        fi
    fi

    cd "$PROJECT_ROOT"

    if [ "$issues" -eq 0 ]; then
        echo -e "  ${GREEN}✓ Swift naming conventions followed${NC}"
    else
        echo -e "  ${YELLOW}⚠ Found $issues naming issues${NC}"
        VIOLATIONS=$((VIOLATIONS + issues))
        EXIT_CODE=1
    fi

    echo ""
    echo "Swift Naming Standards:"
    echo "  • Types: PascalCase (e.g., ContentView, TaskManager)"
    echo "  • Functions/Variables: camelCase (e.g., loadView, taskCount)"
    echo "  • Constants: camelCase or PascalCase for enum cases"
    echo "  • Private properties: leading underscore discouraged"
}

# =============================================================================
# Manual Checks for Common Violations
# =============================================================================
check_manual_patterns() {
    echo "------------------------------------------"
    echo "Manual Pattern Checks"
    echo "------------------------------------------"

    local issues=0

    echo "Checking for functions without iox_ prefix in public APIs..."

    # Find potential public functions that should have iox_ prefix
    # This is a heuristic check for common patterns
    find ixland-libc/src ixland-system/kernel ixland-system/runtime \
        -name "*.c" -not -path "*/wamr/*" \
        -exec grep -l "^[a-z_]*_t\s*\*" {} \; 2>/dev/null | while read -r file; do
        # Check for function definitions without iox_ prefix
        grep -n "^[a-zA-Z_][a-zA-Z0-9_]*\s*(" "$file" 2>/dev/null | \
            grep -v "^\s*//" | \
            grep -v "^\s*/\*" | \
            grep -v "static\|inline\|typedef" | \
            grep -v "^\s*if\|else\|while\|for\|switch\|return" | \
            grep -v "ios_\|ixland_\|iox_\|main\|test_" | \
            head -5 > /tmp/potential_violations.txt || true

        if [ -s /tmp/potential_violations.txt ]; then
            if [ "$FIX_MODE" = true ]; then
                echo -e "${YELLOW}Potential issues in $file:${NC}"
                cat /tmp/potential_violations.txt
            fi
            issues=$((issues + 1))
        fi
    done

    rm -f /tmp/potential_violations.txt

    if [ $issues -eq 0 ]; then
        echo -e "  ${GREEN}✓ No obvious naming violations found${NC}"
    else
        echo -e "  ${YELLOW}⚠ Found $issues potential violations${NC}"
    fi
}

# =============================================================================
# Summary
# =============================================================================

# Run checks
check_c_naming
echo ""

check_swift_naming
echo ""

check_manual_patterns
echo ""

echo "=========================================="
echo "Naming Convention Check Complete"
echo "=========================================="

if [ $VIOLATIONS -eq 0 ]; then
    echo -e "${GREEN}✅ All naming conventions followed!${NC}"
else
    echo -e "${YELLOW}⚠ Found $VIOLATIONS naming violation(s)${NC}"
    echo ""
    echo "Naming Convention Reference:"
    echo "  C Code:"
    echo "    • Public functions: iox_function_name()"
    echo "    • Types: iox_type_name_t"
    echo "    • Constants: IOX_CONSTANT_NAME"
    echo "    • Macros: IOX_MACRO_NAME"
    echo ""
    echo "  Swift Code:"
    echo "    • Types: PascalCase (MyClass)"
    echo "    • Functions/Vars: camelCase (myFunction)"
    echo ""
    echo "See AGENTS.md for full naming conventions."
fi

if [ "$STRICT_MODE" = true ]; then
    exit $EXIT_CODE
else
    echo ""
    echo "Use --strict to exit with error on violations"
    exit 0
fi

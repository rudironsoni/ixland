#!/bin/bash
# Technical Debt Scanner for iXland
# Scans for TODO/FIXME comments and enforces linking to issues
#
# Usage:
#   ./scripts/check-tech-debt.sh           # Check for unlinked technical debt
#   ./scripts/check-tech-debt.sh --ci      # CI mode (exit with error if issues found)

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
echo "Technical Debt Scanner"
echo "=========================================="
echo ""

EXIT_CODE=0
UNLINKED_TODOS=0
UNLINKED_FIXMES=0

# Valid TODO/FIXME patterns that include issue references
# Valid: TODO(TICKET-123), TODO(#123), TODO: See issue #123, FIXME(bd-123)
VALID_PATTERN='TODO\s*\([A-Za-z0-9_\-#]+\)|FIXME\s*\([A-Za-z0-9_\-#]+\)|TODO.*#[0-9]+|FIXME.*#[0-9]+|TODO.*bd-[0-9]+|FIXME.*bd-[0-9]+'

# Patterns that indicate unlinked technical debt
# These are TODO/FIXME without any issue reference
UNLINKED_PATTERN='TODO[^\(]|FIXME[^\(]'

# =============================================================================
# C Code Technical Debt
# =============================================================================
echo "------------------------------------------"
echo "C Code Technical Debt Check"
echo "------------------------------------------"

echo "Scanning C files for TODO/FIXME comments..."

# Find all C files and scan for TODO/FIXME
TODO_COUNT=0
FIXME_COUNT=0
VALID_LINKED=0
UNLINKED_ISSUES=0

find ixland-libc ixland-system ixland-wasm ixland-packages ixland-toolchain \
    -type f \( -name "*.c" -o -name "*.h" \) \
    -not -path "*/wamr/*" \
    -not -path "*/node_modules/*" \
    -not -path "*/Resources/*" \
    -not -path "*/.build/*" \
    -not -path "*/build/*" \
    2>/dev/null | while read -r file; do

    if [ -f "$file" ]; then
        # Count total TODO/FIXME
        FILE_TODOS=$(grep -n "TODO\|FIXME" "$file" 2>/dev/null | wc -l || echo 0)

        if [ "$FILE_TODOS" -gt 0 ]; then
            # Check if they are properly linked
            while IFS= read -r line; do
                if echo "$line" | grep -qE "TODO|FIXME"; then
                    LINE_NUM=$(echo "$line" | cut -d: -f1)
                    LINE_CONTENT=$(echo "$line" | cut -d: -f2-)

                    # Check if it's properly linked to an issue
                    if echo "$LINE_CONTENT" | grep -qE "TODO\s*\([A-Za-z0-9_\-#]+\)|FIXME\s*\([A-Za-z0-9_\-#]+\)|TODO.*#[0-9]+|FIXME.*#[0-9]+|TODO.*bd-[0-9]+|FIXME.*bd-[0-9]+"; then
                        VALID_LINKED=$((VALID_LINKED + 1))
                    else
                        # Check if it's just "TODO:" or "FIXME:" without issue reference
                        if echo "$LINE_CONTENT" | grep -qE "TODO[^A-Za-z]|FIXME[^A-Za-z]|TODO$|FIXME$"; then
                            echo "  ⚠ Unlinked in $file:$LINE_NUM: $LINE_CONTENT"
                            UNLINKED_ISSUES=$((UNLINKED_ISSUES + 1))
                        fi
                    fi
                fi
            done < <(grep -n "TODO\|FIXME" "$file" 2>/dev/null || true)
        fi
    fi
done

echo "C code scan complete."
echo ""

# =============================================================================
# Shell Script Technical Debt
# =============================================================================
echo "------------------------------------------"
echo "Shell Script Technical Debt Check"
echo "------------------------------------------"

echo "Scanning shell scripts for TODO/FIXME comments..."

SHELL_TODOS=0
SHELL_UNLINKED=0

find . -name "*.sh" -not -path "./ixland-app/Resources/*" -not -path "./.git/*" -not -path "./.dolt/*" 2>/dev/null | while read -r file; do
    if [ -f "$file" ]; then
        # Check for TODO/FIXME in shell scripts
        while IFS= read -r line; do
            if echo "$line" | grep -qE "^\s*#.*TODO|^\s*#.*FIXME"; then
                SHELL_TODOS=$((SHELL_TODOS + 1))

                # Check if linked to issue
                if ! echo "$line" | grep -qE "TODO\s*\([A-Za-z0-9_\-#]+\)|FIXME\s*\([A-Za-z0-9_\-#]+\)|TODO.*#[0-9]+|FIXME.*#[0-9]+|TODO.*bd-[0-9]+|FIXME.*bd-[0-9]+"; then
                    LINE_NUM=$(echo "$line" | cut -d: -f1)
                    echo "  ⚠ Unlinked TODO/FIXME in $file: $line"
                    SHELL_UNLINKED=$((SHELL_UNLINKED + 1))
                fi
            fi
        done < <(grep -n "TODO\|FIXME" "$file" 2>/dev/null || true)
    fi
done

echo "Shell script scan complete."
echo ""

# =============================================================================
# Swift Code Technical Debt
# =============================================================================
echo "------------------------------------------"
echo "Swift Code Technical Debt Check"
echo "------------------------------------------"

echo "Scanning Swift files for TODO/FIXME comments..."

SWIFT_TODOS=0
SWIFT_UNLINKED=0

find ixland-app -name "*.swift" -not -path "*/Resources/*" -not -path "*/.build/*" 2>/dev/null | while read -r file; do
    if [ -f "$file" ]; then
        while IFS= read -r line; do
            if echo "$line" | grep -qE "//.*TODO|//.*FIXME|/\*.*TODO|/\*.*FIXME"; then
                SWIFT_TODOS=$((SWIFT_TODOS + 1))

                if ! echo "$line" | grep -qE "TODO\s*\([A-Za-z0-9_\-#]+\)|FIXME\s*\([A-Za-z0-9_\-#]+\)|TODO.*#[0-9]+|FIXME.*#[0-9]+|TODO.*bd-[0-9]+|FIXME.*bd-[0-9]+"; then
                    LINE_NUM=$(echo "$line" | cut -d: -f1)
                    echo "  ⚠ Unlinked TODO/FIXME in $file: $line"
                    SWIFT_UNLINKED=$((SWIFT_UNLINKED + 1))
                fi
            fi
        done < <(grep -n "TODO\|FIXME" "$file" 2>/dev/null || true)
    fi
done

echo "Swift code scan complete."
echo ""

# =============================================================================
# Summary
# =============================================================================
echo "=========================================="
echo "Technical Debt Scan Complete"
echo "=========================================="

TOTAL_UNLINKED=$((UNLINKED_ISSUES + SHELL_UNLINKED + SWIFT_UNLINKED))

if [ $TOTAL_UNLINKED -eq 0 ]; then
    echo "✅ All TODO/FIXME comments are properly linked to issues!"
    echo ""
    echo "Accepted formats:"
    echo "  - TODO(TICKET-123)"
    echo "  - FIXME(#456)"
    echo "  - TODO: See issue #123"
    echo "  - FIXME(bd-789)"
else
    echo "⚠ Found $TOTAL_UNLINKED unlinked TODO/FIXME comments"
    echo ""
    echo "Please link technical debt to issues using one of these formats:"
    echo "  - TODO(TICKET-123) or FIXME(TICKET-123)"
    echo "  - TODO(#456) or FIXME(#456)"
    echo "  - TODO: See issue #123 or FIXME: See issue #123"
    echo "  - TODO(bd-789) or FIXME(bd-789)"
    echo ""
    if [ "$CI_MODE" = true ]; then
        EXIT_CODE=1
    fi
fi

exit $EXIT_CODE

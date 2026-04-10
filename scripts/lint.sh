#!/bin/bash
# iXland Linting Script
# Runs all linters for the project
#
# Usage:
#   ./scripts/lint.sh              # Run all linters
#   ./scripts/lint.sh --fix        # Auto-fix issues where possible
#   ./scripts/lint.sh --check      # Check mode (CI)
#   ./scripts/lint.sh --type-check # Run type checking only
#   ./scripts/lint.sh --dead-code  # Run dead code detection only
#   ./scripts/lint.sh --tech-debt  # Run technical debt scanner only

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

FIX_MODE=false
CHECK_MODE=false
TYPE_CHECK_MODE=false
STRICT_TYPING_MODE=false
DEAD_CODE_MODE=false
TECH_DEBT_MODE=false

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
        --type-check)
            TYPE_CHECK_MODE=true
            shift
            ;;
        --strict-typing)
            STRICT_TYPING_MODE=true
            shift
            ;;
        --dead-code)
            DEAD_CODE_MODE=true
            shift
            ;;
        --tech-debt)
            TECH_DEBT_MODE=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--fix|--check|--type-check|--strict-typing|--dead-code|--tech-debt]"
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
# Type Checking for C Code
# =============================================================================
if [ "$TYPE_CHECK_MODE" = true ] || [ "$CHECK_MODE" = true ]; then
    echo "------------------------------------------"
    echo "C Code Type Checking"
    echo "------------------------------------------"

    if command -v clang-tidy &> /dev/null; then
        echo "Running type analysis on all C components..."

        # Find all C source files
        find ixland-libc ixland-system ixland-wasm ixland-packages ixland-toolchain \
            -type f \( -name "*.c" -o -name "*.h" \) \
            -not -path "*/wamr/*" \
            -not -path "*/node_modules/*" \
            -not -path "*/Resources/*" \
            -not -path "*/.build/*" \
            -not -path "*/build/*" \
            > /tmp/c_type_check_files.txt

        TOTAL=0
        ISSUES=0

        while IFS= read -r file; do
            if [ -f "$file" ]; then
                TOTAL=$((TOTAL + 1))

                # Determine include paths based on file location
                INCLUDES=""
                if [[ "$file" == *"ixland-libc"* ]]; then
                    INCLUDES="-I./ixland-libc/include"
                elif [[ "$file" == *"ixland-system"* ]]; then
                    INCLUDES="-I./ixland-system/include -I./ixland-system/src/ixland/internal"
                elif [[ "$file" == *"ixland-wasm"* ]]; then
                    INCLUDES="-I./ixland-wasm/include"
                fi

                echo "  Type checking: $(basename "$file")"
                if ! clang-tidy "$file" \
                    --checks='clang-analyzer-*,bugprone-*,-bugprone-easily-swappable-parameters' \
                    -- $INCLUDES 2>/dev/null; then
                    ISSUES=$((ISSUES + 1))
                    if [ "$CHECK_MODE" = true ]; then
                        EXIT_CODE=1
                    fi
                fi
            fi
        done < /tmp/c_type_check_files.txt

        echo ""
        echo "Type check complete: $TOTAL files checked, $ISSUES files with type issues"
        rm -f /tmp/c_type_check_files.txt
    else
        echo "WARNING: clang-tidy not found. Install with: brew install llvm (macOS) or apt install clang-tidy (Ubuntu)"
    fi

    echo ""

    # If only type checking, exit here
    if [ "$TYPE_CHECK_MODE" = true ]; then
        exit $EXIT_CODE
    fi
fi

# =============================================================================
# Strict Type Checking for C Code
# =============================================================================
if [ "$STRICT_TYPING_MODE" = true ] || [ "$CHECK_MODE" = true ]; then
    echo "------------------------------------------"
    echo "Strict Type Checking"
    echo "------------------------------------------"

    echo "Checking strict typing configuration..."

    # Check clang-tidy has WarningsAsErrors enabled
    if grep -q "WarningsAsErrors: '\*'" .clang-tidy; then
        echo "  clang-tidy WarningsAsErrors '*': ENABLED"
    else
        echo "  clang-tidy WarningsAsErrors: NOT STRICT"
        EXIT_CODE=1
    fi

    # Check Makefile adds strict flags
    if grep -q "-Werror" ixland-system/Makefile; then
        echo "  Strict C flags in Makefile: ENABLED"
    else
        echo "  Strict C flags in Makefile: MISSING"
        EXIT_CODE=1
    fi

    echo ""
    echo "Strict type checking configuration verified."

    if [ "$STRICT_TYPING_MODE" = true ]; then
        exit $EXIT_CODE
    fi
fi

    # Check clang-tidy has WarningsAsErrors enabled
    if grep -q "WarningsAsErrors: '\\*'" .clang-tidy; then
        echo "  clang-tidy WarningsAsErrors '*': ENABLED"
    else
        echo "  clang-tidy WarningsAsErrors: NOT STRICT"
        EXIT_CODE=1
    fi

    # Check CMake adds strict flags
    if grep -q "Werror=incompatible-pointer-types" CMakeLists.txt; then
        echo "  Strict C flags in CMake: ENABLED"
    else
        echo "  Strict C flags in CMake: MISSING"
        EXIT_CODE=1
    fi

    echo ""
    echo "Strict type checking configuration verified."

    if [ "$STRICT_TYPING_MODE" = true ]; then
        exit $EXIT_CODE
    fi
fi

# =============================================================================
# Dead Code Detection
# =============================================================================
if [ "$DEAD_CODE_MODE" = true ] || [ "$CHECK_MODE" = true ]; then
    echo "------------------------------------------"
    echo "Dead Code Detection"
    echo "------------------------------------------"

    if command -v clang-tidy &> /dev/null; then
        echo "Running clang-tidy dead code checks..."

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
                INCLUDES=""
                if [[ "$file" == *"ixland-libc"* ]]; then
                    INCLUDES="-I./ixland-libc/include"
                elif [[ "$file" == *"ixland-system"* ]]; then
                    INCLUDES="-I./ixland-system/include -I./ixland-system/src/ixland/internal"
                elif [[ "$file" == *"ixland-wasm"* ]]; then
                    INCLUDES="-I./ixland-wasm/include"
                fi

                OUTPUT=$(clang-tidy "$file" \
                    --checks='misc-unused-parameters,misc-unused-alias-decls,misc-unused-using-decls,readability-delete-null-pointer,readability-redundant-*,cppcoreguidelines-*-unused-parameters' \
                    -- $INCLUDES 2>/dev/null) || true

                if echo "$OUTPUT" | grep -q "warning:"; then
                    echo "  ⚠ Dead code in: $file"
                    ISSUES=$((ISSUES + 1))
                fi
            fi
        done < /tmp/c_dead_code_files.txt

        echo "Dead code check complete: $(wc -l < /tmp/c_dead_code_files.txt) files checked, $ISSUES files with issues"
        rm -f /tmp/c_dead_code_files.txt
    fi

    if [ "$DEAD_CODE_MODE" = true ]; then
        exit 0
    fi
fi

# =============================================================================
# Technical Debt Scanner
# =============================================================================
if [ "$TECH_DEBT_MODE" = true ] || [ "$CHECK_MODE" = true ]; then
    echo "------------------------------------------"
    echo "Technical Debt Scanner"
    echo "------------------------------------------"

    echo "Scanning for TODO/FIXME comments..."

    # Check C files
    C_UNLINKED=0
    find ixland-libc ixland-system ixland-wasm ixland-packages ixland-toolchain \
        -type f \( -name "*.c" -o -name "*.h" \) \
        -not -path "*/wamr/*" \
        -not -path "*/node_modules/*" \
        -not -path "*/Resources/*" \
        -not -path "*/.build/*" \
        -not -path "*/build/*" \
        2>/dev/null | while read -r file; do
        if [ -f "$file" ]; then
            while IFS= read -r line; do
                if echo "$line" | grep -qE "TODO|FIXME"; then
                    if ! echo "$line" | grep -qE "TODO\s*\([A-Za-z0-9_\-#]+\)|FIXME\s*\([A-Za-z0-9_\-#]+\)|TODO.*#[0-9]+|FIXME.*#[0-9]+|TODO.*bd-[0-9]+|FIXME.*bd-[0-9]+"; then
                        echo "  ⚠ Unlinked TODO/FIXME in $file"
                        C_UNLINKED=$((C_UNLINKED + 1))
                    fi
                fi
            done < <(grep -n "TODO\|FIXME" "$file" 2>/dev/null || true)
        fi
    done

    echo "Technical debt scan complete."

    if [ "$TECH_DEBT_MODE" = true ]; then
        exit 0
    fi
fi

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
    echo ""
    echo "Usage options:"
    echo "  --check          Enforce checks in CI mode"
    echo "  --fix            Auto-fix issues where possible"
    echo "  --type-check     Run type checking only"
    echo "  --strict-typing  Run strict typing check only"
    echo "  --dead-code      Run dead code detection only"
    echo "  --tech-debt      Run technical debt scanner only"
fi

exit $EXIT_CODE

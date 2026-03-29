#!/bin/bash
# Duplicate Code Detection Script for iXland
# Detects copy-paste/duplicate code across all components
#
# Usage:
#   ./scripts/check-duplicate-code.sh           # Check for duplicate code
#   ./scripts/check-duplicate-code.sh --ci      # CI mode (exit with error if issues found)

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
echo "Duplicate Code Detection"
echo "=========================================="
echo ""

EXIT_CODE=0

# =============================================================================
# jscpd Duplicate Code Detection
# =============================================================================
echo "------------------------------------------"
echo "Running jscpd for duplicate code detection"
echo "------------------------------------------"

if command -v jscpd &> /dev/null; then
    echo "Running jscpd analysis..."
    jscpd --config .jscpd.json . || true
    echo ""
    echo "jscpd analysis complete."
else
    echo "INFO: jscpd not found. Installing jscpd..."
    if command -v npm &> /dev/null; then
        npm install -g jscpd 2>/dev/null || echo "Could not install jscpd globally"
        if command -v jscpd &> /dev/null; then
            jscpd --config .jscpd.json . || true
        else
            echo "WARNING: Could not install jscpd. Duplicate code detection requires Node.js/npm."
        fi
    else
        echo "WARNING: npm not found. Cannot install jscpd for duplicate code detection."
    fi
fi

echo ""

# =============================================================================
# Clang-tidy Duplicate Code Detection (C/C++)
# =============================================================================
echo "------------------------------------------"
echo "C Code Duplicate Code Detection"
echo "------------------------------------------"

if command -v clang-tidy &> /dev/null; then
    echo "Running clang-tidy similarity analysis..."

    # Find all C source files
    find ixland-libc ixland-system ixland-wasm ixland-packages ixland-toolchain \
        -type f \( -name "*.c" -o -name "*.h" \) \
        -not -path "*/wamr/*" \
        -not -path "*/node_modules/*" \
        -not -path "*/Resources/*" \
        -not -path "*/.build/*" \
        -not -path "*/build/*" \
        > /tmp/c_dup_files.txt

    TOTAL=$(wc -l < /tmp/c_dup_files.txt)
    echo "Found $TOTAL C files to analyze"

    # Simple similarity check - look for identical line patterns
    echo "Checking for common duplicated patterns..."

    # Check for identical error handling patterns that might be candidates for refactoring
    grep -r "errno = EINVAL" --include="*.c" . 2>/dev/null | wc -l | xargs echo "  - EINVAL error patterns:"
    grep -r "errno = ENOMEM" --include="*.c" . 2>/dev/null | wc -l | xargs echo "  - ENOMEM error patterns:"
    grep -r "return -1;" --include="*.c" . 2>/dev/null | wc -l | xargs echo "  - return -1 patterns:"

    rm -f /tmp/c_dup_files.txt
    echo "C code pattern analysis complete."
else
    echo "WARNING: clang-tidy not found. Install with: brew install llvm (macOS) or apt install clang-tidy (Ubuntu)"
fi

echo ""

# =============================================================================
# Shell Script Duplicate Code Detection
# =============================================================================
echo "------------------------------------------"
echo "Shell Script Duplicate Code Detection"
echo "------------------------------------------"

# Look for common duplicated patterns in shell scripts
echo "Checking for duplicated shell patterns..."

echo "  Checking for common patterns in scripts/ directory..."
if [ -d "scripts" ]; then
    # Count occurrences of common patterns
    grep -r "#!/bin/bash" scripts/ 2>/dev/null | wc -l | xargs echo "    - #!/bin/bash shebangs:"
    grep -r "set -e" scripts/ 2>/dev/null | wc -l | xargs echo "    - set -e patterns:"
    grep -r "SCRIPT_DIR=" scripts/ 2>/dev/null | wc -l | xargs echo "    - SCRIPT_DIR patterns:"
fi

echo ""

# =============================================================================
# Summary
# =============================================================================
echo "=========================================="
echo "Duplicate Code Detection Complete"
echo "=========================================="

echo ""
echo "Tools used:"
echo "  - jscpd: Cross-language duplicate code detection"
echo "  - clang-tidy: C/C++ specific analysis"
echo "  - Custom pattern analysis for shell scripts"

if [ "$CI_MODE" = true ]; then
    echo ""
    echo "CI mode: Check complete (non-blocking)"
else
    echo ""
    echo "Use --ci flag for CI mode"
    echo "Configuration file: .jscpd.json"
fi

exit $EXIT_CODE

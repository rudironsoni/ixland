#!/bin/bash
# test-all.sh - Run tests for all core packages
#
# Usage: ./test-all.sh [--target simulator|ios|universal] [--continue-on-failure]
#
# This script runs tests for all packages in dependency order

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
A_SHELL_PACKAGES_DIR="$SCRIPT_DIR/.."
TARGET="simulator"
CONTINUE_ON_FAILURE=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

log_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --target|-t)
            TARGET="$2"
            shift 2
            ;;
        --continue-on-failure)
            CONTINUE_ON_FAILURE=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--target simulator|ios|universal] [--continue-on-failure]"
            echo ""
            echo "Run tests for all core packages"
            echo ""
            echo "Options:"
            echo "  --target               Target platform (default: simulator)"
            echo "  --continue-on-failure  Continue testing even if some packages fail"
            echo ""
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Define packages in dependency order
# Wave 1: Foundation
WAVE1=("libz")

# Wave 2: Crypto
WAVE2=("libssl")

# Wave 3: Terminal libraries  
WAVE3=("ncurses" "readline")

# Wave 4: Tools
WAVE4=("libcurl" "bash" "coreutils" "make")

# Wave 5: High-level tools
WAVE5=("git" "vim")

# Combine all packages
ALL_PACKAGES=(${WAVE1[@]} ${WAVE2[@]} ${WAVE3[@]} ${WAVE4[@]} ${WAVE5[@]})

log_step "Running tests for all packages"
log_info "Target: $TARGET"
log_info "Packages: ${#ALL_PACKAGES[@]}"
echo ""

# Create results directory
RESULTS_DIR="$A_SHELL_PACKAGES_DIR/.build/test-results"
mkdir -p "$RESULTS_DIR"

# Track results
passed_packages=()
failed_packages=()
skipped_packages=()

# Run tests for each package
for pkg in "${ALL_PACKAGES[@]}"; do
    log_step "Testing package: $pkg"
    
    if [ -f "$A_SHELL_PACKAGES_DIR/packages/core/$pkg/build.sh" ]; then
        if "$SCRIPT_DIR/test-package.sh" "$pkg" --target "$TARGET"; then
            passed_packages+=("$pkg")
            log_info "✓ $pkg passed"
        else
            failed_packages+=("$pkg")
            log_error "✗ $pkg failed"
            
            if [ "$CONTINUE_ON_FAILURE" != "true" ]; then
                log_error "Stopping due to failure. Use --continue-on-failure to continue."
                break
            fi
        fi
    else
        skipped_packages+=("$pkg")
        log_warn "⚠ $pkg skipped (no build.sh)"
    fi
    
    echo ""
done

# Generate summary
summary_file="$RESULTS_DIR/summary-$(date +%Y%m%d-%H%M%S).json"

cat > "$summary_file" <<EOF
{
  "timestamp": "$(date -u +"%Y-%m-%dT%H:%M:%SZ")",
  "target": "$TARGET",
  "summary": {
    "total": ${#ALL_PACKAGES[@]},
    "passed": ${#passed_packages[@]},
    "failed": ${#failed_packages[@]},
    "skipped": ${#skipped_packages[@]}
  },
  "passed": [$(printf '"%s",' "${passed_packages[@]}" | sed 's/,$//')],
  "failed": [$(printf '"%s",' "${failed_packages[@]}" | sed 's/,$//')],
  "skipped": [$(printf '"%s",' "${skipped_packages[@]}" | sed 's/,$//')]
}
EOF

# Print final summary
echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo "Total:   ${#ALL_PACKAGES[@]}"
echo "Passed:  ${#passed_packages[@]}"
echo "Failed:  ${#failed_packages[@]}"
echo "Skipped: ${#skipped_packages[@]}"
echo ""

if [ ${#passed_packages[@]} -gt 0 ]; then
    echo "Passed packages:"
    for pkg in "${passed_packages[@]}"; do
        echo "  ✓ $pkg"
    done
fi

if [ ${#failed_packages[@]} -gt 0 ]; then
    echo ""
    echo "Failed packages:"
    for pkg in "${failed_packages[@]}"; do
        echo "  ✗ $pkg"
    done
fi

if [ ${#skipped_packages[@]} -gt 0 ]; then
    echo ""
    echo "Skipped packages:"
    for pkg in "${skipped_packages[@]}"; do
        echo "  ⚠ $pkg"
    done
fi

echo ""
echo "Summary saved to: $summary_file"
echo "========================================"

# Exit with appropriate code
if [ ${#failed_packages[@]} -gt 0 ]; then
    exit 1
else
    exit 0
fi

#!/bin/bash
# ashell-packages/scripts/post-process-wheel.sh
# Post-process Python wheels for iOS - converts .so files to frameworks

set -euo pipefail

# =============================================================================
# CONFIGURATION
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ASHELL_PREFIX="${ASHELL_PREFIX:-$HOME/Library/ashell}"
PYTHON_VERSION="${PYTHON_VERSION:-3.12}"

# =============================================================================
# FUNCTIONS
# =============================================================================

log_info() {
    echo "[INFO] $*" >&2
}

log_error() {
    echo "[ERROR] $*" >&2
}

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS] <wheel-file>

Post-process Python wheel for iOS installation.
Converts .so files to frameworks and repackages the wheel.

OPTIONS:
    -o, --output DIR    Output directory (default: current directory)
    -p, --python VER    Python version (default: $PYTHON_VERSION)
    -s, --sign ID       Sign frameworks with identity
    -v, --verbose       Verbose output
    -h, --help          Show this help

EXAMPLES:
    $(basename "$0") numpy-1.26.0-cp312-cp312-ios_13_0_arm64.whl
    $(basename "$0") -o ~/wheels -s "Apple Development" mypackage-1.0.whl
EOF
}

# Extract wheel contents
extract_wheel() {
    local wheel="$1"
    local dest="$2"

    log_info "Extracting wheel: $(basename "$wheel")"

    unzip -q "$wheel" -d "$dest"
}

# Find all .so files in extracted wheel
find_so_files() {
    local dir="$1"
    find "$dir" -name "*.so" -o -name "*.so.*" 2>/dev/null || true
}

# Get architecture from .so file (simplified for iOS)
get_architecture() {
    local so_file="$1"

    # Default to arm64 for iOS devices
    # In production, would use lipo/file to detect
    echo "arm64"
}

# Convert single .so to framework
convert_so_to_framework() {
    local so_file="$1"
    local output_dir="$2"
    local sign_identity="${3:-}"

    local so_name=$(basename "$so_file")
    local module_name="${so_name%.so}"
    # Handle .so.X.Y.Z versions
    module_name="${module_name%%.so*}"

    local framework_name="${module_name}.framework"
    local framework_path="$output_dir/$framework_name"

    log_info "Converting $so_name to $framework_name"

    # Create framework structure
    mkdir -p "$framework_path"

    # Copy binary
    cp "$so_file" "$framework_path/$module_name"
    chmod +x "$framework_path/$module_name"

    # Get minimum iOS version from so file if possible
    local min_ios="16.0"

    # Create Info.plist
    cat > "$framework_path/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>$module_name</string>
    <key>CFBundleIdentifier</key>
    <string>com.rudironsoni.ashell.python.$module_name</string>
    <key>CFBundleName</key>
    <string>$module_name</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>MinimumOSVersion</key>
    <string>$min_ios</string>
</dict>
</plist>
EOF

    # Sign if identity provided
    if [[ -n "$sign_identity" ]]; then
        codesign --sign "$sign_identity" \
                 --force \
                 --timestamp \
                 "$framework_path"
        log_info "Signed $framework_name"
    fi

    echo "$framework_path"
}

# Create module loader stub
create_loader_stub() {
    local module_name="$1"
    local output_file="$2"

    cat > "$output_file" <<EOF
# Auto-generated loader for $module_name
import sys
import os
import ctypes.util

# Framework path
_FRAMEWORK_PATH = os.path.join(
    os.environ.get('ASHELL_PREFIX', os.path.expanduser('~/Library/ashell')),
    'Frameworks',
    '${module_name}.framework',
    '${module_name}'
)

# Load the framework
try:
    import ctypes
    _lib = ctypes.CDLL(_FRAMEWORK_PATH)
except OSError:
    # Fallback to standard loading
    pass

# Import the actual module
from ._${module_name} import *
EOF
}

# Process wheel directory
process_wheel_dir() {
    local wheel_dir="$1"
    local output_dir="$2"
    local sign_identity="${3:-}"

    local frameworks_dir="$output_dir/Frameworks"
    mkdir -p "$frameworks_dir"

    # Track converted modules
    local converted=()

    # Find and convert all .so files
    while IFS= read -r so_file; do
        if [[ -f "$so_file" ]]; then
            local framework_path
            framework_path=$(convert_so_to_framework "$so_file" "$frameworks_dir" "$sign_identity")
            converted+=("$(basename "$framework_path" .framework)")

            # Remove original .so from wheel
            rm -f "$so_file"

            # Create loader stub
            local module_dir=$(dirname "$so_file")
            local module_name=$(basename "$so_file" .so)
            module_name="${module_name%%.so*}"

            if [[ -d "$module_dir" ]]; then
                create_loader_stub "$module_name" "$module_dir/__init__.py"
            fi
        fi
    done < <(find_so_files "$wheel_dir")

    # Output conversion report
    if [[ ${#converted[@]} -gt 0 ]]; then
        log_info "Converted ${#converted[@]} modules: ${converted[*]}"
    else
        log_info "No .so files found - pure Python wheel"
    fi

    echo "${converted[*]}"
}

# Create metadata for the processed wheel
create_wheel_metadata() {
    local wheel_dir="$1"
    local output_file="$2"
    local frameworks="$3"

    local name=$(basename "$wheel_dir")
    local timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

    cat > "$output_file" <<EOF
{
    "name": "$name",
    "processed_at": "$timestamp",
    "frameworks": [$(echo "$frameworks" | tr ' ' '\n' | sed 's/^/"/' | sed 's/$/"/' | tr '\n' ',' | sed 's/,$//')],
    "platform": "ios",
    "arch": "arm64",
    "min_ios_version": "16.0"
}
EOF
}

# Repackage wheel
repackage_wheel() {
    local wheel_dir="$1"
    local output_file="$2"

    log_info "Repackaging wheel: $(basename "$output_file")"

    # Create wheel from directory
    (cd "$wheel_dir" && zip -r "$output_file" . -x "*.pyc" -x "__pycache__/*")

    log_info "Created: $output_file"
}

# =============================================================================
# MAIN
# =============================================================================

main() {
    local wheel_file=""
    local output_dir="."
    local sign_identity=""
    local verbose=false

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -o|--output)
                output_dir="$2"
                shift 2
                ;;
            -p|--python)
                PYTHON_VERSION="$2"
                shift 2
                ;;
            -s|--sign)
                sign_identity="$2"
                shift 2
                ;;
            -v|--verbose)
                verbose=true
                shift
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            -*)
                log_error "Unknown option: $1"
                usage
                exit 1
                ;;
            *)
                if [[ -z "$wheel_file" ]]; then
                    wheel_file="$1"
                else
                    log_error "Multiple wheel files not supported"
                    exit 1
                fi
                shift
                ;;
        esac
    done

    # Validate input
    if [[ -z "$wheel_file" ]]; then
        log_error "No wheel file specified"
        usage
        exit 1
    fi

    if [[ ! -f "$wheel_file" ]]; then
        log_error "Wheel file not found: $wheel_file"
        exit 1
    fi

    # Create output directory
    mkdir -p "$output_dir"

    # Create temporary directory
    local temp_dir=$(mktemp -d)
    trap "rm -rf '$temp_dir'" EXIT

    # Extract wheel
    extract_wheel "$wheel_file" "$temp_dir/extracted"

    # Process wheel
    local frameworks
    frameworks=$(process_wheel_dir "$temp_dir/extracted" "$temp_dir" "$sign_identity")

    # Create metadata
    create_wheel_metadata "$temp_dir/extracted" "$temp_dir/metadata.json" "$frameworks"

    # Determine output filename
    local wheel_name=$(basename "$wheel_file" .whl)
    local output_wheel="$output_dir/${wheel_name}-ashell.whl"

    # Repackage
    repackage_wheel "$temp_dir/extracted" "$output_wheel"

    # Copy frameworks to output
    if [[ -d "$temp_dir/Frameworks" ]]; then
        cp -R "$temp_dir/Frameworks" "$output_dir/"
        log_info "Copied Frameworks to $output_dir/"
    fi

    # Copy metadata
    cp "$temp_dir/metadata.json" "$output_dir/${wheel_name}-metadata.json"

    log_info "Processing complete: $output_wheel"
}

main "$@"

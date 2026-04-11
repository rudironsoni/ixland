#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

if ! command -v swiftformat >/dev/null 2>&1; then
    exit 0
fi

swiftformat "$PROJECT_ROOT/IXLand"

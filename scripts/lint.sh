#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PROJECT_PATH="$PROJECT_ROOT/IXLand/IXLand.xcodeproj"
DESTINATION="platform=iOS Simulator,name=iPhone 17 Pro"

xcodebuild -project "$PROJECT_PATH" -list
xcodebuild -project "$PROJECT_PATH" -scheme IXLand -sdk iphonesimulator -destination "$DESTINATION" build
xcodebuild -project "$PROJECT_PATH" -scheme IXLandTests -sdk iphonesimulator -destination "$DESTINATION" test

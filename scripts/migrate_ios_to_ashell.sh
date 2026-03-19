#!/bin/bash
# Migration script: ios_* to a_shell_* 
# Run this to perform the full migration

set -e

echo "=== a-Shell Migration: ios_* to a_shell_* ==="
echo ""

# Step 1: Verify we're in the right directory
if [ ! -f "a-shell-kernel/a_shell_kernel.m" ]; then
    echo "Error: Must run from repository root"
    exit 1
fi

cd a-shell-kernel

echo "Step 1: Creating header structure..."

# Create proper header files in root
# a_shell_kernel.h already exists from a_shell_kernel/ios_system.h
cp a_shell_error.h ios_error.h 2>/dev/null || true

echo "Step 2: Updating #include statements in source files..."

# Find all C/C++/Objective-C files and update includes
find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.m" -o -name "*.mm" \) \
    -not -path "./.build/*" \
    -not -path "./build/*" \
    -not -path "./wamr/*" \
    -not -path "./.git/*" | while read file; do
    
    # Update ios_error.h to a_shell_error.h
    if grep -q '#include.*"ios_error.h"' "$file" 2>/dev/null; then
        sed -i '' 's/#include.*"ios_error.h"/#include "a_shell_error.h"/g' "$file"
        echo "  Updated: $file"
    fi
    
    # Also check for angle bracket includes
    if grep -q '#include.*<ios_error.h>' "$file" 2>/dev/null; then
        sed -i '' 's/#include.*<ios_error.h>/#include "a_shell_error.h"/g' "$file"
        echo "  Updated: $file"
    fi
done

echo "Step 3: Updating Xcode project..."

# Update project.pbxproj references
sed -i '' 's/ios_error\.h/a_shell_error.h/g' ios_system.xcodeproj/project.pbxproj
sed -i '' 's/ios_system\.h/a_shell_kernel.h/g' ios_system.xcodeproj/project.pbxproj
sed -i '' 's/ios_system\.m/a_shell_kernel.m/g' ios_system.xcodeproj/project.pbxproj

echo "Step 4: Creating compatibility headers..."

# Create ios_system directory with compatibility header
mkdir -p ios_system
cat > ios_system/ios_system.h << 'EOF'
// Compatibility header - redirects to a_shell_kernel.h
// This file exists for backward compatibility with existing code
#ifndef IOS_SYSTEM_COMPAT_H
#define IOS_SYSTEM_COMPAT_H

#include "../a_shell_kernel.h"

#endif
EOF

echo "Step 5: Verifying structure..."

echo "Headers in root:"
ls -la *.h 2>/dev/null | grep -E "a_shell|ios_"

echo ""
echo "=== Migration Complete ==="
echo ""
echo "Next steps:"
echo "1. Review changes: git diff --stat"
echo "2. Test build: ./scripts/build-kernel.sh simulator"
echo "3. Commit changes: git add -A && git commit -m 'Complete ios to a_shell migration'"

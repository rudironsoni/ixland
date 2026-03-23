# iOS Test App for libiox

This directory contains a test iOS application for validating libiox functionality on iOS Simulator or Device.

## Overview

The test app runs a series of tests to verify:
- Library initialization
- Version information
- getcwd/getpid/getppid
- chdir
- Basic file operations

Results are displayed both in the console and as log output.

## Prerequisites

- Xcode 14.0 or later
- iOS 16.0+ Simulator or Device
- libiox built for the target (see below)

## Setup Instructions

### Step 1: Build libiox

```bash
cd /path/to/a-shell-kernel

# Build for iOS Simulator
make ios-sim

# Or build for iOS Device
make ios-device
```

This creates:
- `libiox-sim.a` - for iOS Simulator
- `libiox-device.a` - for physical iOS devices

### Step 2: Create Xcode Project

**Option A: Manual Setup (Recommended)**

1. Open Xcode
2. Create new project: File → New → Project
3. Select "iOS" → "App"
4. Configure:
   - Name: `IOSTestApp`
   - Organization: (your choice)
   - Interface: `Storyboard` (or SwiftUI)
   - Language: `Objective-C`
   - Minimum iOS Version: `16.0`
5. Save to: `/path/to/a-shell-kernel/ios-test-app/IOSTestApp/`

### Step 3: Configure Project

1. **Replace main.m:**
   - Delete the auto-generated `main.m`
   - Copy `IOSTestApp/IOSTestApp/main.m` from this directory to your project

2. **Add libiox library:**
   - Drag `libiox-sim.a` (or `libiox-device.a`) into your project
   - Check "Copy items if needed" and add to target

3. **Add header search path:**
   - Select project → Build Settings
   - Search for "Header Search Paths"
   - Add: `$(SRCROOT)/../../include`
   - Add: `$(SRCROOT)/../../src/iox/internal`

4. **Link binary with libraries:**
   - Select target → Build Phases → Link Binary With Libraries
   - Add:
     - `libiox-sim.a` (or `libiox-device.a`)
     - `libpthread.tbd` (optional, usually auto-linked)

5. **Set deployment target:**
   - Ensure iOS Deployment Target is 16.0+

### Step 4: Run Tests

1. Select iOS Simulator target (e.g., iPhone 15 Pro)
2. Build and run (Cmd+R)
3. Check console output for test results:

```
[libiox] ========================================
[libiox] libiox iOS Test Suite
[libiox] ========================================

[libiox] Test 1: Library Initialization
[libiox] ✓ Library is initialized

[libiox] Test 2: Version Check
[libiox] ✓ Version: 1.0.0

[libiox] Test 3: getcwd
[libiox] ✓ CWD: /Users/.../Documents

[libiox] Test 4: getpid
[libiox] ✓ PID: 1234

[libiox] Test 5: getppid
[libiox] ✓ PPID: 1

[libiox] Test 6: chdir
[libiox] ✓ chdir to /tmp succeeded, new CWD: /tmp

[libiox] ========================================
[libiox] Test Results: 6 passed, 0 failed
[libiox] ✓ ALL TESTS PASSED
[libiox] ========================================
```

## Troubleshooting

### Library not found
```
ld: library not found for -liox-sim
```
**Solution:** Ensure library path is correct in Build Settings → Library Search Paths

### Header not found
```
'iox/iox.h' file not found
```
**Solution:** Add header search paths as described in Step 3

### Architecture mismatch
```
building for iOS-simulator, but linking in object file built for macOS
```
**Solution:** Use `libiox-sim.a` for simulator, `libiox-device.a` for device

### Crash on startup
If the app crashes immediately, check:
- Constructor is running (look for "iox: constructor started" in console)
- HOME environment variable is available in iOS context
- File permissions are correct

## Adding More Tests

To add custom tests, modify `IOSTestApp/main.m`:

```objc
// Add new test method
+ (void)test_custom_feature {
    [self log:@"Test: Custom Feature"];
    
    // Your test code here
    int result = iox_custom_function();
    
    if (result == 0) {
        [self log:@"✓ Custom test passed"];
    } else {
        [self log:@"✗ Custom test failed"];
    }
}
```

Then add to `runAllTests:`

## File Structure

```
ios-test-app/
├── README.md                 # This file
├── IOSTestApp/
│   └── IOSTestApp/
│       ├── main.m           # Test app source
│       └── Info.plist       # App configuration
└── create-xcode-project.sh  # (Optional) Setup script
```

## Notes

- **Do not run tests on macOS** - the library is iOS-only
- The test app uses a minimal UI just to launch tests
- All output goes to the Xcode console
- Tests run automatically on app launch

## Success Criteria

✓ All 6 tests pass
✓ No crashes or exceptions
✓ Console shows "ALL TESTS PASSED"

This confirms libiox is working correctly on iOS!

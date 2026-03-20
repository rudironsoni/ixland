# Kernel Integration Plan
## Connecting a-shell-kernel to the iOS App

### Current State
- **a-shell-kernel/**: New kernel with Linux headers (builds successfully)
- **a-shell/**: iOS app still uses OLD `ios_system.xcframework`
- **Gap**: App doesn't use our new kernel yet

### Integration Steps

#### Step 1: Framework Replacement (Critical)
Replace `ios_system.xcframework` reference in a-Shell.xcodeproj with `a-shell-kernel.xcframework`:

```
OLD: xcfs/.build/artifacts/xcfs/ios_system/ios_system.xcframework
NEW: ../build/a-shell-kernel.xcframework (or integrated build)
```

#### Step 2: Header Compatibility
Ensure headers are compatible:
- `a_shell_system.h` provides same API as old `ios_system.h`
- Test: App compiles without code changes

#### Step 3: Runtime Verification
- App launches successfully
- Basic commands work (ls, cd, pwd)
- Session management functional

### Two Approaches

**A) Conservative (Recommended)**
1. Keep old ios_system for now
2. Add a-shell-kernel as additional framework
3. Gradually migrate components
4. Test thoroughly before removing old

**B) Big Bang (Risky)**
1. Replace ios_system entirely
2. Update all imports
3. Fix breaking changes
4. Full regression test

### Testing Strategy

#### Phase 1: Unit Tests (Kernel)
- Test syscall implementations
- Test process simulation
- Test signal handling

#### Phase 2: Integration Tests (Kernel + App)
- Test command execution
- Test session management
- Test file operations

#### Phase 3: E2E Tests (Full App)
- Launch app
- Execute command sequences
- Test package installation
- Test WASM execution

### IPA Generation

```bash
# Build for device
xcodebuild -project a-Shell.xcodeproj \
    -scheme "a-Shell" \
    -destination 'generic/platform=iOS' \
    -configuration Release \
    -archivePath a-Shell.xcarchive \
    archive

# Export IPA
xcodebuild -exportArchive \
    -archivePath a-Shell.xcarchive \
    -exportPath a-Shell-IPA \
    -exportOptionsPlist exportOptions.plist
```

### Success Criteria
- [ ] App builds with new kernel
- [ ] All existing tests pass
- [ ] Basic commands work
- [ ] IPA generated successfully
- [ ] App installs on iOS device
- [ ] No regression in functionality

### Risk Mitigation
1. Keep old framework as backup
2. Feature flag for kernel selection
3. Extensive testing before release
4. Rollback plan ready

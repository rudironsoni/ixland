# Reference Implementations

Working code examples and templates for a-Shell Next development.

## Files

### Bash/Shell

- **`step-functions.sh`** - Complete implementations of all `ashell_step_*` functions
  - `ashell_step_extract_package()` - Download with SHA256 verification
  - `ashell_step_patch_package()` - Apply git diff patches in order
  - `ashell_step_configure()` - Run cmake/configure with iOS flags
  - `ashell_step_make()` - Build with parallel jobs
  - `ashell_step_make_install()` - Install to staging directory
  - `ashell_step_create_xcframework()` - Create Apple XCFramework
  - `ashell_step_generate_plist()` - Generate command metadata
  - `ashell_step_codesign()` - Sign frameworks for iOS

- **`package-template/build.sh`** - Starting point for new packages
  - Copy to `ashell-packages/{package}/build.sh`
  - Customize ASHELL_PKG_* variables
  - Override step functions as needed
  - Includes troubleshooting guide

### Swift

- **`swift-command/command.swift`** - Template for Swift-implemented commands
  - C-interop with `@_cdecl`
  - Argument parsing with error handling
  - ios_system I/O integration
  - Test structure

## Usage

### Creating a New Package

1. Copy the template:
   ```bash
   cp -r .beads/reference/package-template ashell-packages/packages/mypackage
   cd ashell-packages/packages/mypackage
   ```

2. Edit `build.sh`:
   - Set `ASHELL_PKG_NAME`, `ASHELL_PKG_VERSION`
   - Set `ASHELL_PKG_SRCURL`, `ASHELL_PKG_SHA256`
   - Define `ASHELL_PKG_COMMANDS`
   - Add custom step overrides if needed

3. Create patches (if needed):
   ```bash
   mkdir patches
   # Create 01-fix-ios.patch, 02-prefix.patch, etc.
   ```

4. Test:
   ```bash
   cd ashell-packages
   ./build.sh mypackage
   ```

### Implementing a Step Function

Reference `step-functions.sh` for the complete implementation:

```bash
# In ashell_package.sh, implement:
ashell_step_my_custom_step() {
    # See .beads/reference/step-functions.sh for patterns
    # Copy pattern, customize for your needs
}
```

### Creating a Swift Command

1. Copy the template:
   ```bash
   cp .beads/reference/swift-command/command.swift \
      ashell-core/Sources/Commands/MyCommand.swift
   ```

2. Replace placeholders:
   - `{CommandName}` → `MyCommand`
   - `{command}` → `mycommand`

3. Implement execute() logic

4. Register in commands.plist

## Patterns

### Error Handling

Always check return codes:
```bash
if ! command; then
    ashell_log_error "Command failed"
    return 1
fi
```

Use trap for cleanup:
```bash
trap 'rm -rf "$temp_dir"' EXIT
```

### iOS Cross-Compilation

```bash
export CC="clang"
export CFLAGS="-arch arm64 -isysroot $(xcrun --sdk iphoneos --show-sdk-path) -mios-version-min=16.0"
```

### XCFramework Creation

```bash
# 1. Create .framework bundle
# 2. Copy headers
# 3. Generate Info.plist
# 4. Wrap in .xcframework
# 5. Generate XCFramework Info.plist
```

## Conventions

- **ASHELL_** prefix for environment variables
- **ashell_** prefix for functions
- **Step functions** follow Termux pattern: `ashell_step_<action>()`
- **Logging** uses `ashell_log_info()`, `ashell_log_error()`
- **Error handling** returns 1 on failure, logs before returning

## Testing

All reference code is designed to be testable:

```bash
# Test bash functions
source .beads/reference/step-functions.sh
ashell_step_extract_package "url" "hash" "/tmp/test"

# Test Swift
swift build --target MyCommand
```

## See Also

- `../TASK_SPECIFICATION.md` - Issue format standard
- `../AGENT_GUIDE.md` - AI assistant onboarding
- `../../ashell-packages/README.md` - Package system docs
- `../../docs/api/ios_system_contract.md` - API reference

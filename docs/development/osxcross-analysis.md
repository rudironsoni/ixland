# osxcross Analysis for a-Shell Next

**osxcross** (https://github.com/tpoechtrager/osxcross) is a macOS/iOS cross-compilation toolchain for Linux. Here's the analysis for our project.

## What is osxcross?

osxcross provides:
- Clang/LLVM cross-compilers targeting macOS/iOS
- SDK extraction tools (requires macOS to extract SDK initially)
- CMake toolchain files for cross-compilation

## ⚠️ Legal Considerations

### Apple SDK License (Xcode License)

Apple's Xcode and SDKs are licensed under the **Apple Developer Program License Agreement**, which states:

> "You may not install, use or run the Apple SDKs on any non-Apple-branded computer."

**Key points:**
1. You **can** use osxcross if you:
   - Extract the SDK from a Mac you legally own
   - Use it for personal/educational purposes
   - Don't redistribute the SDK

2. You **cannot** (legally):
   - Download an SDK from the internet (piracy)
   - Use someone else's extracted SDK
   - Redistribute the SDK in a Docker image
   - Use it in a commercial CI/CD without proper licensing

### App Store Submission

Even if you build with osxcross:
- **Code signing** still requires a Mac (codesign tool)
- **Notarization** requires a Mac or cloud service
- **App Store upload** requires Transporter or Xcode on macOS

## Technical Limitations

### What Works

| Feature | Status | Notes |
|---------|--------|-------|
| Compile C/C++ | ✅ | Basic compilation works |
| Static libraries (.a) | ✅ | Full support |
| Dynamic libraries (.dylib) | ⚠️ | Partial, rpath issues |
| Frameworks | ⚠️ | Structure requires scripting |
| Objective-C | ⚠️ | Runtime issues |
| Swift | ❌ | Not supported |

### What Doesn't Work

1. **Code Signing**
   - No codesign tool on Linux
   - Cannot create signed binaries
   - Ad-hoc signing not available

2. **XCFrameworks**
   - osxcross doesn't create XCFramework bundles
   - You'd need to manually structure them
   - Info.plist generation not automated

3. **iOS Simulator**
   - Cannot run iOS simulator on Linux
   - No way to test without a Mac

4. **Device Testing**
   - No iOS device support on Linux
   - No debugging tools

5. **System Frameworks**
   - Some iOS frameworks don't work correctly
   - Security frameworks especially problematic
   - Keychain access doesn't translate

## Comparison: osxcross vs Remote Mac

| Aspect | osxcross (Linux) | Remote Mac |
|--------|------------------|------------|
| **Setup complexity** | High | Low |
| **Build speed** | Fast | Network dependent |
| **Code signing** | ❌ Impossible | ✅ Native |
| **XCFrameworks** | Manual | ✅ Automated |
| **Testing** | ❌ None | ✅ Simulator/Device |
| **App Store ready** | ❌ No | ✅ Yes |
| **Legal risk** | ⚠️ Gray area | ✅ Clean |
| **CI/CD** | ⚠️ Complex | ✅ GitHub Actions |
| **Team scaling** | ⚠️ SDK sharing issues | ✅ Clean |

## Recommendation for a-Shell Next

### ❌ NOT Recommended for Production

**Don't use osxcross because:**

1. **Legal complexity** - Apple SDK licensing is restrictive
2. **No code signing** - Cannot produce signed binaries
3. **No testing** - Cannot run iOS simulator
4. **CI/CD issues** - Cannot legally distribute SDK in Docker
5. **Maintenance burden** - osxcross breaks with SDK updates

### ✅ Recommended Approach

**Use Remote Mac Orchestration** (already implemented):

```bash
# From Linux/Docker
MAC_HOST=my-mac ./scripts/build-on-mac.sh hello
```

**Advantages:**
- ✅ Legally clean
- ✅ Full Xcode toolchain
- ✅ Code signing works
- ✅ Can test on simulator/device
- ✅ App Store submission ready
- ✅ CI/CD friendly (GitHub Actions)

## If You Still Want osxcross

### For Personal Development Only

1. **Extract SDK yourself** (from your Mac):
   ```bash
   # On your Mac
   git clone https://github.com/tpoechtrager/osxcross
   cd osxcross
   ./tools/gen_sdk_package.sh
   # This creates MacOSX.sdk.tar.xz
   ```

2. **Build osxcross** (on Linux):
   ```bash
   # On Linux
   tar xf MacOSX.sdk.tar.xz -C tarballs/
   SDK_VERSION=14.0 UNATTENDED=1 ./build.sh
   ```

3. **Use in Docker** (private only):
   ```dockerfile
   FROM ubuntu:24.04
   # DO NOT DISTRIBUTE THIS IMAGE (contains SDK)
   COPY osxcross/ /usr/local/osxcross/
   ENV PATH=/usr/local/osxcross/bin:$PATH
   ```

### ⚠️ Warnings

1. **Never commit SDK to git** - Violates Apple license
2. **Never push to Docker Hub** - Public SDK redistribution
3. **Never use in CI for public repos** - Legal risk
4. **Keep private** - Only for personal development

## Conclusion

**For a-Shell Next:**

- **Development**: Use the remote Mac orchestration we've built
- **CI/CD**: Use GitHub Actions with macOS runners (free for public repos)
- **Team**: Share a Mac or use Mac cloud (MacStadium, AWS EC2 Mac)

**osxcross is not worth it** for this project because:
1. It doesn't solve the code signing problem
2. Legal complexity outweighs benefits
3. Remote Mac approach is cleaner and more capable
4. GitHub Actions provides free macOS runners

The only valid use case for osxcross is personal experimentation on a private system where you've extracted the SDK yourself.

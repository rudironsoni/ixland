# Building iOS Targets in Docker (Development Workflow)

**⚠️ LEGAL NOTICE**: Apple's SDK and Xcode are only licensed for use on Apple hardware. This document describes development workflows for teams that have access to Macs but want to use Docker for orchestration and CI/CD pipeline testing.

## Overview

While you cannot legally build iOS binaries entirely on Linux (without macOS), you can use Docker to:

1. **Orchestrate builds** on remote Macs
2. **Test build scripts** and package metadata
3. **Validate CI/CD pipelines** before running on real Macs
4. **Cache dependencies** and speed up Mac builds

## Approaches

### 1. Remote Mac Build via SSH (Recommended)

Use Docker to coordinate builds that execute on a remote Mac.

```yaml
# docker-compose.yml
services:
  builder:
    image: alpine:latest
    volumes:
      - .:/workspace
      - ~/.ssh:/root/.ssh:ro
    environment:
      - MAC_HOST=mac-builder.local
      - MAC_USER=builder
    command: >
      sh -c "
        echo 'Syncing to Mac...' &&
        rsync -avz /workspace/ \$MAC_USER@\$MAC_HOST:/tmp/build/ &&
        echo 'Building on Mac...' &&
        ssh \$MAC_USER@\$MAC_HOST 'cd /tmp/build/ashell-packages && ./build.sh hello' &&
        echo 'Fetching results...' &&
        rsync -avz \$MAC_USER@\$MAC_HOST:/tmp/build/ashell-packages/.build/ /workspace/ashell-packages/.build/
      "
```

**Requirements:**
- A Mac (local network or cloud like MacStadium)
- SSH key-based authentication
- Xcode installed on the Mac

### 2. GitHub Actions with macOS Runners

For CI/CD, use GitHub's macOS runners (free for public repos):

```yaml
# .github/workflows/ios-build.yml
name: iOS Build

on: [push, pull_request]

jobs:
  build-packages:
    runs-on: macos-latest  # GitHub's macOS runner

    steps:
    - uses: actions/checkout@v4

    - name: Select Xcode
      run: sudo xcode-select -s /Applications/Xcode_15.0.app

    - name: Build hello package
      run: |
        cd ashell-packages
        ./build.sh hello

    - name: Upload artifacts
      uses: actions/upload-artifact@v4
      with:
        name: hello-xcframework
        path: ashell-packages/.build/hello/*.xcframework
```

**Pros:**
- No local Mac needed for CI
- Free for public repositories
- Always has latest Xcode

**Cons:**
- Limited to 2000 minutes/month for free private repos
- Queue times can be long

### 3. Self-Hosted Mac Runner with Docker

If you have a Mac, make it a GitHub Actions runner:

```bash
# On your Mac:
# 1. Install Docker Desktop
# 2. Install GitHub Actions runner
#    https://github.com/actions/runner/releases

# Configure runner for this repo
./config.sh --url https://github.com/rudironsoni/a-shell-next --token <TOKEN>

# Run as a service
./svc.sh install
./svc.sh start
```

Then use `runs-on: self-hosted` in workflows.

### 4. Docker-in-Mac (macOS Containers)

Use Docker Desktop on Mac directly:

```bash
# On macOS with Docker Desktop:
docker run -v $(pwd):/workspace alpine:latest sh -c "
  cd /workspace/ashell-packages &&
  ./build.sh hello
"
```

**Note**: This runs natively on macOS, not Linux. The container has access to the host's macOS tools.

### 5. Cross-Compilation with osx-cross (Experimental)

**⚠️ LEGAL WARNING**: Requires extracting SDK from a real Mac you own.

```dockerfile
FROM ubuntu:24.04

# Install osx-cross toolchain
RUN apt-get update && apt-get install -y \
    clang \
    llvm \
    lld \
    cmake \
    build-essential

# Add extracted iOS SDK (you must extract this from your own Mac)
COPY ios-sdk/ /opt/ios-sdk/

ENV CC=clang
ENV CXX=clang++
ENV CFLAGS="--target=arm64-apple-ios16.0 -isysroot /opt/ios-sdk"
```

**Not recommended** due to legal/licensing complexity.

## Recommended Development Workflow

### For Individual Developers

1. **Local development**: Use Docker to test build scripts and metadata
   ```bash
   docker-compose run test
   ```

2. **Real iOS builds**: Run on your Mac or remote Mac
   ```bash
   # Via SSH to your Mac
   ./scripts/build-on-mac.sh hello
   ```

### For Teams

1. **Docker for**: Script validation, dependency management
2. **Remote Mac for**: Actual iOS compilation and testing
3. **CI/CD**: GitHub Actions with macOS runners

## Docker-Based Orchestration Script

Create `scripts/build-on-mac.sh`:

```bash
#!/bin/bash
# Orchestrate iOS builds on a remote Mac

PACKAGE=${1:-hello}
MAC_HOST=${MAC_HOST:-mac-builder.local}
MAC_USER=${MAC_USER:-$(whoami)}

echo "Building $PACKAGE on $MAC_HOST..."

# Sync to Mac
rsync -avz --delete \
    --exclude='.build' \
    --exclude='.git' \
    ./ "$MAC_USER@$MAC_HOST:/tmp/ashell-build/"

# Build on Mac
ssh "$MAC_USER@$MAC_HOST" "
    cd /tmp/ashell-build/ashell-packages &&
    ./build.sh $PACKAGE
"

# Fetch results
mkdir -p ashell-packages/.build/
rsync -avz \
    "$MAC_USER@$MAC_HOST:/tmp/ashell-build/ashell-packages/.build/$PACKAGE/" \
    "ashell-packages/.build/$PACKAGE/"

echo "Build complete: ashell-packages/.build/$PACKAGE/"
```

## What's Possible vs Impossible

| Task | Docker on Linux | Docker on Mac | Real Mac |
|------|-----------------|---------------|----------|
| Shell script validation | ✅ Yes | ✅ Yes | ✅ Yes |
| Package metadata validation | ✅ Yes | ✅ Yes | ✅ Yes |
| Download/extract sources | ✅ Yes | ✅ Yes | ✅ Yes |
| Apply patches | ✅ Yes | ✅ Yes | ✅ Yes |
| Generate plists | ✅ Yes | ✅ Yes | ✅ Yes |
| **Compile iOS binaries** | ❌ No | ✅ Yes | ✅ Yes |
| **Create XCFrameworks** | ❌ No | ✅ Yes | ✅ Yes |
| **Code signing** | ❌ No | ✅ Yes | ✅ Yes |
| **Device testing** | ❌ No | ❌ No | ✅ Yes |

## Summary

**The practical answer:**

1. **Use Docker** for everything except the final compilation
2. **Use a real Mac** (local, remote, or GitHub Actions) for iOS builds
3. **Use Docker on macOS** if you want containerized builds locally

This gives you:
- Fast iteration on build scripts (Docker)
- Legal compliance (using real Macs)
- CI/CD integration (GitHub Actions)
- Team scalability (remote Macs)

**The legal constraint**: You must use Apple hardware to build iOS binaries. Docker can orchestrate, but macOS/Xcode must do the actual compilation.

# Dockerfile for a-Shell Next Package Build Testing
# This image tests the build system on Linux (without iOS SDK)
#
# Usage:
#   docker build -t ashell-builder .
#   docker run -v $(pwd):/workspace ashell-builder ./test.sh
#
# Note: This CANNOT build actual iOS packages (requires macOS + Xcode)
# It validates the build system, scripts, and package metadata.

FROM ubuntu:24.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# =============================================================================
# INSTALL DEPENDENCIES
# =============================================================================

RUN apt-get update && apt-get install -y \
    # Shell scripting
    bash \
    shellcheck \
    # Build tools
    make \
    cmake \
    build-essential \
    # Archive tools
    tar \
    gzip \
    bzip2 \
    xz-utils \
    unzip \
    curl \
    wget \
    # Checksum verification
    sha256sum \
    # Text processing
    grep \
    sed \
    awk \
    # Git (for patches)
    git \
    # XML validation
    libxml2-utils \
    # Python (for test scripts)
    python3 \
    python3-pip \
    # Utilities
    file \
    tree \
    jq \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# =============================================================================
# SETUP WORKSPACE
# =============================================================================

WORKDIR /workspace

# Create mock iOS SDK structure for testing
# This allows the build system to load without errors on Linux
RUN mkdir -p /opt/mock-ios-sdk/usr/include && \
    mkdir -p /opt/mock-ios-sdk/usr/lib && \
    echo '16.0' > /opt/mock-ios-sdk/SDKSettings.plist

# Set environment variables
ENV ASHELL_SDK_PATH=/opt/mock-ios-sdk
ENV ASHELL_TARGET_PLATFORM=arm64-apple-ios16.0
ENV ASHELL_DEPLOYMENT_TARGET=16.0

# =============================================================================
# ENTRYPOINT
# =============================================================================

# Default to running the test suite
CMD ["./scripts/test-build-system.sh"]

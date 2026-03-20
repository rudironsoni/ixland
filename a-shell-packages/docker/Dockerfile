# a-Shell Package Builder
# Docker environment for cross-compiling packages for iOS

FROM macos-12:latest

# Install Xcode command line tools
RUN xcode-select --install || true

# Install Homebrew dependencies
RUN /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
RUN brew install \
    cmake \
    automake \
    autoconf \
    libtool \
    pkg-config \
    dpkg \
    ninja \
    wget \
    curl \
    git

# Set up environment
ENV A_SHELL_PREFIX=/usr/local/a-shell
ENV A_SHELL_CONFIG=/usr/local/a-shell-config
ENV PATH=$A_SHELL_PREFIX/bin:$PATH

# iOS SDK settings
ENV IOS_SDK=iphoneos
ENV IOS_ARCH=arm64
ENV IOS_VERSION=16.0

# Copy build scripts
COPY ../scripts /usr/local/bin
RUN chmod +x /usr/local/bin/*.sh

# Working directory
WORKDIR /src

# Default command
CMD ["/bin/bash"]

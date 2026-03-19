# curl XCFramework Build
# iOS cross-compilation with proper architecture support

.ONESHELL:
SHELL := /bin/bash

# =============================================================================
# Configuration
# =============================================================================

PKG_NAME := curl
PKG_VERSION := 8.5.0

# Build directories (relative to this Makefile)
BUILD_ROOT := $(CURDIR)/.build
SRC_DIR := $(BUILD_ROOT)/curl-$(PKG_VERSION)
BUILD_DIR := $(BUILD_ROOT)/build
OUTPUT_DIR := $(CURDIR)/.dist

# XCFramework output
XCFRAMEWORK := $(OUTPUT_DIR)/$(PKG_NAME).xcframework

# Architectures
IOS_ARCH := arm64
SIM_ARCHS := arm64 x86_64

# SDK settings
IOS_SDK := iphoneos
SIM_SDK := iphonesimulator
DEPLOYMENT_TARGET := 16.0

# Dependencies (paths relative to this Makefile)
OPENSSL_ROOT := /Users/rudironsoni/src/github/rudironsoni/a-shell-next/a-shell-kernel/xcfs/.build/artifacts/xcfs/openssl/openssl.xcframework
ZLIB_ROOT := /Users/rudironsoni/src/github/rudironsoni/a-shell-next/ashell-packages/root-packages/libz/.dist/libz.xcframework

# Source URL
SRC_URL := https://curl.se/download/curl-$(PKG_VERSION).tar.gz

# =============================================================================
# Phony Targets
# =============================================================================

.PHONY: all clean download build-ios build-simulator xcframework

all: xcframework

# =============================================================================
# Download and Extract
# =============================================================================

download: $(SRC_DIR)/configure

$(SRC_DIR)/configure:
	@echo "Downloading curl $(PKG_VERSION)..."
	@mkdir -p $(BUILD_ROOT)
	@cd $(BUILD_ROOT)
	@if [ ! -f curl-$(PKG_VERSION).tar.gz ]; then \
		curl -L -o curl-$(PKG_VERSION).tar.gz "$(SRC_URL)"; \
	fi
	@echo "Extracting..."
	@tar -xzf curl-$(PKG_VERSION).tar.gz
	@touch $(SRC_DIR)/configure

# =============================================================================
# iOS Device Build (arm64)
# =============================================================================

build-ios: $(BUILD_DIR)/ios/lib/libcurl.a

$(BUILD_DIR)/ios/lib/libcurl.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building curl for iOS (arm64)"
	@echo "========================================"
	@mkdir -p $(BUILD_DIR)/ios
	@cd $(SRC_DIR)

	# Get SDK paths
	SDK_PATH=$$(xcrun --sdk $(IOS_SDK) --show-sdk-path)
	CC=$$(xcrun --sdk $(IOS_SDK) --find clang)

	# Clean any previous build
	$(MAKE) clean || true

	# Configure for iOS
	export CC="$$CC"
	export CFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -fembed-bitcode -O3 -I$(OPENSSL_ROOT)/ios-arm64/Headers -I$(ZLIB_ROOT)/ios-arm64/Headers"
	export LDFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)"
	export PKG_CONFIG_PATH=""

	./configure \
		--prefix=$(BUILD_DIR)/ios/install \
		--host=arm64-apple-darwin \
		--enable-static \
		--disable-shared \
		--with-ssl=$(OPENSSL_ROOT)/ios-arm64 \
		--with-zlib=$(ZLIB_ROOT)/ios-arm64 \
		--disable-ldap \
		--disable-ldaps \
		--without-librtmp \
		--without-libidn2 \
		--without-nghttp2 \
		--without-brotli \
		--without-zstd \
		--disable-ftp \
		--disable-file \
		--disable-pop3 \
		--disable-imap \
		--disable-smb \
		--disable-smtp \
		--disable-gopher \
		--disable-telnet \
		--disable-tftp \
		--disable-dict \
		--disable-debug \
		--enable-optimize \
		--enable-ipv6

	# Build
	$(MAKE) -j$$(sysctl -n hw.ncpu)
	$(MAKE) install

	@echo "iOS build complete"

# =============================================================================
# iOS Simulator Build (arm64 + x86_64)
# =============================================================================

build-simulator: $(BUILD_DIR)/simulator/lib/libcurl.a

$(BUILD_DIR)/simulator/lib/libcurl.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building curl for iOS Simulator"
	@echo "========================================"
	@mkdir -p $(BUILD_DIR)/simulator

	# Build for arm64 simulator
	@echo "Building for arm64 simulator..."
	@cd $(SRC_DIR)
	$(MAKE) clean || true

	SDK_PATH=$$(xcrun --sdk $(SIM_SDK) --show-sdk-path)
	CC=$$(xcrun --sdk $(SIM_SDK) --find clang)

	export CC="$$CC"
	export CFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O3 -I$(OPENSSL_ROOT)/ios-arm64_x86_64-simulator/Headers -I$(ZLIB_ROOT)/ios-arm64_x86_64-simulator/Headers"
	export LDFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)"
	export PKG_CONFIG_PATH=""

	./configure \
		--prefix=$(BUILD_DIR)/simulator/install_arm64 \
		--host=arm64-apple-darwin \
		--enable-static \
		--disable-shared \
		--with-ssl=$(OPENSSL_ROOT)/ios-arm64_x86_64-simulator \
		--with-zlib=$(ZLIB_ROOT)/ios-arm64_x86_64-simulator \
		--disable-ldap \
		--disable-ldaps \
		--without-librtmp \
		--without-libidn2 \
		--without-nghttp2 \
		--without-brotli \
		--without-zstd \
		--disable-ftp \
		--disable-file \
		--disable-pop3 \
		--disable-imap \
		--disable-smb \
		--disable-smtp \
		--disable-gopher \
		--disable-telnet \
		--disable-tftp \
		--disable-dict \
		--disable-debug \
		--enable-optimize \
		--enable-ipv6

	$(MAKE) -j$$(sysctl -n hw.ncpu)
	$(MAKE) install

	# Build for x86_64 simulator
	@echo "Building for x86_64 simulator..."
	$(MAKE) clean || true

	export CC="$$CC"
	export CFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O3 -I$(OPENSSL_ROOT)/ios-arm64_x86_64-simulator/Headers -I$(ZLIB_ROOT)/ios-arm64_x86_64-simulator/Headers"
	export LDFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)"

	./configure \
		--prefix=$(BUILD_DIR)/simulator/install_x86_64 \
		--host=x86_64-apple-darwin \
		--enable-static \
		--disable-shared \
		--with-ssl=$(OPENSSL_ROOT)/ios-arm64_x86_64-simulator \
		--with-zlib=$(ZLIB_ROOT)/ios-arm64_x86_64-simulator \
		--disable-ldap \
		--disable-ldaps \
		--without-librtmp \
		--without-libidn2 \
		--without-nghttp2 \
		--without-brotli \
		--without-zstd \
		--disable-ftp \
		--disable-file \
		--disable-pop3 \
		--disable-imap \
		--disable-smb \
		--disable-smtp \
		--disable-gopher \
		--disable-telnet \
		--disable-tftp \
		--disable-dict \
		--disable-debug \
		--enable-optimize \
		--enable-ipv6

	$(MAKE) -j$$(sysctl -n hw.ncpu)
	$(MAKE) install

	# Create universal library
	@echo "Creating universal simulator library..."
	mkdir -p $(BUILD_DIR)/simulator/install/lib
	lipo -create \
		$(BUILD_DIR)/simulator/install_arm64/lib/libcurl.a \
		$(BUILD_DIR)/simulator/install_x86_64/lib/libcurl.a \
		-output $(BUILD_DIR)/simulator/install/lib/libcurl.a

	# Copy headers from arm64 build
	cp -R $(BUILD_DIR)/simulator/install_arm64/include $(BUILD_DIR)/simulator/install/

	@echo "Simulator build complete"

# =============================================================================
# XCFramework Creation
# =============================================================================

xcframework: $(XCFRAMEWORK)

$(XCFRAMEWORK): build-ios build-simulator
	@echo "========================================"
	@echo "Creating XCFramework"
	@echo "========================================"
	@mkdir -p $(OUTPUT_DIR)
	@rm -rf $@

	# Create iOS framework
	@mkdir -p $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/Headers
	@cp $(BUILD_DIR)/ios/install/lib/libcurl.a $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/$(PKG_NAME)
	@cp -R $(BUILD_DIR)/ios/install/include/curl/* $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/Headers/
	@cat > $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/Info.plist << 'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>curl</string>
    <key>CFBundleIdentifier</key>
    <string>com.ashell.curl</string>
    <key>CFBundleName</key>
    <string>curl</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>8.5.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>MinimumOSVersion</key>
    <string>16.0</string>
</dict>
</plist>
PLIST

	# Create simulator framework
	@mkdir -p $(BUILD_DIR)/frameworks/sim/$(PKG_NAME).framework/Headers
	@cp $(BUILD_DIR)/simulator/install/lib/libcurl.a $(BUILD_DIR)/frameworks/sim/$(PKG_NAME).framework/$(PKG_NAME)
	@cp -R $(BUILD_DIR)/simulator/install/include/curl/* $(BUILD_DIR)/frameworks/sim/$(PKG_NAME).framework/Headers/
	@cp $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/Info.plist $(BUILD_DIR)/frameworks/sim/$(PKG_NAME).framework/

	# Create XCFramework
	xcodebuild -create-xcframework \
		-framework $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework \
		-framework $(BUILD_DIR)/frameworks/sim/$(PKG_NAME).framework \
		-output $@

	@echo "XCFramework created: $@"

# =============================================================================
# Utility Targets
# =============================================================================

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_ROOT) $(OUTPUT_DIR)
	@echo "Clean complete"

info: $(XCFRAMEWORK)
	@echo "XCFramework Info:"
	@xcodebuild -xcframework $(XCFRAMEWORK) -list 2>/dev/null || ls -la $@

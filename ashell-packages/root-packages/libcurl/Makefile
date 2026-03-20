# libcurl XCFramework Build
# iOS cross-compilation with proper architecture support

.ONESHELL:
SHELL := /bin/bash

# =============================================================================
# Configuration
# =============================================================================

PKG_NAME := libcurl
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

# Source URL
SRC_URL := https://curl.se/download/curl-$(PKG_VERSION).tar.gz

# Dependencies
LIBZ_XCFRAMEWORK := $(CURDIR)/../libz/.dist/libz.xcframework
LIBSSL_XCFRAMEWORK := $(CURDIR)/../libssl/.dist/libssl.xcframework

# =============================================================================
# Phony Targets
# =============================================================================

.PHONY: all clean download build-ios build-simulator xcframework check-deps

all: xcframework

# =============================================================================
# Dependency Check
# =============================================================================

check-deps:
	@echo "Checking dependencies..."
	@if [ ! -d "$(LIBZ_XCFRAMEWORK)" ]; then \
		echo "Error: libz XCFramework not found at $(LIBZ_XCFRAMEWORK)"; \
		echo "Please build libz first: cd ../libz && make"; \
		exit 1; \
	fi
	@echo "✓ libz found"
	@if [ ! -d "$(LIBSSL_XCFRAMEWORK)" ]; then \
		echo "Error: libssl XCFramework not found at $(LIBSSL_XCFRAMEWORK)"; \
		echo "Please build libssl first: cd ../libssl && make"; \
		exit 1; \
	fi
	@echo "✓ libssl found"

# =============================================================================
# Download and Extract
# =============================================================================

download: $(SRC_DIR)/configure

$(SRC_DIR)/configure: check-deps
	@echo "Downloading curl $(PKG_VERSION)..."
	@mkdir -p $(BUILD_ROOT)
	@if [ ! -f $(BUILD_ROOT)/curl-$(PKG_VERSION).tar.gz ]; then \
		cd $(BUILD_ROOT) && curl -L -o curl-$(PKG_VERSION).tar.gz "$(SRC_URL)"; \
	fi
	@echo "Extracting..."
	@cd $(BUILD_ROOT) && tar -xzf curl-$(PKG_VERSION).tar.gz
	@touch $(SRC_DIR)/configure

# =============================================================================
# iOS Device Build (arm64)
# =============================================================================

build-ios: $(BUILD_DIR)/ios/libcurl.a

$(BUILD_DIR)/ios/libcurl.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building curl for iOS (arm64)"
	@echo "========================================"
	@mkdir -p $(BUILD_DIR)/ios

	SDK_PATH=$$(xcrun --sdk $(IOS_SDK) --show-sdk-path) && \
	CC=$$(xcrun --sdk $(IOS_SDK) --find clang) && \
	cd $(SRC_DIR) && \
	$(MAKE) clean 2>/dev/null || true && \
	CC="$$CC" \
	CFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -fembed-bitcode -O3 -I$(LIBZ_XCFRAMEWORK)/ios-arm64/include -I$(LIBSSL_XCFRAMEWORK)/ios-arm64/include" \
	LDFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)" \
	./configure \
		--host=arm-apple-darwin \
		--prefix=$(BUILD_DIR)/ios/install \
		--enable-static \
		--disable-shared \
		--disable-ldap \
		--disable-ldaps \
		--with-zlib=$(LIBZ_XCFRAMEWORK)/ios-arm64 \
		--with-ssl=$(LIBSSL_XCFRAMEWORK)/ios-arm64 \
		--without-libpsl \
		--without-librtmp \
		--without-libidn2 \
		--without-nghttp2 \
		--without-ngtcp2 \
		--without-nghttp3 \
		--disable-rtsp \
		--disable-dict \
		--disable-file \
		--disable-ftp \
		--disable-gopher \
		--disable-imap \
		--disable-mqtt \
		--disable-pop3 \
		--disable-smtp \
		--disable-telnet \
		--disable-tftp \
		&& \
	$(MAKE) -j$$(sysctl -n hw.ncpu) && \
	$(MAKE) install
	
	@echo "iOS build complete"

# =============================================================================
# iOS Simulator Build (arm64 + x86_64)
# =============================================================================

build-simulator: $(BUILD_DIR)/simulator/libcurl.a

$(BUILD_DIR)/simulator/libcurl.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building curl for iOS Simulator"
	@echo "========================================"
	@mkdir -p $(BUILD_DIR)/simulator

	# Build for arm64 simulator
	@echo "Building for arm64 simulator..."
	SDK_PATH=$$(xcrun --sdk $(SIM_SDK) --show-sdk-path) && \
	CC=$$(xcrun --sdk $(SIM_SDK) --find clang) && \
	cd $(SRC_DIR) && \
	$(MAKE) clean 2>/dev/null || true && \
	CC="$$CC" \
	CFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O3 -I$(LIBZ_XCFRAMEWORK)/ios-arm64_x86_64-simulator/include -I$(LIBSSL_XCFRAMEWORK)/ios-arm64_x86_64-simulator/include" \
	LDFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)" \
	./configure \
		--host=arm-apple-darwin \
		--prefix=$(BUILD_DIR)/simulator/install_arm64 \
		--enable-static \
		--disable-shared \
		--disable-ldap \
		--disable-ldaps \
		--with-zlib=$(LIBZ_XCFRAMEWORK)/ios-arm64_x86_64-simulator \
		--with-ssl=$(LIBSSL_XCFRAMEWORK)/ios-arm64_x86_64-simulator \
		--without-libpsl \
		--without-librtmp \
		--without-libidn2 \
		--without-nghttp2 \
		--without-ngtcp2 \
		--without-nghttp3 \
		&& \
	$(MAKE) -j$$(sysctl -n hw.ncpu) && \
	$(MAKE) install

	# Build for x86_64 simulator
	@echo "Building for x86_64 simulator..."
	SDK_PATH=$$(xcrun --sdk $(SIM_SDK) --show-sdk-path) && \
	CC=$$(xcrun --sdk $(SIM_SDK) --find clang) && \
	cd $(SRC_DIR) && \
	$(MAKE) clean 2>/dev/null || true && \
	CC="$$CC" \
	CFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O3 -I$(LIBZ_XCFRAMEWORK)/ios-arm64_x86_64-simulator/include -I$(LIBSSL_XCFRAMEWORK)/ios-arm64_x86_64-simulator/include" \
	LDFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)" \
	./configure \
		--host=x86_64-apple-darwin \
		--prefix=$(BUILD_DIR)/simulator/install_x86_64 \
		--enable-static \
		--disable-shared \
		--disable-ldap \
		--disable-ldaps \
		--with-zlib=$(LIBZ_XCFRAMEWORK)/ios-arm64_x86_64-simulator \
		--with-ssl=$(LIBSSL_XCFRAMEWORK)/ios-arm64_x86_64-simulator \
		--without-libpsl \
		--without-librtmp \
		--without-libidn2 \
		--without-nghttp2 \
		--without-ngtcp2 \
		--without-nghttp3 \
		&& \
	$(MAKE) -j$$(sysctl -n hw.ncpu) && \
	$(MAKE) install

	# Create universal library
	@echo "Creating universal simulator library..."
	@mkdir -p $(BUILD_DIR)/simulator/install/lib $(BUILD_DIR)/simulator/install/include && \
	lipo -create \
		$(BUILD_DIR)/simulator/install_arm64/lib/libcurl.a \
		$(BUILD_DIR)/simulator/install_x86_64/lib/libcurl.a \
		-output $(BUILD_DIR)/simulator/install/lib/libcurl.a && \
	cp $(BUILD_DIR)/simulator/install_arm64/include/curl/*.h $(BUILD_DIR)/simulator/install/include/

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

	# Create XCFramework structure manually
	@mkdir -p $@/ios-arm64 $@/ios-arm64_x86_64-simulator
	@cp $(BUILD_DIR)/ios/install/lib/libcurl.a $@/ios-arm64/
	@cp $(BUILD_DIR)/simulator/install/lib/libcurl.a $@/ios-arm64_x86_64-simulator/
	@cp -r $(BUILD_DIR)/ios/install/include $@/ios-arm64/
	@cp -r $(BUILD_DIR)/simulator/install/include $@/ios-arm64_x86_64-simulator/

	# Create Info.plist
	@cat > $@/Info.plist << 'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundlePackageType</key>
    <string>XFWK</string>
    <key>XCFrameworkFormatVersion</key>
    <string>1.0</string>
    <key>AvailableLibraries</key>
    <array>
        <dict>
            <key>LibraryIdentifier</key>
            <string>ios-arm64</string>
            <key>LibraryPath</key>
            <string>libcurl.a</string>
            <key>HeadersPath</key>
            <string>include</string>
            <key>SupportedArchitectures</key>
            <array>
                <string>arm64</string>
            </array>
            <key>SupportedPlatform</key>
            <string>ios</string>
        </dict>
        <dict>
            <key>LibraryIdentifier</key>
            <string>ios-arm64_x86_64-simulator</string>
            <key>LibraryPath</key>
            <string>libcurl.a</string>
            <key>HeadersPath</key>
            <string>include</string>
            <key>SupportedArchitectures</key>
            <array>
                <string>arm64</string>
                <string>x86_64</string>
            </array>
            <key>SupportedPlatform</key>
            <string>ios</string>
            <key>SupportedPlatformVariant</key>
            <string>simulator</string>
        </dict>
    </array>
</dict>
</plist>
PLIST

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
	@echo "Location: $(XCFRAMEWORK)"
	@echo "iOS Device:"
	@file $(XCFRAMEWORK)/ios-arm64/libcurl.a
	@echo "iOS Simulator:"
	@file $(XCFRAMEWORK)/ios-arm64_x86_64-simulator/libcurl.a

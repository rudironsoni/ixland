# libz (zlib) XCFramework Build
# iOS cross-compilation with proper architecture support

.ONESHELL:
SHELL := /bin/bash

# =============================================================================
# Configuration
# =============================================================================

PKG_NAME := libz
PKG_VERSION := 1.3.2

# Build directories (relative to this Makefile)
BUILD_ROOT := $(CURDIR)/.build
SRC_DIR := $(BUILD_ROOT)/zlib-$(PKG_VERSION)
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
SRC_URL := https://github.com/madler/zlib/releases/download/v$(PKG_VERSION)/zlib-$(PKG_VERSION).tar.gz

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
	@echo "Downloading zlib $(PKG_VERSION)..."
	@mkdir -p $(BUILD_ROOT)
	@if [ ! -f $(BUILD_ROOT)/zlib-$(PKG_VERSION).tar.gz ]; then \
		cd $(BUILD_ROOT) && curl -L -o zlib-$(PKG_VERSION).tar.gz "$(SRC_URL)"; \
	fi
	@echo "Extracting..."
	@cd $(BUILD_ROOT) && tar -xzf zlib-$(PKG_VERSION).tar.gz
	@touch $(SRC_DIR)/configure

# =============================================================================
# iOS Device Build (arm64)
# =============================================================================

build-ios: $(BUILD_DIR)/ios/libz.a

$(BUILD_DIR)/ios/libz.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building zlib for iOS (arm64)"
	@echo "========================================"
	@mkdir -p $(BUILD_DIR)/ios

	SDK_PATH=$$(xcrun --sdk $(IOS_SDK) --show-sdk-path) && \
	CC=$$(xcrun --sdk $(IOS_SDK) --find clang) && \
	cd $(SRC_DIR) && \
	$(MAKE) clean 2>/dev/null || true && \
	CC="$$CC" CFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -fembed-bitcode -O3" \
	LDFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)" \
	./configure --prefix=$(BUILD_DIR)/ios/install --static && \
	$(MAKE) -j$$(sysctl -n hw.ncpu) libz.a && \
	mkdir -p $(BUILD_DIR)/ios/install/lib $(BUILD_DIR)/ios/install/include && \
	cp libz.a $(BUILD_DIR)/ios/install/lib/ && \
	cp zlib.h zconf.h $(BUILD_DIR)/ios/install/include/
	
	@echo "iOS build complete"

# =============================================================================
# iOS Simulator Build (arm64 + x86_64)
# =============================================================================

build-simulator: $(BUILD_DIR)/simulator/libz.a

$(BUILD_DIR)/simulator/libz.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building zlib for iOS Simulator"
	@echo "========================================"
	@mkdir -p $(BUILD_DIR)/simulator

	# Build for arm64 simulator
	@echo "Building for arm64 simulator..."
	SDK_PATH=$$(xcrun --sdk $(SIM_SDK) --show-sdk-path) && \
	CC=$$(xcrun --sdk $(SIM_SDK) --find clang) && \
	cd $(SRC_DIR) && \
	$(MAKE) clean 2>/dev/null || true && \
	CC="$$CC" CFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O3" \
	LDFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)" \
	./configure --prefix=$(BUILD_DIR)/simulator/install_arm64 --static && \
	$(MAKE) -j$$(sysctl -n hw.ncpu) libz.a && \
	mkdir -p $(BUILD_DIR)/simulator/install_arm64/lib $(BUILD_DIR)/simulator/install_arm64/include && \
	cp libz.a $(BUILD_DIR)/simulator/install_arm64/lib/ && \
	cp zlib.h zconf.h $(BUILD_DIR)/simulator/install_arm64/include/

	# Build for x86_64 simulator
	@echo "Building for x86_64 simulator..."
	SDK_PATH=$$(xcrun --sdk $(SIM_SDK) --show-sdk-path) && \
	CC=$$(xcrun --sdk $(SIM_SDK) --find clang) && \
	cd $(SRC_DIR) && \
	$(MAKE) clean 2>/dev/null || true && \
	CC="$$CC" CFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O3" \
	LDFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)" \
	./configure --prefix=$(BUILD_DIR)/simulator/install_x86_64 --static && \
	$(MAKE) -j$$(sysctl -n hw.ncpu) libz.a && \
	mkdir -p $(BUILD_DIR)/simulator/install_x86_64/lib $(BUILD_DIR)/simulator/install_x86_64/include && \
	cp libz.a $(BUILD_DIR)/simulator/install_x86_64/lib/ && \
	cp zlib.h zconf.h $(BUILD_DIR)/simulator/install_x86_64/include/

	# Create universal library
	@echo "Creating universal simulator library..."
	@mkdir -p $(BUILD_DIR)/simulator/install/lib $(BUILD_DIR)/simulator/install/include && \
	lipo -create \
		$(BUILD_DIR)/simulator/install_arm64/lib/libz.a \
		$(BUILD_DIR)/simulator/install_x86_64/lib/libz.a \
		-output $(BUILD_DIR)/simulator/install/lib/libz.a && \
	cp $(BUILD_DIR)/simulator/install_arm64/include/*.h $(BUILD_DIR)/simulator/install/include/

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

	# Create XCFramework structure manually (xcodebuild -create-xcframework doesn't support fat binaries well)
	@mkdir -p $@/ios-arm64 $@/ios-arm64_x86_64-simulator
	@cp $(BUILD_DIR)/ios/install/lib/libz.a $@/ios-arm64/
	@cp $(BUILD_DIR)/simulator/install/lib/libz.a $@/ios-arm64_x86_64-simulator/
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
            <string>libz.a</string>
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
            <string>libz.a</string>
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
	@file $(XCFRAMEWORK)/ios-arm64/libz.a
	@echo "iOS Simulator:"
	@file $(XCFRAMEWORK)/ios-arm64_x86_64-simulator/libz.a
	@echo "Headers:"
	@ls $(XCFRAMEWORK)/ios-arm64/include/

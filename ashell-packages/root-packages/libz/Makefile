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
	@cd $(BUILD_ROOT)
	@if [ ! -f zlib-$(PKG_VERSION).tar.gz ]; then \
		curl -L -o zlib-$(PKG_VERSION).tar.gz "$(SRC_URL)"; \
	fi
	@echo "Extracting..."
	@tar -xzf zlib-$(PKG_VERSION).tar.gz
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
	@cd $(SRC_DIR)

	# Get SDK paths
	SDK_PATH=$$(xcrun --sdk $(IOS_SDK) --show-sdk-path)
	CC=$$(xcrun --sdk $(IOS_SDK) --find clang)

	# Clean any previous build
	$(MAKE) clean || true

	# Configure for iOS
	export CC="$$CC"
	export CFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -fembed-bitcode -O3"
	export LDFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)"

	./configure --prefix=$(BUILD_DIR)/ios/install --static

	# Build only the library (skip tests that fail to link)
	$(MAKE) -j$$(sysctl -n hw.ncpu) libz.a
	
	# Manual install (just copy files)
	mkdir -p $(BUILD_DIR)/ios/install/lib $(BUILD_DIR)/ios/install/include
	cp libz.a $(BUILD_DIR)/ios/install/lib/
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
	@cd $(SRC_DIR)

	# Get SDK paths
	SDK_PATH=$$(xcrun --sdk $(SIM_SDK) --show-sdk-path)
	CC=$$(xcrun --sdk $(SIM_SDK) --find clang)

	# Build for arm64 simulator
	@echo "Building for arm64 simulator..."
	$(MAKE) clean || true
	export CC="$$CC"
	export CFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O3"
	export LDFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)"
	./configure --prefix=$(BUILD_DIR)/simulator/install_arm64 --static
	$(MAKE) -j$$(sysctl -n hw.ncpu) libz.a
	mkdir -p $(BUILD_DIR)/simulator/install_arm64/lib $(BUILD_DIR)/simulator/install_arm64/include
	cp libz.a $(BUILD_DIR)/simulator/install_arm64/lib/
	cp zlib.h zconf.h $(BUILD_DIR)/simulator/install_arm64/include/

	# Build for x86_64 simulator
	@echo "Building for x86_64 simulator..."
	$(MAKE) clean || true
	export CC="$$CC"
	export CFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O3"
	export LDFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET)"
	./configure --prefix=$(BUILD_DIR)/simulator/install_x86_64 --static
	$(MAKE) -j$$(sysctl -n hw.ncpu) libz.a
	mkdir -p $(BUILD_DIR)/simulator/install_x86_64/lib $(BUILD_DIR)/simulator/install_x86_64/include
	cp libz.a $(BUILD_DIR)/simulator/install_x86_64/lib/
	cp zlib.h zconf.h $(BUILD_DIR)/simulator/install_x86_64/include/

	# Create universal library
	@echo "Creating universal simulator library..."
	mkdir -p $(BUILD_DIR)/simulator/install/lib
	lipo -create \
		$(BUILD_DIR)/simulator/install_arm64/lib/libz.a \
		$(BUILD_DIR)/simulator/install_x86_64/lib/libz.a \
		-output $(BUILD_DIR)/simulator/install/lib/libz.a

	# Copy headers from arm64 build
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

	# Create iOS framework
	@mkdir -p $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/Headers
	@cp $(BUILD_DIR)/ios/install/lib/libz.a $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/$(PKG_NAME)
	@cp $(BUILD_DIR)/ios/install/include/*.h $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/Headers/
	@cat > $(BUILD_DIR)/frameworks/ios/$(PKG_NAME).framework/Info.plist << 'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>libz</string>
    <key>CFBundleIdentifier</key>
    <string>com.ashell.libz</string>
    <key>CFBundleName</key>
    <string>libz</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>1.3.2</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>MinimumOSVersion</key>
    <string>16.0</string>
</dict>
</plist>
PLIST

	# Create simulator framework
	@mkdir -p $(BUILD_DIR)/frameworks/sim/$(PKG_NAME).framework/Headers
	@cp $(BUILD_DIR)/simulator/install/lib/libz.a $(BUILD_DIR)/frameworks/sim/$(PKG_NAME).framework/$(PKG_NAME)
	@cp $(BUILD_DIR)/simulator/install/include/*.h $(BUILD_DIR)/frameworks/sim/$(PKG_NAME).framework/Headers/
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

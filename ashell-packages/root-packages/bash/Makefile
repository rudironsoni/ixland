# bash XCFramework Build
# iOS cross-compilation with proper architecture support

.ONESHELL:
SHELL := /bin/bash

# =============================================================================
# Configuration
# =============================================================================

PKG_NAME := bash
PKG_VERSION := 5.2.21

# Build directories (relative to this Makefile)
BUILD_ROOT := $(CURDIR)/.build
SRC_DIR := $(BUILD_ROOT)/bash-$(PKG_VERSION)
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
SRC_URL := https://ftp.gnu.org/gnu/bash/bash-$(PKG_VERSION).tar.gz

# Dependencies
LIBZ_XCFRAMEWORK := $(CURDIR)/../libz/.dist/libz.xcframework

# =============================================================================
# Phony Targets
# =============================================================================

.PHONY: all clean download build-ios build-simulator xcframework check-deps info

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

# =============================================================================
# Download and Extract
# =============================================================================

download: $(SRC_DIR)/configure

$(SRC_DIR)/configure: check-deps
	@echo "Downloading bash $(PKG_VERSION)..."
	@mkdir -p $(BUILD_ROOT)
	@if [ ! -f $(BUILD_ROOT)/bash-$(PKG_VERSION).tar.gz ]; then \
		cd $(BUILD_ROOT) && curl -L -o bash-$(PKG_VERSION).tar.gz "$(SRC_URL)"; \
	fi
	@echo "Extracting..."
	@cd $(BUILD_ROOT) && tar -xzf bash-$(PKG_VERSION).tar.gz
	@touch $(SRC_DIR)/configure

# =============================================================================
# iOS Device Build (arm64)
# =============================================================================

build-ios: $(BUILD_DIR)/ios/bin/bash

$(BUILD_DIR)/ios/bin/bash: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building bash for iOS (arm64)"
	@echo "========================================"
	@mkdir -p $(BUILD_DIR)/ios

	SDK_PATH=$$(xcrun --sdk $(IOS_SDK) --show-sdk-path) && \
	CC=$$(xcrun --sdk $(IOS_SDK) --find clang) && \
	cd $(SRC_DIR) && \
	$(MAKE) clean 2>/dev/null || true && \
	CC="$$CC" \
	CFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -fembed-bitcode -O2 -I$(LIBZ_XCFRAMEWORK)/ios-arm64/include" \
	LDFLAGS="-arch $(IOS_ARCH) -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -L$(LIBZ_XCFRAMEWORK)/ios-arm64" \
	./configure \
		--host=arm-apple-darwin \
		--prefix=$(BUILD_DIR)/ios/install \
		--without-bash-malloc \
		--with-installed-readline=no \
		--with-libz \
		--disable-nls \
		--disable-rpath \
		bash_cv_getcwd_malloc=yes \
		bash_cv_job_control_missing=present \
		bash_cv_sys_named_pipes=present \
		bash_cv_func_sigsetjmp=present \
		&& \
	$(MAKE) -j$$(sysctl -n hw.ncpu) && \
	$(MAKE) install
	
	@echo "iOS build complete"

# =============================================================================
# iOS Simulator Build (arm64 + x86_64)
# =============================================================================

build-simulator: $(BUILD_DIR)/simulator/bin/bash

$(BUILD_DIR)/simulator/bin/bash: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building bash for iOS Simulator"
	@echo "========================================"
	@mkdir -p $(BUILD_DIR)/simulator

	# Build for arm64 simulator
	@echo "Building for arm64 simulator..."
	SDK_PATH=$$(xcrun --sdk $(SIM_SDK) --show-sdk-path) && \
	CC=$$(xcrun --sdk $(SIM_SDK) --find clang) && \
	cd $(SRC_DIR) && \
	$(MAKE) clean 2>/dev/null || true && \
	CC="$$CC" \
	CFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O2 -I$(LIBZ_XCFRAMEWORK)/ios-arm64_x86_64-simulator/include" \
	LDFLAGS="-arch arm64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -L$(LIBZ_XCFRAMEWORK)/ios-arm64_x86_64-simulator" \
	./configure \
		--host=arm-apple-darwin \
		--prefix=$(BUILD_DIR)/simulator/install_arm64 \
		--without-bash-malloc \
		--with-installed-readline=no \
		--with-libz \
		--disable-nls \
		--disable-rpath \
		bash_cv_getcwd_malloc=yes \
		bash_cv_job_control_missing=present \
		bash_cv_sys_named_pipes=present \
		bash_cv_func_sigsetjmp=present \
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
	CFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -O2 -I$(LIBZ_XCFRAMEWORK)/ios-arm64_x86_64-simulator/include" \
	LDFLAGS="-arch x86_64 -isysroot $$SDK_PATH -mios-version-min=$(DEPLOYMENT_TARGET) -L$(LIBZ_XCFRAMEWORK)/ios-arm64_x86_64-simulator" \
	./configure \
		--host=x86_64-apple-darwin \
		--prefix=$(BUILD_DIR)/simulator/install_x86_64 \
		--without-bash-malloc \
		--with-installed-readline=no \
		--with-libz \
		--disable-nls \
		--disable-rpath \
		bash_cv_getcwd_malloc=yes \
		bash_cv_job_control_missing=present \
		bash_cv_sys_named_pipes=present \
		bash_cv_func_sigsetjmp=present \
		&& \
	$(MAKE) -j$$(sysctl -n hw.ncpu) && \
	$(MAKE) install

	# Create universal binary
	@echo "Creating universal simulator binary..."
	@mkdir -p $(BUILD_DIR)/simulator/install/bin $(BUILD_DIR)/simulator/install/lib && \
	lipo -create \
		$(BUILD_DIR)/simulator/install_arm64/bin/bash \
		$(BUILD_DIR)/simulator/install_x86_64/bin/bash \
		-output $(BUILD_DIR)/simulator/install/bin/bash && \
	cp -r $(BUILD_DIR)/simulator/install_arm64/lib/* $(BUILD_DIR)/simulator/install/lib/ 2>/dev/null || true && \
	cp -r $(BUILD_DIR)/simulator/install_arm64/share $(BUILD_DIR)/simulator/install/ 2>/dev/null || true

	@echo "Simulator build complete"

# =============================================================================
# XCFramework Creation (for command binaries)
# =============================================================================

xcframework: $(XCFRAMEWORK)

$(XCFRAMEWORK): build-ios build-simulator
	@echo "========================================"
	@echo "Creating XCFramework for bash"
	@echo "========================================"
	@mkdir -p $(OUTPUT_DIR)
	@rm -rf $@

	# Create XCFramework structure manually
	@mkdir -p $@/ios-arm64/bin $@/ios-arm64_x86_64-simulator/bin
	@cp $(BUILD_DIR)/ios/install/bin/bash $@/ios-arm64/bin/
	@cp $(BUILD_DIR)/simulator/install/bin/bash $@/ios-arm64_x86_64-simulator/bin/

	# Create Info.plist
	@echo "Creating Info.plist..."
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $@/Info.plist
	@echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >> $@/Info.plist
	@echo '<plist version="1.0">' >> $@/Info.plist
	@echo '<dict>' >> $@/Info.plist
	@echo '    <key>CFBundlePackageType</key>' >> $@/Info.plist
	@echo '    <string>XFWK</string>' >> $@/Info.plist
	@echo '    <key>XCFrameworkFormatVersion</key>' >> $@/Info.plist
	@echo '    <string>1.0</string>' >> $@/Info.plist
	@echo '    <key>AvailableLibraries</key>' >> $@/Info.plist
	@echo '    <array>' >> $@/Info.plist
	@echo '        <dict>' >> $@/Info.plist
	@echo '            <key>LibraryIdentifier</key>' >> $@/Info.plist
	@echo '            <string>ios-arm64</string>' >> $@/Info.plist
	@echo '            <key>LibraryPath</key>' >> $@/Info.plist
	@echo '            <string>bin/bash</string>' >> $@/Info.plist
	@echo '            <key>SupportedArchitectures</key>' >> $@/Info.plist
	@echo '            <array>' >> $@/Info.plist
	@echo '                <string>arm64</string>' >> $@/Info.plist
	@echo '            </array>' >> $@/Info.plist
	@echo '            <key>SupportedPlatform</key>' >> $@/Info.plist
	@echo '            <string>ios</string>' >> $@/Info.plist
	@echo '        </dict>' >> $@/Info.plist
	@echo '        <dict>' >> $@/Info.plist
	@echo '            <key>LibraryIdentifier</key>' >> $@/Info.plist
	@echo '            <string>ios-arm64_x86_64-simulator</string>' >> $@/Info.plist
	@echo '            <key>LibraryPath</key>' >> $@/Info.plist
	@echo '            <string>bin/bash</string>' >> $@/Info.plist
	@echo '            <key>SupportedArchitectures</key>' >> $@/Info.plist
	@echo '            <array>' >> $@/Info.plist
	@echo '                <string>arm64</string>' >> $@/Info.plist
	@echo '                <string>x86_64</string>' >> $@/Info.plist
	@echo '            </array>' >> $@/Info.plist
	@echo '            <key>SupportedPlatform</key>' >> $@/Info.plist
	@echo '            <string>ios</string>' >> $@/Info.plist
	@echo '            <key>SupportedPlatformVariant</key>' >> $@/Info.plist
	@echo '            <string>simulator</string>' >> $@/Info.plist
	@echo '        </dict>' >> $@/Info.plist
	@echo '    </array>' >> $@/Info.plist
	@echo '</dict>' >> $@/Info.plist
	@echo '</plist>' >> $@/Info.plist

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
	@echo "iOS Device binary:"
	@file $(XCFRAMEWORK)/ios-arm64/bin/bash
	@echo "iOS Simulator binary:"
	@file $(XCFRAMEWORK)/ios-arm64_x86_64-simulator/bin/bash

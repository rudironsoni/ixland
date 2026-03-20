# readline XCFramework Build
# iOS cross-compilation with proper architecture support

.ONESHELL:
SHELL := /bin/bash

# =============================================================================
# Configuration
# =============================================================================

PKG_NAME := readline
PKG_VERSION := 8.2

# Build directories (relative to this Makefile)
BUILD_ROOT := $(CURDIR)/.build
SRC_DIR := $(BUILD_ROOT)/readline-$(PKG_VERSION)
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
SRC_URL := https://ftp.gnu.org/gnu/readline/readline-$(PKG_VERSION).tar.gz

# =============================================================================
# Phony Targets
# =============================================================================

.PHONY: all clean download build-ios build-simulator xcframework info

all: xcframework

# =============================================================================
# Download and Extract
# =============================================================================

download: $(SRC_DIR)/configure

$(SRC_DIR)/configure:
	@echo "Downloading readline $(PKG_VERSION)..."
	@mkdir -p $(BUILD_ROOT)
	@cd $(BUILD_ROOT)
	
	@if [ ! -f "readline-$(PKG_VERSION).tar.gz" ]; then \
		curl -L $(SRC_URL) -o readline-$(PKG_VERSION).tar.gz; \
	fi
	
	@echo "Extracting..."
	@tar -xzf readline-$(PKG_VERSION).tar.gz
	
	@echo "✓ readline source ready"

# =============================================================================
# Build for iOS Device (arm64)
# =============================================================================

build-ios: $(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libreadline.a

$(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libreadline.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building readline for iOS $(IOS_ARCH)"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/ios-$(IOS_ARCH)
	
	cd $(SRC_DIR)
	$(MAKE) distclean 2>/dev/null || true
	
	# Set up environment
	export CC="xcrun -sdk $(IOS_SDK) clang"
	export CFLAGS="-arch $(IOS_ARCH) -isysroot $(shell xcrun -sdk $(IOS_SDK) --show-sdk-path) -mios-version-min=$(DEPLOYMENT_TARGET) -O2 -DHAVE_TERMIOS_H=1"
	export LDFLAGS="-arch $(IOS_ARCH) -isysroot $(shell xcrun -sdk $(IOS_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/ios-$(IOS_ARCH) \
		--host=arm-apple-darwin \
		--target=arm-apple-darwin \
		--enable-static \
		--disable-shared \
		--without-curses \
		--with-termcap \
		bash_cv_termcap_lib=libtermcap
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ readline iOS build complete"

# =============================================================================
# Build for iOS Simulator
# =============================================================================

build-simulator: $(BUILD_DIR)/simulator-arm64/lib/libreadline.a $(BUILD_DIR)/simulator-x86_64/lib/libreadline.a

$(BUILD_DIR)/simulator-arm64/lib/libreadline.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building readline for Simulator arm64"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/simulator-arm64
	
	cd $(SRC_DIR)
	$(MAKE) distclean 2>/dev/null || true
	
	export CC="xcrun -sdk $(SIM_SDK) clang"
	export CFLAGS="-arch arm64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path) -mios-simulator-version-min=$(DEPLOYMENT_TARGET) -O2 -DHAVE_TERMIOS_H=1"
	export LDFLAGS="-arch arm64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/simulator-arm64 \
		--host=arm-apple-darwin \
		--enable-static \
		--disable-shared \
		--without-curses \
		--with-termcap \
		bash_cv_termcap_lib=libtermcap
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ readline simulator arm64 build complete"

$(BUILD_DIR)/simulator-x86_64/lib/libreadline.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building readline for Simulator x86_64"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/simulator-x86_64
	
	cd $(SRC_DIR)
	$(MAKE) distclean 2>/dev/null || true
	
	export CC="xcrun -sdk $(SIM_SDK) clang"
	export CFLAGS="-arch x86_64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path) -mios-simulator-version-min=$(DEPLOYMENT_TARGET) -O2 -DHAVE_TERMIOS_H=1"
	export LDFLAGS="-arch x86_64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/simulator-x86_64 \
		--host=x86_64-apple-darwin \
		--enable-static \
		--disable-shared \
		--without-curses \
		--with-termcap \
		bash_cv_termcap_lib=libtermcap
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ readline simulator x86_64 build complete"

# =============================================================================
# Create XCFramework
# =============================================================================

xcframework: $(XCFRAMEWORK)

$(XCFRAMEWORK): $(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libreadline.a $(BUILD_DIR)/simulator-arm64/lib/libreadline.a $(BUILD_DIR)/simulator-x86_64/lib/libreadline.a
	@echo "========================================"
	@echo "Creating XCFramework"
	@echo "========================================"
	
	@mkdir -p $(OUTPUT_DIR)
	@rm -rf $@
	
	# Create framework for iOS device
	@mkdir -p $@/ios-$(IOS_ARCH)
	@cp -r $(BUILD_DIR)/ios-$(IOS_ARCH)/include $@/ios-$(IOS_ARCH)/
	@mkdir -p $@/ios-$(IOS_ARCH)/lib
	@cp $(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libreadline.a $@/ios-$(IOS_ARCH)/lib/
	@cp $(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libhistory.a $@/ios-$(IOS_ARCH)/lib/ 2>/dev/null || true
	
	# Create framework for simulator (combined arm64 + x86_64)
	@mkdir -p $@/ios-$(SIM_ARCHS)-simulator
	@cp -r $(BUILD_DIR)/simulator-arm64/include $@/ios-$(SIM_ARCHS)-simulator/
	@mkdir -p $@/ios-$(SIM_ARCHS)-simulator/lib
	
	# Create universal library for simulator
	@lipo -create \
		$(BUILD_DIR)/simulator-arm64/lib/libreadline.a \
		$(BUILD_DIR)/simulator-x86_64/lib/libreadline.a \
		-output $@/ios-$(SIM_ARCHS)-simulator/lib/libreadline.a
	
	@lipo -create \
		$(BUILD_DIR)/simulator-arm64/lib/libhistory.a \
		$(BUILD_DIR)/simulator-x86_64/lib/libhistory.a \
		-output $@/ios-$(SIM_ARCHS)-simulator/lib/libhistory.a 2>/dev/null || true
	
	@echo "✓ XCFramework created: $@"

# =============================================================================
# Utility Targets
# =============================================================================

clean:
	@echo "Cleaning readline build artifacts..."
	@rm -rf $(BUILD_ROOT) $(OUTPUT_DIR)
	@echo "Clean complete"

info: $(XCFRAMEWORK)
	@echo "========================================"
	@echo "readline XCFramework Info"
	@echo "========================================"
	@echo "Location: $(XCFRAMEWORK)"
	@echo "Version: $(PKG_VERSION)"
	@echo ""
	@echo "Available platforms:"
	@ls -1 $(XCFRAMEWORK)/ 2>/dev/null || echo "  (not built yet)"

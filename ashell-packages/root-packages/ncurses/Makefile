# ncurses XCFramework Build
# iOS cross-compilation with proper architecture support

.ONESHELL:
SHELL := /bin/bash

# =============================================================================
# Configuration
# =============================================================================

PKG_NAME := ncurses
PKG_VERSION := 6.4

# Build directories (relative to this Makefile)
BUILD_ROOT := $(CURDIR)/.build
SRC_DIR := $(BUILD_ROOT)/ncurses-$(PKG_VERSION)
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
SRC_URL := https://ftp.gnu.org/gnu/ncurses/ncurses-$(PKG_VERSION).tar.gz

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
	@echo "Downloading ncurses $(PKG_VERSION)..."
	@mkdir -p $(BUILD_ROOT)
	@cd $(BUILD_ROOT)
	
	@if [ ! -f "ncurses-$(PKG_VERSION).tar.gz" ]; then \
		curl -L $(SRC_URL) -o ncurses-$(PKG_VERSION).tar.gz; \
	fi
	
	@echo "Extracting..."
	@tar -xzf ncurses-$(PKG_VERSION).tar.gz
	
	@echo "✓ ncurses source ready"

# =============================================================================
# Build for iOS Device (arm64)
# =============================================================================

build-ios: $(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libncurses.a

$(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libncurses.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building ncurses for iOS $(IOS_ARCH)"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/ios-$(IOS_ARCH)
	
	cd $(SRC_DIR)
	
	# Set up environment
	export CC="xcrun -sdk $(IOS_SDK) clang"
	export CFLAGS="-arch $(IOS_ARCH) -isysroot $(shell xcrun -sdk $(IOS_SDK) --show-sdk-path) -mios-version-min=$(DEPLOYMENT_TARGET) -O2"
	export LDFLAGS="-arch $(IOS_ARCH) -isysroot $(shell xcrun -sdk $(IOS_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/ios-$(IOS_ARCH) \
		--host=arm-apple-darwin \
		--target=arm-apple-darwin \
		--with-shared \
		--without-ada \
		--without-cxx \
		--without-cxx-binding \
		--without-debug \
		--without-manpages \
		--without-progs \
		--without-tests \
		--enable-widec \
		--disable-stripping
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ ncurses iOS build complete"

# =============================================================================
# Build for iOS Simulator
# =============================================================================

build-simulator: $(BUILD_DIR)/simulator-arm64/lib/libncurses.a $(BUILD_DIR)/simulator-x86_64/lib/libncurses.a

$(BUILD_DIR)/simulator-arm64/lib/libncurses.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building ncurses for Simulator arm64"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/simulator-arm64
	
	cd $(SRC_DIR)
	$(MAKE) distclean || true
	
	export CC="xcrun -sdk $(SIM_SDK) clang"
	export CFLAGS="-arch arm64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path) -mios-simulator-version-min=$(DEPLOYMENT_TARGET) -O2"
	export LDFLAGS="-arch arm64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/simulator-arm64 \
		--host=arm-apple-darwin \
		--with-shared \
		--without-ada \
		--without-cxx \
		--without-cxx-binding \
		--without-debug \
		--without-manpages \
		--without-progs \
		--without-tests \
		--enable-widec \
		--disable-stripping
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ ncurses simulator arm64 build complete"

$(BUILD_DIR)/simulator-x86_64/lib/libncurses.a: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building ncurses for Simulator x86_64"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/simulator-x86_64
	
	cd $(SRC_DIR)
	$(MAKE) distclean || true
	
	export CC="xcrun -sdk $(SIM_SDK) clang"
	export CFLAGS="-arch x86_64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path) -mios-simulator-version-min=$(DEPLOYMENT_TARGET) -O2"
	export LDFLAGS="-arch x86_64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/simulator-x86_64 \
		--host=x86_64-apple-darwin \
		--with-shared \
		--without-ada \
		--without-cxx \
		--without-cxx-binding \
		--without-debug \
		--without-manpages \
		--without-progs \
		--without-tests \
		--enable-widec \
		--disable-stripping
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ ncurses simulator x86_64 build complete"

# =============================================================================
# Create XCFramework
# =============================================================================

xcframework: $(XCFRAMEWORK)

$(XCFRAMEWORK): $(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libncurses.a $(BUILD_DIR)/simulator-arm64/lib/libncurses.a $(BUILD_DIR)/simulator-x86_64/lib/libncurses.a
	@echo "========================================"
	@echo "Creating XCFramework"
	@echo "========================================"
	
	@mkdir -p $(OUTPUT_DIR)
	@rm -rf $@
	
	# Create framework for iOS device
	@mkdir -p $@/ios-$(IOS_ARCH)
	@cp -r $(BUILD_DIR)/ios-$(IOS_ARCH)/include $@/ios-$(IOS_ARCH)/
	@mkdir -p $@/ios-$(IOS_ARCH)/lib
	@cp $(BUILD_DIR)/ios-$(IOS_ARCH)/lib/libncurses*.a $@/ios-$(IOS_ARCH)/lib/ 2>/dev/null || true
	
	# Create framework for simulator (combined arm64 + x86_64)
	@mkdir -p $@/ios-$(SIM_ARCHS)-simulator
	@cp -r $(BUILD_DIR)/simulator-arm64/include $@/ios-$(SIM_ARCHS)-simulator/
	@mkdir -p $@/ios-$(SIM_ARCHS)-simulator/lib
	
	# Create universal library for simulator
	@lipo -create \
		$(BUILD_DIR)/simulator-arm64/lib/libncurses.a \
		$(BUILD_DIR)/simulator-x86_64/lib/libncurses.a \
		-output $@/ios-$(SIM_ARCHS)-simulator/lib/libncurses.a 2>/dev/null || \
		cp $(BUILD_DIR)/simulator-arm64/lib/libncurses.a $@/ios-$(SIM_ARCHS)-simulator/lib/
	
	@echo "✓ XCFramework created: $@"

# =============================================================================
# Utility Targets
# =============================================================================

clean:
	@echo "Cleaning ncurses build artifacts..."
	@rm -rf $(BUILD_ROOT) $(OUTPUT_DIR)
	@echo "Clean complete"

info: $(XCFRAMEWORK)
	@echo "========================================"
	@echo "ncurses XCFramework Info"
	@echo "========================================"
	@echo "Location: $(XCFRAMEWORK)"
	@echo "Version: $(PKG_VERSION)"
	@echo ""
	@echo "Available platforms:"
	@ls -1 $(XCFRAMEWORK)/ 2>/dev/null || echo "  (not built yet)"

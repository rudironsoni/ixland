# grep XCFramework Build
# iOS cross-compilation with proper architecture support

.ONESHELL:
SHELL := /bin/bash

# =============================================================================
# Configuration
# =============================================================================

PKG_NAME := grep
PKG_VERSION := 3.11

# Build directories (relative to this Makefile)
BUILD_ROOT := $(CURDIR)/.build
SRC_DIR := $(BUILD_ROOT)/grep-$(PKG_VERSION)
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
SRC_URL := https://ftp.gnu.org/gnu/grep/grep-$(PKG_VERSION).tar.gz

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
	@echo "Downloading grep $(PKG_VERSION)..."
	@mkdir -p $(BUILD_ROOT)
	@if [ ! -f $(BUILD_ROOT)/grep-$(PKG_VERSION).tar.gz ]; then \
		cd $(BUILD_ROOT) && curl -L -o grep-$(PKG_VERSION).tar.gz "$(SRC_URL)"; \
	fi
	@echo "Extracting..."
	@cd $(BUILD_ROOT) && tar -xzf grep-$(PKG_VERSION).tar.gz
	@touch $(SRC_DIR)/configure

# =============================================================================
# iOS Device Build (arm64)
# =============================================================================

build-ios: $(BUILD_DIR)/ios-$(IOS_ARCH)/bin/grep

$(BUILD_DIR)/ios-$(IOS_ARCH)/bin/grep: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building grep for iOS $(IOS_ARCH)"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/ios-$(IOS_ARCH)
	
	cd $(SRC_DIR)
	$(MAKE) distclean 2>/dev/null || true
	
	# Set up environment
	export CC="xcrun -sdk $(IOS_SDK) clang"
	export CFLAGS="-arch $(IOS_ARCH) -isysroot $(shell xcrun -sdk $(IOS_SDK) --show-sdk-path) -mios-version-min=$(DEPLOYMENT_TARGET) -O2"
	export LDFLAGS="-arch $(IOS_ARCH) -isysroot $(shell xcrun -sdk $(IOS_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/ios-$(IOS_ARCH) \
		--host=arm-apple-darwin \
		--target=arm-apple-darwin \
		--disable-nls \
		--disable-dependency-tracking \
		--disable-perl-regexp
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ grep iOS build complete"

# =============================================================================
# iOS Simulator Build
# =============================================================================

build-simulator: $(BUILD_DIR)/simulator-arm64/bin/grep $(BUILD_DIR)/simulator-x86_64/bin/grep

$(BUILD_DIR)/simulator-arm64/bin/grep: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building grep for Simulator arm64"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/simulator-arm64
	
	cd $(SRC_DIR)
	$(MAKE) distclean 2>/dev/null || true
	
	export CC="xcrun -sdk $(SIM_SDK) clang"
	export CFLAGS="-arch arm64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path) -mios-simulator-version-min=$(DEPLOYMENT_TARGET) -O2"
	export LDFLAGS="-arch arm64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/simulator-arm64 \
		--host=arm-apple-darwin \
		--disable-nls \
		--disable-dependency-tracking \
		--disable-perl-regexp
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ grep simulator arm64 build complete"

$(BUILD_DIR)/simulator-x86_64/bin/grep: $(SRC_DIR)/configure
	@echo "========================================"
	@echo "Building grep for Simulator x86_64"
	@echo "========================================"
	
	@mkdir -p $(BUILD_DIR)/simulator-x86_64
	
	cd $(SRC_DIR)
	$(MAKE) distclean 2>/dev/null || true
	
	export CC="xcrun -sdk $(SIM_SDK) clang"
	export CFLAGS="-arch x86_64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path) -mios-simulator-version-min=$(DEPLOYMENT_TARGET) -O2"
	export LDFLAGS="-arch x86_64 -isysroot $(shell xcrun -sdk $(SIM_SDK) --show-sdk-path)"
	
	./configure \
		--prefix=$(BUILD_DIR)/simulator-x86_64 \
		--host=x86_64-apple-darwin \
		--disable-nls \
		--disable-dependency-tracking \
		--disable-perl-regexp
	
	$(MAKE) -j$(shell sysctl -n hw.ncpu)
	$(MAKE) install
	
	@echo "✓ grep simulator x86_64 build complete"

# =============================================================================
# Create XCFramework
# =============================================================================

xcframework: $(XCFRAMEWORK)

$(XCFRAMEWORK): $(BUILD_DIR)/ios-$(IOS_ARCH)/bin/grep $(BUILD_DIR)/simulator-arm64/bin/grep $(BUILD_DIR)/simulator-x86_64/bin/grep
	@echo "========================================"
	@echo "Creating XCFramework"
	@echo "========================================"
	
	@mkdir -p $(OUTPUT_DIR)
	@rm -rf $@
	
	# Create framework for iOS device
	@mkdir -p $@/ios-$(IOS_ARCH)/bin
	@cp $(BUILD_DIR)/ios-$(IOS_ARCH)/bin/* $@/ios-$(IOS_ARCH)/bin/ 2>/dev/null || true
	
	# Create framework for simulator (combine arm64 + x86_64 binaries)
	@mkdir -p $@/ios-$(SIM_ARCHS)-simulator/bin
	
	# Universal binaries for each tool
	@for tool in grep egrep fgrep; do \
		if [ -f "$(BUILD_DIR)/simulator-arm64/bin/$$tool" ] && [ -f "$(BUILD_DIR)/simulator-x86_64/bin/$$tool" ]; then \
			lipo -create \
				$(BUILD_DIR)/simulator-arm64/bin/$$tool \
				$(BUILD_DIR)/simulator-x86_64/bin/$$tool \
				-output $@/ios-$(SIM_ARCHS)-simulator/bin/$$tool 2>/dev/null || \
				cp $(BUILD_DIR)/simulator-arm64/bin/$$tool $@/ios-$(SIM_ARCHS)-simulator/bin/; \
		elif [ -f "$(BUILD_DIR)/simulator-arm64/bin/$$tool" ]; then \
			cp $(BUILD_DIR)/simulator-arm64/bin/$$tool $@/ios-$(SIM_ARCHS)-simulator/bin/; \
		fi \
	done
	
	@echo "✓ XCFramework created: $@"
	@echo ""
	@echo "Grep tools included:"
	@ls $@/ios-$(IOS_ARCH)/bin/ 2>/dev/null | head -20 || echo "  (binaries in bin/)"

# =============================================================================
# Utility Targets
# =============================================================================

clean:
	@echo "Cleaning grep build artifacts..."
	@rm -rf $(BUILD_ROOT) $(OUTPUT_DIR)
	@echo "Clean complete"

info: $(XCFRAMEWORK)
	@echo "========================================"
	@echo "grep XCFramework Info"
	@echo "========================================"
	@echo "Location: $(XCFRAMEWORK)"
	@echo "Version: $(PKG_VERSION)"
	@echo ""
	@echo "Available platforms:"
	@ls -1 $(XCFRAMEWORK)/ 2>/dev/null || echo "  (not built yet)"
	@echo ""
	@echo "Tools included:"
	@ls $(XCFRAMEWORK)/ios-$(IOS_ARCH)/bin/ 2>/dev/null | wc -l | xargs echo "  Total binaries:"

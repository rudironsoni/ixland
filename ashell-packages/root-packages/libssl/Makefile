# libssl (OpenSSL) XCFramework - Uses prebuilt from xcfs
# OpenSSL is complex to build; we use holzschu's prebuilt XCFramework

.ONESHELL:
SHELL := /bin/bash

# =============================================================================
# Configuration
# =============================================================================

PKG_NAME := libssl
PKG_VERSION := 1.1.1w

# Directories
OUTPUT_DIR := $(CURDIR)/.dist
XCFRAMEWORK := $(OUTPUT_DIR)/$(PKG_NAME).xcframework

# Source: Prebuilt OpenSSL from xcfs (holzschu/openssl-apple)
# This is a well-maintained OpenSSL build for Apple platforms
XCFRAMEWORK_SRC := $(CURDIR)/../../../a-shell-kernel/xcfs/.build/artifacts/xcfs/openssl/openssl.xcframework

# =============================================================================
# Phony Targets
# =============================================================================

.PHONY: all clean info

all: $(XCFRAMEWORK)

# =============================================================================
# XCFramework Creation
# =============================================================================

$(XCFRAMEWORK):
	@echo "========================================"
	@echo "Setting up OpenSSL XCFramework"
	@echo "========================================"
	
	@if [ ! -d "$(XCFRAMEWORK_SRC)" ]; then \
		echo "Error: Source XCFramework not found at $(XCFRAMEWORK_SRC)"; \
		echo "Please ensure xcfs dependencies are built first."; \
		echo "Run: make -C a-shell-kernel xcframework"; \
		exit 1; \
	fi
	
	@mkdir -p $(OUTPUT_DIR)
	@rm -rf $@
	
	@echo "Copying OpenSSL XCFramework..."
	@cp -R $(XCFRAMEWORK_SRC) $@
	
	@echo "Verifying XCFramework contents..."
	@if [ -d "$@/ios-arm64" ] && [ -d "$@/ios-arm64_x86_64-simulator" ]; then \
		echo "✓ XCFramework verified (iOS + Simulator)"; \
	else \
		echo "⚠ Warning: Expected platforms not found"; \
		ls -la $@/; \
	fi
	
	@echo "XCFramework created: $@"
	@echo ""
	@echo "Note: Using prebuilt OpenSSL from xcfs (holzschu/openssl-apple)"
	@echo "Version: $(PKG_VERSION)"
	@echo "Location: $@"

# =============================================================================
# Utility Targets
# =============================================================================

clean:
	@echo "Cleaning libssl build artifacts..."
	@rm -rf $(OUTPUT_DIR)
	@echo "Clean complete"

info: $(XCFRAMEWORK)
	@echo "========================================"
	@echo "OpenSSL XCFramework Info"
	@echo "========================================"
	@echo "Location: $(XCFRAMEWORK)"
	@echo ""
	@echo "Available platforms:"
	@ls -1 $(XCFRAMEWORK)/ | grep -v Info.plist || echo "  (checking structure...)"
	@echo ""
	@echo "Libraries:"
	@find $(XCFRAMEWORK) -name "*.a" -o -name "openssl" 2>/dev/null | head -5
	@echo ""
	@echo "Headers:"
	@find $(XCFRAMEWORK) -name "*.h" 2>/dev/null | head -5

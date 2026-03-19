# a-Shell Next - Top-level Makefile
# Unified build system for the entire project
# 
# Usage:
#   make                  - Build everything
#   make build            - Build the a-shell-kernel framework
#   make build-ios        - Build for iOS device only
#   make build-simulator  - Build for iOS Simulator only
#   make clean            - Clean all build artifacts
#   make test             - Run tests
#   make install          - Install built artifacts
#
# This Makefile delegates to component-specific Makefiles

# ==============================================================================
# Configuration
# ==============================================================================

# Build configuration
BUILD_ROOT := build
CONFIGURATION ?= Release

# Component directories
KERNEL_DIR := a-shell-kernel

# ==============================================================================
# Phony Targets
# ==============================================================================
.PHONY: all build build-ios build-simulator clean test install help

# Default target
all: build

# ==============================================================================
# Build Targets
# ==============================================================================

# Build a-shell-kernel XCFramework (both iOS and Simulator)
build:
	@echo "=========================================="
	@echo "Building a-shell-kernel"
	@echo "=========================================="
	@$(MAKE) -C $(KERNEL_DIR) xcframework CONFIGURATION=$(CONFIGURATION)

# Build for iOS device only
build-ios:
	@echo "=========================================="
	@echo "Building a-shell-kernel for iOS Device"
	@echo "=========================================="
	@$(MAKE) -C $(KERNEL_DIR) ios CONFIGURATION=$(CONFIGURATION)

# Build for iOS Simulator only
build-simulator:
	@echo "=========================================="
	@echo "Building a-shell-kernel for iOS Simulator"
	@echo "=========================================="
	@$(MAKE) -C $(KERNEL_DIR) simulator CONFIGURATION=$(CONFIGURATION)

# Clean kernel build artifacts
clean-kernel:
	@$(MAKE) -C $(KERNEL_DIR) clean

# ==============================================================================
# Global Targets
# ==============================================================================

# Clean all build artifacts across all components
clean: clean-kernel
	@echo "Cleaning all build artifacts..."
	@rm -rf $(BUILD_ROOT)
	@echo "Clean complete"

# Run tests
test:
	@echo "Running tests..."
	@$(MAKE) -C $(KERNEL_DIR) test

# Install built artifacts
install: build
	@echo "Installing built artifacts..."
	@$(MAKE) -C $(KERNEL_DIR) install

# ==============================================================================
# Utility Targets
# ==============================================================================

# Show build settings for kernel
settings:
	@$(MAKE) -C $(KERNEL_DIR) settings

# List available build destinations
destinations:
	@$(MAKE) -C $(KERNEL_DIR) destinations

# Dry run - show what would be built
dry-run:
	@echo "Would build:"
	@echo "  - a-shell-kernel XCFramework"
	@$(MAKE) -C $(KERNEL_DIR) dry-run

# ==============================================================================
# Development/Debug Targets
# ==============================================================================

# Build with verbose output
verbose:
	@$(MAKE) -C $(KERNEL_DIR) ios-verbose

# Quick build (Debug configuration)
debug:
	@$(MAKE) kernel CONFIGURATION=Debug

# Release build (explicit)
release:
	@$(MAKE) kernel CONFIGURATION=Release

# ==============================================================================
# Help
# ==============================================================================

help:
	@echo "a-Shell Next - Build System"
	@echo ""
	@echo "Usage: make [target] [VARIABLE=value]"
	@echo ""
	@echo "Main Targets:"
	@echo "  all              - Build everything (default)"
	@echo "  build            - Build a-shell-kernel XCFramework"
	@echo "  build-ios        - Build for iOS device only"
	@echo "  build-simulator  - Build for iOS Simulator only"
	@echo "  clean-kernel     - Clean kernel build artifacts"
	@echo "  clean            - Clean all build artifacts"
	@echo "  test             - Run tests"
	@echo "  install          - Install built artifacts"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Development Targets:"
	@echo "  debug            - Build with Debug configuration"
	@echo "  release          - Build with Release configuration"
	@echo "  verbose          - Build with verbose output"
	@echo "  dry-run          - Show what would be built"
	@echo "  settings         - Show build settings"
	@echo "  destinations     - List available build destinations"
	@echo ""
	@echo "Variables:"
	@echo "  CONFIGURATION=Release    - Build configuration (Debug/Release)"
	@echo "  BUILD_ROOT=build         - Output directory"
	@echo ""
	@echo "Examples:"
	@echo "  make                         # Build everything"
	@echo "  make build CONFIGURATION=Debug"
	@echo "  make build-simulator         # Build simulator only"
	@echo "  make clean                   # Clean everything"

# ==============================================================================
# Project Info
# ==============================================================================

version:
	@echo "a-Shell Next Build System"
	@echo "Project Root: $(shell pwd)"
	@echo "Build Directory: $(BUILD_ROOT)"
	@echo "Configuration: $(CONFIGURATION)"

# a-shell-kernel Makefile
# Proper build system for iOS framework
# Following Linux kernel engineering principles

# ==============================================================================
# Configuration
# ==============================================================================

# Project settings
PROJECT_NAME := a_shell_system
SCHEME := a_shell_system
PROJECT_FILE := $(PROJECT_NAME).xcodeproj

# Build directories
BUILD_ROOT := ../build
DERIVED_DATA_IOS := $(BUILD_ROOT)/DerivedData/ios
DERIVED_DATA_SIM := $(BUILD_ROOT)/DerivedData/simulator
OUTPUT_DIR := $(BUILD_ROOT)

# Framework paths
IOS_FRAMEWORK := $(DERIVED_DATA_IOS)/Build/Products/Release-iphoneos/$(PROJECT_NAME).framework
SIM_FRAMEWORK := $(DERIVED_DATA_SIM)/Build/Products/Release-iphonesimulator/$(PROJECT_NAME).framework
XCFRAMEWORK := $(OUTPUT_DIR)/a-shell-kernel.xcframework

# Xcode settings
XCODEBUILD := xcodebuild
CONFIGURATION := Release

# Architectures
IOS_DESTINATION := generic/platform=iOS
SIM_DESTINATION := generic/platform=iOS Simulator

# ==============================================================================
# Phony Targets
# ==============================================================================
.PHONY: all clean ios simulator xcframework install help test

# Default target: build everything
all: xcframework

# ==============================================================================
# Main Build Targets
# ==============================================================================

# Build iOS device framework (arm64)
ios: $(IOS_FRAMEWORK)

$(IOS_FRAMEWORK): $(PROJECT_FILE) $(wildcard a_shell_system/*.m) $(wildcard a_shell_system/*.h) a_shell_system.m a_shell_system.h ios_error.h
	@echo "=========================================="
	@echo "Building iOS Device Framework (arm64)"
	@echo "=========================================="
	@mkdir -p $(DERIVED_DATA_IOS)
	$(XCODEBUILD) -project $(PROJECT_FILE) \
		-scheme $(SCHEME) \
		-destination '$(IOS_DESTINATION)' \
		-configuration $(CONFIGURATION) \
		-derivedDataPath $(DERIVED_DATA_IOS) \
		clean build \
		|| (echo "iOS build failed - check signing settings" && exit 1)
	@echo "iOS framework built: $@"

# Build iOS Simulator framework (arm64, x86_64)
simulator: $(SIM_FRAMEWORK)

$(SIM_FRAMEWORK): $(PROJECT_FILE) $(wildcard a_shell_system/*.m) $(wildcard a_shell_system/*.h) a_shell_system.m a_shell_system.h ios_error.h
	@echo "=========================================="
	@echo "Building iOS Simulator Framework"
	@echo "=========================================="
	@mkdir -p $(DERIVED_DATA_SIM)
	$(XCODEBUILD) -project $(PROJECT_FILE) \
		-scheme $(SCHEME) \
		-destination '$(SIM_DESTINATION)' \
		-configuration $(CONFIGURATION) \
		-derivedDataPath $(DERIVED_DATA_SIM) \
		clean build \
		|| (echo "Simulator build failed - check signing settings" && exit 1)
	@echo "Simulator framework built: $@"

# Create universal XCFramework
xcframework: $(XCFRAMEWORK)

$(XCFRAMEWORK): $(IOS_FRAMEWORK) $(SIM_FRAMEWORK)
	@echo "=========================================="
	@echo "Creating XCFramework"
	@echo "=========================================="
	@mkdir -p $(OUTPUT_DIR)
	$(XCODEBUILD) -create-xcframework \
		-framework $(IOS_FRAMEWORK) \
		-framework $(SIM_FRAMEWORK) \
		-output $@
	@echo "XCFramework created: $@"

# ==============================================================================
# Utility Targets
# ==============================================================================

# Clean all build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_ROOT)
	@$(XCODEBUILD) -project $(PROJECT_FILE) clean
	@echo "Clean complete"

# Clean only derived data
clean-derived:
	@echo "Cleaning derived data..."
	@rm -rf $(DERIVED_DATA_IOS) $(DERIVED_DATA_SIM)

# Install framework to system (for development)
install: $(XCFRAMEWORK)
	@echo "Installing XCFramework to $(OUTPUT_DIR)"
	@cp -R $(XCFRAMEWORK) $(OUTPUT_DIR)/

# Show help
help:
	@echo "a-shell-kernel Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build XCFramework (default)"
	@echo "  ios          - Build iOS device framework (arm64)"
	@echo "  simulator    - Build iOS Simulator framework"
	@echo "  xcframework  - Create universal XCFramework"
	@echo "  clean        - Remove all build artifacts"
	@echo "  clean-derived- Clean only derived data"
	@echo "  install      - Install XCFramework to output directory"
	@echo "  test         - Run tests (TODO: implement)"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  CONFIGURATION=Release  - Build configuration (Debug/Release)"
	@echo "  BUILD_ROOT=../build    - Output directory"

# ==============================================================================
# Test Targets
# ==============================================================================

# Run tests
test:
	@echo "=========================================="
	@echo "Running Tests"
	@echo "=========================================="
	@echo "TODO: Implement test runner"
	@echo ""
	@echo "Test files available in tests/:"
	@ls -1 tests/*.swift 2>/dev/null | sed 's/^/  - /' || echo "  (No Swift test files found)"
	@echo ""
	@echo "Tests should include:"
	@echo "  - Unit tests for a_shell_system components"
	@echo "  - Integration tests for command execution"
	@echo "  - Performance benchmarks"
	@echo "  - Memory safety tests"

# ==============================================================================
# Debug/Development Targets
# ==============================================================================

# Show build settings
settings:
	$(XCODEBUILD) -project $(PROJECT_FILE) -scheme $(SCHEME) -showBuildSettings

# List available destinations
destinations:
	$(XCODEBUILD) -project $(PROJECT_FILE) -scheme $(SCHEME) -showdestinations

# Dry run - show what would be built
dry-run:
	@echo "Would build:"
	@echo "  iOS Framework: $(IOS_FRAMEWORK)"
	@echo "  Simulator Framework: $(SIM_FRAMEWORK)"
	@echo "  XCFramework: $(XCFRAMEWORK)"

# Verbose build (for debugging)
ios-verbose: CONFIGURATION=Release
ios-verbose:
	$(XCODEBUILD) -project $(PROJECT_FILE) \
		-scheme $(SCHEME) \
		-destination '$(IOS_DESTINATION)' \
		-configuration $(CONFIGURATION) \
		-derivedDataPath $(DERIVED_DATA_IOS) \
		clean build -verbose

# ==============================================================================
# Dependencies
# ==============================================================================

# Ensure build directories exist
$(BUILD_ROOT):
	@mkdir -p $@

$(DERIVED_DATA_IOS): | $(BUILD_ROOT)
	@mkdir -p $@

$(DERIVED_DATA_SIM): | $(BUILD_ROOT)
	@mkdir -p $@

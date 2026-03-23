# libiox - iOS Subsystem for Linux
# Makefile with iOS target support
#
# IMPORTANT: This library is designed for iOS only.
# Running on macOS will cause failures due to iOS-specific APIs and constraints.
#
# Build targets:
#   make ios-device    - Build for iOS device (arm64)
#   make ios-sim       - Build for iOS Simulator (arm64/x86_64)
#
# iOS Deployment Target: 16.0+

# Detect iOS SDK
IOS_SDK_PATH := $(shell xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)
SIM_SDK_PATH := $(shell xcrun --sdk iphonesimulator --show-sdk-path 2>/dev/null)

# Architecture detection
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),arm64)
  SIM_ARCH := arm64
else
  SIM_ARCH := x86_64
endif

# iOS Target Settings
IOS_TARGET := arm64-apple-ios16.0
SIM_TARGET := $(SIM_ARCH)-apple-ios16.0-simulator

# Compiler
CC := clang

# Source files - in dependency order
SOURCES := \
	src/iox/core/iox_init.c \
	src/iox/core/iox_stubs.c \
	src/iox/util/iox_path.c \
	src/iox/core/iox_vfs.c \
	src/iox/core/iox_process.c \
	src/iox/core/iox_context.c \
	src/iox/core/iox_file.c \
	src/iox/core/iox_network.c \
	src/iox/interpose/iox_interpose.c \
	src/iox/wamr/iox_wamr_simple.c

# Default target
.DEFAULT_GOAL := help

.PHONY: all ios-device ios-sim clean check-sdk info help libiox-sim.a libiox-device.a

# Check iOS SDK availability
check-sdk:
	@if [ -z "$(IOS_SDK_PATH)" ]; then \
		echo "ERROR: iOS SDK not found. Install Xcode and iOS SDK."; \
		exit 1; \
	fi
	@if [ -z "$(SIM_SDK_PATH)" ]; then \
		echo "ERROR: iOS Simulator SDK not found. Install Xcode."; \
		exit 1; \
	fi

# Build for iOS Simulator (arm64 or x86_64)
iox-sim: check-sdk libiox-sim.a

libiox-sim.a: $(SOURCES) build/wamr-simulator/libiwasm.a
	@echo "Building for iOS Simulator ($(SIM_TARGET))..."
	@for src in $(SOURCES); do \
		obj=$${src%.c}.o; \
		$(CC) -Wall -Wextra -g -O0 \
			-target $(SIM_TARGET) \
			-isysroot $(SIM_SDK_PATH) \
			-I./include -I./src/iox/internal \
			-DIOX_IOS_BUILD -DIOX_SIMULATOR_BUILD \
			-c $$src -o $$obj; \
	done
	# Create combined library - extract WAMR objects first
	@mkdir -p build/wamr-objs-sim
	@cd build/wamr-objs-sim && ar x ../wamr-simulator/libiwasm.a
	@ar rcs libiox-sim.a $(SOURCES:.c=.o) build/wamr-objs-sim/*.o
	@rm -rf build/wamr-objs-sim
	@echo "Created libiox-sim.a ($(shell ar -t libiox-sim.a | wc -l) objects) for iOS Simulator ($(SIM_TARGET))"

wamr-sim:
	@if [ ! -f build/wamr-simulator/libiwasm.a ]; then \
		echo "Building WAMR for iOS Simulator..."; \
		./build_wamr_ios_static.sh; \
	fi

# Build for iOS device (arm64)
iox-device: check-sdk libiox-device.a

libiox-device.a: $(SOURCES) build/wamr-device/libiwasm.a
	@echo "Building for iOS Device ($(IOS_TARGET))..."
	@for src in $(SOURCES); do \
		obj=$${src%.c}.o; \
		$(CC) -Wall -Wextra -g -O0 \
			-target $(IOS_TARGET) \
			-isysroot $(IOS_SDK_PATH) \
			-I./include -I./src/iox/internal \
			-DIOX_IOS_BUILD \
			-c $$src -o $$obj; \
	done
	@ar rcs libiox-device.a $(SOURCES:.c=.o) build/wamr-device/libiwasm.a
	@echo "Created libiox-device.a for iOS Device ($(IOS_TARGET))"

wamr-device:
	@if [ ! -f build/wamr-device/libiwasm.a ]; then \
		echo "Building WAMR for iOS Device..."; \
		./build_wamr_ios_static.sh; \
	fi

all: ios-sim

clean:
	@rm -f $(SOURCES:.c=.o)
	@rm -f libiox*.a
	@rm -rf test_* tests/test_* *.dSYM tests/*.dSYM
	@echo "Cleaned build artifacts"

help:
	@echo "========================================"
	@echo "libiox - iOS Subsystem for Linux"
	@echo "========================================"
	@echo ""
	@echo "iOS SDK:      $(IOS_SDK_PATH)"
	@echo "Sim SDK:      $(SIM_SDK_PATH)"
	@echo "Device Arch:  $(IOS_TARGET)"
	@echo "Sim Arch:     $(SIM_TARGET)"
	@echo ""
	@echo "Available targets:"
	@echo "  make ios-device   - Build for iOS device (arm64)"
	@echo "  make ios-sim      - Build for iOS Simulator"
	@echo "  make clean        - Clean all build artifacts"
	@echo "  make info         - Show build configuration"
	@echo ""
	@echo "IMPORTANT: This library is for iOS only!"
	@echo "           macOS builds will fail at runtime."
	@echo "========================================"

info: help

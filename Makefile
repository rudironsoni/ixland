# libiox - iOS-only Linux compatibility layer
#
# Architecture policy:
# - libiox and libiwasm are separate iOS artifacts
# - deps/wamr is treated as read-only upstream source
# - WAMR is built out-of-tree by wrapper scripts owned in this repo
# - app consumers use iox_wamr_* only, never raw WAMR APIs

IOS_SDK_PATH := $(shell xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)
SIM_SDK_PATH := $(shell xcrun --sdk iphonesimulator --show-sdk-path 2>/dev/null)

UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),arm64)
SIM_ARCH := arm64
else
SIM_ARCH := x86_64
endif

IOS_TARGET := arm64-apple-ios16.0
SIM_TARGET := $(SIM_ARCH)-apple-ios16.0-simulator

CC := clang
AR := ar
BUILD_ROOT := build
LIB_DIR := lib
WAMR_SCRIPT := ./build_wamr_ios_static.sh
WAMR_INCLUDE_DIR := ./deps/wamr/core/iwasm/include

IOX_SOURCES := \
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

COMMON_CFLAGS := -Wall -Wextra -g -O0 -I./include -I./src/iox/internal -I$(WAMR_INCLUDE_DIR)
SIM_CFLAGS := $(COMMON_CFLAGS) -target $(SIM_TARGET) -isysroot $(SIM_SDK_PATH) -DIOX_IOS_BUILD -DIOX_SIMULATOR_BUILD
DEVICE_CFLAGS := $(COMMON_CFLAGS) -target $(IOS_TARGET) -isysroot $(IOS_SDK_PATH) -DIOX_IOS_BUILD

SIM_OBJ_DIR := $(BUILD_ROOT)/iox-sim/obj
DEVICE_OBJ_DIR := $(BUILD_ROOT)/iox-device/obj

LIBIOX_SIM := $(LIB_DIR)/libiox-sim.a
LIBIOX_DEVICE := $(LIB_DIR)/libiox-device.a
LIBIWASM_SIM := $(BUILD_ROOT)/wamr-simulator/libiwasm.a
LIBIWASM_DEVICE := $(BUILD_ROOT)/wamr-device/libiwasm.a

.DEFAULT_GOAL := help

.PHONY: help info check-sdk clean \
	iox-sim iox-device wamr-sim wamr-device sdk-sim sdk-device sdk

check-sdk:
	@if [ -z "$(IOS_SDK_PATH)" ]; then \
		echo "ERROR: iOS SDK not found. Install Xcode and iOS SDK."; \
		exit 1; \
	fi
	@if [ -z "$(SIM_SDK_PATH)" ]; then \
		echo "ERROR: iOS Simulator SDK not found. Install Xcode."; \
		exit 1; \
	fi

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

$(SIM_OBJ_DIR):
	@mkdir -p $(SIM_OBJ_DIR)

$(DEVICE_OBJ_DIR):
	@mkdir -p $(DEVICE_OBJ_DIR)

iox-sim: check-sdk $(LIBIOX_SIM)

$(LIBIOX_SIM): $(IOX_SOURCES) | check-sdk $(LIB_DIR) $(SIM_OBJ_DIR)
	@echo "Building libiox for iOS Simulator ($(SIM_TARGET))..."
	@for src in $(IOX_SOURCES); do \
		obj="$(SIM_OBJ_DIR)/$$(basename $${src%.c}.o)"; \
		$(CC) $(SIM_CFLAGS) -c $$src -o $$obj; \
	done
	@$(AR) rcs $@ $(SIM_OBJ_DIR)/*.o
	@echo "Created $@"

iox-device: check-sdk $(LIBIOX_DEVICE)

$(LIBIOX_DEVICE): $(IOX_SOURCES) | check-sdk $(LIB_DIR) $(DEVICE_OBJ_DIR)
	@echo "Building libiox for iOS Device ($(IOS_TARGET))..."
	@for src in $(IOX_SOURCES); do \
		obj="$(DEVICE_OBJ_DIR)/$$(basename $${src%.c}.o)"; \
		$(CC) $(DEVICE_CFLAGS) -c $$src -o $$obj; \
	done
	@$(AR) rcs $@ $(DEVICE_OBJ_DIR)/*.o
	@echo "Created $@"

wamr-sim: check-sdk $(LIBIWASM_SIM)

$(LIBIWASM_SIM): | check-sdk
	@$(WAMR_SCRIPT) simulator

wamr-device: check-sdk $(LIBIWASM_DEVICE)

$(LIBIWASM_DEVICE): | check-sdk
	@$(WAMR_SCRIPT) device

sdk-sim: iox-sim wamr-sim
	@echo "Built simulator SDK artifacts: $(LIBIOX_SIM) + $(LIBIWASM_SIM)"

sdk-device: iox-device wamr-device
	@echo "Built device SDK artifacts: $(LIBIOX_DEVICE) + $(LIBIWASM_DEVICE)"

sdk: sdk-sim sdk-device
	@echo "Built all iOS SDK artifacts"

clean:
	@rm -rf $(BUILD_ROOT)/iox-sim $(BUILD_ROOT)/iox-device
	@rm -f $(LIBIOX_SIM) $(LIBIOX_DEVICE)
	@rm -rf test_* tests/test_* *.dSYM tests/*.dSYM
	@echo "Cleaned libiox build artifacts"

help:
	@echo "========================================"
	@echo "libiox / WAMR iOS SDK build"
	@echo "========================================"
	@echo ""
	@echo "Platform policy: iOS only"
	@echo "WAMR policy: external dependency, out-of-tree build, no edits in deps/wamr"
	@echo "Customer API policy: use iox_wamr_* only"
	@echo ""
	@echo "iOS SDK:      $(IOS_SDK_PATH)"
	@echo "Sim SDK:      $(SIM_SDK_PATH)"
	@echo "Device Arch:  $(IOS_TARGET)"
	@echo "Sim Arch:     $(SIM_TARGET)"
	@echo ""
	@echo "Targets:"
	@echo "  make iox-sim      - build lib/libiox-sim.a"
	@echo "  make iox-device   - build lib/libiox-device.a"
	@echo "  make wamr-sim     - build build/wamr-simulator/libiwasm.a"
	@echo "  make wamr-device  - build build/wamr-device/libiwasm.a"
	@echo "  make sdk-sim      - build both simulator artifacts"
	@echo "  make sdk-device   - build both device artifacts"
	@echo "  make sdk          - build all iOS artifacts"
	@echo "  make clean        - clean libiox artifacts"
	@echo ""
	@echo "Note: app/test targets must link libiox and libiwasm separately."
	@echo "========================================"

info: help

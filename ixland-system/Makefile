# libixland - iOS-only Linux compatibility layer
#
# Architecture policy:
# - libixland and libiwasm are separate iOS artifacts
# - deps/wamr is treated as read-only upstream source
# - WAMR is built out-of-tree by wrapper scripts owned in this repo
# - app consumers use ixland_wamr_* only, never raw WAMR APIs

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

IXLAND_SOURCES := \
	src/ixland/core/ixland_init.c \
	src/ixland/core/ixland_libc_delegate.c \
	src/ixland/util/ixland_path.c \
	src/ixland/core/ixland_vfs.c \
	src/ixland/core/ixland_process.c \
	src/ixland/core/ixland_context.c \
	src/ixland/core/ixland_file.c \
	src/ixland/core/ixland_network.c \
	src/ixland/interpose/ixland_interpose.c \
	src/ixland/wamr/ixland_wamr_simple.c

COMMON_CFLAGS := -Wall -Wextra -g -O0 -I./include -I./src/ixland/internal -I$(WAMR_INCLUDE_DIR)
SIM_CFLAGS := $(COMMON_CFLAGS) -target $(SIM_TARGET) -isysroot $(SIM_SDK_PATH) -DIXLAND_IOS_BUILD -DIXLAND_SIMULATOR_BUILD
DEVICE_CFLAGS := $(COMMON_CFLAGS) -target $(IOS_TARGET) -isysroot $(IOS_SDK_PATH) -DIXLAND_IOS_BUILD

SIM_OBJ_DIR := $(BUILD_ROOT)/ixland-sim/obj
DEVICE_OBJ_DIR := $(BUILD_ROOT)/ixland-device/obj

LIBIXLAND_SIM := $(LIB_DIR)/libixland-sim.a
LIBIXLAND_DEVICE := $(LIB_DIR)/libixland-device.a
LIBIWASM_SIM := $(BUILD_ROOT)/wamr-simulator/libiwasm.a
LIBIWASM_DEVICE := $(BUILD_ROOT)/wamr-device/libiwasm.a

.DEFAULT_GOAL := help

.PHONY: help info check-sdk clean \
	ixland-sim ixland-device wamr-sim wamr-device sdk-sim sdk-device sdk

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

ixland-sim: check-sdk $(LIBIXLAND_SIM)

$(LIBIXLAND_SIM): $(IXLAND_SOURCES) | check-sdk $(LIB_DIR) $(SIM_OBJ_DIR)
	@echo "Building libixland for iOS Simulator ($(SIM_TARGET))..."
	@for src in $(IXLAND_SOURCES); do \
		obj="$(SIM_OBJ_DIR)/$$(basename $${src%.c}.o)"; \
		$(CC) $(SIM_CFLAGS) -c $$src -o $$obj; \
	done
	@$(AR) rcs $@ $(SIM_OBJ_DIR)/*.o
	@echo "Created $@"

ixland-device: check-sdk $(LIBIXLAND_DEVICE)

$(LIBIXLAND_DEVICE): $(IXLAND_SOURCES) | check-sdk $(LIB_DIR) $(DEVICE_OBJ_DIR)
	@echo "Building libixland for iOS Device ($(IOS_TARGET))..."
	@for src in $(IXLAND_SOURCES); do \
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

sdk-sim: ixland-sim wamr-sim
	@echo "Built simulator SDK artifacts: $(LIBIXLAND_SIM) + $(LIBIWASM_SIM)"

sdk-device: ixland-device wamr-device
	@echo "Built device SDK artifacts: $(LIBIXLAND_DEVICE) + $(LIBIWASM_DEVICE)"

sdk: sdk-sim sdk-device
	@echo "Built all iOS SDK artifacts"

clean:
	@rm -rf $(BUILD_ROOT)/ixland-sim $(BUILD_ROOT)/ixland-device
	@rm -f $(LIBIXLAND_SIM) $(LIBIXLAND_DEVICE)
	@rm -rf test_* tests/test_* *.dSYM tests/*.dSYM
	@echo "Cleaned libixland build artifacts"

help:
	@echo "========================================"
	@echo "libixland / WAMR iOS SDK build"
	@echo "========================================"
	@echo ""
	@echo "Platform policy: iOS only"
	@echo "WAMR policy: external dependency, out-of-tree build, no edits in deps/wamr"
	@echo "Customer API policy: use ixland_wamr_* only"
	@echo ""
	@echo "iOS SDK:      $(IOS_SDK_PATH)"
	@echo "Sim SDK:      $(SIM_SDK_PATH)"
	@echo "Device Arch:  $(IOS_TARGET)"
	@echo "Sim Arch:     $(SIM_TARGET)"
	@echo ""
	@echo "Targets:"
	@echo "  make ixland-sim      - build lib/libixland-sim.a"
	@echo "  make ixland-device   - build lib/libixland-device.a"
	@echo "  make wamr-sim     - build build/wamr-simulator/libiwasm.a"
	@echo "  make wamr-device  - build build/wamr-device/libiwasm.a"
	@echo "  make sdk-sim      - build both simulator artifacts"
	@echo "  make sdk-device   - build both device artifacts"
	@echo "  make sdk          - build all iOS artifacts"
	@echo "  make clean        - clean libixland artifacts"
	@echo ""
	@echo "Note: app/test targets must link libixland and libiwasm separately."
	@echo "========================================"

info: help

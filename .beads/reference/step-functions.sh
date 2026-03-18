#!/bin/bash
# Reference implementation of all ashell_step_* functions
# Copy and modify for actual implementation
#
# Source: /home/rrj/.beads/reference/step-functions.sh
# Purpose: Working examples of build system step functions
# Usage: Reference when implementing ashell_package.sh or package build.sh

# =============================================================================
# EXTRACT PACKAGE STEP
# =============================================================================

ashell_step_extract_package() {
    local url="$1"
    local sha256="$2"
    local dest_dir="$3"

    ashell_log_info "Downloading from $url"

    mkdir -p "$dest_dir"
    local tmp_file="$dest_dir/.download.tmp"

    # Download with retry
    local retries=3
    while ((retries-- > 0)); do
        if curl -fsSL --connect-timeout 30 "$url" -o "$tmp_file" 2>/dev/null; then
            break
        fi
        ashell_log_warn "Download failed, retrying... ($retries left)"
        sleep 2
    done

    if [[ ! -f "$tmp_file" ]]; then
        ashell_log_error "Download failed after all retries"
        return 1
    fi

    # Verify SHA256
    if command -v shasum >/dev/null 2>&1; then
        local computed_hash=$(shasum -a 256 "$tmp_file" | cut -d' ' -f1)
    else
        local computed_hash=$(sha256sum "$tmp_file" | cut -d' ' -f1)
    fi

    if [[ "$computed_hash" != "$sha256" ]]; then
        ashell_log_error "SHA256 mismatch!"
        ashell_log_error "  Expected: $sha256"
        ashell_log_error "  Computed: $computed_hash"
        rm -f "$tmp_file"
        return 1
    fi

    # Extract based on extension
    case "$url" in
        *.tar.gz|*.tgz)
            tar -xzf "$tmp_file" -C "$dest_dir" --strip-components=1
            ;;
        *.tar.bz2|*.tbz2)
            tar -xjf "$tmp_file" -C "$dest_dir" --strip-components=1
            ;;
        *.tar.xz)
            tar -xJf "$tmp_file" -C "$dest_dir" --strip-components=1
            ;;
        *.zip)
            unzip -q "$tmp_file" -d "$dest_dir"
            ;;
        *)
            ashell_log_error "Unknown archive format: $url"
            return 1
            ;;
    esac

    rm -f "$tmp_file"
    ashell_log_success "Extracted to $dest_dir"
    return 0
}

# =============================================================================
# PATCH PACKAGE STEP
# =============================================================================

ashell_step_patch_package() {
    local src_dir="$1"
    local patches_dir="$2"

    if [[ ! -d "$patches_dir" ]]; then
        ashell_log_info "No patches directory found"
        return 0
    fi

    # Find and apply patches in order
    local patches=($(ls -1 "$patches_dir"/*.patch 2>/dev/null | sort))

    if [[ ${#patches[@]} -eq 0 ]]; then
        ashell_log_info "No patches to apply"
        return 0
    fi

    cd "$src_dir"

    for patch in "${patches[@]}"; do
        ashell_log_info "Applying patch: $(basename "$patch")"
        if ! patch -p1 < "$patch"; then
            ashell_log_error "Failed to apply patch: $patch"
            return 1
        fi
    done

    ashell_log_success "Applied ${#patches[@]} patch(es)"
    return 0
}

# =============================================================================
# CONFIGURE STEP
# =============================================================================

ashell_step_configure() {
    local src_dir="$1"
    local build_dir="$2"
    local install_dir="$3"

    cd "$build_dir"

    # Set up iOS cross-compilation environment
    export CC="${ASHELL_CC}"
    export CXX="${ASHELL_CXX}"
    export CFLAGS="${ASHELL_CFLAGS} -arch arm64 -isysroot ${ASHELL_SDK_PATH} -mios-version-min=${ASHELL_DEPLOYMENT_TARGET}"
    export CXXFLAGS="${ASHELL_CXXFLAGS} -arch arm64 -isysroot ${ASHELL_SDK_PATH} -mios-version-min=${ASHELL_DEPLOYMENT_TARGET}"
    export LDFLAGS="${ASHELL_LDFLAGS} -arch arm64 -isysroot ${ASHELL_SDK_PATH}"

    # Substitute @ASHELL_PREFIX@ with actual path
    local configure_args="${ASHELL_PKG_EXTRA_CONFIGURE_ARGS//@ASHELL_PREFIX@/$install_dir}"

    if [[ -f "$src_dir/configure" ]]; then
        ashell_log_info "Running autotools configure..."
        "$src_dir/configure" \
            --prefix="$install_dir" \
            --host="${ASHELL_TARGET_PLATFORM}" \
            --build="${ASHELL_HOST_PLATFORM}" \
            $configure_args
    elif [[ -f "$src_dir/CMakeLists.txt" ]]; then
        ashell_log_info "Running cmake..."
        cmake "$src_dir" \
            -DCMAKE_INSTALL_PREFIX="$install_dir" \
            -DCMAKE_SYSTEM_NAME=iOS \
            -DCMAKE_OSX_ARCHITECTURES=arm64 \
            -DCMAKE_OSX_DEPLOYMENT_TARGET="${ASHELL_DEPLOYMENT_TARGET}" \
            -DCMAKE_SYSROOT="${ASHELL_SDK_PATH}" \
            $configure_args
    else
        ashell_log_warn "No configure script found, skipping configuration"
    fi

    return 0
}

# =============================================================================
# MAKE STEP
# =============================================================================

ashell_step_make() {
    local build_dir="$1"
    local jobs="${ASHELL_MAKE_JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

    cd "$build_dir"

    ashell_log_info "Building with $jobs parallel jobs..."

    # Set compiler flags
    export CC="${ASHELL_CC}"
    export CXX="${ASHELL_CXX}"
    export CFLAGS="${ASHELL_CFLAGS} -arch arm64 -isysroot ${ASHELL_SDK_PATH} -mios-version-min=${ASHELL_DEPLOYMENT_TARGET}"

    if ! make -j"$jobs" ${ASHELL_PKG_EXTRA_MAKE_ARGS}; then
        ashell_log_error "Build failed"
        return 1
    fi

    ashell_log_success "Build completed"
    return 0
}

# =============================================================================
# MAKE INSTALL STEP
# =============================================================================

ashell_step_make_install() {
    local build_dir="$1"
    local staging_dir="$2"

    cd "$build_dir"

    ashell_log_info "Installing to staging directory..."

    if ! make DESTDIR="$staging_dir" install; then
        ashell_log_error "Install failed"
        return 1
    fi

    ashell_log_success "Installed to $staging_dir"
    return 0
}

# =============================================================================
# CREATE XCFRAMEWORK STEP
# =============================================================================

ashell_step_create_xcframework() {
    local staging_dir="$1"
    local output_dir="$2"
    local framework_name="${3:-$ASHELL_PKG_NAME}"

    ashell_log_info "Creating XCFramework..."

    # Create framework structure
    local framework_dir="$output_dir/${framework_name}.framework"
    mkdir -p "$framework_dir/Headers"

    # Find and copy library
    local lib_files=($(find "$staging_dir" -name "*.a" -o -name "*.dylib" 2>/dev/null))

    if [[ ${#lib_files[@]} -eq 0 ]]; then
        # Look for executables
        local bin_files=($(find "$staging_dir" -type f -perm +111 2>/dev/null | head -5))
        if [[ ${#bin_files[@]} -eq 0 ]]; then
            ashell_log_error "No libraries or executables found in staging"
            return 1
        fi
        # For executables, we create a special framework structure
        cp "${bin_files[0]}" "$framework_dir/$framework_name"
    else
        # Use the first library found
        cp "${lib_files[0]}" "$framework_dir/$framework_name"
    fi

    # Copy headers if present
    local header_files=($(find "$staging_dir" -name "*.h" 2>/dev/null | head -20))
    if [[ ${#header_files[@]} -gt 0 ]]; then
        for header in "${header_files[@]}"; do
            cp "$header" "$framework_dir/Headers/"
        done
    fi

    # Generate Info.plist
    cat > "$framework_dir/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>${framework_name}</string>
    <key>CFBundleIdentifier</key>
    <string>com.rudironsoni.ashell.${framework_name}</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>${framework_name}</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>${ASHELL_PKG_VERSION}</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>MinimumOSVersion</key>
    <string>${ASHELL_DEPLOYMENT_TARGET}</string>
</dict>
</plist>
EOF

    # Create XCFramework wrapper
    local xcframework_dir="$output_dir/${framework_name}.xcframework"
    rm -rf "$xcframework_dir"
    mkdir -p "$xcframework_dir"

    # Move framework into XCFramework structure
    # For now, single architecture
    mv "$framework_dir" "$xcframework_dir/ios-arm64/"

    # Generate Info.plist for XCFramework
    cat > "$xcframework_dir/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>AvailableLibraries</key>
    <array>
        <dict>
            <key>LibraryIdentifier</key>
            <string>ios-arm64</string>
            <key>LibraryPath</key>
            <string>${framework_name}.framework</string>
            <key>SupportedArchitectures</key>
            <array>
                <string>arm64</string>
            </array>
            <key>SupportedPlatform</key>
            <string>ios</string>
        </dict>
    </array>
    <key>CFBundlePackageType</key>
    <string>XFWK</string>
    <key>XCFrameworkFormatVersion</key>
    <string>1.0</string>
</dict>
</plist>
EOF

    ashell_log_success "Created $xcframework_dir"
    return 0
}

# =============================================================================
# GENERATE PLIST STEP
# =============================================================================

ashell_step_generate_plist() {
    local staging_dir="$1"
    local output_file="$2"
    local commands="${3:-$ASHELL_PKG_COMMANDS}"

    ashell_log_info "Generating command plist..."

    # commands format: "cmd1:entry1::type1 cmd2:entry2::type2"
    # type is optional: no (default), root, file

    cat > "$output_file" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
EOF

    # Parse commands
    IFS=' ' read -ra CMD_ARRAY <<< "$commands"
    for cmd_spec in "${CMD_ARRAY[@]}"; do
        IFS=':' read -ra PARTS <<< "$cmd_spec"
        local cmd="${PARTS[0]}"
        local entry="${PARTS[1]:-${cmd}_main}"
        local hash="${PARTS[2]}"
        local type="${PARTS[3]:-no}"

        # Generate hash if not provided
        if [[ -z "$hash" ]]; then
            hash=$(echo -n "$cmd" | shasum -a 256 | cut -c1-8)
        fi

        cat >> "$output_file" <<EOF
    <key>$cmd</key>
    <array>
        <string>${ASHELL_PKG_NAME}.framework/${ASHELL_PKG_NAME}</string>
        <string>$entry</string>
        <string>$hash</string>
        <string>$type</string>
    </array>
EOF
    done

    cat >> "$output_file" <<'EOF'
</dict>
</plist>
EOF

    ashell_log_success "Generated $output_file"
    return 0
}

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

ashell_log_info() {
    echo "[INFO] $1"
}

ashell_log_warn() {
    echo "[WARN] $1" >&2
}

ashell_log_error() {
    echo "[ERROR] $1" >&2
}

ashell_log_success() {
    echo "[SUCCESS] $1"
}

# Example usage:
# source "$ASHELL_PKG_BUILDER_DIR/ashell_package.sh"
# ashell_step_extract_package "$url" "$sha256" "$dest"
# ashell_step_patch_package "$dest" "$patches_dir"

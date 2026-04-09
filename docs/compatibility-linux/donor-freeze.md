# Donor freeze (G0)

## Donor reference

- Repository: https://github.com/rudironsoni/ish
- Branch: feat/aarch64-migration
- SHA: cba9421dbd9ebe0e27745b02867ce326071554e1
- Import date: 2026-04-09
- Scope: Linux compatibility lane extraction only

## Included families (G0-G2)

- emu/aarch64/* (AArch64 guest emulation)
- emu/* common internals (tlb/mmu/cpu/interrupt helpers)
- tcti/* and tcti/aarch64/*
- Linux ABI leaf headers (lane-local only)
- Observability stubs for compatibility lane
- Documentation under docs/compatibility-linux/

## Excluded families (G0-G2)

- Donor terminal app code
- TerminalViewController.m
- Donor compile-time runtime modes
- Canonical task/signal/fs/proc/dev/tty/pty/socket/poll/exec ownership
- Donor Package.swift and IXLand.xcodeproj as build truth

## Ownership summary

- Canonical semantic ownership stays in ixland-system.
- Linux ABI/runtime translation stays in ixland-system/compatibility/linux.
- Host boundary is ixland-system/runtime/linux_host.
- ixland-app remains UX-only with adapter boundary.

## Gates

- G0: freeze donor SHA and scope (this document)
- G1: import allowed families into compatibility/linux and compile
- G2: host-boundary linked; runtime bootstrap only through host-issued handle

## Acceptance criteria

- All imports traceable to this SHA.
- No direct donor UI/runtime coupling.
- CMake targets build without donor SwiftPM/Xcode artifacts.
- Compatibility lane remains quarantined under compatibility/linux.

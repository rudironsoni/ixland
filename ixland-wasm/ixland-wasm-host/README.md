# ixland-wasm-host

Host-service contract between guest runtimes and iXland.

## Purpose

This boundary defines the interface for how WebAssembly guests interact with the iXland host environment.

## Service Areas

Future host services will include:

- **File descriptor I/O** - Read, write, poll on FDs
- **Path and filesystem** - Open, stat, mkdir, symlink
- **Clocks and timers** - Wall clock, monotonic clock, sleeps
- **Random number generation** - Secure random
- **Sockets and networking** - TCP, UDP, address resolution
- **Process semantics** - Exit, interruption, environment

## Current State

Host services are currently implemented within `ixland-system`. This boundary will receive extraction once the service contracts are well-defined.

## Design Principle

Host services should provide iXland-native semantics to guests, not just passthrough to iOS APIs. This ensures consistent behavior between native and WASM execution.

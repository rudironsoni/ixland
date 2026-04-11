# IXLandWasmHost

Host-service contract between guest runtimes and IXLand.

## Purpose

This boundary defines the interface for how WebAssembly guests interact with the IXLand host environment.

## Service Areas

Host services include:

- **File descriptor I/O** - `fd_read`, `fd_write`, `fd_close`
- **Path and filesystem** - `path_open`, `path_stat`, `path_mkdir`
- **Clocks and timers** - `clock_gettime`, `nanosleep`
- **Random number generation** - `random_get`
- **Sockets and networking** - `socket_create`, `socket_connect`, etc.
- **Process semantics** - `proc_exit`, `proc_is_interrupted`

## Public Headers

- `../include/ixland/wasm/types.h` - Fundamental types
- `../include/ixland/wasm/host.h` - Host service vtable

## Current State

- Contract is defined in public headers (host.h)
- Implementation remains in `IXLandSystem`
- Host services provide IXLand-native semantics, not iOS passthrough

## Documentation

- `../../docs/WASM_BOUNDARY_SPEC.md` - Section on IXLandWasmHost

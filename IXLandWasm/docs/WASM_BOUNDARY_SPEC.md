# iXland WebAssembly Boundary Specification

## Overview

This document specifies the WebAssembly boundary architecture for iXland. It defines how WebAssembly guests interact with the iXland host environment and how different Wasm engines can be integrated.

## Current State

The current implementation uses WAMR (WebAssembly Micro Runtime) as the backend. All WAMR-specific code lives in `IXLandSystem`. The boundaries defined here (`IXLandWasmEngine`, `IXLandWasmHost`, `IXLandWasmWASI`) are future extraction targets.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   WebAssembly Guest                         │
│              (WASM module, WASI-compatible)                 │
└──────────────────────────┬──────────────────────────────────┘
                           │ WASI ABI calls
┌──────────────────────────▼──────────────────────────────────┐
│                  IXLandWasmWASI                           │
│          WASI syscall mapping to iXland semantics           │
└──────────────────────────┬──────────────────────────────────┘
                           │ Host service calls
┌──────────────────────────▼──────────────────────────────────┐
│                  IXLandWasmHost                           │
│     Host contract: fd, path, clock, random, socket          │
└──────────────────────────┬──────────────────────────────────┘
                           │ Engine-neutral calls
┌──────────────────────────▼──────────────────────────────────┐
│                  IXLandWasmEngine                         │
│          Engine-neutral runtime interface                   │
│    (WAMR, Wasmtime, etc. implement this interface)          │
└──────────────────────────┬──────────────────────────────────┘
                           │ Engine-specific calls
┌──────────────────────────▼──────────────────────────────────┐
│                     iXland kernel                              │
│           (current implementation in IXLandSystem)         │
└─────────────────────────────────────────────────────────────┘
```

## IXLandWasmEngine (Engine Contract)

### Purpose

Abstract interface that allows iXland to work with different WebAssembly engines without tight coupling.

### Interface (Draft)

```c
/* Engine lifecycle */
int ixland_wasm_engine_init(ixland_wasm_engine_t **engine, const ixland_wasm_config_t *config);
void ixland_wasm_engine_destroy(ixland_wasm_engine_t *engine);

/* Module management */
int ixland_wasm_module_load(ixland_wasm_engine_t *engine, const uint8_t *wasm_bytes,
                         size_t wasm_size, ixland_wasm_module_t **module);
void ixland_wasm_module_unload(ixland_wasm_module_t *module);

/* Instance management */
int ixland_wasm_instance_create(ixland_wasm_module_t *module,
                              ixland_wasm_instance_t **instance);
void ixland_wasm_instance_destroy(ixland_wasm_instance_t *instance);

/* Execution */
int ixland_wasm_instance_run(ixland_wasm_instance_t *instance,
                          const char *entry_point,
                          int argc, char *argv[]);

/* Memory access */
void *ixland_wasm_memory_get(ixland_wasm_instance_t *instance, uint32_t offset);
uint32_t ixland_wasm_memory_size(ixland_wasm_instance_t *instance);
```

### Implementation Notes

- WAMR implements this interface in `IXLandSystem/runtime/wasi/` currently
- Future: Move WAMR-specific implementation to a plugin/module
- Future: Wasmtime could implement same interface

## IXLandWasmHost (Host Services)

### Purpose

Define the contract between guest runtimes and iXland host semantics.

### Service Areas

#### 1. File Descriptor I/O

```c
/* Read from file descriptor */
ssize_t ixland_host_fd_read(int fd, void *buf, size_t count);

/* Write to file descriptor */
ssize_t ixland_host_fd_write(int fd, const void *buf, size_t count);

/* Close file descriptor */
int ixland_host_fd_close(int fd);

/* Seek in file descriptor */
off_t ixland_host_fd_seek(int fd, off_t offset, int whence);
```

#### 2. Path and Filesystem

```c
/* Open file at path */
int ixland_host_path_open(const char *path, int flags, mode_t mode);

/* Get file status */
int ixland_host_path_stat(const char *path, struct stat *statbuf);

/* Create directory */
int ixland_host_path_mkdir(const char *path, mode_t mode);

/* Remove file/directory */
int ixland_host_path_unlink(const char *path);

/* Read symbolic link */
ssize_t ixland_host_path_readlink(const char *path, char *buf, size_t bufsiz);
```

#### 3. Clocks and Timers

```c
/* Get wall clock time */
int ixland_host_clock_gettime(clockid_t clk_id, struct timespec *tp);

/* Get monotonic clock time */
int ixland_host_clock_getres(clockid_t clk_id, struct timespec *tp);

/* Sleep */
int ixland_host_nanosleep(const struct timespec *req, struct timespec *rem);
```

#### 4. Random Number Generation

```c
/* Get cryptographically secure random bytes */
int ixland_host_random_get(void *buf, size_t buflen);
```

#### 5. Sockets and Networking

```c
/* Create socket */
int ixland_host_socket(int domain, int type, int protocol);

/* Connect socket */
int ixland_host_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/* Bind socket */
int ixland_host_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/* Listen for connections */
int ixland_host_listen(int sockfd, int backlog);

/* Accept connection */
int ixland_host_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/* Send/receive */
ssize_t ixland_host_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t ixland_host_recv(int sockfd, void *buf, size_t len, int flags);
```

#### 6. Process Semantics

```c
/* Exit process */
noreturn void ixland_host_proc_exit(int status);

/* Get process ID */
pid_t ixland_host_proc_getpid(void);

/* Get parent process ID */
pid_t ixland_host_proc_getppid(void);

/* Send signal to process */
int ixland_host_proc_kill(pid_t pid, int sig);
```

### Design Principles

1. **iXland-native semantics**: Host services provide iXland behavior, not direct iOS passthrough
2. **Consistent with native**: Native and WASM guests see same behavior
3. **Virtualization preserved**: Virtual PIDs, process groups work in WASM too

## IXLandWasmWASI (WASI Policy)

### Purpose

Define how WASI syscalls map to iXland host semantics.

### Mapping Strategy

| WASI Function | iXland Host Service | Notes |
|--------------|---------------------|-------|
| `fd_read` | `ixland_host_fd_read` | Uses ixland file descriptor table |
| `fd_write` | `ixland_host_fd_write` | Uses ixland file descriptor table |
| `fd_close` | `ixland_host_fd_close` | Uses ixland file descriptor table |
| `path_open` | `ixland_host_path_open` | Resolves through ixland VFS |
| `path_filestat_get` | `ixland_host_path_stat` | Returns ixland VFS metadata |
| `clock_time_get` | `ixland_host_clock_gettime` | ixland kernel clocks |
| `random_get` | `ixland_host_random_get` | ixland secure random |
| `proc_exit` | `ixland_host_proc_exit` | Triggers ixland process cleanup |

### Capability Model

WASI capabilities map to iXland resource limits:

- File access rights → ixland file descriptor rights
- Directory rights → ixland VFS mount permissions
- Network rights → ixland socket policy

### Preopens

WASI preopened directories map to iXland bind mounts:

```
WASI preopen "/"    → ixland VFS root
WASI preopen "."    → ixland current working directory
WASI preopen "/tmp" → ixland temp directory
```

## Implementation Path

### Phase 1: Specification (Current)

- Document intended boundaries
- Define interface contracts
- Keep implementation in `IXLandSystem`

### Phase 2: Header Extraction

- Move public interfaces to `ixland-wasm/*/include/`
- `ixland-wasm-engine/include/ixland/wasm_engine.h`
- `ixland-wasm-host/include/ixland/wasm_host.h`
- `ixland-wasm-wasi/include/ixland/wasm_wasi.h`
- Use symlinks in `IXLandSystem` during transition

### Phase 3: Code Extraction

- Extract host service implementations to `ixland-wasm-host/`
- Extract WASI mapping to `ixland-wasm-wasi/`
- Keep engine-specific code in `IXLandSystem/runtime/wasi/`

### Phase 4: Engine Abstraction

- Implement engine-neutral interface
- Refactor WAMR to implement interface
- Document interface for future engines

## Constraints

- **No JIT**: iOS forbids dynamic code generation; WASM interpreted or AOT compiled
- **Sandbox**: WASI capabilities must respect iOS App Sandbox
- **No raw syscalls**: WASI must go through ixland host services
- **Single address space**: All WASM runs in app process; no real process isolation

## Future Considerations

- **Multiple engines**: Support both WAMR (small) and Wasmtime (fast) via engine interface
- **AOT artifacts**: Pre-compiled WASM modules as `IXLandPackages` artifacts
- **Component Model**: WASI Component Model support when stabilized

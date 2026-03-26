# libiox Migration Status

## Completed ✅

### Documentation
- ✅ `docs/LIBIOX_ARCHITECTURE.md` - Complete architecture specification
- ✅ `README.md` - Updated with new project branding
- ✅ `AGENTS.md` - Updated developer guide
- ✅ `MIGRATION_STATUS.md` - This file

### Directory Structure
```
a-shell-kernel/
├── include/iox/              # Public headers
│   ├── iox.h                 # Master umbrella header
│   ├── iox_syscalls.h        # 300+ syscall prototypes
│   ├── iox_types.h           # Type definitions
│   └── sys/
│       └── types.h           # Linux-compatible types
│
├── src/iox/                  # Implementation
│   ├── internal/
│   │   └── iox_internal.h    # Private data structures
│   └── core/
│       └── iox_process.c     # Process syscalls (fork, exec, wait, etc.)
│
├── libiox.a                  # Built static library ✅
├── Makefile                  # Simple build system
└── .old-backup/              # Backed up old files
```

### Implementation Progress
- ✅ Internal data structures (process, thread, session)
- ✅ Process syscalls (16 functions):
  - fork, vfork, execve, execv, exit, _exit
  - getpid, getppid, getpgrp, setpgrp
  - getpgid, setpgid
  - wait, waitpid, wait3, wait4
  - system
- ✅ Basic Makefile for compilation
- ✅ Old files backed up to `.old-backup/`

### Build Status
```bash
$ make clean && make
rm -f src/iox/core/iox_process.o libiox.a
rm -f test_process
clang -Wall -Wextra -g -O0 -I./include -I./src/iox/internal -c src/iox/core/iox_process.c -o src/iox/core/iox_process.o
ar rcs libiox.a src/iox/core/iox_process.o
Created libiox.a
```

✅ **Builds successfully!**

## Remaining Work 🚧

### Phase 1: Core Syscalls (In Progress)
- [ ] File syscalls (open, read, write, close, lseek, etc.) - 20 functions
- [ ] Filesystem syscalls (stat, mkdir, link, etc.) - 24 functions
- [ ] Signal syscalls (signal, kill, sigaction, etc.) - 16 functions
- [ ] Memory syscalls (mmap, mprotect, etc.) - 6 functions
- [ ] Time syscalls (sleep, gettimeofday, etc.) - 7 functions
- [ ] Network syscalls (socket, connect, etc.) - 20 functions (passthrough)
- [ ] Pipe syscalls (pipe, mkfifo) - 4 functions
- [ ] Select/Poll - 4 functions
- [ ] TTY syscalls - 7 functions

### Phase 2: Runtime Layer
- [ ] Move libc_replacement.c functions to iox_stdio.c, iox_env.c
- [ ] C runtime startup (iox_crt0.c)
- [ ] Thread-local storage management

### Phase 3: Symbol Interposition
- [ ] Create iox_interpose.c with all 300+ syscall wrappers
- [ ] Generate weak aliases or strong symbols

### Phase 4: WAMR Integration
- [ ] Add WAMR as git submodule
- [ ] Build WAMR for iOS
- [ ] Implement WASI-to-iox bridge
- [ ] Create iox-wamr runner

### Phase 5: Build System
- [ ] Create proper CMakeLists.txt
- [ ] XCFramework generation
- [ ] iox-cc compiler wrapper

### Phase 6: Testing
- [ ] Unit tests for each syscall
- [ ] Integration tests with real programs
- [ ] Performance benchmarks

## Next Steps

1. **Implement file syscalls** - Start with iox_file.c
2. **Create symbol interposition layer** - iox_interpose.c
3. **Set up WAMR** - Add as submodule, implement bridge
4. **Test compilation** - Verify Linux programs compile

## Key Decisions Made

✅ **Naming**: `iox` (iOS eXtension)  
✅ **Structure**: Internal `__iox_*_impl()` → Public `iox_*()` → Linux names  
✅ **Migration**: Big bang - no backward compatibility  
✅ **Architecture**: Static library with symbol interposition  
✅ **WAMR**: Full integration, AoT mode, WASI bridge  

## Files Ready for Implementation

- `src/iox/core/iox_file.c` - File operations
- `src/iox/core/iox_signal.c` - Signal handling  
- `src/iox/core/iox_memory.c` - Memory management
- `src/iox/core/iox_time.c` - Time functions
- `src/iox/core/iox_network.c` - Network passthrough
- `src/iox/interpose/iox_interpose.c` - Symbol wrappers

---

**Status**: Foundation Complete  
**Next Milestone**: Complete core syscalls (file, signal, memory)  
**Estimated Timeline**: 1-2 weeks for Phase 1 completion

# Cleanup Analysis

## Files To Remove (Safe Deletion)

### Object Files (.o) - All Safe to Delete
These are build artifacts that can be regenerated:
- `src/iox/core/iox_context.o` - Deleted source
- `src/iox/core/iox_file.o` - Deleted source
- `src/iox/core/iox_init.o` - Will rebuild
- `src/iox/core/iox_network.o` - Moved to net/
- `src/iox/core/iox_process.o` - Will rebuild
- `src/iox/core/iox_stubs.o` - Obsolete
- `src/iox/core/iox_vfs.o` - Moved to fs/vfs/
- `src/iox/interpose/iox_interpose.o` - Moved to compat/interpose/
- `src/iox/util/iox_path.o` - Moved to fs/
- `src/iox/wamr/iox_wamr_simple.o` - Will rebuild

### Obsolete Directories
- `src/iox/core/` - Superseded by kernel/, fs/, etc.
- `src/iox/util/` - Superseded by fs/
- `src/iox/interpose/` - Moved to compat/interpose/

### Source Files - MIGRATION NEEDED (Don't delete yet)
These contain logic that needs to be migrated to new structure:

**Phase 1 (Migrate to kernel/task/):**
- `src/iox/core/iox_process.c` (1,628 lines) - Core process logic
  - Migrate: fork, wait, signal delivery, process groups
  - Keep: PID allocation logic (now in kernel/task/pid.c)
  - Delete after migration complete

**Phase 2 (Migrate to kernel/init/):**
- `src/iox/core/iox_init.c` (108 lines) - Initialization
  - Merge into kernel/init/main.c

**Phase 3 (Evaluate):**
- `src/iox/core/iox_file_v2.c` (568 lines) - VFS-aware file ops
  - Compare with new fs/fdtable.c
  - Migrate useful logic, then delete
  
- `src/iox/core/iox_libc_delegate.c` (486 lines) - Passthrough layer
  - Should be replaced with kernel/syscalls.c
  - Keep as reference during transition
  
- `src/iox/core/iox_minimal.c` (201 lines) - Minimal stubs
  - Review for any useful stubs
  - Probably delete after review

**Phase 4 (Migrate to runtime/wasi/):**
- `src/iox/wamr/iox_wamr_simple.c` (303 lines) - Simplified WAMR
  - Will be replaced by full runtime/wasi/
  
- `src/iox/wamr/iox_wamr.c` (649 lines) - Full WAMR
  - Evaluate for unification

**Phase 5 (Keep during transition):**
- `src/iox/internal/iox_internal.h` (749 lines) - Unified internal header
  - Still referenced by old code
  - Split into subsystem headers gradually
  - Delete after all code migrated

## Deep Analysis

### What's Safe to Remove Immediately:
1. All .o files (rebuildable)
2. src/iox/core/iox_context.c (deleted)
3. src/iox/core/iox_file.c (deleted)
4. Empty directories after file moves

### What Needs Careful Migration:
1. `iox_process.c` - Contains 1,628 lines of process logic
   - Has: process table, wait queues, signals, pgrps, sessions
   - Needs: Extract useful logic into new kernel/task/
   - Risk: High - contains working code

2. `iox_internal.h` - 749 lines of declarations
   - Used by existing code
   - Remove after migrating all code

### Files Already Migrated:
- iox_vfs.c -> fs/vfs/
- iox_network.c -> net/
- iox_path.c -> fs/
- iox_interpose.c -> compat/interpose/

### Build System:
- Makefile - Will be replaced by CMake
- Keep until CMake fully functional

## Recommended Order:

1. **Immediate**: Remove all .o files
2. **Phase 1**: Migrate iox_process.c logic to kernel/task/
3. **Phase 2**: Remove empty/old directories
4. **Phase 3**: Migrate remaining src/iox/core/ files
5. **Phase 4**: Remove src/iox/internal/ after migration
6. **Phase 5**: Final cleanup

/* iXland libc - Linux-compatible mman.h
 *
 * Memory management and mapping definitions matching Linux kernel.
 * Provides mmap, mprotect, and related memory operations.
 */

#ifndef _LINUX_MMAN_H
#define _LINUX_MMAN_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * MEMORY PROTECTION FLAGS
 * ============================================================================ */

/* Protection flags for mmap/mprotect */
#define IXLAND_PROT_READ 0x1              /* Pages can be read */
#define IXLAND_PROT_WRITE 0x2             /* Pages can be written */
#define IXLAND_PROT_EXEC 0x4              /* Pages can be executed */
#define IXLAND_PROT_NONE 0x0              /* Pages cannot be accessed */
#define IXLAND_PROT_GROWSDOWN 0x01000000  /* Extend segment down (stack-like) */
#define IXLAND_PROT_GROWSUP 0x02000000    /* Extend segment up */
#define IXLAND_PROT_SEM 0x4               /* Page may be used for semaphores */
#define IXLAND_PROT_EXEC_STACK 0x04000000 /* Executable stack (denied on iOS) */

/* Protection mask */
#define IXLAND_PROT_MASK (IXLAND_PROT_READ | IXLAND_PROT_WRITE | IXLAND_PROT_EXEC)

/* Compatibility aliases */
#define PROT_READ IXLAND_PROT_READ
#define PROT_WRITE IXLAND_PROT_WRITE
#define PROT_EXEC IXLAND_PROT_EXEC
#define PROT_NONE IXLAND_PROT_NONE
#define PROT_GROWSDOWN IXLAND_PROT_GROWSDOWN
#define PROT_GROWSUP IXLAND_PROT_GROWSUP
#define PROT_SEM IXLAND_PROT_SEM

/* ============================================================================
 * MEMORY MAPPING FLAGS
 * ============================================================================ */

/* Mapping type (required, mutually exclusive) */
#define IXLAND_MAP_SHARED 0x01          /* Share changes */
#define IXLAND_MAP_PRIVATE 0x02         /* Changes are private */
#define IXLAND_MAP_SHARED_VALIDATE 0x03 /* Validate mapping flags */

/* Mapping flags */
#define IXLAND_MAP_FIXED 0x10               /* Interpret addr exactly */
#define IXLAND_MAP_ANONYMOUS 0x20           /* Don't use a file */
#define IXLAND_MAP_32BIT 0x40               /* Map in the low 2GB (x86_64) */
#define IXLAND_MAP_GROWSDOWN 0x00100        /* Stack-like segment */
#define IXLAND_MAP_DENYWRITE 0x00800        /* ETXTBSY on write attempts */
#define IXLAND_MAP_EXECUTABLE 0x01000       /* Mark it as an executable */
#define IXLAND_MAP_LOCKED 0x02000           /* Lock the pages in memory */
#define IXLAND_MAP_NORESERVE 0x04000        /* Don't check for reservations */
#define IXLAND_MAP_POPULATE 0x08000         /* Populate page tables */
#define IXLAND_MAP_NONBLOCK 0x10000         /* Don't block on I/O */
#define IXLAND_MAP_STACK 0x20000            /* Allocation for a stack */
#define IXLAND_MAP_HUGETLB 0x40000          /* Create huge page mapping */
#define IXLAND_MAP_SYNC 0x80000             /* Synchronous page faults */
#define IXLAND_MAP_FIXED_NOREPLACE 0x100000 /* Don't clobber existing mappings */

/* Huge page sizes for MAP_HUGETLB */
#define IXLAND_MAP_HUGE_SHIFT 26
#define IXLAND_MAP_HUGE_MASK 0x3f
#define IXLAND_MAP_HUGE_2MB (21 << IXLAND_MAP_HUGE_SHIFT)
#define IXLAND_MAP_HUGE_1GB (30 << IXLAND_MAP_HUGE_SHIFT)
#define IXLAND_MAP_HUGE_16MB (24 << IXLAND_MAP_HUGE_SHIFT)
#define IXLAND_MAP_HUGE_16GB (34 << IXLAND_MAP_HUGE_SHIFT)

/* Deprecated/legacy names */
#define IXLAND_MAP_ANON IXLAND_MAP_ANONYMOUS
#define IXLAND_MAP_FILE 0 /* Ignored */

/* Compatibility aliases */
#define MAP_SHARED IXLAND_MAP_SHARED
#define MAP_PRIVATE IXLAND_MAP_PRIVATE
#define MAP_SHARED_VALIDATE IXLAND_MAP_SHARED_VALIDATE
#define MAP_FIXED IXLAND_MAP_FIXED
#define MAP_ANONYMOUS IXLAND_MAP_ANONYMOUS
#define MAP_ANON IXLAND_MAP_ANON
#define MAP_32BIT IXLAND_MAP_32BIT
#define MAP_GROWSDOWN IXLAND_MAP_GROWSDOWN
#define MAP_DENYWRITE IXLAND_MAP_DENYWRITE
#define MAP_EXECUTABLE IXLAND_MAP_EXECUTABLE
#define MAP_LOCKED IXLAND_MAP_LOCKED
#define MAP_NORESERVE IXLAND_MAP_NORESERVE
#define MAP_POPULATE IXLAND_MAP_POPULATE
#define MAP_NONBLOCK IXLAND_MAP_NONBLOCK
#define MAP_STACK IXLAND_MAP_STACK
#define MAP_HUGETLB IXLAND_MAP_HUGETLB
#define MAP_SYNC IXLAND_MAP_SYNC
#define MAP_FIXED_NOREPLACE IXLAND_MAP_FIXED_NOREPLACE
#define MAP_HUGE_2MB IXLAND_MAP_HUGE_2MB
#define MAP_HUGE_1GB IXLAND_MAP_HUGE_1GB

/* ============================================================================
 * MEMORY ADVICE FLAGS
 * ============================================================================ */

/* Memory advice constants for madvise */
#define IXLAND_MADV_NORMAL 0       /* No special treatment */
#define IXLAND_MADV_RANDOM 1       /* Expect random page references */
#define IXLAND_MADV_SEQUENTIAL 2   /* Expect sequential page references */
#define IXLAND_MADV_WILLNEED 3     /* Will need these pages */
#define IXLAND_MADV_DONTNEED 4     /* Don't need these pages */
#define IXLAND_MADV_FREE 8         /* Free pages (with lazy reclaim) */
#define IXLAND_MADV_REMOVE 9       /* Remove these pages */
#define IXLAND_MADV_DONTFORK 10    /* Don't inherit across fork */
#define IXLAND_MADV_DOFORK 11      /* Do inherit across fork */
#define IXLAND_MADV_MERGEABLE 12   /* KSM: merge identical pages */
#define IXLAND_MADV_UNMERGEABLE 13 /* KSM: unmerge merged pages */
#define IXLAND_MADV_HUGEPAGE 14    /* Worth backing with huge pages */
#define IXLAND_MADV_NOHUGEPAGE 15  /* Not worth backing with huge pages */
#define IXLAND_MADV_DONTDUMP 16    /* Don't include in core dump */
#define IXLAND_MADV_DODUMP 17      /* Clear MADV_DONTDUMP */
#define IXLAND_MADV_WIPEONFORK 18  /* Zero memory at fork */
#define IXLAND_MADV_KEEPONFORK 19  /* Undo MADV_WIPEONFORK */
#define IXLAND_MADV_COLD 20        /* Deactivate these pages */
#define IXLAND_MADV_PAGEOUT 21     /* Reclaim these pages */

/* Linux-specific advice */
#define IXLAND_MADV_HWPOISON 100     /* Poison pages for testing */
#define IXLAND_MADV_SOFT_OFFLINE 101 /* Soft offline pages */

/* Compatibility aliases */
#define MADV_NORMAL IXLAND_MADV_NORMAL
#define MADV_RANDOM IXLAND_MADV_RANDOM
#define MADV_SEQUENTIAL IXLAND_MADV_SEQUENTIAL
#define MADV_WILLNEED IXLAND_MADV_WILLNEED
#define MADV_DONTNEED IXLAND_MADV_DONTNEED
#define MADV_FREE IXLAND_MADV_FREE
#define MADV_REMOVE IXLAND_MADV_REMOVE
#define MADV_DONTFORK IXLAND_MADV_DONTFORK
#define MADV_DOFORK IXLAND_MADV_DOFORK
#define MADV_MERGEABLE IXLAND_MADV_MERGEABLE
#define MADV_UNMERGEABLE IXLAND_MADV_UNMERGEABLE
#define MADV_HUGEPAGE IXLAND_MADV_HUGEPAGE
#define MADV_NOHUGEPAGE IXLAND_MADV_NOHUGEPAGE
#define MADV_DONTDUMP IXLAND_MADV_DONTDUMP
#define MADV_DODUMP IXLAND_MADV_DODUMP
#define MADV_WIPEONFORK IXLAND_MADV_WIPEONFORK
#define MADV_KEEPONFORK IXLAND_MADV_KEEPONFORK
#define MADV_COLD IXLAND_MADV_COLD
#define MADV_PAGEOUT IXLAND_MADV_PAGEOUT
#define MADV_HWPOISON IXLAND_MADV_HWPOISON
#define MADV_SOFT_OFFLINE IXLAND_MADV_SOFT_OFFLINE

/* ============================================================================
 * MEMORY LOCKING
 * ============================================================================ */

/* Memory locking flags for mlock/munlock/mlockall/munlockall */
#define IXLAND_MLOCK_MAYBE 0x1   /* Deprecated */
#define IXLAND_MLOCK_ONFAULT 0x2 /* Lock pages only on fault */

/* Mlockall flags */
#define IXLAND_MCL_CURRENT 0x1 /* Lock all current mappings */
#define IXLAND_MCL_FUTURE 0x2  /* Lock all future mappings */
#define IXLAND_MCL_ONFAULT 0x4 /* Lock pages only on fault */

/* Compatibility aliases */
#define MCL_CURRENT IXLAND_MCL_CURRENT
#define MCL_FUTURE IXLAND_MCL_FUTURE
#define MCL_ONFAULT IXLAND_MCL_ONFAULT

/* ============================================================================
 * SYNC FLAGS
 * ============================================================================ */

/* Sync flags for msync */
#define IXLAND_MS_ASYNC 1      /* Asynchronous sync */
#define IXLAND_MS_INVALIDATE 2 /* Invalidate mappings */
#define IXLAND_MS_SYNC 4       /* Synchronous sync */

/* Compatibility aliases */
#define MS_ASYNC IXLAND_MS_ASYNC
#define MS_INVALIDATE IXLAND_MS_INVALIDATE
#define MS_SYNC IXLAND_MS_SYNC

/* ============================================================================
 * MEMORY MAPPING STRUCTURES
 * ============================================================================ */

/* Memory mapping information structure */
struct linux_mmap_args {
    void *addr;     /* Suggested address */
    size_t len;     /* Length of mapping */
    int prot;       /* Protection flags */
    int flags;      /* Mapping flags */
    int fd;         /* File descriptor */
    int64_t offset; /* Offset in file */
};

/* Kernel file handle for file mapping (legacy) */
struct linux_file_handle {
    unsigned int handle_bytes; /* Size of handle */
    int handle_type;           /* Type of handle */
    /* File identifier follows */
    unsigned char f_handle[0];
};

/* Memfd creation info (Linux 3.17+) */
struct linux_memfd_create_info {
    const char *name;   /* Name for memfd */
    unsigned int flags; /* Flags */
};

/* Memfd flags */
#define IXLAND_MFD_CLOEXEC 0x0001       /* Close on exec */
#define IXLAND_MFD_ALLOW_SEALING 0x0002 /* Allow sealing */
#define IXLAND_MFD_HUGETLB 0x0004       /* Create huge page backed file */
#define IXLAND_MFD_HUGE_2MB IXLAND_MAP_HUGE_2MB
#define IXLAND_MFD_HUGE_1GB IXLAND_MAP_HUGE_1GB

/* Compatibility aliases */
#define MFD_CLOEXEC IXLAND_MFD_CLOEXEC
#define MFD_ALLOW_SEALING IXLAND_MFD_ALLOW_SEALING
#define MFD_HUGETLB IXLAND_MFD_HUGETLB
#define MFD_HUGE_2MB IXLAND_MFD_HUGE_2MB
#define MFD_HUGE_1GB IXLAND_MFD_HUGE_1GB

/* ============================================================================
 * MEMORY SEALING (memfd_create seals)
 * ============================================================================ */

/* File seal definitions */
#define IXLAND_F_SEAL_SEAL 0x0001   /* Prevent further sealing */
#define IXLAND_F_SEAL_SHRINK 0x0002 /* Prevent shrinking */
#define IXLAND_F_SEAL_GROW 0x0004   /* Prevent growing */
#define IXLAND_F_SEAL_WRITE 0x0008  /* Prevent writes */

/* Compatibility aliases */
#define F_SEAL_SEAL IXLAND_F_SEAL_SEAL
#define F_SEAL_SHRINK IXLAND_F_SEAL_SHRINK
#define F_SEAL_GROW IXLAND_F_SEAL_GROW
#define F_SEAL_WRITE IXLAND_F_SEAL_WRITE

/* ============================================================================
 * MAP ALIGNMENT
 * ============================================================================ */

/* Page size and alignment */
#ifndef IXLAND_PAGE_SIZE
#define IXLAND_PAGE_SIZE 16384 /* iOS ARM64 page size (16KB) */
#define IXLAND_PAGE_MASK (IXLAND_PAGE_SIZE - 1)
#define IXLAND_PAGE_SHIFT 14 /* log2(16384) */
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE IXLAND_PAGE_SIZE
#define PAGE_MASK IXLAND_PAGE_MASK
#define PAGE_SHIFT IXLAND_PAGE_SHIFT
#endif

/* Page alignment macros */
#define IXLAND_PAGE_ALIGN(addr) (((addr) + IXLAND_PAGE_SIZE - 1) & ~IXLAND_PAGE_MASK)
#define IXLAND_PAGE_TRUNC(addr) ((addr) & ~IXLAND_PAGE_MASK)
#define IXLAND_PAGE_OFFSET(addr) ((addr) & IXLAND_PAGE_MASK)
#define IXLAND_PAGE_ROUNDUP(size) \
    (((size) + IXLAND_PAGE_SIZE - 1) / IXLAND_PAGE_SIZE * IXLAND_PAGE_SIZE)

/* Huge page sizes */
#define IXLAND_HUGE_2MB_SIZE (2 * 1024 * 1024)                    /* 2 MB */
#define IXLAND_HUGE_1GB_SIZE (1024 * 1024 * 1024)                 /* 1 GB */
#define IXLAND_HUGE_16MB_SIZE (16 * 1024 * 1024)                  /* 16 MB */
#define IXLAND_HUGE_16GB_SIZE ((uint64_t)16 * 1024 * 1024 * 1024) /* 16 GB */

/* ============================================================================
 * SPECIAL ADDRESS VALUES
 * ============================================================================ */

/* Hint that kernel should choose address */
#define IXLAND_MAP_FAILED ((void *)-1) /* mmap failure return */
#define IXLAND_MAP_FAILED_ERRNO ENOMEM

/* Some architectures define this */
#ifndef IXLAND_MAP_BASE
#define IXLAND_MAP_BASE ((void *)0) /* Default base address */
#endif

/* ============================================================================
 * MEMORY TAGGING (ARM64 MTE) - Extension
 * ============================================================================ */

/* Memory tagging types */
#define IXLAND_PROT_MTE 0x20   /* Enable memory tagging */
#define IXLAND_MAP_MTE 0x40000 /* Allocate tagged memory */

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Map files or devices into memory
 *
 * @param addr Suggested address (NULL = kernel chooses)
 * @param length Number of bytes to map
 * @param prot Memory protection (IXLAND_PROT_*)
 * @param flags Mapping flags (IXLAND_MAP_*)
 * @param fd File descriptor (-1 for anonymous mappings)
 * @param offset File offset (must be page-aligned)
 * @return void* Mapped address on success, IXLAND_MAP_FAILED on error
 */
void *ixland_mmap(void *addr, size_t length, int prot, int flags, int fd, int64_t offset);

/**
 * @brief Unmap memory pages
 *
 * @param addr Address to unmap (must be page-aligned)
 * @param length Number of bytes to unmap
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_munmap(void *addr, size_t length);

/**
 * @brief Set memory protection
 *
 * @param addr Starting address (must be page-aligned)
 * @param len Length of region
 * @param prot New protection (IXLAND_PROT_*)
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_mprotect(void *addr, size_t len, int prot);

/**
 * @brief Synchronize memory with physical storage
 *
 * @param addr Starting address
 * @param length Length of region
 * @param flags Synchronization flags (IXLAND_MS_*)
 * @return int 0 on success, -1 on error
 */
int ixland_msync(void *addr, size_t length, int flags);

/**
 * @brief Give advice about use of memory
 *
 * @param addr Starting address
 * @param length Length of region
 * @param advice Memory advice (IXLAND_MADV_*)
 * @return int 0 on success, -1 on error
 */
int ixland_madvise(void *addr, size_t length, int advice);

/**
 * @brief Lock pages in memory
 *
 * @param addr Starting address
 * @param len Length of region
 * @return int 0 on success, -1 on error
 */
int ixland_mlock(const void *addr, size_t len);

/**
 * @brief Lock pages in memory (with flags)
 *
 * @param addr Starting address
 * @param len Length of region
 * @param flags Lock flags (IXLAND_MLOCK_*)
 * @return int 0 on success, -1 on error
 */
int ixland_mlock2(const void *addr, size_t len, unsigned int flags);

/**
 * @brief Unlock pages in memory
 *
 * @param addr Starting address
 * @param len Length of region
 * @return int 0 on success, -1 on error
 */
int ixland_munlock(const void *addr, size_t len);

/**
 * @brief Lock all pages in address space
 *
 * @param flags Lock flags (IXLAND_MCL_*)
 * @return int 0 on success, -1 on error
 */
int ixland_mlockall(int flags);

/**
 * @brief Unlock all pages in address space
 *
 * @return int 0 on success, -1 on error
 */
int ixland_munlockall(void);

/**
 * @brief Remap a memory mapping
 *
 * @param old_address Old mapping address
 * @param old_size Old size
 * @param new_size New size
 * @param flags Remap flags
 * @param new_address New address (if MREMAP_FIXED)
 * @return void* Remapped address on success, IXLAND_MAP_FAILED on error
 */
void *ixland_mremap(void *old_address, size_t old_size, size_t new_size, int flags,
                    void *new_address);

/* Mremap flags */
#define IXLAND_MREMAP_MAYMOVE 1   /* May relocate mapping */
#define IXLAND_MREMAP_FIXED 2     /* Map at exact address */
#define IXLAND_MREMAP_DONTUNMAP 4 /* Keep old mapping */

#define MREMAP_MAYMOVE IXLAND_MREMAP_MAYMOVE
#define MREMAP_FIXED IXLAND_MREMAP_FIXED
#define MREMAP_DONTUNMAP IXLAND_MREMAP_DONTUNMAP

/**
 * @brief Check if memory is accessible
 *
 * @param addr Address to check
 * @param len Length of region
 * @param writable Check for write access
 * @return int 0 if accessible, -1 on error
 */
int ixland_maccess(const void *addr, size_t len, int writable);

/**
 * @brief Get filename for anonymous mapping
 *
 * @param len Length of name buffer
 * @param buf Buffer for name
 * @param fd File descriptor
 * @return int 0 on success, -1 on error
 */
int ixland_memfd_create(const char *name, unsigned int flags);

/**
 * @brief Set or get file seals
 *
 * @param fd File descriptor
 * @param seals Seal operation
 * @return int 0 on success, -1 on error
 */
int ixland_fcntl_seal(int fd, int seals);

/**
 * @brief Get system page size
 *
 * @return long Page size in bytes
 */
long ixland_getpagesize(void);

/**
 * @brief Get system page size (POSIX)
 *
 * @return int Page size in bytes
 */
int ixland_getpagesize_posix(void);

/* ============================================================================
 * MINCORE
 * ============================================================================ */

/**
 * @brief Check if pages are in core memory
 *
 * @param addr Starting address
 * @param length Length of region
 * @param vec Result vector (one byte per page)
 * @return int 0 on success, -1 on error
 */
int ixland_mincore(void *addr, size_t length, unsigned char *vec);

/* Mincore return values */
#define IXLAND_MINCORE_INCORE 0x1           /* Page is in core */
#define IXLAND_MINCORE_REFERENCED 0x2       /* Page has been referenced */
#define IXLAND_MINCORE_MODIFIED 0x4         /* Page has been modified */
#define IXLAND_MINCORE_REFERENCED_OTHER 0x8 /* Referenced by another process */
#define IXLAND_MINCORE_MODIFIED_OTHER 0x10  /* Modified by another process */

/* ============================================================================
 * POSIX SHARED MEMORY (shm_*)
 * ============================================================================ */

/* POSIX shared memory constants */
#define IXLAND_SHM_RDONLY (1 << 2)     /* Read-only access */
#define IXLAND_SHM_RND (1 << 4)        /* Round address */
#define IXLAND_SHM_REMAP (1 << 5)      /* Remap (Linux extension) */
#define IXLAND_SHM_EXEC (1 << 6)       /* Executable */
#define IXLAND_SHM_DEST (1 << 8)       /* Destroy on last detach */
#define IXLAND_SHM_LOCKED (1 << 9)     /* Locked in memory */
#define IXLAND_SHM_HUGETLB (1 << 10)   /* Huge pages */
#define IXLAND_SHM_NORESERVE (1 << 11) /* No reservation */

#define SHM_RDONLY IXLAND_SHM_RDONLY
#define SHM_RND IXLAND_SHM_RND
#define SHM_REMAP IXLAND_SHM_REMAP
#define SHM_EXEC IXLAND_SHM_EXEC
#define SHM_DEST IXLAND_SHM_DEST
#define SHM_LOCKED IXLAND_SHM_LOCKED
#define SHM_HUGETLB IXLAND_SHM_HUGETLB
#define SHM_NORESERVE IXLAND_SHM_NORESERVE

/* Mode bits for shm_open */
#define IXLAND_S_IRUSR 0400 /* User read */
#define IXLAND_S_IWUSR 0200 /* User write */
#define IXLAND_S_IRGRP 0040 /* Group read */
#define IXLAND_S_IWGRP 0020 /* Group write */
#define IXLAND_S_IROTH 0004 /* Other read */
#define IXLAND_S_IWOTH 0002 /* Other write */

/**
 * @brief Open a POSIX shared memory object
 *
 * @param name Name of shared memory object
 * @param flags Open flags (O_*)
 * @param mode Permission mode
 * @return int File descriptor on success, -1 on error
 */
int ixland_shm_open(const char *name, int flags, mode_t mode);

/**
 * @brief Remove a POSIX shared memory object
 *
 * @param name Name of shared memory object
 * @return int 0 on success, -1 on error
 */
int ixland_shm_unlink(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_MMAN_H */

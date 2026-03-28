/* iXland libc - Linux-compatible mman.h
 *
 * Memory management and mapping definitions matching Linux kernel.
 * Provides mmap, mprotect, and related memory operations.
 */

#ifndef _LINUX_MMAN_H
#define _LINUX_MMAN_H

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * MEMORY PROTECTION FLAGS
 * ============================================================================ */

/* Protection flags for mmap/mprotect */
#define IOX_PROT_READ       0x1     /* Pages can be read */
#define IOX_PROT_WRITE      0x2     /* Pages can be written */
#define IOX_PROT_EXEC       0x4     /* Pages can be executed */
#define IOX_PROT_NONE       0x0     /* Pages cannot be accessed */
#define IOX_PROT_GROWSDOWN  0x01000000  /* Extend segment down (stack-like) */
#define IOX_PROT_GROWSUP    0x02000000  /* Extend segment up */
#define IOX_PROT_SEM        0x4     /* Page may be used for semaphores */
#define IOX_PROT_EXEC_STACK 0x04000000  /* Executable stack (denied on iOS) */

/* Protection mask */
#define IOX_PROT_MASK       (IOX_PROT_READ | IOX_PROT_WRITE | IOX_PROT_EXEC)

/* Compatibility aliases */
#define PROT_READ           IOX_PROT_READ
#define PROT_WRITE          IOX_PROT_WRITE
#define PROT_EXEC           IOX_PROT_EXEC
#define PROT_NONE           IOX_PROT_NONE
#define PROT_GROWSDOWN      IOX_PROT_GROWSDOWN
#define PROT_GROWSUP        IOX_PROT_GROWSUP
#define PROT_SEM            IOX_PROT_SEM

/* ============================================================================
 * MEMORY MAPPING FLAGS
 * ============================================================================ */

/* Mapping type (required, mutually exclusive) */
#define IOX_MAP_SHARED      0x01    /* Share changes */
#define IOX_MAP_PRIVATE     0x02    /* Changes are private */
#define IOX_MAP_SHARED_VALIDATE 0x03 /* Validate mapping flags */

/* Mapping flags */
#define IOX_MAP_FIXED       0x10    /* Interpret addr exactly */
#define IOX_MAP_ANONYMOUS   0x20    /* Don't use a file */
#define IOX_MAP_32BIT       0x40    /* Map in the low 2GB (x86_64) */
#define IOX_MAP_GROWSDOWN   0x00100 /* Stack-like segment */
#define IOX_MAP_DENYWRITE   0x00800 /* ETXTBSY on write attempts */
#define IOX_MAP_EXECUTABLE  0x01000 /* Mark it as an executable */
#define IOX_MAP_LOCKED      0x02000 /* Lock the pages in memory */
#define IOX_MAP_NORESERVE   0x04000 /* Don't check for reservations */
#define IOX_MAP_POPULATE    0x08000 /* Populate page tables */
#define IOX_MAP_NONBLOCK    0x10000 /* Don't block on I/O */
#define IOX_MAP_STACK       0x20000 /* Allocation for a stack */
#define IOX_MAP_HUGETLB     0x40000 /* Create huge page mapping */
#define IOX_MAP_SYNC        0x80000 /* Synchronous page faults */
#define IOX_MAP_FIXED_NOREPLACE 0x100000 /* Don't clobber existing mappings */

/* Huge page sizes for MAP_HUGETLB */
#define IOX_MAP_HUGE_SHIFT  26
#define IOX_MAP_HUGE_MASK   0x3f
#define IOX_MAP_HUGE_2MB    (21 << IOX_MAP_HUGE_SHIFT)
#define IOX_MAP_HUGE_1GB    (30 << IOX_MAP_HUGE_SHIFT)
#define IOX_MAP_HUGE_16MB   (24 << IOX_MAP_HUGE_SHIFT)
#define IOX_MAP_HUGE_16GB   (34 << IOX_MAP_HUGE_SHIFT)

/* Deprecated/legacy names */
#define IOX_MAP_ANON        IOX_MAP_ANONYMOUS
#define IOX_MAP_FILE        0       /* Ignored */

/* Compatibility aliases */
#define MAP_SHARED          IOX_MAP_SHARED
#define MAP_PRIVATE         IOX_MAP_PRIVATE
#define MAP_SHARED_VALIDATE IOX_MAP_SHARED_VALIDATE
#define MAP_FIXED           IOX_MAP_FIXED
#define MAP_ANONYMOUS       IOX_MAP_ANONYMOUS
#define MAP_ANON            IOX_MAP_ANON
#define MAP_32BIT           IOX_MAP_32BIT
#define MAP_GROWSDOWN       IOX_MAP_GROWSDOWN
#define MAP_DENYWRITE       IOX_MAP_DENYWRITE
#define MAP_EXECUTABLE      IOX_MAP_EXECUTABLE
#define MAP_LOCKED          IOX_MAP_LOCKED
#define MAP_NORESERVE       IOX_MAP_NORESERVE
#define MAP_POPULATE        IOX_MAP_POPULATE
#define MAP_NONBLOCK        IOX_MAP_NONBLOCK
#define MAP_STACK           IOX_MAP_STACK
#define MAP_HUGETLB         IOX_MAP_HUGETLB
#define MAP_SYNC            IOX_MAP_SYNC
#define MAP_FIXED_NOREPLACE IOX_MAP_FIXED_NOREPLACE
#define MAP_HUGE_2MB        IOX_MAP_HUGE_2MB
#define MAP_HUGE_1GB        IOX_MAP_HUGE_1GB

/* ============================================================================
 * MEMORY ADVICE FLAGS
 * ============================================================================ */

/* Memory advice constants for madvise */
#define IOX_MADV_NORMAL     0       /* No special treatment */
#define IOX_MADV_RANDOM     1       /* Expect random page references */
#define IOX_MADV_SEQUENTIAL 2       /* Expect sequential page references */
#define IOX_MADV_WILLNEED   3       /* Will need these pages */
#define IOX_MADV_DONTNEED   4       /* Don't need these pages */
#define IOX_MADV_FREE       8       /* Free pages (with lazy reclaim) */
#define IOX_MADV_REMOVE     9       /* Remove these pages */
#define IOX_MADV_DONTFORK   10      /* Don't inherit across fork */
#define IOX_MADV_DOFORK     11      /* Do inherit across fork */
#define IOX_MADV_MERGEABLE  12      /* KSM: merge identical pages */
#define IOX_MADV_UNMERGEABLE 13     /* KSM: unmerge merged pages */
#define IOX_MADV_HUGEPAGE   14      /* Worth backing with huge pages */
#define IOX_MADV_NOHUGEPAGE 15      /* Not worth backing with huge pages */
#define IOX_MADV_DONTDUMP   16      /* Don't include in core dump */
#define IOX_MADV_DODUMP     17      /* Clear MADV_DONTDUMP */
#define IOX_MADV_WIPEONFORK 18      /* Zero memory at fork */
#define IOX_MADV_KEEPONFORK 19      /* Undo MADV_WIPEONFORK */
#define IOX_MADV_COLD       20      /* Deactivate these pages */
#define IOX_MADV_PAGEOUT    21      /* Reclaim these pages */

/* Linux-specific advice */
#define IOX_MADV_HWPOISON   100     /* Poison pages for testing */
#define IOX_MADV_SOFT_OFFLINE 101   /* Soft offline pages */

/* Compatibility aliases */
#define MADV_NORMAL         IOX_MADV_NORMAL
#define MADV_RANDOM         IOX_MADV_RANDOM
#define MADV_SEQUENTIAL     IOX_MADV_SEQUENTIAL
#define MADV_WILLNEED       IOX_MADV_WILLNEED
#define MADV_DONTNEED       IOX_MADV_DONTNEED
#define MADV_FREE           IOX_MADV_FREE
#define MADV_REMOVE         IOX_MADV_REMOVE
#define MADV_DONTFORK       IOX_MADV_DONTFORK
#define MADV_DOFORK         IOX_MADV_DOFORK
#define MADV_MERGEABLE      IOX_MADV_MERGEABLE
#define MADV_UNMERGEABLE    IOX_MADV_UNMERGEABLE
#define MADV_HUGEPAGE       IOX_MADV_HUGEPAGE
#define MADV_NOHUGEPAGE     IOX_MADV_NOHUGEPAGE
#define MADV_DONTDUMP       IOX_MADV_DONTDUMP
#define MADV_DODUMP         IOX_MADV_DODUMP
#define MADV_WIPEONFORK     IOX_MADV_WIPEONFORK
#define MADV_KEEPONFORK     IOX_MADV_KEEPONFORK
#define MADV_COLD           IOX_MADV_COLD
#define MADV_PAGEOUT        IOX_MADV_PAGEOUT
#define MADV_HWPOISON       IOX_MADV_HWPOISON
#define MADV_SOFT_OFFLINE   IOX_MADV_SOFT_OFFLINE

/* ============================================================================
 * MEMORY LOCKING
 * ============================================================================ */

/* Memory locking flags for mlock/munlock/mlockall/munlockall */
#define IOX_MLOCK_MAYBE     0x1     /* Deprecated */
#define IOX_MLOCK_ONFAULT  0x2     /* Lock pages only on fault */

/* Mlockall flags */
#define IOX_MCL_CURRENT     0x1     /* Lock all current mappings */
#define IOX_MCL_FUTURE      0x2     /* Lock all future mappings */
#define IOX_MCL_ONFAULT     0x4     /* Lock pages only on fault */

/* Compatibility aliases */
#define MCL_CURRENT         IOX_MCL_CURRENT
#define MCL_FUTURE          IOX_MCL_FUTURE
#define MCL_ONFAULT         IOX_MCL_ONFAULT

/* ============================================================================
 * SYNC FLAGS
 * ============================================================================ */

/* Sync flags for msync */
#define IOX_MS_ASYNC        1       /* Asynchronous sync */
#define IOX_MS_INVALIDATE   2       /* Invalidate mappings */
#define IOX_MS_SYNC         4       /* Synchronous sync */

/* Compatibility aliases */
#define MS_ASYNC            IOX_MS_ASYNC
#define MS_INVALIDATE       IOX_MS_INVALIDATE
#define MS_SYNC             IOX_MS_SYNC

/* ============================================================================
 * MEMORY MAPPING STRUCTURES
 * ============================================================================ */

/* Memory mapping information structure */
struct linux_mmap_args {
    void *addr;         /* Suggested address */
    size_t len;         /* Length of mapping */
    int prot;           /* Protection flags */
    int flags;          /* Mapping flags */
    int fd;             /* File descriptor */
    int64_t offset;     /* Offset in file */
};

/* Kernel file handle for file mapping (legacy) */
struct linux_file_handle {
    unsigned int handle_bytes;  /* Size of handle */
    int handle_type;            /* Type of handle */
    /* File identifier follows */
    unsigned char f_handle[0];
};

/* Memfd creation info (Linux 3.17+) */
struct linux_memfd_create_info {
    const char *name;           /* Name for memfd */
    unsigned int flags;         /* Flags */
};

/* Memfd flags */
#define IOX_MFD_CLOEXEC     0x0001  /* Close on exec */
#define IOX_MFD_ALLOW_SEALING   0x0002  /* Allow sealing */
#define IOX_MFD_HUGETLB     0x0004  /* Create huge page backed file */
#define IOX_MFD_HUGE_2MB    IOX_MAP_HUGE_2MB
#define IOX_MFD_HUGE_1GB    IOX_MAP_HUGE_1GB

/* Compatibility aliases */
#define MFD_CLOEXEC         IOX_MFD_CLOEXEC
#define MFD_ALLOW_SEALING   IOX_MFD_ALLOW_SEALING
#define MFD_HUGETLB         IOX_MFD_HUGETLB
#define MFD_HUGE_2MB        IOX_MFD_HUGE_2MB
#define MFD_HUGE_1GB        IOX_MFD_HUGE_1GB

/* ============================================================================
 * MEMORY SEALING (memfd_create seals)
 * ============================================================================ */

/* File seal definitions */
#define IOX_F_SEAL_SEAL     0x0001  /* Prevent further sealing */
#define IOX_F_SEAL_SHRINK   0x0002  /* Prevent shrinking */
#define IOX_F_SEAL_GROW     0x0004  /* Prevent growing */
#define IOX_F_SEAL_WRITE    0x0008  /* Prevent writes */

/* Compatibility aliases */
#define F_SEAL_SEAL         IOX_F_SEAL_SEAL
#define F_SEAL_SHRINK       IOX_F_SEAL_SHRINK
#define F_SEAL_GROW         IOX_F_SEAL_GROW
#define F_SEAL_WRITE        IOX_F_SEAL_WRITE

/* ============================================================================
 * MAP ALIGNMENT
 * ============================================================================ */

/* Page size and alignment */
#ifndef IOX_PAGE_SIZE
#define IOX_PAGE_SIZE       16384   /* iOS ARM64 page size (16KB) */
#define IOX_PAGE_MASK       (IOX_PAGE_SIZE - 1)
#define IOX_PAGE_SHIFT      14      /* log2(16384) */
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE           IOX_PAGE_SIZE
#define PAGE_MASK           IOX_PAGE_MASK
#define PAGE_SHIFT          IOX_PAGE_SHIFT
#endif

/* Page alignment macros */
#define IOX_PAGE_ALIGN(addr) (((addr) + IOX_PAGE_SIZE - 1) & ~IOX_PAGE_MASK)
#define IOX_PAGE_TRUNC(addr) ((addr) & ~IOX_PAGE_MASK)
#define IOX_PAGE_OFFSET(addr) ((addr) & IOX_PAGE_MASK)
#define IOX_PAGE_ROUNDUP(size) (((size) + IOX_PAGE_SIZE - 1) / IOX_PAGE_SIZE * IOX_PAGE_SIZE)

/* Huge page sizes */
#define IOX_HUGE_2MB_SIZE   (2 * 1024 * 1024)   /* 2 MB */
#define IOX_HUGE_1GB_SIZE   (1024 * 1024 * 1024) /* 1 GB */
#define IOX_HUGE_16MB_SIZE  (16 * 1024 * 1024)  /* 16 MB */
#define IOX_HUGE_16GB_SIZE  ((uint64_t)16 * 1024 * 1024 * 1024) /* 16 GB */

/* ============================================================================
 * SPECIAL ADDRESS VALUES
 * ============================================================================ */

/* Hint that kernel should choose address */
#define IOX_MAP_FAILED      ((void *) -1)   /* mmap failure return */
#define IOX_MAP_FAILED_ERRNO   ENOMEM

/* Some architectures define this */
#ifndef IOX_MAP_BASE
#define IOX_MAP_BASE        ((void *) 0)    /* Default base address */
#endif

/* ============================================================================
 * MEMORY TAGGING (ARM64 MTE) - Extension
 * ============================================================================ */

/* Memory tagging types */
#define IOX_PROT_MTE        0x20    /* Enable memory tagging */
#define IOX_MAP_MTE         0x40000 /* Allocate tagged memory */

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Map files or devices into memory
 *
 * @param addr Suggested address (NULL = kernel chooses)
 * @param length Number of bytes to map
 * @param prot Memory protection (IOX_PROT_*)
 * @param flags Mapping flags (IOX_MAP_*)
 * @param fd File descriptor (-1 for anonymous mappings)
 * @param offset File offset (must be page-aligned)
 * @return void* Mapped address on success, IOX_MAP_FAILED on error
 */
void *iox_mmap(void *addr, size_t length, int prot, int flags, int fd, int64_t offset);

/**
 * @brief Unmap memory pages
 *
 * @param addr Address to unmap (must be page-aligned)
 * @param length Number of bytes to unmap
 * @return int 0 on success, -1 on error with errno set
 */
int iox_munmap(void *addr, size_t length);

/**
 * @brief Set memory protection
 *
 * @param addr Starting address (must be page-aligned)
 * @param len Length of region
 * @param prot New protection (IOX_PROT_*)
 * @return int 0 on success, -1 on error with errno set
 */
int iox_mprotect(void *addr, size_t len, int prot);

/**
 * @brief Synchronize memory with physical storage
 *
 * @param addr Starting address
 * @param length Length of region
 * @param flags Synchronization flags (IOX_MS_*)
 * @return int 0 on success, -1 on error
 */
int iox_msync(void *addr, size_t length, int flags);

/**
 * @brief Give advice about use of memory
 *
 * @param addr Starting address
 * @param length Length of region
 * @param advice Memory advice (IOX_MADV_*)
 * @return int 0 on success, -1 on error
 */
int iox_madvise(void *addr, size_t length, int advice);

/**
 * @brief Lock pages in memory
 *
 * @param addr Starting address
 * @param len Length of region
 * @return int 0 on success, -1 on error
 */
int iox_mlock(const void *addr, size_t len);

/**
 * @brief Lock pages in memory (with flags)
 *
 * @param addr Starting address
 * @param len Length of region
 * @param flags Lock flags (IOX_MLOCK_*)
 * @return int 0 on success, -1 on error
 */
int iox_mlock2(const void *addr, size_t len, unsigned int flags);

/**
 * @brief Unlock pages in memory
 *
 * @param addr Starting address
 * @param len Length of region
 * @return int 0 on success, -1 on error
 */
int iox_munlock(const void *addr, size_t len);

/**
 * @brief Lock all pages in address space
 *
 * @param flags Lock flags (IOX_MCL_*)
 * @return int 0 on success, -1 on error
 */
int iox_mlockall(int flags);

/**
 * @brief Unlock all pages in address space
 *
 * @return int 0 on success, -1 on error
 */
int iox_munlockall(void);

/**
 * @brief Remap a memory mapping
 *
 * @param old_address Old mapping address
 * @param old_size Old size
 * @param new_size New size
 * @param flags Remap flags
 * @param new_address New address (if MREMAP_FIXED)
 * @return void* Remapped address on success, IOX_MAP_FAILED on error
 */
void *iox_mremap(void *old_address, size_t old_size, size_t new_size,
                 int flags, void *new_address);

/* Mremap flags */
#define IOX_MREMAP_MAYMOVE  1       /* May relocate mapping */
#define IOX_MREMAP_FIXED    2       /* Map at exact address */
#define IOX_MREMAP_DONTUNMAP   4    /* Keep old mapping */

#define MREMAP_MAYMOVE      IOX_MREMAP_MAYMOVE
#define MREMAP_FIXED        IOX_MREMAP_FIXED
#define MREMAP_DONTUNMAP    IOX_MREMAP_DONTUNMAP

/**
 * @brief Check if memory is accessible
 *
 * @param addr Address to check
 * @param len Length of region
 * @param writable Check for write access
 * @return int 0 if accessible, -1 on error
 */
int iox_maccess(const void *addr, size_t len, int writable);

/**
 * @brief Get filename for anonymous mapping
 *
 * @param len Length of name buffer
 * @param buf Buffer for name
 * @param fd File descriptor
 * @return int 0 on success, -1 on error
 */
int iox_memfd_create(const char *name, unsigned int flags);

/**
 * @brief Set or get file seals
 *
 * @param fd File descriptor
 * @param seals Seal operation
 * @return int 0 on success, -1 on error
 */
int iox_fcntl_seal(int fd, int seals);

/**
 * @brief Get system page size
 *
 * @return long Page size in bytes
 */
long iox_getpagesize(void);

/**
 * @brief Get system page size (POSIX)
 *
 * @return int Page size in bytes
 */
int iox_getpagesize_posix(void);

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
int iox_mincore(void *addr, size_t length, unsigned char *vec);

/* Mincore return values */
#define IOX_MINCORE_INCORE        0x1   /* Page is in core */
#define IOX_MINCORE_REFERENCED    0x2   /* Page has been referenced */
#define IOX_MINCORE_MODIFIED      0x4   /* Page has been modified */
#define IOX_MINCORE_REFERENCED_OTHER  0x8   /* Referenced by another process */
#define IOX_MINCORE_MODIFIED_OTHER    0x10  /* Modified by another process */

/* ============================================================================
 * POSIX SHARED MEMORY (shm_*)
 * ============================================================================ */

/* POSIX shared memory constants */
#define IOX_SHM_RDONLY      (1 << 2)    /* Read-only access */
#define IOX_SHM_RND         (1 << 4)    /* Round address */
#define IOX_SHM_REMAP       (1 << 5)    /* Remap (Linux extension) */
#define IOX_SHM_EXEC        (1 << 6)    /* Executable */
#define IOX_SHM_DEST        (1 << 8)    /* Destroy on last detach */
#define IOX_SHM_LOCKED      (1 << 9)    /* Locked in memory */
#define IOX_SHM_HUGETLB     (1 << 10)   /* Huge pages */
#define IOX_SHM_NORESERVE   (1 << 11)   /* No reservation */

#define SHM_RDONLY          IOX_SHM_RDONLY
#define SHM_RND             IOX_SHM_RND
#define SHM_REMAP           IOX_SHM_REMAP
#define SHM_EXEC            IOX_SHM_EXEC
#define SHM_DEST            IOX_SHM_DEST
#define SHM_LOCKED          IOX_SHM_LOCKED
#define SHM_HUGETLB         IOX_SHM_HUGETLB
#define SHM_NORESERVE       IOX_SHM_NORESERVE

/* Mode bits for shm_open */
#define IOX_S_IRUSR         0400    /* User read */
#define IOX_S_IWUSR         0200    /* User write */
#define IOX_S_IRGRP         0040    /* Group read */
#define IOX_S_IWGRP         0020    /* Group write */
#define IOX_S_IROTH         0004    /* Other read */
#define IOX_S_IWOTH         0002    /* Other write */

/**
 * @brief Open a POSIX shared memory object
 *
 * @param name Name of shared memory object
 * @param flags Open flags (O_*)
 * @param mode Permission mode
 * @return int File descriptor on success, -1 on error
 */
int iox_shm_open(const char *name, int flags, mode_t mode);

/**
 * @brief Remove a POSIX shared memory object
 *
 * @param name Name of shared memory object
 * @return int 0 on success, -1 on error
 */
int iox_shm_unlink(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_MMAN_H */

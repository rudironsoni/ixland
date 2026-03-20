/*
 * linux/mman.h - Memory management declarations
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: mmap, mprotect, etc.
 */

#ifndef _LINUX_MMAN_H
#define _LINUX_MMAN_H

#include <sys/mman.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Memory Mapping
 * ============================================================================ */

extern void *a_shell_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern void *a_shell_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset);

/* ============================================================================
 * Memory Unmapping
 * ============================================================================ */

extern int a_shell_munmap(void *addr, size_t length);

/* ============================================================================
 * Memory Protection
 * ============================================================================ */

extern int a_shell_mprotect(void *addr, size_t len, int prot);

/* ============================================================================
 * Memory Synchronization
 * ============================================================================ */

extern int a_shell_msync(void *addr, size_t length, int flags);

/* ============================================================================
 * Memory Locking
 * ============================================================================ */

extern int a_shell_mlock(const void *addr, size_t len);
extern int a_shell_munlock(const void *addr, size_t len);
extern int a_shell_mlockall(int flags);
extern int a_shell_munlockall(void);

/* ============================================================================
 * Memory Advice
 * ============================================================================ */

extern int a_shell_madvise(void *addr, size_t length, int advice);
extern int a_shell_mincore(void *addr, size_t length, unsigned char *vec);
extern int a_shell_posix_madvise(void *addr, size_t len, int advice);

/* ============================================================================
 * Shared Memory
 * ============================================================================ */

extern int a_shell_shm_open(const char *name, int oflag, mode_t mode);
extern int a_shell_shm_unlink(const char *name);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define mmap(addr, length, prot, flags, fd, offset) \
                                a_shell_mmap(addr, length, prot, flags, fd, offset)
#define mmap2(addr, length, prot, flags, fd, pgoffset) \
                                a_shell_mmap2(addr, length, prot, flags, fd, pgoffset)
#define munmap(addr, length)    a_shell_munmap(addr, length)
#define mprotect(addr, len, prot) \
                                a_shell_mprotect(addr, len, prot)
#define msync(addr, length, flags) \
                                a_shell_msync(addr, length, flags)
#define mlock(addr, len)        a_shell_mlock(addr, len)
#define munlock(addr, len)      a_shell_munlock(addr, len)
#define mlockall(flags)         a_shell_mlockall(flags)
#define munlockall()            a_shell_munlockall()
#define madvise(addr, length, advice) \
                                a_shell_madvise(addr, length, advice)
#define mincore(addr, length, vec) \
                                a_shell_mincore(addr, length, vec)
#define posix_madvise(addr, len, advice) \
                                a_shell_posix_madvise(addr, len, advice)
#define shm_open(name, oflag, mode) \
                                a_shell_shm_open(name, oflag, mode)
#define shm_unlink(name)        a_shell_shm_unlink(name)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_MMAN_H */

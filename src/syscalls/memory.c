/*
 * memory.c - Memory management syscalls
 */

#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

#include "../../include/linux/mman.h"

/* ============================================================================
 * Memory Mapping
 * ============================================================================ */

void *a_shell_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return mmap(addr, length, prot, flags, fd, offset);
}

void *a_shell_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset) {
    /* mmap2 uses page offset instead of byte offset */
    long page_size = sysconf(_SC_PAGE_SIZE);
    return mmap(addr, length, prot, flags, fd, pgoffset * page_size);
}

int a_shell_munmap(void *addr, size_t length) {
    return munmap(addr, length);
}

/* ============================================================================
 * Memory Protection
 * ============================================================================ */

int a_shell_mprotect(void *addr, size_t len, int prot) {
    return mprotect(addr, len, prot);
}

/* ============================================================================
 * Memory Synchronization
 * ============================================================================ */

int a_shell_msync(void *addr, size_t length, int flags) {
    return msync(addr, length, flags);
}

/* ============================================================================
 * Memory Locking
 * ============================================================================ */

int a_shell_mlock(const void *addr, size_t len) {
    return mlock(addr, len);
}

int a_shell_munlock(const void *addr, size_t len) {
    return munlock(addr, len);
}

int a_shell_mlockall(int flags) {
    return mlockall(flags);
}

int a_shell_munlockall(void) {
    return munlockall();
}

/* ============================================================================
 * Memory Advice
 * ============================================================================ */

int a_shell_madvise(void *addr, size_t length, int advice) {
    return madvise(addr, length, advice);
}

int a_shell_mincore(void *addr, size_t length, unsigned char *vec) {
    return mincore(addr, length, vec);
}

int a_shell_posix_madvise(void *addr, size_t len, int advice) {
    return posix_madvise(addr, len, advice);
}

/* ============================================================================
 * Shared Memory
 * ============================================================================ */

int a_shell_shm_open(const char *name, int oflag, mode_t mode) {
    return shm_open(name, oflag, mode);
}

int a_shell_shm_unlink(const char *name) {
    return shm_unlink(name);
}

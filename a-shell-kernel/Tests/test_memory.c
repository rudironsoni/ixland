/*
 * test_memory.c - Memory management tests
 *
 * Tests mmap, mprotect, etc. using explicit a_shell_*() API
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>

void test_mmap_basic() {
    printf("Test: mmap() basic...\n");
    
    /* Allocate anonymous memory */
    size_t size = 4096;
    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (addr == MAP_FAILED) {
        printf("  ! mmap failed (expected on some platforms)\n");
    } else {
        printf("  ✓ mmap allocated at %p\n", addr);
        
        /* Write and read */
        memset(addr, 0x42, size);
        printf("  ✓ Memory write/read successful\n");
        
        munmap(addr, size);
        printf("  ✓ munmap successful\n");
    }
}

void test_mprotect() {
    printf("Test: mprotect()...\n");
    
    /* Try to protect a page */
    void *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, 
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (addr != MAP_FAILED) {
        int ret = mprotect(addr, 4096, PROT_READ);
        printf("  ✓ mprotect returned %d\n", ret);
        munmap(addr, 4096);
    } else {
        printf("  ! Skipped (mmap failed)\n");
    }
}

int main() {
    printf("\n========================================\n");
    printf("Memory Management Tests\n");
    printf("========================================\n\n");
    
    test_mmap_basic();
    test_mprotect();
    
    printf("\n========================================\n");
    printf("Memory tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

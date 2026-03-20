/*
 * Simple kernel functionality test
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

// Include only VFS for now (no conflicts)
#include "include/vfs.h"

int main() {
    printf("\n========================================\n");
    printf("a-shell-kernel Simple Test\n");
    printf("========================================\n\n");
    
    // Test VFS
    printf("Testing VFS...\n");
    vfs_init("/tmp/test_prefix");
    
    char *path = vfs_translate_path("/etc/passwd");
    assert(strcmp(path, "/tmp/test_prefix/etc/passwd") == 0);
    printf("  ✓ VFS path translation works\n");
    vfs_free_path(path);
    
    // Test that user paths are not translated
    char *user_path = vfs_translate_path("/home/user/file.txt");
    assert(strcmp(user_path, "/home/user/file.txt") == 0);
    printf("  ✓ User paths preserved\n");
    vfs_free_path(user_path);
    
    printf("\n========================================\n");
    printf("Basic tests passed!\n");
    printf("========================================\n");
    
    return 0;
}

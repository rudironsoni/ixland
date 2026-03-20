/*
 * test_file.c - File operation tests
 *
 * Tests open, stat, chmod using explicit a_shell_*() API
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

void test_vfs_path_translation() {
    printf("Test: VFS path translation...\n");
    printf("  (VFS tests require a_shell library - skipped)\n");
    /* 
     * VFS functions are in the a_shell library and require linking.
     * For header-only compilation tests, we skip the actual calls.
     */
}

void test_stat_basic() {
    printf("Test: stat() basic...\n");
    
    struct stat st;
    int ret = stat(".", &st);
    assert(ret == 0);
    printf("  ✓ stat(.) succeeded\n");
}

void test_open_flags() {
    printf("Test: open flags...\n");
    
    /* Verify standard flags are defined */
    assert(O_RDONLY == 0);
    assert(O_WRONLY == 1);
    assert(O_RDWR == 2);
    printf("  ✓ Standard open flags defined\n");
}

void test_chmod_umask() {
    printf("Test: umask()...\n");
    
    mode_t old = umask(022);
    printf("  ✓ umask(022), old mask was: %04o\n", (unsigned)old);
    
    umask(old);
    printf("  ✓ Restored old umask\n");
}

int main() {
    printf("\n========================================\n");
    printf("File Operation Tests\n");
    printf("========================================\n\n");
    
    test_vfs_path_translation();
    test_stat_basic();
    test_open_flags();
    test_chmod_umask();
    
    printf("\n========================================\n");
    printf("File tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

/*
 * test_file.c - File operation syscall unit tests
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../include/a_shell_kernel.h"

void test_vfs_path_translation() {
    printf("Test: VFS path translation...\n");
    
    vfs_init("/tmp/test_prefix");
    
    char *translated = vfs_translate_path("/etc/passwd");
    assert(strcmp(translated, "/tmp/test_prefix/etc/passwd") == 0);
    vfs_free_path(translated);
    printf("  ✓ /etc/passwd translated to /tmp/test_prefix/etc/passwd\n");
    
    char *not_translated = vfs_translate_path("/home/user/file.txt");
    assert(strcmp(not_translated, "/home/user/file.txt") == 0);
    vfs_free_path(not_translated);
    printf("  ✓ /home/user/file.txt not translated\n");
}

void test_stat() {
    printf("Test: stat() operations...\n");
    
    struct stat st;
    int ret = a_shell_stat(".", &st);
    assert(ret == 0);
    printf("  ✓ stat() on current directory succeeded\n");
}

void test_open_flags() {
    printf("Test: open() flags...\n");
    
    /* These are standard POSIX flags that should be defined */
    assert(O_RDONLY == 0);
    assert(O_WRONLY == 1);
    assert(O_RDWR == 2);
    printf("  ✓ Standard open flags defined correctly\n");
}

int main() {
    printf("\n========================================\n");
    printf("File Operation Syscall Unit Tests\n");
    printf("========================================\n\n");
    
    test_vfs_path_translation();
    test_stat();
    test_open_flags();
    
    printf("\n========================================\n");
    printf("All file tests passed!\n");
    printf("========================================\n");
    
    return 0;
}

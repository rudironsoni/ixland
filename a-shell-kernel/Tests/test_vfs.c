/*
 * test_vfs.c - VFS layer unit tests
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../include/vfs.h"

void test_vfs_init() {
    printf("Test: vfs_init()...\n");
    vfs_init("/Users/test/Library/ashell");
    assert(strcmp(vfs_get_prefix(), "/Users/test/Library/ashell") == 0);
    printf("  ✓ VFS prefix set correctly\n");
}

void test_vfs_translate_system_paths() {
    printf("Test: vfs_translate_path() for system paths...\n");
    vfs_init("/Users/test/Library/ashell");
    
    char *path1 = vfs_translate_path("/etc/passwd");
    assert(strcmp(path1, "/Users/test/Library/ashell/etc/passwd") == 0);
    vfs_free_path(path1);
    printf("  ✓ /etc/passwd translated correctly\n");
    
    char *path2 = vfs_translate_path("/usr/bin/ls");
    assert(strcmp(path2, "/Users/test/Library/ashell/usr/bin/ls") == 0);
    vfs_free_path(path2);
    printf("  ✓ /usr/bin/ls translated correctly\n");
    
    char *path3 = vfs_translate_path("/var/log/syslog");
    assert(strcmp(path3, "/Users/test/Library/ashell/var/log/syslog") == 0);
    vfs_free_path(path3);
    printf("  ✓ /var/log/syslog translated correctly\n");
}

void test_vfs_translate_user_paths() {
    printf("Test: vfs_translate_path() for user paths...\n");
    vfs_init("/Users/test/Library/ashell");
    
    char *path1 = vfs_translate_path("/home/user/file.txt");
    assert(strcmp(path1, "/home/user/file.txt") == 0);
    vfs_free_path(path1);
    printf("  ✓ /home/user/file.txt unchanged (not system path)\n");
    
    char *path2 = vfs_translate_path("~/Documents/file.txt");
    assert(strcmp(path2, "~/Documents/file.txt") == 0);
    vfs_free_path(path2);
    printf("  ✓ ~/Documents/file.txt unchanged\n");
}

void test_vfs_needs_translation() {
    printf("Test: vfs_path_needs_translation()...\n");
    assert(vfs_path_needs_translation("/etc/passwd") == 1);
    assert(vfs_path_needs_translation("/usr/bin/ls") == 1);
    assert(vfs_path_needs_translation("/home/user/file.txt") == 0);
    assert(vfs_path_needs_translation("relative/path") == 0);
    printf("  ✓ Path translation detection works correctly\n");
}

int main() {
    printf("\n========================================\n");
    printf("VFS Layer Unit Tests\n");
    printf("========================================\n\n");
    
    test_vfs_init();
    test_vfs_translate_system_paths();
    test_vfs_translate_user_paths();
    test_vfs_needs_translation();
    
    printf("\n========================================\n");
    printf("All VFS tests passed!\n");
    printf("========================================\n");
    
    return 0;
}

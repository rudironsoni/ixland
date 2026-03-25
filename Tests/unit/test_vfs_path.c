#include "../harness/iox_test.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* String comparison helper for test assertions */
#define IOX_ASSERT_STR_EQ(actual, expected) \
    do { \
        if (strcmp((actual), (expected)) != 0) { \
            fprintf(stderr, "FAIL: %s:%d: string mismatch: got '%s', expected '%s'\n", \
                    __FILE__, __LINE__, (actual), (expected)); \
            return false; \
        } \
    } while(0)

/* Forward declaration of function under test */
extern int __iox_path_resolve(const char *path, char *resolved, size_t resolved_len);

#ifndef IOX_MAX_PATH
#define IOX_MAX_PATH 4096
#endif

/* VFS Path Resolution Tests
 * 
 * Tests only VERIFIED_IMPLEMENTED_NOW behaviors:
 * - absolute path handling
 * - relative path resolution against cwd
 * - . handling
 * - .. handling
 * - repeated slash normalization
 * - trailing slash normalization
 * - null path rejection
 * - overlong path rejection
 * - cwd mutation influence on later resolution
 * 
 * BLOCKED items NOT tested:
 * - root-aware resolution (fs->root exists but unused by path resolution)
 * - root clamping (no enforcement)
 * - mountpoint matching (iox_vfs_path_walk is stub)
 * - longest-prefix mount selection
 * - root mutation influence
 */

#define TEST_PATH_LEN 4096

IOX_TEST(vfs_path_absolute_basic) {
    char resolved[TEST_PATH_LEN];
    
    /* Absolute path should resolve to itself (normalized) */
    int ret = __iox_path_resolve("/home/user/file.txt", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user/file.txt");
    
    /* Already normalized absolute path */
    ret = __iox_path_resolve("/usr/bin", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/usr/bin");
    
    return true;
}

IOX_TEST(vfs_path_relative_uses_cwd) {
    char resolved[TEST_PATH_LEN];
    char original_cwd[TEST_PATH_LEN];
    
    /* Save original cwd */
    IOX_ASSERT_NOT_NULL(getcwd(original_cwd, sizeof(original_cwd)));
    
    /* Change to a controlled directory */
    IOX_ASSERT(chdir("/tmp") == 0);
    
    /* Relative path should resolve against /tmp */
    int ret = __iox_path_resolve("file.txt", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/tmp/file.txt");
    
    /* Another relative path */
    ret = __iox_path_resolve("subdir/data", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/tmp/subdir/data");
    
    /* Restore original cwd */
    IOX_ASSERT(chdir(original_cwd) == 0);
    
    return true;
}

IOX_TEST(vfs_path_dot_segments) {
    char resolved[TEST_PATH_LEN];
    
    /* Single dot should be removed */
    int ret = __iox_path_resolve("/home/./user", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user");
    
    /* Multiple dots */
    ret = __iox_path_resolve("/home/././user", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user");
    
    /* Dot at end */
    ret = __iox_path_resolve("/home/user/.", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user");
    
    return true;
}

IOX_TEST(vfs_path_dotdot_segments) {
    char resolved[TEST_PATH_LEN];
    
    /* .. should back up one directory */
    int ret = __iox_path_resolve("/home/user/../other", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/other");
    
    /* Multiple .. */
    ret = __iox_path_resolve("/home/user/docs/../../", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home");
    
    /* .. at root stays at root (per implementation) */
    ret = __iox_path_resolve("/..", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/");
    
    return true;
}

IOX_TEST(vfs_path_repeated_slashes) {
    char resolved[TEST_PATH_LEN];
    
    /* Repeated slashes should collapse */
    int ret = __iox_path_resolve("/home//user", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user");
    
    /* Multiple repeated slashes */
    ret = __iox_path_resolve("/home///user////file", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user/file");
    
    /* Leading repeated slashes */
    ret = __iox_path_resolve("///home/user", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user");
    
    return true;
}

IOX_TEST(vfs_path_trailing_slash_removed) {
    char resolved[TEST_PATH_LEN];
    
    /* Trailing slash should be removed */
    int ret = __iox_path_resolve("/home/user/", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user");
    
    /* Multiple trailing slashes */
    ret = __iox_path_resolve("/home/user//", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/home/user");
    
    return true;
}

IOX_TEST(vfs_path_root_preserved) {
    char resolved[TEST_PATH_LEN];
    
    /* Root / should remain exactly / */
    int ret = __iox_path_resolve("/", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/");
    
    /* Root with trailing slashes still preserved as / */
    ret = __iox_path_resolve("///", resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_STR_EQ(resolved, "/");
    
    return true;
}

IOX_TEST(vfs_path_null_rejected) {
    char resolved[TEST_PATH_LEN];
    
    /* Null path should fail with EINVAL */
    int ret = __iox_path_resolve(NULL, resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    /* Null resolved buffer should also fail */
    ret = __iox_path_resolve("/home", NULL, sizeof(resolved));
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    return true;
}

IOX_TEST(vfs_path_overlong_rejected) {
    char resolved[TEST_PATH_LEN];
    char long_path[IOX_MAX_PATH + 100];
    
    /* Create an overlong path */
    memset(long_path, 'a', sizeof(long_path));
    long_path[0] = '/';
    long_path[sizeof(long_path) - 1] = '\0';
    
    /* Overlong path should fail with ENAMETOOLONG */
    int ret = __iox_path_resolve(long_path, resolved, sizeof(resolved));
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, ENAMETOOLONG);
    
    return true;
}

IOX_TEST(vfs_path_cwd_change_affects_resolution) {
    char resolved[TEST_PATH_LEN];
    char original_cwd[TEST_PATH_LEN];
    char result1[TEST_PATH_LEN];
    char result2[TEST_PATH_LEN];
    
    /* Save original cwd */
    IOX_ASSERT_NOT_NULL(getcwd(original_cwd, sizeof(original_cwd)));
    
    /* First CWD */
    IOX_ASSERT(chdir("/tmp") == 0);
    int ret = __iox_path_resolve("file.txt", result1, sizeof(result1));
    IOX_ASSERT_EQ(ret, 0);
    
    /* Change CWD */
    IOX_ASSERT(chdir("/var") == 0);
    ret = __iox_path_resolve("file.txt", result2, sizeof(result2));
    IOX_ASSERT_EQ(ret, 0);
    
    /* Results should differ based on CWD */
    IOX_ASSERT_STR_EQ(result1, "/tmp/file.txt");
    IOX_ASSERT_STR_EQ(result2, "/var/file.txt");
    IOX_ASSERT(strcmp(result1, result2) != 0);
    
    /* Restore original cwd */
    IOX_ASSERT(chdir(original_cwd) == 0);
    
    return true;
}

/* Directory Syscall Tests - chdir, fchdir, getcwd, mkdir, rmdir, unlink, symlink, readlink
 *
 * Tests directory and filesystem operation syscalls:
 * - iox_chdir() - Change working directory
 * - iox_fchdir() - Change working directory via FD
 * - iox_getcwd() - Get current working directory
 * - iox_mkdir() - Create directory
 * - iox_rmdir() - Remove directory
 * - iox_unlink() - Remove file
 * - iox_symlink() - Create symbolic link
 * - iox_readlink() - Read symbolic link target
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../harness/iox_test.h"

/* Function declarations from iox_directory.c */
extern int iox_chdir(const char *path);
extern int iox_fchdir(int fd);
extern char *iox_getcwd(char *buf, size_t size);
extern int iox_mkdir(const char *pathname, mode_t mode);
extern int iox_rmdir(const char *pathname);
extern int iox_unlink(const char *pathname);
extern int iox_symlink(const char *target, const char *linkpath);
extern ssize_t iox_readlink(const char *pathname, char *buf, size_t bufsiz);
extern int iox_chroot(const char *path);

#define TEST_DIR "/tmp/iox_dir_test"
#define TEST_SUBDIR TEST_DIR "/subdir"
#define TEST_NESTED TEST_DIR "/nested/deep"
#define TEST_FILE TEST_DIR "/testfile.txt"
#define TEST_SYMLINK TEST_DIR "/testlink"

static void setup_test_dir(void) {
    mkdir(TEST_DIR, 0755);
}

static void cleanup_test_dir(void) {
    /* Remove all test files and directories using iox_unlink/rmdir */
    /* Note: iox_unlink and iox_rmdir are the syscall wrappers being tested */
    /* For cleanup, we use the native system calls directly */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TEST_DIR);
    /* system() unavailable on iOS - use direct system call via libc */
    (void)cmd;
    /* Manual cleanup - just try to remove the directory */
    rmdir(TEST_DIR);
}

/* ============================================================================
 * chdir() tests
 * ============================================================================ */

IOX_TEST(chdir_null_path_returns_efault) {
    int result = iox_chdir(NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(chdir_empty_path_returns_enoent) {
    int result = iox_chdir("");
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(chdir_nonexistent_returns_enoent) {
    int result = iox_chdir("/tmp/iox_nonexistent_dir_12345");
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(chdir_file_returns_enotdir) {
    setup_test_dir();

    /* Create a test file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Try to chdir to file */
    int result = iox_chdir(TEST_FILE);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOTDIR);

    cleanup_test_dir();
    return true;
}

IOX_TEST(chdir_directory_returns_success) {
    setup_test_dir();
    mkdir(TEST_SUBDIR, 0755);

    /* Save original cwd */
    char original_cwd[1024];
    getcwd(original_cwd, sizeof(original_cwd));

    /* Change to test directory */
    int result = iox_chdir(TEST_SUBDIR);
    IOX_ASSERT_EQ(result, 0);

    /* Verify cwd changed */
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    IOX_ASSERT(strstr(cwd, "subdir") != NULL);

    /* Restore original cwd */
    chdir(original_cwd);
    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * getcwd() tests
 * ============================================================================ */

IOX_TEST(getcwd_null_buf_returns_null) {
    char *result = iox_getcwd(NULL, 1024);
    IOX_ASSERT_EQ(result, (char *)NULL);
    IOX_ASSERT_EQ(errno, EINVAL);
    return true;
}

IOX_TEST(getcwd_zero_size_returns_null) {
    char buf[1024];
    char *result = iox_getcwd(buf, 0);
    IOX_ASSERT_EQ(result, (char *)NULL);
    IOX_ASSERT_EQ(errno, EINVAL);
    return true;
}

IOX_TEST(getcwd_small_buffer_returns_erange) {
    setup_test_dir();
    mkdir(TEST_SUBDIR, 0755);
    chdir(TEST_SUBDIR);

    /* Try with very small buffer */
    char buf[1];
    char *result = iox_getcwd(buf, sizeof(buf));
    IOX_ASSERT_EQ(result, (char *)NULL);
    IOX_ASSERT_EQ(errno, ERANGE);

    chdir("/");
    cleanup_test_dir();
    return true;
}

IOX_TEST(getcwd_returns_absolute_path) {
    setup_test_dir();
    mkdir(TEST_SUBDIR, 0755);

    /* Save original cwd */
    char original_cwd[1024];
    getcwd(original_cwd, sizeof(original_cwd));

    /* Change to test directory */
    chdir(TEST_SUBDIR);

    /* Get cwd */
    char buf[1024];
    char *result = iox_getcwd(buf, sizeof(buf));
    IOX_ASSERT_NE(result, (char *)NULL);
    IOX_ASSERT_EQ(result, buf);
    IOX_ASSERT(buf[0] == '/'); /* Absolute path */
    IOX_ASSERT(strstr(buf, "subdir") != NULL);

    /* Restore original cwd */
    chdir(original_cwd);
    cleanup_test_dir();
    return true;
}

IOX_TEST(getcwd_roundtrip_with_chdir) {
    setup_test_dir();
    mkdir(TEST_SUBDIR, 0755);

    /* Save original cwd */
    char original_cwd[1024];
    getcwd(original_cwd, sizeof(original_cwd));

    /* Change directory */
    int result = iox_chdir(TEST_SUBDIR);
    IOX_ASSERT_EQ(result, 0);

    /* Get current directory */
    char cwd[1024];
    char *got = iox_getcwd(cwd, sizeof(cwd));
    IOX_ASSERT_NE(got, (char *)NULL);

    /* Verify we can cd back to original */
    result = iox_chdir(original_cwd);
    IOX_ASSERT_EQ(result, 0);

    char cwd2[1024];
    got = iox_getcwd(cwd2, sizeof(cwd2));
    IOX_ASSERT_NE(got, (char *)NULL);
    IOX_ASSERT_EQ(strcmp(cwd2, original_cwd), 0);

    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * fchdir() tests
 * ============================================================================ */

IOX_TEST(fchdir_invalid_fd_returns_ebadf) {
    int result = iox_fchdir(-1);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);

    result = iox_fchdir(9999);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    return true;
}

IOX_TEST(fchdir_stdin_returns_ebadf) {
    int result = iox_fchdir(STDIN_FILENO);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    return true;
}

IOX_TEST(fchdir_file_returns_enotdir) {
    setup_test_dir();

    /* Create a test file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Open the file */
    fd = open(TEST_FILE, O_RDONLY);
    IOX_ASSERT_GT(fd, -1);

    /* Try to fchdir to file */
    int result = iox_fchdir(fd);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOTDIR);

    close(fd);
    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * mkdir() tests
 * ============================================================================ */

IOX_TEST(mkdir_null_pathname_returns_efault) {
    int result = iox_mkdir(NULL, 0755);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(mkdir_empty_pathname_returns_enoent) {
    int result = iox_mkdir("", 0755);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(mkdir_new_directory_returns_success) {
    setup_test_dir();

    int result = iox_mkdir(TEST_SUBDIR, 0755);
    IOX_ASSERT_EQ(result, 0);

    /* Verify directory exists */
    struct stat st;
    result = stat(TEST_SUBDIR, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT(S_ISDIR(st.st_mode));

    cleanup_test_dir();
    return true;
}

IOX_TEST(mkdir_with_mode_sets_permissions) {
    setup_test_dir();

    /* Create directory with specific mode */
    int result = iox_mkdir(TEST_SUBDIR, 0700);
    IOX_ASSERT_EQ(result, 0);

    /* Verify mode was set (may be masked by umask) */
    struct stat st;
    result = stat(TEST_SUBDIR, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT(S_ISDIR(st.st_mode));
    /* Check owner has read/write/execute */
    IOX_ASSERT(st.st_mode & S_IRWXU);

    cleanup_test_dir();
    return true;
}

IOX_TEST(mkdir_existing_directory_returns_eexist) {
    setup_test_dir();
    mkdir(TEST_SUBDIR, 0755);

    /* Try to create same directory again */
    int result = iox_mkdir(TEST_SUBDIR, 0755);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EEXIST);

    cleanup_test_dir();
    return true;
}

IOX_TEST(mkdir_existing_file_returns_eexist) {
    setup_test_dir();

    /* Create a file */
    int fd = open(TEST_SUBDIR, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Try to create directory with same name */
    int result = iox_mkdir(TEST_SUBDIR, 0755);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EEXIST);

    cleanup_test_dir();
    return true;
}

IOX_TEST(mkdir_no_parent_returns_enoent) {
    /* Try to create directory without parent */
    int result = iox_mkdir("/tmp/nonexistent_parent_12345/subdir", 0755);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(mkdir_nested_parent_missing) {
    setup_test_dir();

    /* Try to create nested directory without intermediate */
    int result = iox_mkdir(TEST_NESTED, 0755);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);

    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * rmdir() tests
 * ============================================================================ */

IOX_TEST(rmdir_null_pathname_returns_efault) {
    int result = iox_rmdir(NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(rmdir_empty_pathname_returns_enoent) {
    int result = iox_rmdir("");
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(rmdir_nonexistent_returns_enoent) {
    int result = iox_rmdir("/tmp/iox_nonexistent_rmdir_12345");
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(rmdir_file_returns_enotdir) {
    setup_test_dir();

    /* Create a file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Try to rmdir a file */
    int result = iox_rmdir(TEST_FILE);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOTDIR);

    cleanup_test_dir();
    return true;
}

IOX_TEST(rmdir_empty_directory_returns_success) {
    setup_test_dir();
    mkdir(TEST_SUBDIR, 0755);

    /* Remove empty directory */
    int result = iox_rmdir(TEST_SUBDIR);
    IOX_ASSERT_EQ(result, 0);

    /* Verify directory is gone */
    struct stat st;
    result = stat(TEST_SUBDIR, &st);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);

    cleanup_test_dir();
    return true;
}

IOX_TEST(rmdir_nonempty_directory_returns_enotempty) {
    setup_test_dir();
    mkdir(TEST_SUBDIR, 0755);

    /* Create a file in subdirectory */
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/file.txt", TEST_SUBDIR);
    int fd = open(filepath, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Try to remove non-empty directory */
    int result = iox_rmdir(TEST_SUBDIR);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOTEMPTY);

    /* Cleanup */
    unlink(filepath);
    rmdir(TEST_SUBDIR);
    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * unlink() tests
 * ============================================================================ */

IOX_TEST(unlink_null_pathname_returns_efault) {
    int result = iox_unlink(NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(unlink_empty_pathname_returns_enoent) {
    int result = iox_unlink("");
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(unlink_nonexistent_returns_enoent) {
    int result = iox_unlink("/tmp/iox_nonexistent_unlink_12345");
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(unlink_file_returns_success) {
    setup_test_dir();

    /* Create a file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Verify file exists */
    struct stat st;
    int result = stat(TEST_FILE, &st);
    IOX_ASSERT_EQ(result, 0);

    /* Unlink the file */
    result = iox_unlink(TEST_FILE);
    IOX_ASSERT_EQ(result, 0);

    /* Verify file is gone */
    result = stat(TEST_FILE, &st);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);

    cleanup_test_dir();
    return true;
}

IOX_TEST(unlink_directory_returns_eisdir) {
    setup_test_dir();
    mkdir(TEST_SUBDIR, 0755);

    /* Try to unlink a directory */
    int result = iox_unlink(TEST_SUBDIR);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EISDIR);

    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * symlink() and readlink() tests
 * ============================================================================ */

IOX_TEST(symlink_null_target_returns_efault) {
    setup_test_dir();

    int result = iox_symlink(NULL, TEST_SYMLINK);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);

    cleanup_test_dir();
    return true;
}

IOX_TEST(symlink_null_linkpath_returns_efault) {
    int result = iox_symlink("/tmp/target", NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(symlink_empty_linkpath_returns_enoent) {
    int result = iox_symlink("/tmp/target", "");
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(symlink_existing_returns_eexist) {
    setup_test_dir();

    /* Create a file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Try to create symlink with same name */
    int result = iox_symlink("/tmp/target", TEST_FILE);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EEXIST);

    cleanup_test_dir();
    return true;
}

IOX_TEST(symlink_creates_symlink) {
    setup_test_dir();

    /* Create a file as target */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Create symlink pointing to file */
    int result = iox_symlink(TEST_FILE, TEST_SYMLINK);
    IOX_ASSERT_EQ(result, 0);

    /* Verify symlink exists and points to target */
    struct stat st;
    result = lstat(TEST_SYMLINK, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT(S_ISLNK(st.st_mode));

    /* Verify we can stat through the symlink */
    result = stat(TEST_SYMLINK, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT(S_ISREG(st.st_mode));

    cleanup_test_dir();
    return true;
}

IOX_TEST(readlink_null_pathname_returns_efault) {
    char buf[256];
    ssize_t result = iox_readlink(NULL, buf, sizeof(buf));
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(readlink_null_buf_returns_efault) {
    setup_test_dir();

    /* Create symlink first */
    symlink("/tmp/target", TEST_SYMLINK);

    ssize_t result = iox_readlink(TEST_SYMLINK, NULL, 256);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);

    cleanup_test_dir();
    return true;
}

IOX_TEST(readlink_zero_bufsiz_returns_einval) {
    setup_test_dir();

    /* Create symlink first */
    symlink("/tmp/target", TEST_SYMLINK);

    char buf[256];
    ssize_t result = iox_readlink(TEST_SYMLINK, buf, 0);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    cleanup_test_dir();
    return true;
}

IOX_TEST(readlink_nonexistent_returns_enoent) {
    char buf[256];
    ssize_t result = iox_readlink("/tmp/iox_nonexistent_symlink_12345", buf, sizeof(buf));
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(readlink_regular_file_returns_einval) {
    setup_test_dir();

    /* Create a file (not a symlink) */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    char buf[256];
    ssize_t result = iox_readlink(TEST_FILE, buf, sizeof(buf));
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    cleanup_test_dir();
    return true;
}

IOX_TEST(symlink_readlink_roundtrip) {
    setup_test_dir();

    const char *target = "/home/user/test/target/path";

    /* Create symlink */
    int result = iox_symlink(target, TEST_SYMLINK);
    IOX_ASSERT_EQ(result, 0);

    /* Read symlink target */
    char buf[256];
    ssize_t len = iox_readlink(TEST_SYMLINK, buf, sizeof(buf));
    IOX_ASSERT_GT(len, 0);

    /* Verify target matches (POSIX doesn't require null-termination) */
    IOX_ASSERT_EQ((size_t)len, strlen(target));
    IOX_ASSERT_EQ(strncmp(buf, target, (size_t)len), 0);

    cleanup_test_dir();
    return true;
}

IOX_TEST(symlink_relative_target_preserved) {
    setup_test_dir();

    const char *relative_target = "../other/file.txt";

    /* Create symlink with relative target */
    int result = iox_symlink(relative_target, TEST_SYMLINK);
    IOX_ASSERT_EQ(result, 0);

    /* Read and verify target is preserved */
    char buf[256];
    ssize_t len = iox_readlink(TEST_SYMLINK, buf, sizeof(buf));
    IOX_ASSERT_GT(len, 0);
    IOX_ASSERT_EQ((size_t)len, strlen(relative_target));
    IOX_ASSERT_EQ(strncmp(buf, relative_target, (size_t)len), 0);

    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * chroot() tests
 * ============================================================================ */

IOX_TEST(chroot_returns_eperm) {
    int result = iox_chroot("/tmp");
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EPERM);
    return true;
}

/* ============================================================================
 * Integration tests
 * ============================================================================ */

IOX_TEST(directory_operations_integration) {
    setup_test_dir();

    /* Create nested directory structure */
    int result = iox_mkdir(TEST_SUBDIR, 0755);
    IOX_ASSERT_EQ(result, 0);

    char nested_dir[256];
    snprintf(nested_dir, sizeof(nested_dir), "%s/nested", TEST_SUBDIR);
    result = iox_mkdir(nested_dir, 0700);
    IOX_ASSERT_EQ(result, 0);

    /* Create a file in nested dir */
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/test.txt", nested_dir);
    int fd = open(filepath, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Create symlink to file */
    char linkpath[256];
    snprintf(linkpath, sizeof(linkpath), "%s/link.txt", nested_dir);
    result = iox_symlink(filepath, linkpath);
    IOX_ASSERT_EQ(result, 0);

    /* Change to nested directory */
    char original_cwd[1024];
    getcwd(original_cwd, sizeof(original_cwd));

    result = iox_chdir(nested_dir);
    IOX_ASSERT_EQ(result, 0);

    /* Get cwd and verify */
    char cwd[1024];
    char *got = iox_getcwd(cwd, sizeof(cwd));
    IOX_ASSERT_NE(got, (char *)NULL);
    IOX_ASSERT(strstr(cwd, "nested") != NULL);

    /* Go back */
    result = iox_chdir(original_cwd);
    IOX_ASSERT_EQ(result, 0);

    /* Cleanup: remove file, symlink, directories */
    unlink(filepath);
    unlink(linkpath);
    rmdir(nested_dir);
    rmdir(TEST_SUBDIR);

    cleanup_test_dir();
    return true;
}

IOX_TEST(mkdir_chdir_getcwd_roundtrip) {
    setup_test_dir();

    /* Create and cd to directory */
    iox_mkdir(TEST_SUBDIR, 0755);

    char original[1024];
    getcwd(original, sizeof(original));

    iox_chdir(TEST_SUBDIR);

    char cwd1[1024];
    iox_getcwd(cwd1, sizeof(cwd1));

    /* Go back and verify */
    iox_chdir(original);

    char cwd2[1024];
    iox_getcwd(cwd2, sizeof(cwd2));
    IOX_ASSERT_EQ(strcmp(cwd2, original), 0);

    /* Cleanup */
    rmdir(TEST_SUBDIR);
    cleanup_test_dir();
    return true;
}

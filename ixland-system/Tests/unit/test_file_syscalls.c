/* File Syscall Tests - stat, fstat, lstat, access
 *
 * Tests file operation syscalls with VFS layer:
 * - iox_stat() - get file status by path
 * - iox_fstat() - get file status by file descriptor
 * - iox_lstat() - get file status without following symlinks
 * - iox_access() - check file accessibility
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../harness/iox_test.h"

/* Function declarations from iox_file_v2.c */
extern int __iox_stat_impl(const char *pathname, struct stat *statbuf);
extern int __iox_fstat_impl(int fd, struct stat *statbuf);
extern int __iox_lstat_impl(const char *pathname, struct stat *statbuf);
extern int __iox_access_impl(const char *pathname, int mode);

/* Public API wrappers */
extern int iox_stat(const char *pathname, struct stat *statbuf);
extern int iox_fstat(int fd, struct stat *statbuf);
extern int iox_lstat(const char *pathname, struct stat *statbuf);
extern int iox_access(const char *pathname, int mode);

#define TEST_DIR "/tmp/iox_test"
#define TEST_FILE TEST_DIR "/testfile.txt"
#define TEST_SYMLINK TEST_DIR "/testlink"

static void setup_test_dir(void) {
    mkdir(TEST_DIR, 0755);
}

static void cleanup_test_dir(void) {
    unlink(TEST_FILE);
    unlink(TEST_SYMLINK);
    rmdir(TEST_DIR);
}

/* ============================================================================
 * stat() tests
 * ============================================================================ */

IOX_TEST(stat_null_pathname_returns_efault) {
    struct stat st;
    int result = iox_stat(NULL, &st);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(stat_null_statbuf_returns_efault) {
    int result = iox_stat("/tmp", NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(stat_existing_file_returns_success) {
    setup_test_dir();

    /* Create a test file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    write(fd, "hello", 5);
    close(fd);

    /* Stat the file */
    struct stat st;
    int result = iox_stat(TEST_FILE, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT_EQ(st.st_size, 5);
    IOX_ASSERT(S_ISREG(st.st_mode));

    cleanup_test_dir();
    return true;
}

IOX_TEST(stat_directory_returns_directory_mode) {
    setup_test_dir();

    struct stat st;
    int result = iox_stat(TEST_DIR, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT(S_ISDIR(st.st_mode));

    cleanup_test_dir();
    return true;
}

IOX_TEST(stat_nonexistent_file_returns_enoent) {
    const char *nonexistent = "/tmp/iox_nonexistent_file_12345";
    struct stat st;
    int result = iox_stat(nonexistent, &st);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

/* ============================================================================
 * fstat() tests
 * ============================================================================ */

IOX_TEST(fstat_null_statbuf_returns_efault) {
    int result = iox_fstat(0, NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(fstat_invalid_fd_returns_ebadf) {
    struct stat st;
    int result = iox_fstat(-1, &st);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);

    result = iox_fstat(9999, &st);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    return true;
}

IOX_TEST(fstat_standard_fd_returns_success) {
    struct stat st;
    int result = iox_fstat(STDIN_FILENO, &st);
    IOX_ASSERT_EQ(result, 0);
    return true;
}

IOX_TEST(fstat_open_file_returns_correct_size) {
    setup_test_dir();

    /* Create a test file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    write(fd, "hello world", 11);
    close(fd);

    /* Reopen and fstat */
    fd = open(TEST_FILE, O_RDONLY);
    IOX_ASSERT_GT(fd, -1);

    struct stat st;
    int result = iox_fstat(fd, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT_EQ(st.st_size, 11);
    IOX_ASSERT(S_ISREG(st.st_mode));

    close(fd);
    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * lstat() tests
 * ============================================================================ */

IOX_TEST(lstat_null_pathname_returns_efault) {
    struct stat st;
    int result = iox_lstat(NULL, &st);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(lstat_null_statbuf_returns_efault) {
    int result = iox_lstat("/tmp", NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(lstat_symlink_returns_symlink_info) {
    setup_test_dir();

    /* Create a test file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* Create a symlink to the file */
    int result = symlink(TEST_FILE, TEST_SYMLINK);
    IOX_ASSERT_EQ(result, 0);

    /* lstat should return info about the symlink, not the target */
    struct stat st;
    result = iox_lstat(TEST_SYMLINK, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT(S_ISLNK(st.st_mode));

    cleanup_test_dir();
    return true;
}

IOX_TEST(lstat_regular_file_same_as_stat) {
    setup_test_dir();

    /* Create a test file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    write(fd, "test", 4);
    close(fd);

    /* lstat and stat should return same info for regular file */
    struct stat lst, st;
    int lresult = iox_lstat(TEST_FILE, &lst);
    int result = iox_stat(TEST_FILE, &st);
    IOX_ASSERT_EQ(lresult, 0);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT_EQ(lst.st_size, st.st_size);
    IOX_ASSERT_EQ(lst.st_mode, st.st_mode);

    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * access() tests
 * ============================================================================ */

IOX_TEST(access_null_pathname_returns_efault) {
    int result = iox_access(NULL, F_OK);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

IOX_TEST(access_existing_file_returns_success) {
    setup_test_dir();

    /* Create a test file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);
    close(fd);

    /* File exists */
    int result = iox_access(TEST_FILE, F_OK);
    IOX_ASSERT_EQ(result, 0);

    /* File is readable */
    result = iox_access(TEST_FILE, R_OK);
    IOX_ASSERT_EQ(result, 0);

    /* File is writable */
    result = iox_access(TEST_FILE, W_OK);
    IOX_ASSERT_EQ(result, 0);

    cleanup_test_dir();
    return true;
}

IOX_TEST(access_nonexistent_file_returns_enoent) {
    const char *nonexistent = "/tmp/iox_nonexistent_access_12345";
    int result = iox_access(nonexistent, F_OK);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);
    return true;
}

IOX_TEST(access_directory_returns_success) {
    setup_test_dir();

    int result = iox_access(TEST_DIR, F_OK);
    IOX_ASSERT_EQ(result, 0);

    result = iox_access(TEST_DIR, R_OK | X_OK);
    IOX_ASSERT_EQ(result, 0);

    cleanup_test_dir();
    return true;
}

/* ============================================================================
 * Integration tests
 * ============================================================================ */

IOX_TEST(file_syscalls_integration) {
    setup_test_dir();

    /* Create a file */
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    IOX_ASSERT_GT(fd, -1);

    /* Write some data */
    const char *data = "integration test data";
    ssize_t written = write(fd, data, strlen(data));
    IOX_ASSERT_EQ(written, (ssize_t)strlen(data));
    close(fd);

    /* Verify with stat */
    struct stat st;
    int result = iox_stat(TEST_FILE, &st);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT_EQ(st.st_size, (off_t)strlen(data));
    IOX_ASSERT(S_ISREG(st.st_mode));

    /* Verify with open + fstat */
    fd = open(TEST_FILE, O_RDONLY);
    IOX_ASSERT_GT(fd, -1);

    struct stat fst;
    result = iox_fstat(fd, &fst);
    IOX_ASSERT_EQ(result, 0);
    IOX_ASSERT_EQ(fst.st_size, (off_t)strlen(data));

    /* Verify with access */
    result = iox_access(TEST_FILE, R_OK);
    IOX_ASSERT_EQ(result, 0);

    close(fd);
    cleanup_test_dir();
    return true;
}

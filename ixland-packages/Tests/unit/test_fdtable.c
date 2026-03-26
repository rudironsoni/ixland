#include "../harness/iox_test.h"
#include "../../fs/fdtable.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/* FD Table Semantics Tests
 * 
 * Tests only VERIFIED_IMPLEMENTED_NOW behaviors from fs/fdtable.c:
 * - fd table allocation/free
 * - allocate lowest available fd
 * - close semantics
 * - bounds checks
 * - full-table behavior
 * - descriptor lookup
 * - whole-table duplication
 * - CLOEXEC flag storage (declared in header but check if implemented)
 * 
 * BLOCKED_NOT_IMPLEMENTED (declared in header but no implementation):
 * - iox_fd_dup (single-fd duplication)
 * - iox_fd_dup2 (dup2 semantics)
 * 
 * CLOEXEC functions declared in header but implementation status UNKNOWN:
 * - iox_fd_set_cloexec (must check if exists)
 * - iox_fd_get_cloexec (must check if exists)
 * - iox_fd_close_cloexec (must check if exists)
 */

#define TEST_MAX_FDS 16

IOX_TEST(fdtable_alloc_basic) {
    iox_files_t *files = iox_files_alloc(TEST_MAX_FDS);
    IOX_ASSERT_NOT_NULL(files);
    IOX_ASSERT_EQ(files->max_fds, TEST_MAX_FDS);
    
    /* All slots should be NULL initially */
    for (size_t i = 0; i < files->max_fds; i++) {
        IOX_ASSERT_NULL(files->fd[i]);
    }
    
    iox_files_free(files);
    return true;
}

IOX_TEST(fdtable_alloc_zero_fails) {
    /* Zero max_fds should still allocate but be useless */
    iox_files_t *files = iox_files_alloc(0);
    IOX_ASSERT_NOT_NULL(files);
    IOX_ASSERT_EQ(files->max_fds, 0);
    /* Note: calloc(0, size) behavior is implementation-defined.
     * On macOS/iOS it returns a valid pointer (not NULL) that can be freed.
     * The important thing is the table works correctly. */
    
    iox_files_free(files);
    return true;
}

IOX_TEST(fdtable_free_null_safe) {
    /* Should not crash on NULL */
    iox_files_free(NULL);
    return true;
}

IOX_TEST(fd_alloc_lowest_available) {
    iox_files_t *files = iox_files_alloc(TEST_MAX_FDS);
    IOX_ASSERT_NOT_NULL(files);
    
    /* Allocate first file */
    iox_file_t *file1 = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file1);
    
    int fd1 = iox_fd_alloc(files, file1);
    IOX_ASSERT_EQ(fd1, 0);  /* Lowest available */
    
    /* Allocate second file */
    iox_file_t *file2 = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file2);
    
    int fd2 = iox_fd_alloc(files, file2);
    IOX_ASSERT_EQ(fd2, 1);  /* Next lowest */
    
    /* Close fd0, allocate again */
    IOX_ASSERT_EQ(iox_fd_free(files, 0), 0);
    
    iox_file_t *file3 = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file3);
    
    int fd3 = iox_fd_alloc(files, file3);
    IOX_ASSERT_EQ(fd3, 0);  /* Should reuse lowest available */
    
    iox_files_free(files);
    iox_file_free(file1);
    iox_file_free(file2);
    iox_file_free(file3);
    return true;
}

IOX_TEST(fd_alloc_table_full) {
    iox_files_t *files = iox_files_alloc(2);
    IOX_ASSERT_NOT_NULL(files);
    
    /* Fill the table */
    iox_file_t *file1 = iox_file_alloc();
    iox_file_t *file2 = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file1);
    IOX_ASSERT_NOT_NULL(file2);
    
    int fd1 = iox_fd_alloc(files, file1);
    int fd2 = iox_fd_alloc(files, file2);
    IOX_ASSERT_EQ(fd1, 0);
    IOX_ASSERT_EQ(fd2, 1);
    
    /* Try to allocate third - should fail */
    iox_file_t *file3 = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file3);
    
    int fd3 = iox_fd_alloc(files, file3);
    IOX_ASSERT_EQ(fd3, -1);
    IOX_ASSERT_EQ(errno, EMFILE);
    
    /* Table should be unchanged */
    IOX_ASSERT_NOT_NULL(files->fd[0]);
    IOX_ASSERT_NOT_NULL(files->fd[1]);
    
    iox_files_free(files);
    iox_file_free(file1);
    iox_file_free(file2);
    iox_file_free(file3);
    return true;
}

IOX_TEST(fd_alloc_null_params) {
    iox_files_t *files = iox_files_alloc(TEST_MAX_FDS);
    iox_file_t *file = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(files);
    IOX_ASSERT_NOT_NULL(file);
    
    /* NULL files */
    errno = 0;
    int ret = iox_fd_alloc(NULL, file);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    /* NULL file */
    errno = 0;
    ret = iox_fd_alloc(files, NULL);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    iox_files_free(files);
    iox_file_free(file);
    return true;
}

IOX_TEST(fd_free_basic) {
    iox_files_t *files = iox_files_alloc(TEST_MAX_FDS);
    iox_file_t *file = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(files);
    IOX_ASSERT_NOT_NULL(file);
    
    int fd = iox_fd_alloc(files, file);
    IOX_ASSERT_EQ(fd, 0);
    IOX_ASSERT_NOT_NULL(files->fd[0]);
    
    /* Close the fd */
    int ret = iox_fd_free(files, fd);
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_NULL(files->fd[0]);
    
    iox_files_free(files);
    iox_file_free(file);
    return true;
}

IOX_TEST(fd_free_invalid_fd) {
    iox_files_t *files = iox_files_alloc(TEST_MAX_FDS);
    IOX_ASSERT_NOT_NULL(files);
    
    /* Negative fd */
    errno = 0;
    int ret = iox_fd_free(files, -1);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    
    /* Out of bounds fd */
    errno = 0;
    ret = iox_fd_free(files, TEST_MAX_FDS);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    
    /* Already closed fd */
    iox_file_t *file = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file);
    int fd = iox_fd_alloc(files, file);
    IOX_ASSERT_EQ(iox_fd_free(files, fd), 0);
    
    errno = 0;
    ret = iox_fd_free(files, fd);  /* Close again */
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    
    iox_files_free(files);
    iox_file_free(file);
    return true;
}

IOX_TEST(fd_free_null_files) {
    errno = 0;
    int ret = iox_fd_free(NULL, 0);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    return true;
}

IOX_TEST(fd_lookup_basic) {
    iox_files_t *files = iox_files_alloc(TEST_MAX_FDS);
    iox_file_t *file = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(files);
    IOX_ASSERT_NOT_NULL(file);
    
    int fd = iox_fd_alloc(files, file);
    IOX_ASSERT_EQ(fd, 0);
    
    /* Lookup should succeed */
    iox_file_t *found = iox_fd_lookup(files, fd);
    IOX_ASSERT_NOT_NULL(found);
    IOX_ASSERT_EQ(found, files->fd[0]);
    
    /* Refcount should be incremented by lookup */
    int refs_after = atomic_load(&found->refs);
    IOX_ASSERT(refs_after >= 2);  /* Original + lookup ref */
    
    iox_file_free(found);  /* Release lookup ref */
    iox_files_free(files);
    iox_file_free(file);
    return true;
}

IOX_TEST(fd_lookup_invalid_fd) {
    iox_files_t *files = iox_files_alloc(TEST_MAX_FDS);
    IOX_ASSERT_NOT_NULL(files);
    
    /* Negative fd */
    errno = 0;
    iox_file_t *found = iox_fd_lookup(files, -1);
    IOX_ASSERT_NULL(found);
    IOX_ASSERT_EQ(errno, EBADF);
    
    /* Out of bounds */
    errno = 0;
    found = iox_fd_lookup(files, TEST_MAX_FDS);
    IOX_ASSERT_NULL(found);
    IOX_ASSERT_EQ(errno, EBADF);
    
    /* Unallocated fd within bounds - returns NULL without setting errno
     * (implementation only sets errno on bounds check, not on unallocated slot) */
    found = iox_fd_lookup(files, 5);  /* Never allocated */
    IOX_ASSERT_NULL(found);
    /* Note: iox_fd_lookup only sets errno on bounds check, not on unallocated slot */
    
    iox_files_free(files);
    return true;
}

IOX_TEST(fd_lookup_null_files) {
    errno = 0;
    iox_file_t *found = iox_fd_lookup(NULL, 0);
    IOX_ASSERT_NULL(found);
    IOX_ASSERT_EQ(errno, EBADF);
    return true;
}

IOX_TEST(fdtable_dup_basic) {
    iox_files_t *parent = iox_files_alloc(TEST_MAX_FDS);
    IOX_ASSERT_NOT_NULL(parent);
    
    /* Add some files to parent */
    iox_file_t *file1 = iox_file_alloc();
    iox_file_t *file2 = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file1);
    IOX_ASSERT_NOT_NULL(file2);
    
    int fd1 = iox_fd_alloc(parent, file1);
    int fd2 = iox_fd_alloc(parent, file2);
    IOX_ASSERT_EQ(fd1, 0);
    IOX_ASSERT_EQ(fd2, 1);
    
    /* After insertion: refs = 2 (caller + table) */
    IOX_ASSERT_EQ(atomic_load(&file1->refs), 2);
    IOX_ASSERT_EQ(atomic_load(&file2->refs), 2);
    
    /* Duplicate the table */
    iox_files_t *child = iox_files_dup(parent);
    IOX_ASSERT_NOT_NULL(child);
    IOX_ASSERT_EQ(child->max_fds, parent->max_fds);
    
    /* After dup: refs = 3 (caller + parent + child) */
    IOX_ASSERT_EQ(atomic_load(&file1->refs), 3);
    IOX_ASSERT_EQ(atomic_load(&file2->refs), 3);
    
    /* Child should have same file object pointers */
    IOX_ASSERT_EQ(child->fd[0], parent->fd[0]);
    IOX_ASSERT_EQ(child->fd[1], parent->fd[1]);
    IOX_ASSERT_NULL(child->fd[2]);
    
    /* Free child table: refs decrements to 2 */
    iox_files_free(child);
    IOX_ASSERT_EQ(atomic_load(&file1->refs), 2);
    IOX_ASSERT_EQ(atomic_load(&file2->refs), 2);
    
    /* Free parent table: refs decrements to 1 (caller's original) */
    iox_files_free(parent);
    IOX_ASSERT_EQ(atomic_load(&file1->refs), 1);
    IOX_ASSERT_EQ(atomic_load(&file2->refs), 1);
    
    /* Caller must still free original file references */
    iox_file_free(file1);
    iox_file_free(file2);
    return true;
}

IOX_TEST(fdtable_dup_null_parent) {
    errno = 0;
    iox_files_t *child = iox_files_dup(NULL);
    IOX_ASSERT_NULL(child);
    IOX_ASSERT_EQ(errno, EINVAL);
    return true;
}

IOX_TEST(fdtable_dup_empty_table) {
    iox_files_t *parent = iox_files_alloc(TEST_MAX_FDS);
    IOX_ASSERT_NOT_NULL(parent);
    
    /* Duplicate empty table */
    iox_files_t *child = iox_files_dup(parent);
    IOX_ASSERT_NOT_NULL(child);
    IOX_ASSERT_EQ(child->max_fds, TEST_MAX_FDS);
    
    /* All slots should be NULL */
    for (size_t i = 0; i < child->max_fds; i++) {
        IOX_ASSERT_NULL(child->fd[i]);
    }
    
    iox_files_free(parent);
    iox_files_free(child);
    return true;
}

IOX_TEST(file_refcount_basic) {
    iox_file_t *file = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file);
    
    /* Initial refcount should be 1 */
    int refs = atomic_load(&file->refs);
    IOX_ASSERT_EQ(refs, 1);
    
    /* Dup increments refcount */
    iox_file_t *dup = iox_file_dup(file);
    IOX_ASSERT_EQ(dup, file);
    refs = atomic_load(&file->refs);
    IOX_ASSERT_EQ(refs, 2);
    
    /* Free decrements */
    iox_file_free(file);  /* Decrement to 1 */
    refs = atomic_load(&file->refs);
    IOX_ASSERT_EQ(refs, 1);
    
    /* Final free should actually free */
    iox_file_free(dup);  /* Decrement to 0, should free */
    
    return true;
}

IOX_TEST(file_dup_null) {
    iox_file_t *dup = iox_file_dup(NULL);
    IOX_ASSERT_NULL(dup);
    return true;
}

/* Note: CLOEXEC helper functions (iox_fd_set_cloexec, iox_fd_get_cloexec,
 * iox_fd_close_cloexec) are declared in fdtable.h but not implemented
 * in fdtable.c. The exec subsystem directly checks flags & FD_CLOEXEC
 * from fcntl.h. Testing CLOEXEC behavior requires the exec path, not
 * these unimplemented helpers.
 */

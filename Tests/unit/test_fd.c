#include "../harness/iox_test.h"
#include "../../fs/fdtable.h"
#include <stdbool.h>

IOX_TEST(fdtable_alloc) {
    iox_files_t *files = iox_files_alloc(256);
    IOX_ASSERT_NOT_NULL(files);
    iox_files_free(files);
    return true;
}

IOX_TEST(fdtable_alloc_zero_fails) {
    iox_files_t *files = iox_files_alloc(0);
    IOX_ASSERT_NOT_NULL(files);
    iox_files_free(files);
    return true;
}

IOX_TEST(fdtable_dup_basic) {
    iox_files_t *parent = iox_files_alloc(256);
    IOX_ASSERT_NOT_NULL(parent);
    
    iox_files_t *child = iox_files_dup(parent);
    IOX_ASSERT_NOT_NULL(child);
    IOX_ASSERT_NE(child, parent);
    
    iox_files_free(child);
    iox_files_free(parent);
    return true;
}

IOX_TEST(fdtable_dup_null_parent_fails) {
    iox_files_t *child = iox_files_dup(NULL);
    IOX_ASSERT_NULL(child);
    return true;
}

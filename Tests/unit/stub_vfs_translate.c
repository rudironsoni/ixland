/* Minimal stub for iox_vfs_translate - only for testing path resolution
 * This stub always returns ENOENT to trigger fallback behavior in __iox_path_resolve
 */
#include <stddef.h>
#include <errno.h>
#include <sys/types.h>

int iox_vfs_translate(const char *vpath, char *ios_path, size_t ios_path_len) {
    (void)vpath;
    (void)ios_path;
    (void)ios_path_len;
    errno = ENOENT;
    return -1;
}

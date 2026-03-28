/* Minimal stub for iox_vfs_translate - only for testing path resolution
 * This stub copies vpath directly to ios_path (passthrough for testing)
 */
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

int iox_vfs_translate(const char *vpath, char *ios_path, size_t ios_path_len) {
    if (!vpath || !ios_path || ios_path_len == 0) {
        errno = EINVAL;
        return -1;
    }

    size_t len = strlen(vpath);
    if (len >= ios_path_len) {
        errno = ENAMETOOLONG;
        return -1;
    }

    strncpy(ios_path, vpath, ios_path_len - 1);
    ios_path[ios_path_len - 1] = '\0';
    return 0;
}

int iox_vfs_reverse_translate(const char *ios_path, char *vpath, size_t vpath_len) {
    if (!ios_path || !vpath || vpath_len == 0) {
        errno = EINVAL;
        return -1;
    }

    size_t len = strlen(ios_path);
    if (len >= vpath_len) {
        errno = ENAMETOOLONG;
        return -1;
    }

    strncpy(vpath, ios_path, vpath_len - 1);
    vpath[vpath_len - 1] = '\0';
    return 0;
}

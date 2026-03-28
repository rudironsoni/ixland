#include "vfs.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

static iox_mount_t *mount_table[IOX_MAX_MOUNTS];
static pthread_mutex_t vfs_lock = PTHREAD_MUTEX_INITIALIZER;

iox_fs_t *iox_fs_alloc(void) {
    iox_fs_t *fs = calloc(1, sizeof(iox_fs_t));
    if (!fs)
        return NULL;

    atomic_init(&fs->refs, 1);
    pthread_mutex_init(&fs->lock, NULL);
    fs->umask = 022;
    strncpy(fs->cwd, "/", sizeof(fs->cwd));
    strncpy(fs->root, "/", sizeof(fs->root));

    return fs;
}

void iox_fs_free(iox_fs_t *fs) {
    if (!fs)
        return;
    if (atomic_fetch_sub(&fs->refs, 1) > 1)
        return;

    pthread_mutex_destroy(&fs->lock);
    free(fs);
}

iox_fs_t *iox_fs_dup(iox_fs_t *parent) {
    if (!parent)
        return NULL;

    iox_fs_t *child = iox_fs_alloc();
    if (!child)
        return NULL;

    pthread_mutex_lock(&parent->lock);
    strncpy(child->cwd, parent->cwd, sizeof(child->cwd));
    strncpy(child->root, parent->root, sizeof(child->root));
    child->umask = parent->umask;
    child->root_mount = parent->root_mount;
    pthread_mutex_unlock(&parent->lock);

    return child;
}

/* VFS init/deinit/mount/umount are implemented in vfs/iox_vfs.c */
/* We declare them as external here to avoid duplicate definitions */
extern int iox_vfs_init(void);
extern void iox_vfs_deinit(void);
extern int iox_vfs_mount(const char *source, const char *target, iox_vfs_ops_t *ops);
extern int iox_vfs_umount(const char *target);

int iox_vfs_path_walk(iox_fs_t *fs, const char *path, iox_vnode_t **vnode) {
    if (!fs || !path) {
        errno = EINVAL;
        return -1;
    }

    /* TODO: Implement full path resolution */
    /* This is a simplified version */

    if (!fs->root_mount) {
        errno = ENOENT;
        return -1;
    }

    *vnode = NULL;
    errno = ENOENT;
    return -1;
}

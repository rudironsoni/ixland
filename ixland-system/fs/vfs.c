#include "vfs.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static iox_mount_t *mount_table[IOX_MAX_MOUNTS];
static pthread_mutex_t vfs_lock = PTHREAD_MUTEX_INITIALIZER;

iox_fs_t *iox_fs_alloc(void) {
    iox_fs_t *fs = calloc(1, sizeof(iox_fs_t));
    if (!fs) return NULL;
    
    atomic_init(&fs->refs, 1);
    pthread_mutex_init(&fs->lock, NULL);
    fs->umask = 022;
    strncpy(fs->cwd, "/", sizeof(fs->cwd));
    strncpy(fs->root, "/", sizeof(fs->root));
    
    return fs;
}

void iox_fs_free(iox_fs_t *fs) {
    if (!fs) return;
    if (atomic_fetch_sub(&fs->refs, 1) > 1) return;
    
    pthread_mutex_destroy(&fs->lock);
    free(fs);
}

iox_fs_t *iox_fs_dup(iox_fs_t *parent) {
    if (!parent) return NULL;
    
    iox_fs_t *child = iox_fs_alloc();
    if (!child) return NULL;
    
    pthread_mutex_lock(&parent->lock);
    strncpy(child->cwd, parent->cwd, sizeof(child->cwd));
    strncpy(child->root, parent->root, sizeof(child->root));
    child->umask = parent->umask;
    child->root_mount = parent->root_mount;
    pthread_mutex_unlock(&parent->lock);
    
    return child;
}

int iox_vfs_init(void) {
    /* Initialize mount table */
    memset(mount_table, 0, sizeof(mount_table));
    return 0;
}

void iox_vfs_deinit(void) {
    /* Cleanup mounts */
    for (int i = 0; i < IOX_MAX_MOUNTS; i++) {
        if (mount_table[i]) {
            free(mount_table[i]);
            mount_table[i] = NULL;
        }
    }
}

int iox_vfs_mount(const char *source, const char *target, iox_vfs_ops_t *ops) {
    if (!target || !ops) {
        errno = EINVAL;
        return -1;
    }
    
    pthread_mutex_lock(&vfs_lock);
    
    /* Find free slot */
    int slot = -1;
    for (int i = 0; i < IOX_MAX_MOUNTS; i++) {
        if (!mount_table[i]) {
            slot = i;
            break;
        }
    }
    
    if (slot < 0) {
        pthread_mutex_unlock(&vfs_lock);
        errno = ENOMEM;
        return -1;
    }
    
    iox_mount_t *mnt = calloc(1, sizeof(iox_mount_t));
    if (!mnt) {
        pthread_mutex_unlock(&vfs_lock);
        errno = ENOMEM;
        return -1;
    }
    
    atomic_init(&mnt->refs, 1);
    strncpy(mnt->mountpoint, target, sizeof(mnt->mountpoint));
    if (source) {
        strncpy(mnt->source, source, sizeof(mnt->source));
    }
    mnt->ops = ops;
    
    mount_table[slot] = mnt;
    pthread_mutex_unlock(&vfs_lock);
    
    return 0;
}

int iox_vfs_umount(const char *target) {
    if (!target) {
        errno = EINVAL;
        return -1;
    }
    
    pthread_mutex_lock(&vfs_lock);
    
    for (int i = 0; i < IOX_MAX_MOUNTS; i++) {
        if (mount_table[i] && strcmp(mount_table[i]->mountpoint, target) == 0) {
            if (atomic_fetch_sub(&mount_table[i]->refs, 1) == 1) {
                free(mount_table[i]);
                mount_table[i] = NULL;
            }
            pthread_mutex_unlock(&vfs_lock);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&vfs_lock);
    errno = ENOENT;
    return -1;
}

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

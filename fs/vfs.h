#ifndef IOX_VFS_H
#define IOX_VFS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOX_MAX_MOUNTS 64

typedef enum {
    IOX_VTYPE_FILE = 1,
    IOX_VTYPE_DIR,
    IOX_VTYPE_LINK,
    IOX_VTYPE_CHR,
    IOX_VTYPE_BLK,
    IOX_VTYPE_FIFO,
    IOX_VTYPE_SOCK
} iox_vtype_t;

typedef struct iox_vnode iox_vnode_t;
typedef struct iox_vfs_ops iox_vfs_ops_t;
typedef struct iox_mount iox_mount_t;
typedef struct iox_fs iox_fs_t;

struct iox_vnode {
    uint64_t ino;
    iox_vtype_t type;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    off_t size;
    struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;
    atomic_int refs;
    void *fs_data;
    iox_mount_t *mount;
};

struct iox_vfs_ops {
    int (*open)(iox_vnode_t *node, int flags);
    int (*close)(iox_vnode_t *node);
    ssize_t (*read)(iox_vnode_t *node, void *buf, size_t count, off_t offset);
    ssize_t (*write)(iox_vnode_t *node, const void *buf, size_t count, off_t offset);
    int (*stat)(iox_vnode_t *node, struct stat *st);
    int (*lookup)(iox_mount_t *mnt, iox_vnode_t *parent, const char *name, iox_vnode_t **result);
    int (*create)(iox_mount_t *mnt, iox_vnode_t *parent, const char *name, mode_t mode);
    int (*mkdir)(iox_mount_t *mnt, iox_vnode_t *parent, const char *name, mode_t mode);
    int (*unlink)(iox_mount_t *mnt, iox_vnode_t *parent, const char *name);
    int (*rmdir)(iox_mount_t *mnt, iox_vnode_t *parent, const char *name);
    int (*readdir)(iox_mount_t *mnt, iox_vnode_t *dir, void *buf, size_t count);
};

struct iox_mount {
    char mountpoint[IOX_MAX_PATH];
    char source[IOX_MAX_PATH];
    iox_vfs_ops_t *ops;
    void *fs_data;
    atomic_int refs;
};

struct iox_fs {
    char cwd[IOX_MAX_PATH];
    char root[IOX_MAX_PATH];
    mode_t umask;
    iox_mount_t *root_mount;
    pthread_mutex_t lock;
    atomic_int refs;
};

iox_fs_t *iox_fs_alloc(void);
void iox_fs_free(iox_fs_t *fs);
iox_fs_t *iox_fs_dup(iox_fs_t *parent);

int iox_vfs_init(void);
void iox_vfs_deinit(void);
int iox_vfs_mount(const char *source, const char *target, iox_vfs_ops_t *ops);
int iox_vfs_umount(const char *target);

int iox_vfs_open(const char *path, int flags, mode_t mode, iox_vnode_t **vnode);
int iox_vfs_close(iox_vnode_t *vnode);
int iox_vfs_stat(const char *path, struct stat *st);
int iox_vfs_mkdir(const char *path, mode_t mode);
int iox_vfs_unlink(const char *path);
int iox_vfs_rmdir(const char *path);

int iox_vfs_lookup(iox_fs_t *fs, const char *path, iox_vnode_t **vnode);
int iox_vfs_path_walk(iox_fs_t *fs, const char *path, iox_vnode_t **vnode);

#ifdef __cplusplus
}
#endif

#endif

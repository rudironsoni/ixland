/* iOS Subsystem for Linux - Virtual File System
 *
 * Complete VFS implementation for path translation, sandboxing,
 * and virtual mount points.
 */

#include "../internal/iox_internal.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <pthread.h>

/* ============================================================================
 * VFS GLOBAL STATE
 * ============================================================================ */

pthread_mutex_t vfs_lock = PTHREAD_MUTEX_INITIALIZER;

/* iOS Sandbox root - determined at runtime */
char ios_sandbox_root[IOX_MAX_PATH] = "";
char ios_home_dir[IOX_MAX_PATH] = "";
char ios_tmp_dir[IOX_MAX_PATH] = "";

/* Virtual mount points */
#define VFS_MAX_MOUNTS 64
iox_vfs_mount_t vfs_mount_table[VFS_MAX_MOUNTS];
int vfs_mount_count = 0;

static int iox_vfs_mount_unlocked(const char *source, const char *target,
                                  unsigned long flags) {
    if (!source || !target) {
        errno = EINVAL;
        return -1;
    }

    if (vfs_mount_count >= VFS_MAX_MOUNTS) {
        errno = ENOMEM;
        return -1;
    }

    for (int i = 0; i < vfs_mount_count; i++) {
        if (vfs_mount_table[i].active &&
            strcmp(vfs_mount_table[i].mountpoint, target) == 0) {
            errno = EBUSY;
            return -1;
        }
    }

    iox_vfs_mount_t *mnt = &vfs_mount_table[vfs_mount_count++];
    mnt->active = true;
    mnt->flags = flags;
    strncpy(mnt->mountpoint, target, IOX_MAX_PATH - 1);
    strncpy(mnt->target, source, IOX_MAX_PATH - 1);
    mnt->mountpoint[IOX_MAX_PATH - 1] = '\0';
    mnt->target[IOX_MAX_PATH - 1] = '\0';

    return 0;
}

/* ============================================================================
 * PATH TRANSLATION
 * ============================================================================ */

int iox_vfs_init(void) {
    pthread_mutex_lock(&vfs_lock);
    
    /* Clear mount table */
    memset(vfs_mount_table, 0, sizeof(vfs_mount_table));
    vfs_mount_count = 0;
    
    /* Determine iOS sandbox root */
    char *home = getenv("HOME");
    if (home) {
        strncpy(ios_sandbox_root, home, IOX_MAX_PATH - 1);
        ios_sandbox_root[IOX_MAX_PATH - 1] = '\0';
        
        /* Standard iOS directories */
        snprintf(ios_home_dir, IOX_MAX_PATH, "%s/Documents", home);
        snprintf(ios_tmp_dir, IOX_MAX_PATH, "%s/tmp", home);
        
        /* Set up default mounts - don't fail if mkdir fails */
        iox_vfs_mount_unlocked("/home/user", ios_home_dir, IOX_VFS_BIND);
        iox_vfs_mount_unlocked("/tmp", ios_tmp_dir, IOX_VFS_BIND);
        
        /* /etc -> ~/Library/etc (config files) - optional */
        char etc_path[IOX_MAX_PATH];
        snprintf(etc_path, IOX_MAX_PATH, "%s/Library/etc", home);
        /* Don't call mkdir here - might fail in restricted contexts */
        iox_vfs_mount_unlocked("/etc", etc_path, IOX_VFS_BIND);
        
        /* /var/tmp -> ~/tmp */
        iox_vfs_mount_unlocked("/var/tmp", ios_tmp_dir, IOX_VFS_BIND);
    } else {
        /* No HOME set - use default paths */
        strncpy(ios_sandbox_root, "/", IOX_MAX_PATH - 1);
        ios_sandbox_root[IOX_MAX_PATH - 1] = '\0';
        strncpy(ios_home_dir, "/", IOX_MAX_PATH - 1);
        ios_home_dir[IOX_MAX_PATH - 1] = '\0';
        strncpy(ios_tmp_dir, "/tmp", IOX_MAX_PATH - 1);
        ios_tmp_dir[IOX_MAX_PATH - 1] = '\0';
        
        /* Set up minimal mounts */
        iox_vfs_mount_unlocked("/home/user", ios_home_dir, IOX_VFS_BIND);
        iox_vfs_mount_unlocked("/tmp", ios_tmp_dir, IOX_VFS_BIND);
    }
    
    pthread_mutex_unlock(&vfs_lock);
    return 0;
}

void iox_vfs_deinit(void) {
    pthread_mutex_lock(&vfs_lock);
    
    /* Unmount everything */
    for (int i = 0; i < vfs_mount_count; i++) {
        if (vfs_mount_table[i].active) {
            vfs_mount_table[i].active = false;
        }
    }
    vfs_mount_count = 0;
    
    pthread_mutex_unlock(&vfs_lock);
}

/* Translate virtual path to iOS path */
int iox_vfs_translate(const char *vpath, char *ios_path, size_t ios_path_len) {
    if (!vpath || !ios_path || ios_path_len == 0) {
        errno = EINVAL;
        return -1;
    }
    
    pthread_mutex_lock(&vfs_lock);
    
    /* Normalize virtual path first */
    char norm_vpath[IOX_MAX_PATH];
    strncpy(norm_vpath, vpath, IOX_MAX_PATH - 1);
    norm_vpath[IOX_MAX_PATH - 1] = '\0';
    __iox_path_normalize(norm_vpath);
    
    /* Check for mount points (longest match first) */
    iox_vfs_mount_t *best_match = NULL;
    size_t best_len = 0;
    
    for (int i = 0; i < vfs_mount_count; i++) {
        if (!vfs_mount_table[i].active) continue;
        
        size_t mount_len = strlen(vfs_mount_table[i].mountpoint);
        if (strncmp(norm_vpath, vfs_mount_table[i].mountpoint, mount_len) == 0) {
            /* Check if it's a complete path component match */
            if (norm_vpath[mount_len] == '/' || norm_vpath[mount_len] == '\0') {
                if (mount_len > best_len) {
                    best_match = &vfs_mount_table[i];
                    best_len = mount_len;
                }
            }
        }
    }
    
    if (best_match) {
        /* Translate path */
        const char *remainder = norm_vpath + best_len;
        if (*remainder == '/') remainder++;
        
        snprintf(ios_path, ios_path_len, "%s/%s", 
                 best_match->target, remainder);
    } else {
        /* No mount point - map to sandbox relative */
        if (norm_vpath[0] == '/') {
            /* Absolute path without mount - use home as fallback */
            snprintf(ios_path, ios_path_len, "%s%s", ios_home_dir, norm_vpath);
        } else {
            /* Relative path - resolve against CWD */
            char cwd[IOX_MAX_PATH];
            if (getcwd(cwd, sizeof(cwd))) {
                snprintf(ios_path, ios_path_len, "%s/%s", cwd, norm_vpath);
            } else {
                strncpy(ios_path, norm_vpath, ios_path_len - 1);
                ios_path[ios_path_len - 1] = '\0';
            }
        }
    }
    
    /* Final normalization */
    __iox_path_normalize(ios_path);
    
    /* Validate result is within sandbox */
    if (!iox_vfs_in_sandbox(ios_path)) {
        pthread_mutex_unlock(&vfs_lock);
        errno = EACCES;
        return -1;
    }
    
    pthread_mutex_unlock(&vfs_lock);
    return 0;
}

/* Reverse translate iOS path to virtual path */
int iox_vfs_reverse_translate(const char *ios_path, char *vpath, size_t vpath_len) {
    if (!ios_path || !vpath || vpath_len == 0) {
        errno = EINVAL;
        return -1;
    }
    
    pthread_mutex_lock(&vfs_lock);
    
    /* Find the mount point this iOS path belongs to */
    for (int i = 0; i < vfs_mount_count; i++) {
        if (!vfs_mount_table[i].active) continue;
        
        size_t target_len = strlen(vfs_mount_table[i].target);
        if (strncmp(ios_path, vfs_mount_table[i].target, target_len) == 0) {
            if (ios_path[target_len] == '/' || ios_path[target_len] == '\0') {
                const char *remainder = ios_path + target_len;
                if (*remainder == '/') remainder++;
                
                if (strlen(remainder) == 0) {
                    strncpy(vpath, vfs_mount_table[i].mountpoint, vpath_len - 1);
                    vpath[vpath_len - 1] = '\0';
                } else {
                    snprintf(vpath, vpath_len, "%s/%s",
                             vfs_mount_table[i].mountpoint, remainder);
                }
                
                pthread_mutex_unlock(&vfs_lock);
                return 0;
            }
        }
    }
    
    /* No matching mount - return as-is */
    strncpy(vpath, ios_path, vpath_len - 1);
    vpath[vpath_len - 1] = '\0';
    
    pthread_mutex_unlock(&vfs_lock);
    return 0;
}

/* ============================================================================
 * MOUNT OPERATIONS
 * ============================================================================ */

int iox_vfs_mount(const char *source, const char *target, unsigned long flags) {
    pthread_mutex_lock(&vfs_lock);
    int result = iox_vfs_mount_unlocked(source, target, flags);
    pthread_mutex_unlock(&vfs_lock);
    return result;
}

int iox_vfs_umount(const char *target) {
    if (!target) {
        errno = EINVAL;
        return -1;
    }
    
    pthread_mutex_lock(&vfs_lock);
    
    for (int i = 0; i < vfs_mount_count; i++) {
        if (vfs_mount_table[i].active &&
            strcmp(vfs_mount_table[i].mountpoint, target) == 0) {
            vfs_mount_table[i].active = false;
            pthread_mutex_unlock(&vfs_lock);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&vfs_lock);
    errno = EINVAL;
    return -1;
}

int iox_vfs_umount2(const char *target, int flags) {
    (void)flags;
    return iox_vfs_umount(target);
}

/* ============================================================================
 * SANDBOX VALIDATION
 * ============================================================================ */

bool iox_vfs_in_sandbox(const char *path) {
    if (!path) return false;
    
    /* Must be absolute path */
    if (path[0] != '/') return false;
    
    /* Check against sandbox root */
    size_t sandbox_len = strlen(ios_sandbox_root);
    if (strncmp(path, ios_sandbox_root, sandbox_len) == 0) {
        /* Path is within sandbox */
        return true;
    }
    
    /* Allow /dev/null, /dev/zero, /dev/random, etc. */
    if (strncmp(path, "/dev/", 5) == 0) {
        /* Allow specific device files */
        const char *dev = path + 5;
        if (strcmp(dev, "null") == 0 ||
            strcmp(dev, "zero") == 0 ||
            strcmp(dev, "random") == 0 ||
            strcmp(dev, "urandom") == 0 ||
            strcmp(dev, "tty") == 0) {
            return true;
        }
    }
    
    return false;
}

int iox_vfs_chroot(const char *path) {
    /* chroot is not allowed in iOS sandbox */
    (void)path;
    errno = EPERM;
    return -1;
}

/* ============================================================================
 * FILE OPERATIONS WITH VFS
 * ============================================================================ */

int iox_vfs_open(const char *pathname, int flags, mode_t mode) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return open(ios_path, flags, mode);
}

int iox_vfs_stat(const char *pathname, struct stat *statbuf) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return stat(ios_path, statbuf);
}

int iox_vfs_lstat(const char *pathname, struct stat *statbuf) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return lstat(ios_path, statbuf);
}

int iox_vfs_access(const char *pathname, int mode) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return access(ios_path, mode);
}

int iox_vfs_mkdir(const char *pathname, mode_t mode) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return mkdir(ios_path, mode);
}

int iox_vfs_rmdir(const char *pathname) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return rmdir(ios_path);
}

int iox_vfs_unlink(const char *pathname) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return unlink(ios_path);
}

int iox_vfs_rename(const char *oldpath, const char *newpath) {
    char ios_old[IOX_MAX_PATH];
    char ios_new[IOX_MAX_PATH];
    
    if (iox_vfs_translate(oldpath, ios_old, sizeof(ios_old)) != 0) {
        return -1;
    }
    
    if (iox_vfs_translate(newpath, ios_new, sizeof(ios_new)) != 0) {
        return -1;
    }
    
    return rename(ios_old, ios_new);
}

int iox_vfs_symlink(const char *target, const char *linkpath) {
    char ios_link[IOX_MAX_PATH];
    
    if (iox_vfs_translate(linkpath, ios_link, sizeof(ios_link)) != 0) {
        return -1;
    }
    
    /* Target is stored as-is in symlink */
    return symlink(target, ios_link);
}

ssize_t iox_vfs_readlink(const char *pathname, char *buf, size_t bufsiz) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return readlink(ios_path, buf, bufsiz);
}

int iox_vfs_chmod(const char *pathname, mode_t mode) {
    char ios_path[IOX_MAX_PATH];
    
    if (iox_vfs_translate(pathname, ios_path, sizeof(ios_path)) != 0) {
        return -1;
    }
    
    return chmod(ios_path, mode);
}

int iox_vfs_chown(const char *pathname, uid_t owner, gid_t group) {
    /* chown is restricted in iOS sandbox */
    (void)pathname;
    (void)owner;
    (void)group;
    errno = EPERM;
    return -1;
}

int iox_vfs_link(const char *oldpath, const char *newpath) {
    char ios_old[IOX_MAX_PATH];
    char ios_new[IOX_MAX_PATH];
    
    if (iox_vfs_translate(oldpath, ios_old, sizeof(ios_old)) != 0) {
        return -1;
    }
    
    if (iox_vfs_translate(newpath, ios_new, sizeof(ios_new)) != 0) {
        return -1;
    }
    
    return link(ios_old, ios_new);
}

/* ============================================================================
 * VFS PUBLIC API
 * ============================================================================ */

const char *iox_vfs_get_sandbox_root(void) {
    return ios_sandbox_root;
}

const char *iox_vfs_get_home(void) {
    return ios_home_dir;
}

const char *iox_vfs_get_tmp(void) {
    return ios_tmp_dir;
}

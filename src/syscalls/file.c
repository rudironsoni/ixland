/*
 * file.c - File operation syscalls
 *
 * File I/O with VFS path translation
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include "../../include/linux/fcntl.h"
#include "../../include/linux/stat.h"
#include "../../include/linux/unistd.h"
#include "../../include/vfs.h"

/* ============================================================================
 * File Opening
 * ============================================================================ */

int a_shell_open(const char *pathname, int flags, ...) {
    mode_t mode = 0;
    
    /* Get mode if O_CREAT is set */
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int fd = open(real_path, flags, mode);
    vfs_free_path(real_path);
    
    return fd;
}

int a_shell_openat(int dirfd, const char *pathname, int flags, ...) {
    mode_t mode = 0;
    
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    
    /* If absolute path or dirfd is AT_FDCWD, use normal open with translation */
    if (pathname[0] == '/' || dirfd == AT_FDCWD) {
        return a_shell_open(pathname, flags, mode);
    }
    
    /* Relative path with dirfd - pass through to native */
    return openat(dirfd, pathname, flags, mode);
}

int a_shell_creat(const char *pathname, mode_t mode) {
    return a_shell_open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

/* ============================================================================
 * File Control
 * ============================================================================ */

int a_shell_fcntl(int fd, int cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    
    int result;
    
    switch (cmd) {
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:
            /* No additional argument */
            result = fcntl(fd, cmd);
            break;
            
        case F_SETFD:
        case F_SETFL:
        case F_SETOWN:
            /* Integer argument */
            result = fcntl(fd, cmd, va_arg(ap, int));
            break;
            
        case F_GETLK:
        case F_SETLK:
        case F_SETLKW:
            /* struct flock argument */
            result = fcntl(fd, cmd, va_arg(ap, struct flock *));
            break;
            
        default:
            /* Unknown command - try without additional arg first */
            result = fcntl(fd, cmd);
            if (result == -1 && errno == EINVAL) {
                /* Try with pointer arg */
                result = fcntl(fd, cmd, va_arg(ap, void *));
            }
            break;
    }
    
    va_end(ap);
    return result;
}

int a_shell_flock(int fd, int operation) {
    return flock(fd, operation);
}

/* ============================================================================
 * File Status
 * ============================================================================ */

int a_shell_stat(const char *pathname, struct stat *statbuf) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = stat(real_path, statbuf);
    vfs_free_path(real_path);
    
    return ret;
}

int a_shell_fstat(int fd, struct stat *statbuf) {
    return fstat(fd, statbuf);
}

int a_shell_lstat(const char *pathname, struct stat *statbuf) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = lstat(real_path, statbuf);
    vfs_free_path(real_path);
    
    return ret;
}

int a_shell_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags) {
    if (pathname[0] == '/' || dirfd == AT_FDCWD) {
        char *real_path = vfs_translate_path(pathname);
        if (!real_path) {
            errno = ENOENT;
            return -1;
        }
        
        int ret;
        if (flags & AT_SYMLINK_NOFOLLOW) {
            ret = lstat(real_path, statbuf);
        } else {
            ret = stat(real_path, statbuf);
        }
        vfs_free_path(real_path);
        return ret;
    }
    
    return fstatat(dirfd, pathname, statbuf, flags);
}

/* ============================================================================
 * Permissions
 * ============================================================================ */

int a_shell_chmod(const char *pathname, mode_t mode) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = chmod(real_path, mode);
    vfs_free_path(real_path);
    
    return ret;
}

int a_shell_fchmod(int fd, mode_t mode) {
    return fchmod(fd, mode);
}

int a_shell_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) {
    if (pathname[0] == '/' || dirfd == AT_FDCWD) {
        if (flags & AT_SYMLINK_NOFOLLOW) {
            /* lchmod not standard, fall back to chmod */
        }
        return a_shell_chmod(pathname, mode);
    }
    
    return fchmodat(dirfd, pathname, mode, flags);
}

/* ============================================================================
 * Ownership
 * ============================================================================ */

int a_shell_chown(const char *pathname, uid_t owner, gid_t group) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = chown(real_path, owner, group);
    vfs_free_path(real_path);
    
    return ret;
}

int a_shell_fchown(int fd, uid_t owner, gid_t group) {
    return fchown(fd, owner, group);
}

int a_shell_lchown(const char *pathname, uid_t owner, gid_t group) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = lchown(real_path, owner, group);
    vfs_free_path(real_path);
    
    return ret;
}

int a_shell_fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags) {
    if (pathname[0] == '/' || dirfd == AT_FDCWD) {
        if (flags & AT_SYMLINK_NOFOLLOW) {
            return a_shell_lchown(pathname, owner, group);
        }
        return a_shell_chown(pathname, owner, group);
    }
    
    return fchownat(dirfd, pathname, owner, group, flags);
}

/* ============================================================================
 * Umask
 * ============================================================================ */

mode_t a_shell_umask(mode_t mask) {
    return umask(mask);
}

/* ============================================================================
 * Directory Creation
 * ============================================================================ */

int a_shell_mkdir(const char *pathname, mode_t mode) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = mkdir(real_path, mode);
    vfs_free_path(real_path);
    
    return ret;
}

int a_shell_mkdirat(int dirfd, const char *pathname, mode_t mode) {
    if (pathname[0] == '/' || dirfd == AT_FDCWD) {
        return a_shell_mkdir(pathname, mode);
    }
    
    return mkdirat(dirfd, pathname, mode);
}

/* ============================================================================
 * Special Files
 * ============================================================================ */

int a_shell_mknod(const char *pathname, mode_t mode, dev_t dev) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = mknod(real_path, mode, dev);
    vfs_free_path(real_path);
    
    return ret;
}

int a_shell_mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev) {
    if (pathname[0] == '/' || dirfd == AT_FDCWD) {
        return a_shell_mknod(pathname, mode, dev);
    }
    
    return mknodat(dirfd, pathname, mode, dev);
}

int a_shell_mkfifo(const char *pathname, mode_t mode) {
    char *real_path = vfs_translate_path(pathname);
    if (!real_path) {
        errno = ENOENT;
        return -1;
    }
    
    int ret = mkfifo(real_path, mode);
    vfs_free_path(real_path);
    
    return ret;
}

int a_shell_mkfifoat(int dirfd, const char *pathname, mode_t mode) {
    if (pathname[0] == '/' || dirfd == AT_FDCWD) {
        return a_shell_mkfifo(pathname, mode);
    }
    
    return mkfifoat(dirfd, pathname, mode);
}

/* Note: statfs/statvfs declared in linux/stat.h, implemented here as static inlines or macros */

/* ============================================================================
 * Advisory Record Locking
 * ============================================================================ */

int a_shell_posix_fadvise(int fd, off_t offset, off_t len, int advice) {
    return posix_fadvise(fd, offset, len, advice);
}

int a_shell_posix_fallocate(int fd, off_t offset, off_t len) {
    return posix_fallocate(fd, offset, len);
}

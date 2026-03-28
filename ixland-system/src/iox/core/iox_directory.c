/* iOS Subsystem for Linux - Directory Operations
 *
 * Directory and filesystem syscalls with VFS integration:
 * - iox_chdir() - Change working directory
 * - iox_fchdir() - Change working directory via FD
 * - iox_getcwd() - Get current working directory
 * - iox_chroot() - Change root directory
 * - iox_mkdir() - Create directory
 * - iox_rmdir() - Remove directory
 * - iox_unlink() - Remove file
 * - iox_symlink() - Create symbolic link
 * - iox_readlink() - Read symbolic link target
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../internal/iox_internal.h"

/* ============================================================================
 * DIRECTORY OPERATIONS
 * ============================================================================ */

/**
 * @brief Change current working directory
 *
 * Changes the calling process's working directory to the specified path.
 * The path is translated through VFS before changing directory.
 *
 * @param path New working directory path
 * @return 0 on success, -1 on error with errno set
 */
int iox_chdir(const char *path) {
    if (!path) {
        errno = EFAULT;
        return -1;
    }

    /* Validate path is not empty */
    if (path[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    /* Translate path through VFS */
    char translated_path[IOX_MAX_PATH];
    if (iox_vfs_translate(path, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    /* Verify path is a directory */
    struct stat st;
    if (stat(translated_path, &st) != 0) {
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    /* Check execute permission on directory */
    if (access(translated_path, X_OK) != 0) {
        errno = EACCES;
        return -1;
    }

    /* Perform the actual chdir */
    if (chdir(translated_path) != 0) {
        return -1;
    }

    /* Update current process's cwd if we have a current process */
    __iox_process_t *proc = __iox_current_process;
    if (proc) {
        pthread_mutex_lock(&proc->thread_lock);
        strncpy(proc->cwd, path, IOX_MAX_PATH - 1);
        proc->cwd[IOX_MAX_PATH - 1] = '\0';
        pthread_mutex_unlock(&proc->thread_lock);
    }

    return 0;
}

/**
 * @brief Change working directory via file descriptor
 *
 * Changes the calling process's working directory to the directory
 * referenced by the file descriptor.
 *
 * @param fd File descriptor of directory
 * @return 0 on success, -1 on error with errno set
 */
int iox_fchdir(int fd) {
    if (fd < 0 || fd >= IOX_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    /* Standard FDs are not directories */
    if (fd <= 2) {
        errno = EBADF;
        return -1;
    }

    /* Get FD entry */
    pthread_mutex_lock(&__iox_process_table_lock);
    __iox_process_t *proc = __iox_current_process;
    if (!proc) {
        pthread_mutex_unlock(&__iox_process_table_lock);
        errno = EBADF;
        return -1;
    }

    pthread_mutex_lock(&proc->fd_lock);
    if (!proc->fd_table[fd].used) {
        pthread_mutex_unlock(&proc->fd_lock);
        pthread_mutex_unlock(&__iox_process_table_lock);
        errno = EBADF;
        return -1;
    }

    int real_fd = proc->fd_table[fd].fd;
    const char *path = proc->fd_table[fd].path;
    pthread_mutex_unlock(&proc->fd_lock);
    pthread_mutex_unlock(&__iox_process_table_lock);

    /* Verify it's a directory */
    struct stat st;
    if (fstat(real_fd, &st) != 0) {
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    /* Use fchdir on the real FD */
    if (fchdir(real_fd) != 0) {
        return -1;
    }

    /* Update process cwd */
    if (proc) {
        pthread_mutex_lock(&proc->thread_lock);
        strncpy(proc->cwd, path, IOX_MAX_PATH - 1);
        proc->cwd[IOX_MAX_PATH - 1] = '\0';
        pthread_mutex_unlock(&proc->thread_lock);
    }

    return 0;
}

/**
 * @brief Get current working directory
 *
 * Returns the absolute pathname of the current working directory.
 * The path is reverse-translated through VFS if necessary.
 *
 * @param buf Buffer to store the path
 * @param size Size of buffer
 * @return Pointer to buf on success, NULL on error with errno set
 */
char *iox_getcwd(char *buf, size_t size) {
    if (size == 0) {
        errno = EINVAL;
        return NULL;
    }

    if (!buf) {
        /* POSIX allows NULL buf, but we require caller to allocate */
        errno = EINVAL;
        return NULL;
    }

    /* Get current working directory from OS */
    char ios_cwd[IOX_MAX_PATH];
    if (!getcwd(ios_cwd, sizeof(ios_cwd))) {
        return NULL;
    }

    /* Try to reverse-translate to virtual path */
    char vpath[IOX_MAX_PATH];
    if (iox_vfs_reverse_translate(ios_cwd, vpath, sizeof(vpath)) == 0) {
        /* Check if buffer is large enough */
        size_t len = strlen(vpath);
        if (len >= size) {
            errno = ERANGE;
            return NULL;
        }
        strncpy(buf, vpath, size - 1);
        buf[size - 1] = '\0';
    } else {
        /* Use iOS path directly */
        size_t len = strlen(ios_cwd);
        if (len >= size) {
            errno = ERANGE;
            return NULL;
        }
        strncpy(buf, ios_cwd, size - 1);
        buf[size - 1] = '\0';
    }

    /* Update process cwd cache */
    __iox_process_t *proc = __iox_current_process;
    if (proc) {
        pthread_mutex_lock(&proc->thread_lock);
        strncpy(proc->cwd, buf, IOX_MAX_PATH - 1);
        proc->cwd[IOX_MAX_PATH - 1] = '\0';
        pthread_mutex_unlock(&proc->thread_lock);
    }

    return buf;
}

/**
 * @brief Change root directory
 *
 * On iOS, chroot is not permitted due to sandbox restrictions.
 * Always returns EPERM.
 *
 * @param path New root directory (ignored)
 * @return -1 with errno=EPERM
 */
int iox_chroot(const char *path) {
    (void)path;
    /* chroot is not allowed in iOS sandbox */
    errno = EPERM;
    return -1;
}

/* ============================================================================
 * FILESYSTEM OPERATIONS
 * ============================================================================ */

/**
 * @brief Create a directory
 *
 * Creates a new directory with the specified mode.
 * The mode is masked by the current umask.
 *
 * @param pathname Path to the new directory
 * @param mode Directory permissions (masked by umask)
 * @return 0 on success, -1 on error with errno set
 */
int iox_mkdir(const char *pathname, mode_t mode) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }

    /* Validate pathname is not empty */
    if (pathname[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    /* Translate path through VFS */
    char translated_path[IOX_MAX_PATH];
    if (iox_vfs_translate(pathname, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    /* Check if parent directory exists and is writable */
    char parent_path[IOX_MAX_PATH];
    strncpy(parent_path, translated_path, IOX_MAX_PATH - 1);
    parent_path[IOX_MAX_PATH - 1] = '\0';

    char *last_slash = strrchr(parent_path, '/');
    if (last_slash && last_slash != parent_path) {
        *last_slash = '\0';
        if (access(parent_path, W_OK | X_OK) != 0) {
            errno = EACCES;
            return -1;
        }
    }

    /* Create the directory */
    if (mkdir(translated_path, mode) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Create a directory (extended)
 *
 * Creates a directory relative to a directory file descriptor.
 * Currently delegates to regular mkdir.
 *
 * @param dirfd Directory file descriptor (or AT_FDCWD)
 * @param pathname Path to the new directory
 * @param mode Directory permissions
 * @return 0 on success, -1 on error with errno set
 */
int iox_mkdirat(int dirfd, const char *pathname, mode_t mode) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }

    if (dirfd == AT_FDCWD) {
        return iox_mkdir(pathname, mode);
    }

    /* TODO: Implement dirfd-relative path resolution */
    (void)dirfd;
    (void)mode;
    errno = ENOSYS;
    return -1;
}

/**
 * @brief Remove a directory
 *
 * Removes an empty directory.
 *
 * @param pathname Path to the directory
 * @return 0 on success, -1 on error with errno set
 */
int iox_rmdir(const char *pathname) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }

    /* Validate pathname is not empty */
    if (pathname[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    /* Translate path through VFS */
    char translated_path[IOX_MAX_PATH];
    if (iox_vfs_translate(pathname, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    /* Verify it's a directory */
    struct stat st;
    if (stat(translated_path, &st) != 0) {
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    /* Check write permission on parent */
    char parent_path[IOX_MAX_PATH];
    strncpy(parent_path, translated_path, IOX_MAX_PATH - 1);
    parent_path[IOX_MAX_PATH - 1] = '\0';

    char *last_slash = strrchr(parent_path, '/');
    if (last_slash && last_slash != parent_path) {
        *last_slash = '\0';
        if (access(parent_path, W_OK | X_OK) != 0) {
            errno = EACCES;
            return -1;
        }
    }

    /* Remove the directory */
    if (rmdir(translated_path) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Remove a file
 *
 * Removes a file or symbolic link. Does not remove directories.
 *
 * @param pathname Path to the file
 * @return 0 on success, -1 on error with errno set
 */
int iox_unlink(const char *pathname) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }

    /* Validate pathname is not empty */
    if (pathname[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    /* Translate path through VFS */
    char translated_path[IOX_MAX_PATH];
    if (iox_vfs_translate(pathname, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    /* Verify it's not a directory */
    struct stat st;
    if (stat(translated_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            errno = EISDIR;
            return -1;
        }
    }

    /* Check write permission on parent directory */
    char parent_path[IOX_MAX_PATH];
    strncpy(parent_path, translated_path, IOX_MAX_PATH - 1);
    parent_path[IOX_MAX_PATH - 1] = '\0';

    char *last_slash = strrchr(parent_path, '/');
    if (last_slash && last_slash != parent_path) {
        *last_slash = '\0';
        if (access(parent_path, W_OK | X_OK) != 0) {
            errno = EACCES;
            return -1;
        }
    }

    /* Remove the file */
    if (unlink(translated_path) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Remove a file relative to directory
 *
 * Removes a file or directory (if AT_REMOVEDIR flag) relative to dirfd.
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to the file
 * @param flags Flags (AT_REMOVEDIR to remove directory)
 * @return 0 on success, -1 on error with errno set
 */
int iox_unlinkat(int dirfd, const char *pathname, int flags) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }

    if (dirfd == AT_FDCWD) {
        if (flags & AT_REMOVEDIR) {
            return iox_rmdir(pathname);
        } else {
            return iox_unlink(pathname);
        }
    }

    /* TODO: Implement dirfd-relative path resolution */
    (void)dirfd;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

/* ============================================================================
 * SYMBOLIC LINK OPERATIONS
 * ============================================================================ */

/**
 * @brief Create a symbolic link
 *
 * Creates a symbolic link named linkpath that contains target.
 *
 * @param target Target path (stored in symlink)
 * @param linkpath Path for the new symbolic link
 * @return 0 on success, -1 on error with errno set
 */
int iox_symlink(const char *target, const char *linkpath) {
    if (!target || !linkpath) {
        errno = EFAULT;
        return -1;
    }

    /* Validate paths are not empty */
    if (linkpath[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    /* Translate linkpath through VFS */
    char translated_link[IOX_MAX_PATH];
    if (iox_vfs_translate(linkpath, translated_link, sizeof(translated_link)) != 0) {
        return -1;
    }

    /* Check if linkpath already exists */
    struct stat st;
    if (stat(translated_link, &st) == 0) {
        errno = EEXIST;
        return -1;
    }

    /* Check write permission on parent */
    char parent_path[IOX_MAX_PATH];
    strncpy(parent_path, translated_link, IOX_MAX_PATH - 1);
    parent_path[IOX_MAX_PATH - 1] = '\0';

    char *last_slash = strrchr(parent_path, '/');
    if (last_slash && last_slash != parent_path) {
        *last_slash = '\0';
        if (access(parent_path, W_OK | X_OK) != 0) {
            errno = EACCES;
            return -1;
        }
    }

    /* Create the symbolic link */
    if (symlink(target, translated_link) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Create a symbolic link relative to directory
 *
 * @param target Target path
 * @param newdirfd Directory file descriptor
 * @param linkpath Path for the new symbolic link
 * @return 0 on success, -1 on error with errno set
 */
int iox_symlinkat(const char *target, int newdirfd, const char *linkpath) {
    if (!target || !linkpath) {
        errno = EFAULT;
        return -1;
    }

    if (newdirfd == AT_FDCWD) {
        return iox_symlink(target, linkpath);
    }

    /* TODO: Implement dirfd-relative path resolution */
    (void)newdirfd;
    errno = ENOSYS;
    return -1;
}

/**
 * @brief Read the target of a symbolic link
 *
 * Reads the value of a symbolic link and stores it in buf.
 * Does NOT null-terminate the buffer (per POSIX).
 *
 * @param pathname Path to the symbolic link
 * @param buf Buffer to store target
 * @param bufsiz Size of buffer
 * @return Number of bytes placed in buf on success, -1 on error
 */
ssize_t iox_readlink(const char *pathname, char *buf, size_t bufsiz) {
    if (!pathname || !buf) {
        errno = EFAULT;
        return -1;
    }

    /* Validate pathname is not empty */
    if (pathname[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    if (bufsiz == 0) {
        errno = EINVAL;
        return -1;
    }

    /* Translate path through VFS */
    char translated_path[IOX_MAX_PATH];
    if (iox_vfs_translate(pathname, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    /* Verify it's a symbolic link */
    struct stat st;
    if (lstat(translated_path, &st) != 0) {
        return -1;
    }

    if (!S_ISLNK(st.st_mode)) {
        errno = EINVAL;
        return -1;
    }

    /* Read the link target */
    ssize_t result = readlink(translated_path, buf, bufsiz);
    if (result < 0) {
        return -1;
    }

    /* Note: POSIX does NOT require null termination */
    return result;
}

/**
 * @brief Read symbolic link target relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to the symbolic link
 * @param buf Buffer to store target
 * @param bufsiz Size of buffer
 * @return Number of bytes on success, -1 on error
 */
ssize_t iox_readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) {
    if (!pathname || !buf) {
        errno = EFAULT;
        return -1;
    }

    if (dirfd == AT_FDCWD) {
        return iox_readlink(pathname, buf, bufsiz);
    }

    /* TODO: Implement dirfd-relative path resolution */
    (void)dirfd;
    (void)bufsiz;
    errno = ENOSYS;
    return -1;
}

/* ============================================================================
 * ADDITIONAL FILESYSTEM OPERATIONS
 * ============================================================================ */

/**
 * @brief Create a hard link
 *
 * Creates a hard link from oldpath to newpath.
 *
 * @param oldpath Existing file path
 * @param newpath New link path
 * @return 0 on success, -1 on error with errno set
 */
int iox_link(const char *oldpath, const char *newpath) {
    if (!oldpath || !newpath) {
        errno = EFAULT;
        return -1;
    }

    if (oldpath[0] == '\0' || newpath[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    /* Translate paths through VFS */
    char translated_old[IOX_MAX_PATH];
    char translated_new[IOX_MAX_PATH];

    if (iox_vfs_translate(oldpath, translated_old, sizeof(translated_old)) != 0) {
        return -1;
    }

    if (iox_vfs_translate(newpath, translated_new, sizeof(translated_new)) != 0) {
        return -1;
    }

    /* Verify oldpath exists and is not a directory */
    struct stat st;
    if (stat(translated_old, &st) != 0) {
        return -1;
    }

    if (S_ISDIR(st.st_mode)) {
        errno = EPERM;
        return -1;
    }

    /* Check if newpath already exists */
    if (stat(translated_new, &st) == 0) {
        errno = EEXIST;
        return -1;
    }

    /* Create the hard link */
    if (link(translated_old, translated_new) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Create a hard link relative to directories
 *
 * @param olddirfd Old directory file descriptor
 * @param oldpath Existing file path
 * @param newdirfd New directory file descriptor
 * @param newpath New link path
 * @param flags Flags
 * @return 0 on success, -1 on error with errno set
 */
int iox_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags) {
    if (!oldpath || !newpath) {
        errno = EFAULT;
        return -1;
    }

    if (olddirfd == AT_FDCWD && newdirfd == AT_FDCWD) {
        return iox_link(oldpath, newpath);
    }

    /* TODO: Implement dirfd-relative path resolution */
    (void)olddirfd;
    (void)newdirfd;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

/* ============================================================================
 * LEGACY/INTERNAL IMPLEMENTATION FUNCTIONS
 * These provide compatibility with the internal API used by other modules
 * ============================================================================ */

int __iox_chdir_impl(const char *path) {
    return iox_chdir(path);
}

int __iox_fchdir_impl(int fd) {
    return iox_fchdir(fd);
}

char *__iox_getcwd_impl(char *buf, size_t size) {
    return iox_getcwd(buf, size);
}

int __iox_mkdir_impl(const char *pathname, mode_t mode) {
    return iox_mkdir(pathname, mode);
}

int __iox_rmdir_impl(const char *pathname) {
    return iox_rmdir(pathname);
}

int __iox_unlink_impl(const char *pathname) {
    return iox_unlink(pathname);
}

int __iox_link_impl(const char *oldpath, const char *newpath) {
    return iox_link(oldpath, newpath);
}

int __iox_symlink_impl(const char *target, const char *linkpath) {
    return iox_symlink(target, linkpath);
}

ssize_t __iox_readlink_impl(const char *pathname, char *buf, size_t bufsiz) {
    return iox_readlink(pathname, buf, bufsiz);
}

int __iox_chroot_impl(const char *path) {
    return iox_chroot(path);
}

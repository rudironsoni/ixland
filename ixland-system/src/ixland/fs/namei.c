#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../internal/ixland_internal.h"

static int ixland_directory_validate_path(const char *path) {
    if (path == NULL) {
        errno = EFAULT;
        return -1;
    }

    if (path[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    return 0;
}

int __ixland_chdir_impl(const char *path) {
    if (ixland_directory_validate_path(path) != 0) {
        return -1;
    }

    char translated_path[IXLAND_MAX_PATH];
    if (ixland_vfs_translate(path, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    struct stat st;
    if (stat(translated_path, &st) != 0) {
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    if (access(translated_path, X_OK) != 0) {
        errno = EACCES;
        return -1;
    }

    if (chdir(translated_path) != 0) {
        return -1;
    }

    return 0;
}

int __ixland_fchdir_impl(int fd) {
    if (fd < 0 || fd >= IXLAND_MAX_FD || fd <= STDERR_FILENO) {
        errno = EBADF;
        return -1;
    }

    return fchdir(fd);
}

char *__ixland_getcwd_impl(char *buf, size_t size) {
    if (size == 0) {
        errno = EINVAL;
        return NULL;
    }

    if (buf == NULL) {
        errno = EINVAL;
        return NULL;
    }

    char ios_cwd[IXLAND_MAX_PATH];
    if (getcwd(ios_cwd, sizeof(ios_cwd)) == NULL) {
        return NULL;
    }

    char virtual_path[IXLAND_MAX_PATH];
    const char *selected_path = ios_cwd;

    if (ixland_vfs_reverse_translate(ios_cwd, virtual_path, sizeof(virtual_path)) == 0) {
        selected_path = virtual_path;
    }

    const size_t selected_len = strlen(selected_path);
    if (selected_len >= size) {
        errno = ERANGE;
        return NULL;
    }

    memcpy(buf, selected_path, selected_len + 1);
    return buf;
}

int __ixland_mkdir_impl(const char *pathname, mode_t mode) {
    if (ixland_directory_validate_path(pathname) != 0) {
        return -1;
    }

    char translated_path[IXLAND_MAX_PATH];
    if (ixland_vfs_translate(pathname, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    return mkdir(translated_path, mode);
}

int __ixland_rmdir_impl(const char *pathname) {
    if (ixland_directory_validate_path(pathname) != 0) {
        return -1;
    }

    char translated_path[IXLAND_MAX_PATH];
    if (ixland_vfs_translate(pathname, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    struct stat st;
    if (stat(translated_path, &st) != 0) {
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    return rmdir(translated_path);
}

int __ixland_unlink_impl(const char *pathname) {
    if (ixland_directory_validate_path(pathname) != 0) {
        return -1;
    }

    char translated_path[IXLAND_MAX_PATH];
    if (ixland_vfs_translate(pathname, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    struct stat st;
    if (stat(translated_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        errno = EISDIR;
        return -1;
    }

    return unlink(translated_path);
}

int __ixland_symlink_impl(const char *target, const char *linkpath) {
    if (target == NULL || linkpath == NULL) {
        errno = EFAULT;
        return -1;
    }

    if (linkpath[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    char translated_link[IXLAND_MAX_PATH];
    if (ixland_vfs_translate(linkpath, translated_link, sizeof(translated_link)) != 0) {
        return -1;
    }

    struct stat target_stat;
    if (stat(translated_link, &target_stat) == 0) {
        errno = EEXIST;
        return -1;
    }

    return symlink(target, translated_link);
}

ssize_t __ixland_readlink_impl(const char *pathname, char *buf, size_t bufsiz) {
    if (pathname == NULL || buf == NULL) {
        errno = EFAULT;
        return -1;
    }

    if (pathname[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    if (bufsiz == 0) {
        errno = EINVAL;
        return -1;
    }

    char translated_path[IXLAND_MAX_PATH];
    if (ixland_vfs_translate(pathname, translated_path, sizeof(translated_path)) != 0) {
        return -1;
    }

    struct stat path_stat;
    if (lstat(translated_path, &path_stat) != 0) {
        return -1;
    }

    if (!S_ISLNK(path_stat.st_mode)) {
        errno = EINVAL;
        return -1;
    }

    return readlink(translated_path, buf, bufsiz);
}

int __ixland_chroot_impl(const char *path) {
    (void)path;
    errno = EPERM;
    return -1;
}

int ixland_chdir(const char *path) {
    return __ixland_chdir_impl(path);
}

int ixland_fchdir(int fd) {
    return __ixland_fchdir_impl(fd);
}

char *ixland_getcwd(char *buf, size_t size) {
    return __ixland_getcwd_impl(buf, size);
}

int ixland_mkdir(const char *pathname, mode_t mode) {
    return __ixland_mkdir_impl(pathname, mode);
}

int ixland_rmdir(const char *pathname) {
    return __ixland_rmdir_impl(pathname);
}

int ixland_unlink(const char *pathname) {
    return __ixland_unlink_impl(pathname);
}

int ixland_symlink(const char *target, const char *linkpath) {
    return __ixland_symlink_impl(target, linkpath);
}

ssize_t ixland_readlink(const char *pathname, char *buf, size_t bufsiz) {
    return __ixland_readlink_impl(pathname, buf, bufsiz);
}

int ixland_chroot(const char *path) {
    return __ixland_chroot_impl(path);
}

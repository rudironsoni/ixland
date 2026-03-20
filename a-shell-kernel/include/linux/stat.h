/*
 * linux/stat.h - File status
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: stat, chmod, mkdir, etc.
 */

#ifndef _LINUX_STAT_H
#define _LINUX_STAT_H

#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * File Status
 * ============================================================================ */

extern int a_shell_stat(const char *pathname, struct stat *statbuf);
extern int a_shell_fstat(int fd, struct stat *statbuf);
extern int a_shell_lstat(const char *pathname, struct stat *statbuf);
extern int a_shell_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);

/* ============================================================================
 * Permissions
 * ============================================================================ */

extern int a_shell_chmod(const char *pathname, mode_t mode);
extern int a_shell_fchmod(int fd, mode_t mode);
extern int a_shell_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);

/* ============================================================================
 * Ownership
 * ============================================================================ */

extern int a_shell_chown(const char *pathname, uid_t owner, gid_t group);
extern int a_shell_fchown(int fd, uid_t owner, gid_t group);
extern int a_shell_lchown(const char *pathname, uid_t owner, gid_t group);
extern int a_shell_fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);

/* ============================================================================
 * Umask
 * ============================================================================ */

extern mode_t a_shell_umask(mode_t mask);

/* ============================================================================
 * Directory Creation
 * ============================================================================ */

extern int a_shell_mkdir(const char *pathname, mode_t mode);
extern int a_shell_mkdirat(int dirfd, const char *pathname, mode_t mode);

/* ============================================================================
 * Special Files
 * ============================================================================ */

extern int a_shell_mknod(const char *pathname, mode_t mode, dev_t dev);
extern int a_shell_mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev);
extern int a_shell_mkfifo(const char *pathname, mode_t mode);
extern int a_shell_mkfifoat(int dirfd, const char *pathname, mode_t mode);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define stat(path, buf)         a_shell_stat(path, buf)
#define fstat(fd, buf)          a_shell_fstat(fd, buf)
#define lstat(path, buf)        a_shell_lstat(path, buf)
#define fstatat(dirfd, path, buf, flags) \
                                a_shell_fstatat(dirfd, path, buf, flags)

#define chmod(path, mode)       a_shell_chmod(path, mode)
#define fchmod(fd, mode)        a_shell_fchmod(fd, mode)
#define fchmodat(dirfd, path, mode, flags) \
                                a_shell_fchmodat(dirfd, path, mode, flags)

#define chown(path, owner, group) \
                                a_shell_chown(path, owner, group)
#define fchown(fd, owner, group) \
                                a_shell_fchown(fd, owner, group)
#define lchown(path, owner, group) \
                                a_shell_lchown(path, owner, group)
#define fchownat(dirfd, path, owner, group, flags) \
                                a_shell_fchownat(dirfd, path, owner, group, flags)

#define umask(mask)             a_shell_umask(mask)

#define mkdir(path, mode)       a_shell_mkdir(path, mode)
#define mkdirat(dirfd, path, mode) \
                                a_shell_mkdirat(dirfd, path, mode)

#define mknod(path, mode, dev)  a_shell_mknod(path, mode, dev)
#define mknodat(dirfd, path, mode, dev) \
                                a_shell_mknodat(dirfd, path, mode, dev)
#define mkfifo(path, mode)      a_shell_mkfifo(path, mode)
#define mkfifoat(dirfd, path, mode) \
                                a_shell_mkfifoat(dirfd, path, mode)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_STAT_H */

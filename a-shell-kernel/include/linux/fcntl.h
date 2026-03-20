/*
 * linux/fcntl.h - File control options
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: open, fcntl
 */

#ifndef _LINUX_FCNTL_H
#define _LINUX_FCNTL_H

#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * File Operations
 * ============================================================================ */

extern int a_shell_open(const char *pathname, int flags, ...);
extern int a_shell_openat(int dirfd, const char *pathname, int flags, ...);
extern int a_shell_creat(const char *pathname, mode_t mode);

/* ============================================================================
 * File Control
 * ============================================================================ */

extern int a_shell_fcntl(int fd, int cmd, ...);

/* ============================================================================
 * Advisory Record Locking
 * ============================================================================ */

extern int a_shell_flock(int fd, int operation);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define open(path, flags, ...)  a_shell_open(path, flags, ##__VA_ARGS__)
#define openat(dirfd, path, flags, ...) \
                                a_shell_openat(dirfd, path, flags, ##__VA_ARGS__)
#define creat(path, mode)       a_shell_creat(path, mode)

#define fcntl(fd, cmd, ...)     a_shell_fcntl(fd, cmd, ##__VA_ARGS__)

#define flock(fd, op)           a_shell_flock(fd, op)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_FCNTL_H */

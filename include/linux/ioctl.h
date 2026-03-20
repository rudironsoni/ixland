/*
 * linux/ioctl.h - Device control
 *
 * ioctl() system call
 */

#ifndef _LINUX_IOCTL_H
#define _LINUX_IOCTL_H

#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ioctl Operation
 * ============================================================================ */

int a_shell_ioctl(int fd, unsigned long request, ...);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#ifndef A_SHELL_NO_COMPAT_MACROS

#define ioctl(fd, request, ...) \
                                a_shell_ioctl(fd, request, ##__VA_ARGS__)

#endif /* A_SHELL_NO_COMPAT_MACROS */

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_IOCTL_H */

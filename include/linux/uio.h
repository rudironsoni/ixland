/*
 * linux/uio.h - Vector I/O
 *
 * readv(), writev() and related calls
 */

#ifndef _LINUX_UIO_H
#define _LINUX_UIO_H

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Vector I/O Operations
 * ============================================================================ */

ssize_t a_shell_readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t a_shell_writev(int fd, const struct iovec *iov, int iovcnt);

/* ============================================================================
 * Positioned Vector I/O
 * ============================================================================ */

ssize_t a_shell_preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t a_shell_pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t a_shell_preadv2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
ssize_t a_shell_pwritev2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);

/* ============================================================================
 * Socket Names
 * ============================================================================ */

int a_shell_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int a_shell_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#ifndef A_SHELL_NO_COMPAT_MACROS

#define readv(fd, iov, iovcnt)  a_shell_readv(fd, iov, iovcnt)
#define writev(fd, iov, iovcnt) a_shell_writev(fd, iov, iovcnt)

#define preadv(fd, iov, iovcnt, offset) \
                                a_shell_preadv(fd, iov, iovcnt, offset)
#define pwritev(fd, iov, iovcnt, offset) \
                                a_shell_pwritev(fd, iov, iovcnt, offset)
#define preadv2(fd, iov, iovcnt, offset, flags) \
                                a_shell_preadv2(fd, iov, iovcnt, offset, flags)
#define pwritev2(fd, iov, iovcnt, offset, flags) \
                                a_shell_pwritev2(fd, iov, iovcnt, offset, flags)

#define getsockname(sockfd, addr, addrlen) \
                                a_shell_getsockname(sockfd, addr, addrlen)
#define getpeername(sockfd, addr, addrlen) \
                                a_shell_getpeername(sockfd, addr, addrlen)

#endif /* A_SHELL_NO_COMPAT_MACROS */

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_UIO_H */

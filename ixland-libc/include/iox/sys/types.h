#ifndef IOX_SYS_TYPES_H
#define IOX_SYS_TYPES_H

/* iXland libc - Linux-compatible types header
 *
 * File descriptor set and other Linux-compatible types.
 * Extracted from ixland-system as part of libc boundary formation.
 */

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* File descriptor set for select */
#ifndef _IOX_FD_SET_SIZE
#define _IOX_FD_SET_SIZE    1024
#endif

typedef struct {
    unsigned long fds_bits[_IOX_FD_SET_SIZE / (8 * sizeof(long))];
} iox_fd_set_t;

#define IOX_FD_ZERO(set)    memset(set, 0, sizeof(iox_fd_set_t))
#define IOX_FD_SET(fd, set)     (((set)->fds_bits[(fd) / (8 * sizeof(long))]) |= (1L << ((fd) % (8 * sizeof(long)))))
#define IOX_FD_CLR(fd, set)     (((set)->fds_bits[(fd) / (8 * sizeof(long))]) &= ~(1L << ((fd) % (8 * sizeof(long)))))
#define IOX_FD_ISSET(fd, set)   (((set)->fds_bits[(fd) / (8 * sizeof(long))]) & (1L << ((fd) % (8 * sizeof(long)))))

#endif /* IOX_SYS_TYPES_H */

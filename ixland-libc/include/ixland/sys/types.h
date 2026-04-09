#ifndef IXLAND_SYS_TYPES_H
#define IXLAND_SYS_TYPES_H

/* iXland libc - Linux-compatible types header
 *
 * File descriptor set and other Linux-compatible types.
 * Extracted from ixland-system as part of libc boundary formation.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

/* File descriptor set for select */
#ifndef _IXLAND_FD_SET_SIZE
#define _IXLAND_FD_SET_SIZE 1024
#endif

typedef struct {
    unsigned long fds_bits[_IXLAND_FD_SET_SIZE / (8 * sizeof(long))];
} ixland_fd_set_t;

#define IXLAND_FD_ZERO(set) memset(set, 0, sizeof(ixland_fd_set_t))
#define IXLAND_FD_SET(fd, set) \
    (((set)->fds_bits[(fd) / (8 * sizeof(long))]) |= (1L << ((fd) % (8 * sizeof(long)))))
#define IXLAND_FD_CLR(fd, set) \
    (((set)->fds_bits[(fd) / (8 * sizeof(long))]) &= ~(1L << ((fd) % (8 * sizeof(long)))))
#define IXLAND_FD_ISSET(fd, set) \
    (((set)->fds_bits[(fd) / (8 * sizeof(long))]) & (1L << ((fd) % (8 * sizeof(long)))))

#endif /* IXLAND_SYS_TYPES_H */

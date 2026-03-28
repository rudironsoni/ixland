#ifndef IOX_H
#define IOX_H

/* iXland libc - Master Umbrella Header
 *
 * Include this file to get all public iox APIs.
 * Extracted from ixland-system as part of libc boundary formation.
 */

#define IOX_VERSION_MAJOR   1
#define IOX_VERSION_MINOR   0
#define IOX_VERSION_PATCH   0

#include "iox_types.h"
#include "iox_syscalls.h"

/* Linux-compatible system headers */
#include "sys/types.h"

/* User and Group Database Headers */
#include "../pwd.h"
#include "../grp.h"

/* Advanced Linux-compatible headers
 * These can be included individually as needed:
 *   #include "linux/time.h"
 *   #include "linux/resource.h"
 *   #include "linux/mman.h"
 *   #include "linux/poll.h"
 *   #include "linux/epoll.h"
 */

#endif /* IOX_H */

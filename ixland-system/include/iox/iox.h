#ifndef IOX_H
#define IOX_H

/* iOS Subsystem for Linux - Master Umbrella Header
 *
 * This is the main header file for libiox.
 * Include this file to get all public APIs.
 */

#define IOX_VERSION_MAJOR   1
#define IOX_VERSION_MINOR   0
#define IOX_VERSION_PATCH   0

#include "iox_types.h"
#include "iox_syscalls.h"

/* Linux-compatible system headers */
#include "sys/types.h"
#include "sys/unistd.h"
#include "sys/wait.h"
#include "sys/stat.h"
#include "sys/time.h"
#include "sys/socket.h"
#include "sys/signal.h"
#include "sys/mman.h"
#include "sys/fcntl.h"
#include "sys/errno.h"

#endif /* IOX_H */

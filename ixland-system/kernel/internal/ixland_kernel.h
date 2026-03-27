/*
 * ixland_kernel.h - Main kernel umbrella header
 *
 * Includes all ixland kernel syscall headers
 */

#ifndef IXLAND_KERNEL_H
#define IXLAND_KERNEL_H

/* ============================================================================
 * System headers (for types, constants, structs)
 * Include these first to avoid conflicts
 * ============================================================================ */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <poll.h>

/* ============================================================================
 * Linux-mirror headers (for our function prototypes and macros)
 * ============================================================================ */

#include "linux/types.h"
#include "linux/unistd.h"
#include "linux/sys/wait.h"
#include "linux/sys/time.h"
#include "linux/signal.h"
#include "linux/time.h"
#include "linux/stat.h"
#include "linux/fcntl.h"
#include "linux/poll.h"
#include "linux/epoll.h"
#include "linux/resource.h"
#include "linux/mman.h"
#include "linux/socket.h"
#include "pwd.h"
#include "grp.h"

/* ============================================================================
 * VFS Layer
 * ============================================================================ */

#include "vfs.h"

/* ============================================================================
 * Version Information
 * ============================================================================ */

#define IXLAND_KERNEL_VERSION_MAJOR 1
#define IXLAND_KERNEL_VERSION_MINOR 0
#define IXLAND_KERNEL_VERSION_PATCH 0

#define IXLAND_KERNEL_VERSION "1.0.0"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Kernel Initialization
 * ============================================================================ */

extern int ixland_kernel_init(const char *prefix);
extern const char *ixland_kernel_get_prefix(void);
extern int ixland_kernel_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_KERNEL_H */

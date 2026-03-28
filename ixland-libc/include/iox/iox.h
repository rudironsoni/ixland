#ifndef IOX_H
#define IOX_H

/* iXland libc - Master Umbrella Header
 *
 * Include this file to get all public iox APIs.
 * Extracted from ixland-system as part of libc boundary formation.
 *
 * Usage:
 *   #include <iox/iox.h>  // Gets all public APIs
 *
 * Or include individual headers:
 *   #include <iox/iox_types.h>     // Type definitions
 *   #include <iox/iox_syscalls.h>  // Syscall declarations
 *   #include <linux/unistd.h>      // Linux-compatible unistd
 *   #include <pwd.h>               // User database
 *   #include <grp.h>               // Group database
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * VERSION
 * ============================================================================ */

#define IOX_VERSION_MAJOR 1
#define IOX_VERSION_MINOR 0
#define IOX_VERSION_PATCH 0

#define IOX_VERSION_STRING "1.0.0"

/* ============================================================================
 * CORE PUBLIC API
 * ============================================================================ */

/* Type definitions and constants */
#include "iox_types.h"

/* Syscall declarations */
#include "iox_syscalls.h"

/* System types (fd_set, etc.) */
#include "sys/types.h"

/* ============================================================================
 * STANDARD DATABASE HEADERS
 * ============================================================================
 *
 * User and group database headers provide POSIX-compliant
 * passwd and group entry structures and lookup functions.
 */

/* User database (struct passwd, getpwnam, etc.) */
#include "../pwd.h"

/* Group database (struct group, getgrnam, etc.) */
#include "../grp.h"

/* ============================================================================
 * LINUX-COMPATIBLE HEADERS (OPTIONAL)
 * ============================================================================
 *
 * These headers provide Linux-compatible syscall interfaces with iox_ prefix.
 * They are organized by functional area for selective inclusion.
 *
 * Note: These headers define iox-specific types (iox_sigset_t, iox_sigaction,
 * etc.) that may conflict with system types. Include only if you need:
 *
 *   #include <linux/types.h>    // Linux kernel types (__s8, __u32, etc.)
 *   #include <linux/unistd.h>   // Process and I/O syscalls
 *   #include <linux/signal.h>   // Signal handling (iox_sigaction, etc.)
 *   #include <linux/wait.h>     // Process waiting
 *   #include <linux/stat.h>     // File status (iox_stat, etc.)
 *   #include <linux/fcntl.h>    // File control flags
 *   #include <linux/time.h>     // Time functions
 *   #include <linux/poll.h>     // Polling
 *   #include <linux/epoll.h>    // Epoll interface
 *   #include <linux/resource.h> // Resource limits
 *   #include <linux/mman.h>     // Memory management
 */

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

/**
 * @brief Initialize the iox library
 *
 * Must be called before using any iox functions.
 *
 * @param config Configuration structure (can be NULL for defaults)
 * @return int 0 on success, -1 on error
 */
int iox_init(const iox_config_t *config);

/**
 * @brief Cleanup the iox library
 *
 * Frees all resources. No iox functions should be called after cleanup.
 */
void iox_cleanup(void);

/**
 * @brief Get iox library version string
 *
 * @return const char* Version string (e.g., "1.0.0")
 */
const char *iox_version(void);

/**
 * @brief Check if iox library is initialized
 *
 * @return int 1 if initialized, 0 otherwise
 */
int iox_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* IOX_H */

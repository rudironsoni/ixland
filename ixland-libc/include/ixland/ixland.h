#ifndef IXLAND_H
#define IXLAND_H

/* iXland libc - Master Umbrella Header
 *
 * Include this file to get all public ixland APIs.
 * Extracted from ixland-system as part of libc boundary formation.
 *
 * Usage:
 *   #include <ixland/ixland.h>  // Gets all public APIs
 *
 * Or include individual headers:
 *   #include <ixland/ixland_types.h>     // Type definitions
 *   #include <ixland/ixland_syscalls.h>  // Syscall declarations
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

#define IXLAND_VERSION_MAJOR 1
#define IXLAND_VERSION_MINOR 0
#define IXLAND_VERSION_PATCH 0

#define IXLAND_VERSION_STRING "1.0.0"

/* ============================================================================
 * CORE PUBLIC API
 * ============================================================================ */

/* Type definitions and constants */
#include "ixland_types.h"

/* Syscall declarations */
#include "ixland_syscalls.h"

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
 * These headers provide Linux-compatible syscall interfaces with ixland_ prefix.
 * They are organized by functional area for selective inclusion.
 *
 * Note: These headers define ixland-specific types (ixland_sigset_t, ixland_sigaction,
 * etc.) that may conflict with system types. Include only if you need:
 *
 *   #include <linux/types.h>    // Linux kernel types (__s8, __u32, etc.)
 *   #include <linux/unistd.h>   // Process and I/O syscalls
 *   #include <linux/signal.h>   // Signal handling (ixland_sigaction, etc.)
 *   #include <linux/wait.h>     // Process waiting
 *   #include <linux/stat.h>     // File status (ixland_stat, etc.)
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
 * @brief Initialize the ixland library
 *
 * Must be called before using any ixland functions.
 *
 * @param config Configuration structure (can be NULL for defaults)
 * @return int 0 on success, -1 on error
 */
int ixland_init(const ixland_config_t *config);

/**
 * @brief Cleanup the ixland library
 *
 * Frees all resources. No ixland functions should be called after cleanup.
 */
void ixland_cleanup(void);

/**
 * @brief Get ixland library version string
 *
 * @return const char* Version string (e.g., "1.0.0")
 */
const char *ixland_version(void);

/**
 * @brief Check if ixland library is initialized
 *
 * @return int 1 if initialized, 0 otherwise
 */
int ixland_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_H */

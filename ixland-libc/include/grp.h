/*
 * grp.h - Group database operations
 *
 * POSIX group information interface
 */

#ifndef _IXLAND_GRP_H
#define _IXLAND_GRP_H

#include <sys/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Buffer Size Limits
 * ============================================================================ */

/** Maximum buffer size for getgrgid_r and getgrnam_r */
#define IOX_GETGR_R_SIZE_MAX    1024

/** Maximum number of groups per user (NGROUPS_MAX) */
#define IOX_NGROUPS_MAX         16

/** Maximum length of group name */
#define IOX_MAX_GROUP_NAME      32

/** Maximum number of members in a group */
#define IOX_MAX_GROUP_MEMBERS   64

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Group database entry structure
 *
 * POSIX-defined structure containing group account information.
 * Fields match POSIX.1-2008 specification.
 */
struct group {
    char *gr_name;      /**< Group name */
    char *gr_passwd;    /**< Group password (or "*" for shadow) */
    gid_t gr_gid;       /**< Group ID */
    char **gr_mem;      /**< NULL-terminated array of member names */
};

/* ============================================================================
 * Get Group Information
 * ============================================================================ */

/**
 * @brief Get group file entry by group name
 *
 * Searches the group database for an entry with the specified name.
 * Returns a pointer to a static buffer that may be overwritten by
 * subsequent calls.
 *
 * @param name Group name to search for
 * @return Pointer to group structure, or NULL if not found
 */
extern struct group *iox_getgrnam(const char *name);

/**
 * @brief Get group file entry by group ID
 *
 * Searches the group database for an entry with the specified GID.
 * Returns a pointer to a static buffer that may be overwritten by
 * subsequent calls.
 *
 * @param gid Group ID to search for
 * @return Pointer to group structure, or NULL if not found
 */
extern struct group *iox_getgrgid(gid_t gid);

/* ============================================================================
 * Reentrant Versions (thread-safe)
 * ============================================================================ */

/**
 * @brief Get group file entry by name (reentrant)
 *
 * Thread-safe version of getgrnam(). Stores result in user-provided buffers.
 *
 * @param name Group name to search for
 * @param grp Pointer to group structure to fill
 * @param buf Buffer for string storage
 * @param buflen Size of buffer
 * @param result Pointer to store result pointer (or NULL on error/not found)
 * @return 0 on success, error code on failure
 */
extern int iox_getgrnam_r(const char *name, struct group *grp,
                          char *buf, size_t buflen, struct group **result);

/**
 * @brief Get group file entry by GID (reentrant)
 *
 * Thread-safe version of getgrgid(). Stores result in user-provided buffers.
 *
 * @param gid Group ID to search for
 * @param grp Pointer to group structure to fill
 * @param buf Buffer for string storage
 * @param buflen Size of buffer
 * @param result Pointer to store result pointer (or NULL on error/not found)
 * @return 0 on success, error code on failure
 */
extern int iox_getgrgid_r(gid_t gid, struct group *grp,
                          char *buf, size_t buflen, struct group **result);

/* ============================================================================
 * Database Iteration
 * ============================================================================ */

/**
 * @brief Reset group database iterator
 *
 * Rewinds the group database iterator to the beginning.
 */
extern void iox_setgrent(void);

/**
 * @brief Get next group file entry
 *
 * Returns the next entry from the group database.
 * Returns a pointer to a static buffer that may be overwritten.
 *
 * @return Pointer to group structure, or NULL on EOF/error
 */
extern struct group *iox_getgrent(void);

/**
 * @brief Close group database iterator
 *
 * Closes any open resources associated with group database iteration.
 */
extern void iox_endgrent(void);

/* ============================================================================
 * Group Membership
 * ============================================================================ */

/**
 * @brief Get supplementary group IDs
 *
 * Retrieves the list of supplementary group IDs for the calling process.
 *
 * @param size Size of the list array
 * @param list Array to store group IDs
 * @return Number of groups on success, -1 on error
 */
extern int iox_getgroups(int size, gid_t list[]);

/**
 * @brief Set supplementary group IDs
 *
 * Sets the list of supplementary group IDs for the calling process.
 * Requires appropriate privileges.
 *
 * @param size Number of groups in list
 * @param list Array of group IDs
 * @return 0 on success, -1 on error
 */
extern int iox_setgroups(size_t size, const gid_t *list);

/**
 * @brief Initialize supplementary group access list
 *
 * Initializes the group access list by reading the group database
 * and using all groups of which the specified user is a member.
 * The specified group is also added to the list.
 *
 * @param user User name to look up
 * @param group Group ID to add
 * @return 0 on success, -1 on error
 */
extern int iox_initgroups(const char *user, gid_t group);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define getgrnam(name)          iox_getgrnam(name)
#define getgrgid(gid)           iox_getgrgid(gid)

#define getgrnam_r(name, grp, buf, buflen, result) \
                                iox_getgrnam_r(name, grp, buf, buflen, result)
#define getgrgid_r(gid, grp, buf, buflen, result) \
                                iox_getgrgid_r(gid, grp, buf, buflen, result)

#define setgrent()              iox_setgrent()
#define getgrent()              iox_getgrent()
#define endgrent()              iox_endgrent()

#define getgroups(size, list)   iox_getgroups(size, list)
#define setgroups(size, list)   iox_setgroups(size, list)
#define initgroups(user, group) iox_initgroups(user, group)

#ifdef __cplusplus
}
#endif

#endif /* _IXLAND_GRP_H */

/*
 * pwd.h - User password database operations
 *
 * POSIX user information interface
 */

#ifndef _IXLAND_PWD_H
#define _IXLAND_PWD_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Buffer Size Limits
 * ============================================================================ */

/** Maximum buffer size for getpwuid_r and getpwnam_r */
#define IXLAND_GETPW_R_SIZE_MAX 1024

/** Maximum length of user name */
#define IXLAND_MAX_USER_NAME 32

/** Maximum length of password field */
#define IXLAND_MAX_PASSWD_FIELD 256

/** Maximum length of home directory path */
#define IXLAND_MAX_HOME_DIR 256

/** Maximum length of shell path */
#define IXLAND_MAX_SHELL_PATH 256

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief User database entry structure
 *
 * POSIX-defined structure containing user account information.
 * Fields match POSIX.1-2008 specification.
 */
struct passwd {
    char *pw_name;   /**< User login name */
    char *pw_passwd; /**< Encrypted password (or "*" for shadow) */
    uid_t pw_uid;    /**< User ID */
    gid_t pw_gid;    /**< Group ID */
    char *pw_gecos;  /**< Real name or comment field */
    char *pw_dir;    /**< Home directory */
    char *pw_shell;  /**< Default shell program */
};

/* ============================================================================
 * Get User Information
 * ============================================================================ */

/**
 * @brief Get password file entry by user name
 *
 * Searches the user database for an entry with the specified name.
 * Returns a pointer to a static buffer that may be overwritten by
 * subsequent calls.
 *
 * @param name User name to search for
 * @return Pointer to passwd structure, or NULL if not found
 */
extern struct passwd *ixland_getpwnam(const char *name);

/**
 * @brief Get password file entry by user ID
 *
 * Searches the user database for an entry with the specified UID.
 * Returns a pointer to a static buffer that may be overwritten by
 * subsequent calls.
 *
 * @param uid User ID to search for
 * @return Pointer to passwd structure, or NULL if not found
 */
extern struct passwd *ixland_getpwuid(uid_t uid);

/* ============================================================================
 * Reentrant Versions (thread-safe)
 * ============================================================================ */

/**
 * @brief Get password file entry by name (reentrant)
 *
 * Thread-safe version of getpwnam(). Stores result in user-provided buffers.
 *
 * @param name User name to search for
 * @param pwd Pointer to passwd structure to fill
 * @param buf Buffer for string storage
 * @param buflen Size of buffer
 * @param result Pointer to store result pointer (or NULL on error/not found)
 * @return 0 on success, error code on failure
 */
extern int ixland_getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen,
                             struct passwd **result);

/**
 * @brief Get password file entry by UID (reentrant)
 *
 * Thread-safe version of getpwuid(). Stores result in user-provided buffers.
 *
 * @param uid User ID to search for
 * @param pwd Pointer to passwd structure to fill
 * @param buf Buffer for string storage
 * @param buflen Size of buffer
 * @param result Pointer to store result pointer (or NULL on error/not found)
 * @return 0 on success, error code on failure
 */
extern int ixland_getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen,
                             struct passwd **result);

/* ============================================================================
 * Database Iteration
 * ============================================================================ */

/**
 * @brief Reset password database iterator
 *
 * Rewinds the password database iterator to the beginning.
 */
extern void ixland_setpwent(void);

/**
 * @brief Get next password file entry
 *
 * Returns the next entry from the password database.
 * Returns a pointer to a static buffer that may be overwritten.
 *
 * @return Pointer to passwd structure, or NULL on EOF/error
 */
extern struct passwd *ixland_getpwent(void);

/**
 * @brief Close password database iterator
 *
 * Closes any open resources associated with password database iteration.
 */
extern void ixland_endpwent(void);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define getpwnam(name) ixland_getpwnam(name)
#define getpwuid(uid) ixland_getpwuid(uid)

#define getpwnam_r(name, pwd, buf, buflen, result) ixland_getpwnam_r(name, pwd, buf, buflen, result)
#define getpwuid_r(uid, pwd, buf, buflen, result) ixland_getpwuid_r(uid, pwd, buf, buflen, result)

#define setpwent() ixland_setpwent()
#define getpwent() ixland_getpwent()
#define endpwent() ixland_endpwent()
#ifdef __cplusplus
}
#endif

#endif /* _IXLAND_PWD_H */

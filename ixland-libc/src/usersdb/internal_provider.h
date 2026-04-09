/*
 * internal_provider.h - Internal provider constants for usersdb
 *
 * This header defines the default user and group constants used by
 * the iXland password and group database implementations.
 *
 * These constants provide the stub/fallback values when no actual
 * user database is available at runtime.
 */

#ifndef _IXLAND_USERSDB_INTERNAL_H_
#define _IXLAND_USERSDB_INTERNAL_H_

/* ============================================================================
 * Default User Constants
 * ============================================================================ */

/* Default user name for mobile environment */
#define IXLAND_DEFAULT_USER_NAME "mobile"

/* Default user UID (standard iOS mobile UID) */
#define IXLAND_DEFAULT_USER_UID 501

/* Default user GID (standard iOS mobile GID) */
#define IXLAND_DEFAULT_USER_GID 501

/* Default user home directory */
#define IXLAND_DEFAULT_USER_DIR "/var/mobile"

/* Default user shell */
#define IXLAND_DEFAULT_USER_SHELL "/bin/sh"

/* ============================================================================
 * Default Group Constants
 * ============================================================================ */

/* Default group name for mobile environment */
#define IXLAND_DEFAULT_GROUP_NAME "mobile"

/* Default group GID (standard iOS mobile GID) */
#define IXLAND_DEFAULT_GROUP_GID 501

#endif /* _IXLAND_USERSDB_INTERNAL_H_ */

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
 * Data Structures
 * ============================================================================ */

struct group {
    char *gr_name;
    char *gr_passwd;
    gid_t gr_gid;
    char **gr_mem;
};

/* ============================================================================
 * Get Group Information
 * ============================================================================ */

extern struct group *ixland_getgrnam(const char *name);
extern struct group *ixland_getgrgid(gid_t gid);

/* ============================================================================
 * Reentrant Versions (thread-safe)
 * ============================================================================ */

extern int ixland_getgrnam_r(const char *name, struct group *grp,
                              char *buf, size_t buflen, struct group **result);
extern int ixland_getgrgid_r(gid_t gid, struct group *grp,
                              char *buf, size_t buflen, struct group **result);

/* ============================================================================
 * Database Iteration
 * ============================================================================ */

extern void ixland_setgrent(void);
extern struct group *ixland_getgrent(void);
extern void ixland_endgrent(void);

/* ============================================================================
 * Group Membership
 * ============================================================================ */

extern int ixland_getgroups(int size, gid_t list[]);
extern int ixland_setgroups(size_t size, const gid_t *list[]);
extern int ixland_initgroups(const char *user, gid_t group);

#ifdef __cplusplus
}
#endif

#endif /* _IXLAND_GRP_H */

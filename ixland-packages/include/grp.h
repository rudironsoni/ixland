/*
 * grp.h - Group database operations
 *
 * POSIX group information interface
 */

#ifndef _A_SHELL_GRP_H
#define _A_SHELL_GRP_H

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

extern struct group *a_shell_getgrnam(const char *name);
extern struct group *a_shell_getgrgid(gid_t gid);

/* ============================================================================
 * Reentrant Versions (thread-safe)
 * ============================================================================ */

extern int a_shell_getgrnam_r(const char *name, struct group *grp,
                              char *buf, size_t buflen, struct group **result);
extern int a_shell_getgrgid_r(gid_t gid, struct group *grp,
                              char *buf, size_t buflen, struct group **result);

/* ============================================================================
 * Database Iteration
 * ============================================================================ */

extern void a_shell_setgrent(void);
extern struct group *a_shell_getgrent(void);
extern void a_shell_endgrent(void);

/* ============================================================================
 * Group Membership
 * ============================================================================ */

extern int a_shell_getgroups(int size, gid_t list[]);
extern int a_shell_setgroups(size_t size, const gid_t *list[]);
extern int a_shell_initgroups(const char *user, gid_t group);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define getgrnam(name)          a_shell_getgrnam(name)
#define getgrgid(gid)           a_shell_getgrgid(gid)

#define getgrnam_r(name, grp, buf, buflen, result) \
                                a_shell_getgrnam_r(name, grp, buf, buflen, result)
#define getgrgid_r(gid, grp, buf, buflen, result) \
                                a_shell_getgrgid_r(gid, grp, buf, buflen, result)

#define setgrent()              a_shell_setgrent()
#define getgrent()              a_shell_getgrent()
#define endgrent()              a_shell_endgrent()

#define getgroups(size, list)   a_shell_getgroups(size, list)
#define setgroups(size, list)   a_shell_setgroups(size, list)
#define initgroups(user, group) a_shell_initgroups(user, group)

#ifdef __cplusplus
}
#endif

#endif /* _A_SHELL_GRP_H */

/*
 * pwd.h - User password database operations
 *
 * POSIX user information interface
 */

#ifndef _IXLAND_PWD_H
#define _IXLAND_PWD_H

#include <sys/types.h>
#include <stddef.h>

/* Include system pwd.h first to get struct passwd */
#include <pwd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Get User Information
 * ============================================================================ */

extern struct passwd *ixland_getpwnam(const char *name);
extern struct passwd *ixland_getpwuid(uid_t uid);

/* ============================================================================
 * Reentrant Versions (thread-safe)
 * ============================================================================ */

extern int ixland_getpwnam_r(const char *name, struct passwd *pwd,
                             char *buf, size_t buflen, struct passwd **result);
extern int ixland_getpwuid_r(uid_t uid, struct passwd *pwd,
                             char *buf, size_t buflen, struct passwd **result);

/* ============================================================================
 * Database Iteration
 * ============================================================================ */

extern void ixland_setpwent(void);
extern struct passwd *ixland_getpwent(void);
extern void ixland_endpwent(void);

/* ============================================================================
 * Standard Compatibility Macros
 * ============================================================================ */

#define getpwnam(name)          ixland_getpwnam(name)
#define getpwuid(uid)           ixland_getpwuid(uid)

#define getpwnam_r(name, pwd, buf, buflen, result) \
                                ixland_getpwnam_r(name, pwd, buf, buflen, result)
#define getpwuid_r(uid, pwd, buf, buflen, result) \
                                ixland_getpwuid_r(uid, pwd, buf, buflen, result)

#define setpwent()              ixland_setpwent()
#define getpwent()              ixland_getpwent()
#define endpwent()              ixland_endpwent()

#ifdef __cplusplus
}
#endif

#endif /* _IXLAND_PWD_H */

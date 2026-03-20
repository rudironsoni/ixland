/*
 * pwd.h - User password database operations
 *
 * POSIX user information interface
 */

#ifndef _A_SHELL_PWD_H
#define _A_SHELL_PWD_H

#include <sys/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Data Structures
 * ============================================================================ */

struct passwd {
    char *pw_name;
    char *pw_passwd;
    uid_t pw_uid;
    gid_t pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

/* ============================================================================
 * Get User Information
 * ============================================================================ */

extern struct passwd *a_shell_getpwnam(const char *name);
extern struct passwd *a_shell_getpwuid(uid_t uid);

/* ============================================================================
 * Reentrant Versions (thread-safe)
 * ============================================================================ */

extern int a_shell_getpwnam_r(const char *name, struct passwd *pwd,
                              char *buf, size_t buflen, struct passwd **result);
extern int a_shell_getpwuid_r(uid_t uid, struct passwd *pwd,
                              char *buf, size_t buflen, struct passwd **result);

/* ============================================================================
 * Database Iteration
 * ============================================================================ */

extern void a_shell_setpwent(void);
extern struct passwd *a_shell_getpwent(void);
extern void a_shell_endpwent(void);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define getpwnam(name)          a_shell_getpwnam(name)
#define getpwuid(uid)           a_shell_getpwuid(uid)

#define getpwnam_r(name, pwd, buf, buflen, result) \
                                a_shell_getpwnam_r(name, pwd, buf, buflen, result)
#define getpwuid_r(uid, pwd, buf, buflen, result) \
                                a_shell_getpwuid_r(uid, pwd, buf, buflen, result)

#define setpwent()              a_shell_setpwent()
#define getpwent()              a_shell_getpwent()
#define endpwent()              a_shell_endpwent()

#ifdef __cplusplus
}
#endif

#endif /* _A_SHELL_PWD_H */

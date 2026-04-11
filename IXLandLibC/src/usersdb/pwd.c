/*
 * pwd.c - Password database operations
 *
 * Stub implementation for iXland password database API.
 * These implementations return ENOSYS (function not implemented)
 * as the actual password database functionality requires runtime
 * integration with the kernel/VFS layer.
 *
 * This is a self-contained implementation suitable for the
 * ixland-libc-usersdb target.
 */

#include <errno.h>
#include <pwd.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include "internal_provider.h"

/* ============================================================================
 * Static Data (Minimal stub data for mobile environment)
 * ============================================================================ */

/* Static buffer for non-reentrant functions */
static struct passwd ixland_static_passwd;

/* Minimal stub user for mobile environment */
static const char *ixland_default_user_name = IXLAND_DEFAULT_USER_NAME;
static const char *ixland_default_user_passwd = "*";
static const char *ixland_default_user_gecos = "Mobile User";
static const char *ixland_default_user_dir = IXLAND_DEFAULT_USER_DIR;
static const char *ixland_default_user_shell = IXLAND_DEFAULT_USER_SHELL;

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static void ixland_fill_default_passwd(struct passwd *pwd) {
    pwd->pw_name = (char *)ixland_default_user_name;
    pwd->pw_passwd = (char *)ixland_default_user_passwd;
    pwd->pw_uid = IXLAND_DEFAULT_USER_UID; /* Standard iOS mobile UID */
    pwd->pw_gid = IXLAND_DEFAULT_USER_GID; /* Standard iOS mobile GID */
    pwd->pw_gecos = (char *)ixland_default_user_gecos;
    pwd->pw_dir = (char *)ixland_default_user_dir;
    pwd->pw_shell = (char *)ixland_default_user_shell;
}

/* ============================================================================
 * User Information Retrieval
 * ============================================================================ */

struct passwd *ixland_getpwnam(const char *name) {
    if (name == NULL) {
        errno = EINVAL;
        return NULL;
    }

    /* Stub: only return data for "mobile" user */
    if (strcmp(name, IXLAND_DEFAULT_USER_NAME) == 0) {
        ixland_fill_default_passwd(&ixland_static_passwd);
        return &ixland_static_passwd;
    }

    /* Not found - this is expected for stub implementation */
    errno = ENOENT;
    return NULL;
}

struct passwd *ixland_getpwuid(uid_t uid) {
    /* Stub: only return data for mobile UID */
    if (uid == IXLAND_DEFAULT_USER_UID) {
        ixland_fill_default_passwd(&ixland_static_passwd);
        return &ixland_static_passwd;
    }

    /* Also accept root (0) for compatibility */
    if (uid == 0) {
        /* Return root user data */
        ixland_static_passwd.pw_name = (char *)"root";
        ixland_static_passwd.pw_passwd = (char *)"*";
        ixland_static_passwd.pw_uid = 0;
        ixland_static_passwd.pw_gid = 0;
        ixland_static_passwd.pw_gecos = (char *)"System Administrator";
        ixland_static_passwd.pw_dir = (char *)"/var/root";
        ixland_static_passwd.pw_shell = (char *)"/bin/sh";
        return &ixland_static_passwd;
    }

    /* Not found - this is expected for stub implementation */
    errno = ENOENT;
    return NULL;
}

/* ============================================================================
 * Reentrant Versions (Thread-safe)
 * ============================================================================ */

int ixland_getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen,
                      struct passwd **result) {
    if (name == NULL || pwd == NULL || buf == NULL || result == NULL) {
        if (result != NULL) {
            *result = NULL;
        }
        return EINVAL;
    }

    /* Calculate required buffer size */
    size_t name_len = strlen(ixland_default_user_name) + 1;
    size_t passwd_len = strlen(ixland_default_user_passwd) + 1;
    size_t gecos_len = strlen(ixland_default_user_gecos) + 1;
    size_t dir_len = strlen(ixland_default_user_dir) + 1;
    size_t shell_len = strlen(ixland_default_user_shell) + 1;
    size_t total_len = name_len + passwd_len + gecos_len + dir_len + shell_len;

    if (buflen < total_len) {
        *result = NULL;
        return ERANGE;
    }

    /* Stub: only return data for "mobile" user */
    if (strcmp(name, IXLAND_DEFAULT_USER_NAME) != 0) {
        *result = NULL;
        return ENOENT;
    }

    /* Copy data to user buffer */
    char *p = buf;

    pwd->pw_name = p;
    memcpy(p, ixland_default_user_name, name_len);
    p += name_len;

    pwd->pw_passwd = p;
    memcpy(p, ixland_default_user_passwd, passwd_len);
    p += passwd_len;

    pwd->pw_uid = IXLAND_DEFAULT_USER_UID;
    pwd->pw_gid = IXLAND_DEFAULT_USER_GID;

    pwd->pw_gecos = p;
    memcpy(p, ixland_default_user_gecos, gecos_len);
    p += gecos_len;

    pwd->pw_dir = p;
    memcpy(p, ixland_default_user_dir, dir_len);
    p += dir_len;

    pwd->pw_shell = p;
    memcpy(p, ixland_default_user_shell, shell_len);

    *result = pwd;
    return 0;
}

int ixland_getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen,
                      struct passwd **result) {
    if (pwd == NULL || buf == NULL || result == NULL) {
        if (result != NULL) {
            *result = NULL;
        }
        return EINVAL;
    }

    /* Determine which user data to return */
    const char *user_name = NULL;
    uid_t user_uid = 0;
    gid_t user_gid = 0;
    const char *user_gecos = NULL;
    const char *user_dir = NULL;
    const char *user_shell = NULL;

    if (uid == IXLAND_DEFAULT_USER_UID) {
        user_name = ixland_default_user_name;
        user_uid = IXLAND_DEFAULT_USER_UID;
        user_gid = IXLAND_DEFAULT_USER_GID;
        user_gecos = ixland_default_user_gecos;
        user_dir = ixland_default_user_dir;
        user_shell = ixland_default_user_shell;
    } else if (uid == 0) {
        user_name = "root";
        user_uid = 0;
        user_gid = 0;
        user_gecos = "System Administrator";
        user_dir = "/var/root";
        user_shell = "/bin/sh";
    } else {
        *result = NULL;
        return ENOENT;
    }

    /* Calculate required buffer size */
    size_t name_len = strlen(user_name) + 1;
    size_t passwd_len = 2; /* "*" + null */
    size_t gecos_len = strlen(user_gecos) + 1;
    size_t dir_len = strlen(user_dir) + 1;
    size_t shell_len = strlen(user_shell) + 1;
    size_t total_len = name_len + passwd_len + gecos_len + dir_len + shell_len;

    if (buflen < total_len) {
        *result = NULL;
        return ERANGE;
    }

    /* Copy data to user buffer */
    char *p = buf;

    pwd->pw_name = p;
    memcpy(p, user_name, name_len);
    p += name_len;

    pwd->pw_passwd = p;
    *p++ = '*';
    *p++ = '\0';

    pwd->pw_uid = user_uid;
    pwd->pw_gid = user_gid;

    pwd->pw_gecos = p;
    memcpy(p, user_gecos, gecos_len);
    p += gecos_len;

    pwd->pw_dir = p;
    memcpy(p, user_dir, dir_len);
    p += dir_len;

    pwd->pw_shell = p;
    memcpy(p, user_shell, shell_len);

    *result = pwd;
    return 0;
}

/* ============================================================================
 * Database Iteration
 * ============================================================================ */

/* Iterator state for sequential access */
static int ixland_pwent_index = 0;

void ixland_setpwent(void) {
    /* Reset iteration */
    ixland_pwent_index = 0;
}

struct passwd *ixland_getpwent(void) {
    /* Stub: iterate through known users (root, then mobile) */
    if (ixland_pwent_index == 0) {
        ixland_pwent_index = 1;
        /* Return root user */
        ixland_static_passwd.pw_name = (char *)"root";
        ixland_static_passwd.pw_passwd = (char *)"*";
        ixland_static_passwd.pw_uid = 0;
        ixland_static_passwd.pw_gid = 0;
        ixland_static_passwd.pw_gecos = (char *)"System Administrator";
        ixland_static_passwd.pw_dir = (char *)"/var/root";
        ixland_static_passwd.pw_shell = (char *)"/bin/sh";
        return &ixland_static_passwd;
    } else if (ixland_pwent_index == 1) {
        ixland_pwent_index = 2;
        ixland_fill_default_passwd(&ixland_static_passwd);
        return &ixland_static_passwd;
    }

    /* No more entries */
    return NULL;
}

void ixland_endpwent(void) {
    /* Reset iteration state */
    ixland_pwent_index = 0;
}

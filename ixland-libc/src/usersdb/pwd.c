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

#include <pwd.h>
#include <errno.h>
#include <string.h>

/* ============================================================================
 * Static Data (Minimal stub data for mobile environment)
 * ============================================================================ */

/* Static buffer for non-reentrant functions */
static struct passwd iox_static_passwd;

/* Minimal stub user for mobile environment */
static const char *iox_default_user_name = "mobile";
static const char *iox_default_user_passwd = "*";
static const char *iox_default_user_gecos = "Mobile User";
static const char *iox_default_user_dir = "/var/mobile";
static const char *iox_default_user_shell = "/bin/sh";

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static void iox_fill_default_passwd(struct passwd *pwd) {
    pwd->pw_name = (char *)iox_default_user_name;
    pwd->pw_passwd = (char *)iox_default_user_passwd;
    pwd->pw_uid = 501;  /* Standard iOS mobile UID */
    pwd->pw_gid = 501;  /* Standard iOS mobile GID */
    pwd->pw_gecos = (char *)iox_default_user_gecos;
    pwd->pw_dir = (char *)iox_default_user_dir;
    pwd->pw_shell = (char *)iox_default_user_shell;
}

/* ============================================================================
 * User Information Retrieval
 * ============================================================================ */

struct passwd *iox_getpwnam(const char *name) {
    if (name == NULL) {
        errno = EINVAL;
        return NULL;
    }

    /* Stub: only return data for "mobile" user */
    if (strcmp(name, "mobile") == 0) {
        iox_fill_default_passwd(&iox_static_passwd);
        return &iox_static_passwd;
    }

    /* Not found - this is expected for stub implementation */
    errno = ENOENT;
    return NULL;
}

struct passwd *iox_getpwuid(uid_t uid) {
    /* Stub: only return data for mobile UID (501) */
    if (uid == 501) {
        iox_fill_default_passwd(&iox_static_passwd);
        return &iox_static_passwd;
    }

    /* Also accept root (0) for compatibility */
    if (uid == 0) {
        /* Return root user data */
        iox_static_passwd.pw_name = (char *)"root";
        iox_static_passwd.pw_passwd = (char *)"*";
        iox_static_passwd.pw_uid = 0;
        iox_static_passwd.pw_gid = 0;
        iox_static_passwd.pw_gecos = (char *)"System Administrator";
        iox_static_passwd.pw_dir = (char *)"/var/root";
        iox_static_passwd.pw_shell = (char *)"/bin/sh";
        return &iox_static_passwd;
    }

    /* Not found - this is expected for stub implementation */
    errno = ENOENT;
    return NULL;
}

/* ============================================================================
 * Reentrant Versions (Thread-safe)
 * ============================================================================ */

int iox_getpwnam_r(const char *name, struct passwd *pwd,
                   char *buf, size_t buflen, struct passwd **result) {
    if (name == NULL || pwd == NULL || buf == NULL || result == NULL) {
        if (result != NULL) {
            *result = NULL;
        }
        return EINVAL;
    }

    /* Calculate required buffer size */
    size_t name_len = strlen(iox_default_user_name) + 1;
    size_t passwd_len = strlen(iox_default_user_passwd) + 1;
    size_t gecos_len = strlen(iox_default_user_gecos) + 1;
    size_t dir_len = strlen(iox_default_user_dir) + 1;
    size_t shell_len = strlen(iox_default_user_shell) + 1;
    size_t total_len = name_len + passwd_len + gecos_len + dir_len + shell_len;

    if (buflen < total_len) {
        *result = NULL;
        return ERANGE;
    }

    /* Stub: only return data for "mobile" user */
    if (strcmp(name, "mobile") != 0) {
        *result = NULL;
        return ENOENT;
    }

    /* Copy data to user buffer */
    char *p = buf;

    pwd->pw_name = p;
    memcpy(p, iox_default_user_name, name_len);
    p += name_len;

    pwd->pw_passwd = p;
    memcpy(p, iox_default_user_passwd, passwd_len);
    p += passwd_len;

    pwd->pw_uid = 501;
    pwd->pw_gid = 501;

    pwd->pw_gecos = p;
    memcpy(p, iox_default_user_gecos, gecos_len);
    p += gecos_len;

    pwd->pw_dir = p;
    memcpy(p, iox_default_user_dir, dir_len);
    p += dir_len;

    pwd->pw_shell = p;
    memcpy(p, iox_default_user_shell, shell_len);

    *result = pwd;
    return 0;
}

int iox_getpwuid_r(uid_t uid, struct passwd *pwd,
                   char *buf, size_t buflen, struct passwd **result) {
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

    if (uid == 501) {
        user_name = iox_default_user_name;
        user_uid = 501;
        user_gid = 501;
        user_gecos = iox_default_user_gecos;
        user_dir = iox_default_user_dir;
        user_shell = iox_default_user_shell;
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
    size_t passwd_len = 2;  /* "*" + null */
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
static int iox_pwent_index = 0;

void iox_setpwent(void) {
    /* Reset iteration */
    iox_pwent_index = 0;
}

struct passwd *iox_getpwent(void) {
    /* Stub: iterate through known users (root, then mobile) */
    if (iox_pwent_index == 0) {
        iox_pwent_index = 1;
        /* Return root user */
        iox_static_passwd.pw_name = (char *)"root";
        iox_static_passwd.pw_passwd = (char *)"*";
        iox_static_passwd.pw_uid = 0;
        iox_static_passwd.pw_gid = 0;
        iox_static_passwd.pw_gecos = (char *)"System Administrator";
        iox_static_passwd.pw_dir = (char *)"/var/root";
        iox_static_passwd.pw_shell = (char *)"/bin/sh";
        return &iox_static_passwd;
    } else if (iox_pwent_index == 1) {
        iox_pwent_index = 2;
        iox_fill_default_passwd(&iox_static_passwd);
        return &iox_static_passwd;
    }

    /* No more entries */
    return NULL;
}

void iox_endpwent(void) {
    /* Reset iteration state */
    iox_pwent_index = 0;
}

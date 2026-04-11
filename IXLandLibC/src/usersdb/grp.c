/*
 * grp.c - Group database operations
 *
 * Stub implementation for iXland group database API.
 * These implementations return ENOSYS (function not implemented)
 * as the actual group database functionality requires runtime
 * integration with the kernel/VFS layer.
 *
 * This is a self-contained implementation suitable for the
 * ixland-libc-usersdb target.
 */

#include <errno.h>
#include <grp.h>
#include <string.h>

#include "internal_provider.h"

/* ============================================================================
 * Static Data (Minimal stub data for mobile environment)
 * ============================================================================ */

/* Static buffer for non-reentrant functions */
static struct group ixland_static_group;

/* Minimal stub group for mobile user environment */
static const char *ixland_default_group_name = IXLAND_DEFAULT_GROUP_NAME;
static const char *ixland_default_group_passwd = "*";
static const char *ixland_default_group_members[] = {NULL};

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static void ixland_fill_default_group(struct group *grp) {
    grp->gr_name = (char *)ixland_default_group_name;
    grp->gr_passwd = (char *)ixland_default_group_passwd;
    grp->gr_gid = IXLAND_DEFAULT_GROUP_GID; /* Standard iOS mobile GID */
    grp->gr_mem = (char **)ixland_default_group_members;
}

/* ============================================================================
 * Group Information Retrieval
 * ============================================================================ */

struct group *ixland_getgrnam(const char *name) {
    if (name == NULL) {
        errno = EINVAL;
        return NULL;
    }

    /* Stub: only return data for "mobile" group */
    if (strcmp(name, IXLAND_DEFAULT_GROUP_NAME) == 0) {
        ixland_fill_default_group(&ixland_static_group);
        return &ixland_static_group;
    }

    /* Not found - this is expected for stub implementation */
    errno = ENOENT;
    return NULL;
}

struct group *ixland_getgrgid(gid_t gid) {
    /* Stub: only return data for mobile GID */
    if (gid == IXLAND_DEFAULT_GROUP_GID) {
        ixland_fill_default_group(&ixland_static_group);
        return &ixland_static_group;
    }

    /* Not found - this is expected for stub implementation */
    errno = ENOENT;
    return NULL;
}

/* ============================================================================
 * Reentrant Versions (Thread-safe)
 * ============================================================================ */

int ixland_getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen,
                      struct group **result) {
    if (name == NULL || grp == NULL || buf == NULL || result == NULL) {
        if (result != NULL) {
            *result = NULL;
        }
        return EINVAL;
    }

    /* Check buffer size */
    size_t name_len = strlen(ixland_default_group_name) + 1;
    size_t passwd_len = strlen(ixland_default_group_passwd) + 1;
    if (buflen < name_len + passwd_len + sizeof(char *)) {
        *result = NULL;
        return ERANGE;
    }

    /* Stub: only return data for "mobile" group */
    if (strcmp(name, IXLAND_DEFAULT_GROUP_NAME) != 0) {
        *result = NULL;
        return ENOENT;
    }

    /* Copy data to user buffer */
    char *p = buf;
    grp->gr_name = p;
    memcpy(p, ixland_default_group_name, name_len);
    p += name_len;

    grp->gr_passwd = p;
    memcpy(p, ixland_default_group_passwd, passwd_len);
    p += passwd_len;

    grp->gr_gid = IXLAND_DEFAULT_GROUP_GID;

    /* Empty member list */
    grp->gr_mem = (char **)p;
    *((char **)p) = NULL;

    *result = grp;
    return 0;
}

int ixland_getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen,
                      struct group **result) {
    if (grp == NULL || buf == NULL || result == NULL) {
        if (result != NULL) {
            *result = NULL;
        }
        return EINVAL;
    }

    /* Check buffer size */
    size_t name_len = strlen(ixland_default_group_name) + 1;
    size_t passwd_len = strlen(ixland_default_group_passwd) + 1;
    if (buflen < name_len + passwd_len + sizeof(char *)) {
        *result = NULL;
        return ERANGE;
    }

    /* Stub: only return data for mobile GID */
    if (gid != IXLAND_DEFAULT_GROUP_GID) {
        *result = NULL;
        return ENOENT;
    }

    /* Copy data to user buffer */
    char *p = buf;
    grp->gr_name = p;
    memcpy(p, ixland_default_group_name, name_len);
    p += name_len;

    grp->gr_passwd = p;
    memcpy(p, ixland_default_group_passwd, passwd_len);
    p += passwd_len;

    grp->gr_gid = IXLAND_DEFAULT_GROUP_GID;

    /* Empty member list */
    grp->gr_mem = (char **)p;
    *((char **)p) = NULL;

    *result = grp;
    return 0;
}

/* ============================================================================
 * Database Iteration
 * ============================================================================ */

/* Iterator state for sequential access */
static int ixland_grent_valid = 0;

void ixland_setgrent(void) {
    /* Reset iteration - mark as ready to return first entry */
    ixland_grent_valid = 1;
}

struct group *ixland_getgrent(void) {
    /* Stub: return mobile group once, then NULL */
    if (ixland_grent_valid) {
        ixland_grent_valid = 0;
        ixland_fill_default_group(&ixland_static_group);
        return &ixland_static_group;
    }

    /* No more entries */
    return NULL;
}

void ixland_endgrent(void) {
    /* Reset iteration state */
    ixland_grent_valid = 0;
}

/* ============================================================================
 * Group Membership
 * ============================================================================ */

int ixland_getgroups(int size, gid_t list[]) {
    if (size < 0) {
        errno = EINVAL;
        return -1;
    }

    if (size == 0) {
        /* Return number of groups */
        return 1;
    }

    /* Stub: return single group (mobile) */
    if (list != NULL && size >= 1) {
        list[0] = IXLAND_DEFAULT_GROUP_GID; /* mobile GID */
    }

    return 1;
}

int ixland_setgroups(size_t size, const gid_t *list) {
    /* Stub: always return ENOSYS as this requires kernel support */
    (void)size;
    (void)list;
    errno = ENOSYS;
    return -1;
}

int ixland_initgroups(const char *user, gid_t group) {
    /* Stub: always return ENOSYS as this requires kernel support */
    (void)user;
    (void)group;
    errno = ENOSYS;
    return -1;
}

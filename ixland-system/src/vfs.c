/*
 * vfs.c - Virtual Filesystem Layer
 *
 * Path translation: /etc/passwd -> $PREFIX/etc/passwd
 */

#include "../include/vfs.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *a_shell_prefix = NULL;

void vfs_init(const char *prefix) {
    if (a_shell_prefix) {
        free(a_shell_prefix);
    }
    a_shell_prefix = strdup(prefix);
}

const char *vfs_get_prefix(void) {
    return a_shell_prefix ? a_shell_prefix : "/usr/local";
}

char *vfs_translate_path(const char *path) {
    if (!path) {
        return NULL;
    }

    /* Absolute paths that need prefix translation */
    if (path[0] == '/') {
        /* Check if it's a system path that needs prefix */
        if (strncmp(path, "/usr/", 5) == 0 || strncmp(path, "/etc/", 5) == 0 ||
            strncmp(path, "/var/", 5) == 0 || strncmp(path, "/bin/", 5) == 0 ||
            strncmp(path, "/lib/", 5) == 0 || strncmp(path, "/sbin/", 6) == 0) {
            size_t len = strlen(vfs_get_prefix()) + strlen(path) + 1;
            char *real_path = malloc(len);
            if (!real_path) {
                errno = ENOMEM;
                return NULL;
            }
            snprintf(real_path, len, "%s%s", vfs_get_prefix(), path);
            return real_path;
        }
    }

    /* Return copy of original path */
    char *copy = strdup(path);
    if (!copy) {
        errno = ENOMEM;
    }
    return copy;
}

void vfs_free_path(char *path) {
    free(path);
}

int vfs_path_needs_translation(const char *path) {
    if (!path || path[0] != '/') {
        return 0;
    }

    return (strncmp(path, "/usr/", 5) == 0 || strncmp(path, "/etc/", 5) == 0 ||
            strncmp(path, "/var/", 5) == 0 || strncmp(path, "/bin/", 5) == 0 ||
            strncmp(path, "/lib/", 5) == 0 || strncmp(path, "/sbin/", 6) == 0);
}

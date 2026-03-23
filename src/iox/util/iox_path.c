/* iOS Subsystem for Linux - Path Utilities
 *
 * Path resolution, validation, and manipulation
 */

#include "../internal/iox_internal.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>

/* ============================================================================
 * PATH RESOLUTION
 * ============================================================================ */

int __iox_path_resolve(const char *path, char *resolved, size_t resolved_len) {
    if (!path || !resolved || resolved_len == 0) {
        errno = EINVAL;
        return -1;
    }
    
    /* Handle absolute paths */
    if (path[0] == '/') {
        if (strlen(path) >= resolved_len) {
            errno = ENAMETOOLONG;
            return -1;
        }
        strncpy(resolved, path, resolved_len - 1);
        resolved[resolved_len - 1] = '\0';
        __iox_path_normalize(resolved);
        return 0;
    }
    
    /* Handle relative paths */
    char cwd[IOX_MAX_PATH];
    if (!getcwd(cwd, sizeof(cwd))) {
        return -1;
    }
    
    /* Join paths */
    if (strlen(cwd) + strlen(path) + 2 >= resolved_len) {
        errno = ENAMETOOLONG;
        return -1;
    }
    
    snprintf(resolved, resolved_len, "%s/%s", cwd, path);
    __iox_path_normalize(resolved);
    
    return 0;
}

void __iox_path_normalize(char *path) {
    if (!path || !*path) return;
    
    char *src = path;
    char *dst = path;
    
    /* Skip leading slashes */
    while (*src == '/') {
        src++;
    }
    
    /* Handle root case */
    if (src == path) {
        *dst++ = '/';
    }
    
    while (*src) {
        if (*src == '/') {
            /* Skip multiple slashes */
            while (*src == '/') src++;
            
            /* Check for . or .. */
            if (*src == '.') {
                if (src[1] == '\0' || src[1] == '/') {
                    /* Single dot - skip it */
                    src++;
                    continue;
                } else if (src[1] == '.' && (src[2] == '\0' || src[2] == '/')) {
                    /* Double dot - go up one directory */
                    src += 2;
                    if (dst > path + 1) {
                        /* Find previous slash */
                        dst--;
                        while (dst > path && *dst != '/') dst--;
                        if (dst == path) dst++;
                    }
                    continue;
                }
            }
            
            /* Add single slash */
            if (dst > path && dst[-1] != '/') {
                *dst++ = '/';
            }
        } else {
            *dst++ = *src++;
        }
    }
    
    /* Remove trailing slash (unless root) */
    if (dst > path + 1 && dst[-1] == '/') {
        dst--;
    }
    
    *dst = '\0';
}

void __iox_path_join(const char *base, const char *rel, char *result, size_t result_len) {
    if (!base || !rel || !result || result_len == 0) {
        if (result && result_len > 0) *result = '\0';
        return;
    }
    
    if (rel[0] == '/') {
        /* rel is absolute */
        if (strlen(rel) >= result_len) {
            if (result_len > 0) *result = '\0';
            return;
        }
        strncpy(result, rel, result_len - 1);
        result[result_len - 1] = '\0';
        return;
    }
    
    /* Join base and rel */
    size_t base_len = strlen(base);
    size_t rel_len = strlen(rel);
    
    if (base_len + rel_len + 2 > result_len) {
        if (result_len > 0) *result = '\0';
        return;
    }
    
    if (base_len > 0 && base[base_len - 1] == '/') {
        snprintf(result, result_len, "%s%s", base, rel);
    } else {
        snprintf(result, result_len, "%s/%s", base, rel);
    }
}

bool __iox_path_in_sandbox(const char *path) {
    /* For now, allow all paths - will implement sandbox checking later */
    /* TODO: Implement proper sandbox path validation */
    (void)path;
    return true;
}

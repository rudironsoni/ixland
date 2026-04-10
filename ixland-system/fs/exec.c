/* iXland - File Execution
 *
 * Canonical owner for exec syscalls:
 * - execve(), execv(), execvp(), execvpe()
 * - execle(), execl(), execlp()
 * - fexecve()
 *
 * Linux-shaped canonical owner - iOS mediation as implementation detail
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../src/ixland/internal/ixland_internal.h"

/* ============================================================================
 * EXECVE - Execute program
 * ============================================================================ */

int ixland_execve(const char *pathname, char *const argv[], char *const envp[]) {
    /* iOS restriction: cannot replace process image */
    (void)pathname;
    (void)argv;
    (void)envp;
    errno = ENOSYS;
    return -1;
}

int ixland_execv(const char *pathname, char *const argv[]) {
    return ixland_execve(pathname, argv, environ);
}

int ixland_execvp(const char *file, char *const argv[]) {
    /* Search PATH for executable */
    if (strchr(file, '/') != NULL) {
        return ixland_execv(file, argv);
    }

    const char *path_env = getenv("PATH");
    if (path_env == NULL) {
        path_env = "/usr/bin:/bin";
    }

    char *path_copy = strdup(path_env);
    if (path_copy == NULL) {
        return -1;
    }

    char *saveptr = NULL;
    char *dir = strtok_r(path_copy, ":", &saveptr);

    while (dir != NULL) {
        char fullpath[IXLAND_MAX_PATH];
        int len = snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, file);

        if (len > 0 && (size_t)len < sizeof(fullpath)) {
            struct stat st;
            if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode) && (access(fullpath, X_OK) == 0)) {
                int result = ixland_execv(fullpath, argv);
                free(path_copy);
                return result;
            }
        }

        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path_copy);
    errno = ENOENT;
    return -1;
}

int ixland_fexecve(int fd, char *const argv[], char *const envp[]) {
    /* iOS restriction: cannot replace process image */
    (void)fd;
    (void)argv;
    (void)envp;
    errno = ENOSYS;
    return -1;
}

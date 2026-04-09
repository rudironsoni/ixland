#include "fdtable.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

ixland_files_t *ixland_files_alloc(size_t max_fds) {
    ixland_files_t *files = calloc(1, sizeof(ixland_files_t));
    if (!files) {
        errno = ENOMEM;
        return NULL;
    }

    files->fd = calloc(max_fds, sizeof(ixland_file_t *));
    if (!files->fd) {
        free(files);
        errno = ENOMEM;
        return NULL;
    }

    files->max_fds = max_fds;
    pthread_mutex_init(&files->lock, NULL);

    return files;
}

void ixland_files_free(ixland_files_t *files) {
    if (!files)
        return;

    pthread_mutex_lock(&files->lock);
    for (size_t i = 0; i < files->max_fds; i++) {
        if (files->fd[i]) {
            ixland_file_free(files->fd[i]);
        }
    }
    pthread_mutex_unlock(&files->lock);

    free(files->fd);
    pthread_mutex_destroy(&files->lock);
    free(files);
}

ixland_files_t *ixland_files_dup(ixland_files_t *parent) {
    if (!parent) {
        errno = EINVAL;
        return NULL;
    }

    ixland_files_t *child = ixland_files_alloc(parent->max_fds);
    if (!child)
        return NULL;

    pthread_mutex_lock(&parent->lock);
    for (size_t i = 0; i < parent->max_fds; i++) {
        if (parent->fd[i]) {
            child->fd[i] = ixland_file_dup(parent->fd[i]);
            if (!child->fd[i]) {
                pthread_mutex_unlock(&parent->lock);
                ixland_files_free(child);
                errno = ENOMEM;
                return NULL;
            }
        }
    }
    pthread_mutex_unlock(&parent->lock);

    return child;
}

ixland_file_t *ixland_file_alloc(void) {
    ixland_file_t *file = calloc(1, sizeof(ixland_file_t));
    if (file) {
        atomic_init(&file->refs, 1);
    }
    return file;
}

void ixland_file_free(ixland_file_t *file) {
    if (!file)
        return;
    if (atomic_fetch_sub(&file->refs, 1) == 1) {
        free(file);
    }
}

ixland_file_t *ixland_file_dup(ixland_file_t *file) {
    if (!file)
        return NULL;
    atomic_fetch_add(&file->refs, 1);
    return file;
}

int ixland_fd_alloc(ixland_files_t *files, ixland_file_t *file) {
    if (!files || !file) {
        errno = EINVAL;
        return -1;
    }

    pthread_mutex_lock(&files->lock);
    for (size_t i = 0; i < files->max_fds; i++) {
        if (!files->fd[i]) {
            files->fd[i] = file;
            atomic_fetch_add(&file->refs, 1);
            pthread_mutex_unlock(&files->lock);
            return (int)i;
        }
    }
    pthread_mutex_unlock(&files->lock);

    errno = EMFILE;
    return -1;
}

int ixland_fd_free(ixland_files_t *files, int fd) {
    if (!files || fd < 0 || (size_t)fd >= files->max_fds) {
        errno = EBADF;
        return -1;
    }

    pthread_mutex_lock(&files->lock);
    ixland_file_t *file = files->fd[fd];
    if (!file) {
        pthread_mutex_unlock(&files->lock);
        errno = EBADF;
        return -1;
    }

    files->fd[fd] = NULL;
    ixland_file_free(file);
    pthread_mutex_unlock(&files->lock);

    return 0;
}

ixland_file_t *ixland_fd_lookup(ixland_files_t *files, int fd) {
    if (!files || fd < 0 || (size_t)fd >= files->max_fds) {
        errno = EBADF;
        return NULL;
    }

    pthread_mutex_lock(&files->lock);
    ixland_file_t *file = files->fd[fd];
    if (file) {
        atomic_fetch_add(&file->refs, 1);
    }
    pthread_mutex_unlock(&files->lock);

    return file;
}

#include "fdtable.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

iox_files_t *iox_files_alloc(size_t max_fds) {
    iox_files_t *files = calloc(1, sizeof(iox_files_t));
    if (!files) {
        errno = ENOMEM;
        return NULL;
    }
    
    files->fd = calloc(max_fds, sizeof(iox_file_t*));
    if (!files->fd) {
        free(files);
        errno = ENOMEM;
        return NULL;
    }
    
    files->max_fds = max_fds;
    pthread_mutex_init(&files->lock, NULL);
    
    return files;
}

void iox_files_free(iox_files_t *files) {
    if (!files) return;
    
    pthread_mutex_lock(&files->lock);
    for (size_t i = 0; i < files->max_fds; i++) {
        if (files->fd[i]) {
            iox_file_free(files->fd[i]);
        }
    }
    pthread_mutex_unlock(&files->lock);
    
    free(files->fd);
    pthread_mutex_destroy(&files->lock);
    free(files);
}

iox_files_t *iox_files_dup(iox_files_t *parent) {
    if (!parent) {
        errno = EINVAL;
        return NULL;
    }
    
    iox_files_t *child = iox_files_alloc(parent->max_fds);
    if (!child) return NULL;
    
    pthread_mutex_lock(&parent->lock);
    for (size_t i = 0; i < parent->max_fds; i++) {
        if (parent->fd[i]) {
            child->fd[i] = iox_file_dup(parent->fd[i]);
            if (!child->fd[i]) {
                pthread_mutex_unlock(&parent->lock);
                iox_files_free(child);
                errno = ENOMEM;
                return NULL;
            }
        }
    }
    pthread_mutex_unlock(&parent->lock);
    
    return child;
}

iox_file_t *iox_file_alloc(void) {
    iox_file_t *file = calloc(1, sizeof(iox_file_t));
    if (file) {
        atomic_init(&file->refs, 1);
    }
    return file;
}

void iox_file_free(iox_file_t *file) {
    if (!file) return;
    if (atomic_fetch_sub(&file->refs, 1) == 1) {
        free(file);
    }
}

iox_file_t *iox_file_dup(iox_file_t *file) {
    if (!file) return NULL;
    atomic_fetch_add(&file->refs, 1);
    return file;
}

int iox_fd_alloc(iox_files_t *files, iox_file_t *file) {
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

int iox_fd_free(iox_files_t *files, int fd) {
    if (!files || fd < 0 || (size_t)fd >= files->max_fds) {
        errno = EBADF;
        return -1;
    }
    
    pthread_mutex_lock(&files->lock);
    iox_file_t *file = files->fd[fd];
    if (!file) {
        pthread_mutex_unlock(&files->lock);
        errno = EBADF;
        return -1;
    }
    
    files->fd[fd] = NULL;
    iox_file_free(file);
    pthread_mutex_unlock(&files->lock);
    
    return 0;
}

iox_file_t *iox_fd_lookup(iox_files_t *files, int fd) {
    if (!files || fd < 0 || (size_t)fd >= files->max_fds) {
        errno = EBADF;
        return NULL;
    }
    
    pthread_mutex_lock(&files->lock);
    iox_file_t *file = files->fd[fd];
    if (file) {
        atomic_fetch_add(&file->refs, 1);
    }
    pthread_mutex_unlock(&files->lock);
    
    return file;
}

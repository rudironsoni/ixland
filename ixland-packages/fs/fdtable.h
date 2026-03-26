#ifndef IOX_FDTABLE_H
#define IOX_FDTABLE_H

#include <sys/types.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iox_file iox_file_t;
typedef struct iox_files iox_files_t;

struct iox_file {
    int fd;
    int real_fd;
    unsigned int flags;
    off_t pos;
    void *vfs_file;
    atomic_int refs;
};

struct iox_files {
    iox_file_t **fd;
    size_t max_fds;
    pthread_mutex_t lock;
};

iox_files_t *iox_files_alloc(size_t max_fds);
void iox_files_free(iox_files_t *files);
iox_files_t *iox_files_dup(iox_files_t *parent);

iox_file_t *iox_file_alloc(void);
void iox_file_free(iox_file_t *file);
iox_file_t *iox_file_dup(iox_file_t *file);

int iox_fd_alloc(iox_files_t *files, iox_file_t *file);
int iox_fd_free(iox_files_t *files, int fd);
iox_file_t *iox_fd_lookup(iox_files_t *files, int fd);
int iox_fd_dup(iox_files_t *files, int oldfd);
int iox_fd_dup2(iox_files_t *files, int oldfd, int newfd);
int iox_fd_set_cloexec(iox_files_t *files, int fd, bool cloexec);
bool iox_fd_get_cloexec(iox_files_t *files, int fd);
int iox_fd_close_cloexec(iox_files_t *files);

#ifdef __cplusplus
}
#endif

#endif

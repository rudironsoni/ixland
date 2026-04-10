#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../internal/ixland_internal.h"

typedef struct {
    int fd;
    int flags;
    mode_t mode;
    off_t offset;
    char path[IXLAND_MAX_PATH];
    bool used;
    bool is_dir;
    pthread_mutex_t lock;
} ixland_fd_entry_t;

static ixland_fd_entry_t ixland_fd_table[IXLAND_MAX_FD];
static pthread_mutex_t ixland_fd_table_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_int ixland_fd_table_initialized = 0;

void __ixland_file_init_impl(void) {
    if (atomic_exchange(&ixland_fd_table_initialized, 1) == 1) {
        return;
    }

    pthread_mutex_lock(&ixland_fd_table_lock);
    memset(ixland_fd_table, 0, sizeof(ixland_fd_table));

    ixland_fd_table[STDIN_FILENO].fd = STDIN_FILENO;
    ixland_fd_table[STDIN_FILENO].flags = O_RDONLY;
    ixland_fd_table[STDIN_FILENO].used = true;
    strcpy(ixland_fd_table[STDIN_FILENO].path, "/dev/stdin");
    pthread_mutex_init(&ixland_fd_table[STDIN_FILENO].lock, NULL);

    ixland_fd_table[STDOUT_FILENO].fd = STDOUT_FILENO;
    ixland_fd_table[STDOUT_FILENO].flags = O_WRONLY;
    ixland_fd_table[STDOUT_FILENO].used = true;
    strcpy(ixland_fd_table[STDOUT_FILENO].path, "/dev/stdout");
    pthread_mutex_init(&ixland_fd_table[STDOUT_FILENO].lock, NULL);

    ixland_fd_table[STDERR_FILENO].fd = STDERR_FILENO;
    ixland_fd_table[STDERR_FILENO].flags = O_WRONLY;
    ixland_fd_table[STDERR_FILENO].used = true;
    strcpy(ixland_fd_table[STDERR_FILENO].path, "/dev/stderr");
    pthread_mutex_init(&ixland_fd_table[STDERR_FILENO].lock, NULL);

    pthread_mutex_unlock(&ixland_fd_table_lock);
}

int __ixland_alloc_fd_impl(void) {
    pthread_mutex_lock(&ixland_fd_table_lock);

    for (int i = 3; i < IXLAND_MAX_FD; i++) {
        if (!ixland_fd_table[i].used) {
            ixland_fd_table[i].used = true;
            ixland_fd_table[i].fd = -1;
            ixland_fd_table[i].offset = 0;
            pthread_mutex_init(&ixland_fd_table[i].lock, NULL);
            pthread_mutex_unlock(&ixland_fd_table_lock);
            return i;
        }
    }

    pthread_mutex_unlock(&ixland_fd_table_lock);
    errno = EMFILE;
    return -1;
}

void __ixland_free_fd_impl(int fd) {
    if (fd < 0 || fd >= IXLAND_MAX_FD || fd <= STDERR_FILENO) {
        return;
    }

    pthread_mutex_lock(&ixland_fd_table_lock);
    if (ixland_fd_table[fd].used) {
        pthread_mutex_destroy(&ixland_fd_table[fd].lock);
        memset(&ixland_fd_table[fd], 0, sizeof(ixland_fd_entry_t));
    }
    pthread_mutex_unlock(&ixland_fd_table_lock);
}

void *__ixland_get_fd_entry_impl(int fd) {
    if (fd < 0 || fd >= IXLAND_MAX_FD) {
        return NULL;
    }

    pthread_mutex_lock(&ixland_fd_table_lock);
    ixland_fd_entry_t *entry = ixland_fd_table[fd].used ? &ixland_fd_table[fd] : NULL;
    if (entry) {
        pthread_mutex_lock(&entry->lock);
    }
    pthread_mutex_unlock(&ixland_fd_table_lock);
    return entry;
}

void __ixland_put_fd_entry_impl(void *entry) {
    if (entry) {
        pthread_mutex_unlock(&((ixland_fd_entry_t *)entry)->lock);
    }
}

int __ixland_get_real_fd_impl(void *entry) {
    return ((ixland_fd_entry_t *)entry)->fd;
}

int __ixland_get_fd_flags_impl(void *entry) {
    return ((ixland_fd_entry_t *)entry)->flags;
}

void __ixland_set_fd_flags_impl(void *entry, int flags) {
    ((ixland_fd_entry_t *)entry)->flags = flags;
}

off_t __ixland_get_fd_offset_impl(void *entry) {
    return ((ixland_fd_entry_t *)entry)->offset;
}

void __ixland_set_fd_offset_impl(void *entry, off_t offset) {
    ((ixland_fd_entry_t *)entry)->offset = offset;
}

void __ixland_init_fd_entry_impl(int fd, int real_fd, int flags, mode_t mode, const char *path) {
    ixland_fd_entry_t *entry = &ixland_fd_table[fd];
    pthread_mutex_lock(&entry->lock);
    entry->fd = real_fd;
    entry->flags = flags;
    entry->mode = mode;
    entry->offset = 0;
    strncpy(entry->path, path, IXLAND_MAX_PATH - 1);
    entry->path[IXLAND_MAX_PATH - 1] = '\0';

    struct stat file_stat;
    if (fstat(real_fd, &file_stat) == 0) {
        entry->is_dir = S_ISDIR(file_stat.st_mode);
    }
    pthread_mutex_unlock(&entry->lock);
}

void __ixland_clone_fd_entry_impl(int newfd, int oldfd) {
    pthread_mutex_lock(&ixland_fd_table_lock);
    memcpy(&ixland_fd_table[newfd], &ixland_fd_table[oldfd], sizeof(ixland_fd_entry_t));
    pthread_mutex_init(&ixland_fd_table[newfd].lock, NULL);
    pthread_mutex_unlock(&ixland_fd_table_lock);
}

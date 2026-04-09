/* iOS Subsystem for Linux - File Operations (VFS Version)
 *
 * File descriptor and file operation syscalls using VFS
 */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../internal/ixland_internal.h"

/* ============================================================================
 * FILE DESCRIPTOR TABLE
 * ============================================================================ */

#define IXLAND_MAX_FD 256

typedef struct {
    int fd;                  /* Real file descriptor */
    int flags;               /* Open flags */
    mode_t mode;             /* File mode */
    off_t offset;            /* Current offset */
    char path[IXLAND_MAX_PATH]; /* Virtual file path */
    bool used;               /* Is slot used? */
    bool is_dir;             /* Is directory? */
    pthread_mutex_t lock;    /* Per-FD lock */
} ixland_fd_entry_t;

static ixland_fd_entry_t fd_table[IXLAND_MAX_FD];
static pthread_mutex_t fd_table_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_int fd_table_initialized = 0;

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

static void __ixland_file_init(void) __attribute__((constructor));

static void __ixland_file_init(void) {
    if (atomic_exchange(&fd_table_initialized, 1) == 1) {
        return; /* Already initialized */
    }

    pthread_mutex_lock(&fd_table_lock);

    /* Clear table */
    memset(fd_table, 0, sizeof(fd_table));

    /* Initialize standard file descriptors */
    fd_table[STDIN_FILENO].fd = STDIN_FILENO;
    fd_table[STDIN_FILENO].flags = O_RDONLY;
    fd_table[STDIN_FILENO].used = true;
    fd_table[STDIN_FILENO].offset = 0;
    strcpy(fd_table[STDIN_FILENO].path, "/dev/stdin");
    pthread_mutex_init(&fd_table[STDIN_FILENO].lock, NULL);

    fd_table[STDOUT_FILENO].fd = STDOUT_FILENO;
    fd_table[STDOUT_FILENO].flags = O_WRONLY;
    fd_table[STDOUT_FILENO].used = true;
    fd_table[STDOUT_FILENO].offset = 0;
    strcpy(fd_table[STDOUT_FILENO].path, "/dev/stdout");
    pthread_mutex_init(&fd_table[STDOUT_FILENO].lock, NULL);

    fd_table[STDERR_FILENO].fd = STDERR_FILENO;
    fd_table[STDERR_FILENO].flags = O_WRONLY;
    fd_table[STDERR_FILENO].used = true;
    fd_table[STDERR_FILENO].offset = 0;
    strcpy(fd_table[STDERR_FILENO].path, "/dev/stderr");
    pthread_mutex_init(&fd_table[STDERR_FILENO].lock, NULL);

    pthread_mutex_unlock(&fd_table_lock);
}

/* ============================================================================
 * FILE DESCRIPTOR ALLOCATION
 * ============================================================================ */

static int __ixland_alloc_fd(void) {
    pthread_mutex_lock(&fd_table_lock);

    for (int i = 3; i < IXLAND_MAX_FD; i++) {
        if (!fd_table[i].used) {
            fd_table[i].used = true;
            fd_table[i].fd = -1; /* Will be set by open */
            fd_table[i].offset = 0;
            pthread_mutex_init(&fd_table[i].lock, NULL);
            pthread_mutex_unlock(&fd_table_lock);
            return i;
        }
    }

    pthread_mutex_unlock(&fd_table_lock);
    errno = EMFILE;
    return -1;
}

static void __ixland_free_fd(int fd) {
    if (fd < 0 || fd >= IXLAND_MAX_FD)
        return;
    if (fd <= 2)
        return; /* Don't free standard FDs */

    pthread_mutex_lock(&fd_table_lock);
    if (fd_table[fd].used) {
        pthread_mutex_destroy(&fd_table[fd].lock);
        memset(&fd_table[fd], 0, sizeof(ixland_fd_entry_t));
    }
    pthread_mutex_unlock(&fd_table_lock);
}

static ixland_fd_entry_t *__ixland_get_fd_entry(int fd) {
    if (fd < 0 || fd >= IXLAND_MAX_FD)
        return NULL;

    pthread_mutex_lock(&fd_table_lock);
    ixland_fd_entry_t *entry = fd_table[fd].used ? &fd_table[fd] : NULL;
    if (entry) {
        pthread_mutex_lock(&entry->lock);
    }
    pthread_mutex_unlock(&fd_table_lock);

    return entry;
}

static void __ixland_put_fd_entry(ixland_fd_entry_t *entry) {
    if (entry) {
        pthread_mutex_unlock(&entry->lock);
    }
}

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */

int __ixland_open_impl(const char *pathname, int flags, mode_t mode) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }

    /* Allocate file descriptor */
    int fd = __ixland_alloc_fd();
    if (fd < 0) {
        return -1;
    }

    /* Open through VFS */
    int real_fd = ixland_vfs_open(pathname, flags, mode);
    if (real_fd < 0) {
        __ixland_free_fd(fd);
        return -1;
    }

    /* Store in table */
    pthread_mutex_lock(&fd_table[fd].lock);
    fd_table[fd].fd = real_fd;
    fd_table[fd].flags = flags;
    fd_table[fd].mode = mode;
    fd_table[fd].offset = 0;
    strncpy(fd_table[fd].path, pathname, IXLAND_MAX_PATH - 1);
    fd_table[fd].path[IXLAND_MAX_PATH - 1] = '\0';

    /* Check if directory */
    struct stat st;
    if (fstat(real_fd, &st) == 0) {
        fd_table[fd].is_dir = S_ISDIR(st.st_mode);
    }
    pthread_mutex_unlock(&fd_table[fd].lock);

    return fd;
}

ssize_t __ixland_read_impl(int fd, void *buf, size_t count) {
    if (!buf) {
        errno = EFAULT;
        return -1;
    }

    if (fd < 0 || fd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    /* Standard FDs pass through directly */
    if (fd <= 2) {
        return read(fd, buf, count);
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }

    ssize_t bytes = read(entry->fd, buf, count);
    if (bytes > 0) {
        entry->offset += bytes;
    }

    __ixland_put_fd_entry(entry);
    return bytes;
}

ssize_t __ixland_write_impl(int fd, const void *buf, size_t count) {
    if (!buf) {
        errno = EFAULT;
        return -1;
    }

    if (fd < 0 || fd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    /* Standard FDs pass through directly */
    if (fd <= 2) {
        return write(fd, buf, count);
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }

    ssize_t bytes = write(entry->fd, buf, count);
    if (bytes > 0) {
        entry->offset += bytes;
    }

    __ixland_put_fd_entry(entry);
    return bytes;
}

int __ixland_close_impl(int fd) {
    if (fd < 0 || fd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    /* Don't close standard FDs */
    if (fd <= 2) {
        return 0;
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }

    int real_fd = entry->fd;
    __ixland_put_fd_entry(entry);

    /* Close real FD */
    close(real_fd);

    /* Free table entry */
    __ixland_free_fd(fd);
    return 0;
}

off_t __ixland_lseek_impl(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    if (fd <= 2) {
        errno = ESPIPE;
        return -1;
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }

    off_t result = lseek(entry->fd, offset, whence);
    if (result >= 0) {
        entry->offset = result;
    }

    __ixland_put_fd_entry(entry);
    return result;
}

ssize_t __ixland_pread_impl(int fd, void *buf, size_t count, off_t offset) {
    if (fd <= 2) {
        errno = ESPIPE;
        return -1;
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }

    ssize_t bytes = pread(entry->fd, buf, count, offset);
    __ixland_put_fd_entry(entry);
    return bytes;
}

ssize_t __ixland_pwrite_impl(int fd, const void *buf, size_t count, off_t offset) {
    if (fd <= 2) {
        errno = ESPIPE;
        return -1;
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }

    ssize_t bytes = pwrite(entry->fd, buf, count, offset);
    __ixland_put_fd_entry(entry);
    return bytes;
}

int __ixland_dup_impl(int oldfd) {
    /* For now, just return the same FD */
    /* TODO: Implement proper dup with reference counting */
    if (oldfd < 0 || oldfd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(oldfd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }
    __ixland_put_fd_entry(entry);

    return oldfd;
}

int __ixland_dup2_impl(int oldfd, int newfd) {
    if (oldfd < 0 || oldfd >= IXLAND_MAX_FD || newfd < 0 || newfd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    if (oldfd == newfd) {
        return newfd;
    }

    /* Get old entry */
    ixland_fd_entry_t *old_entry = __ixland_get_fd_entry(oldfd);
    if (!old_entry) {
        errno = EBADF;
        return -1;
    }

    /* Close newfd if open */
    ixland_fd_entry_t *new_entry = __ixland_get_fd_entry(newfd);
    if (new_entry) {
        __ixland_put_fd_entry(new_entry);
        __ixland_close_impl(newfd);
    }

    /* Copy entry */
    pthread_mutex_lock(&fd_table_lock);
    memcpy(&fd_table[newfd], &fd_table[oldfd], sizeof(ixland_fd_entry_t));
    fd_table[newfd].fd = old_entry->fd; /* Same real FD */
    pthread_mutex_init(&fd_table[newfd].lock, NULL);
    pthread_mutex_unlock(&fd_table_lock);

    __ixland_put_fd_entry(old_entry);

    return newfd;
}

int __ixland_fcntl_impl(int fd, int cmd, ...) {
    if (fd < 0 || fd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    va_list ap;
    va_start(ap, cmd);
    int result = -1;

    switch (cmd) {
    case F_DUPFD:
    case F_DUPFD_CLOEXEC: {
        int minfd = va_arg(ap, int);
        result = __ixland_dup2_impl(fd, minfd);
        break;
    }

    case F_GETFD: {
        result = 0;
        break;
    }

    case F_SETFD: {
        int flags = va_arg(ap, int);
        (void)flags;
        result = 0;
        break;
    }

    case F_GETFL: {
        ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
        if (entry) {
            result = entry->flags;
            __ixland_put_fd_entry(entry);
        } else {
            errno = EBADF;
        }
        break;
    }

    case F_SETFL: {
        int flags = va_arg(ap, int);
        ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
        if (entry) {
            entry->flags = flags;
            __ixland_put_fd_entry(entry);
            result = 0;
        } else {
            errno = EBADF;
        }
        break;
    }

    default:
        /* Pass through to real fcntl for other operations */
        if (fd > 2) {
            ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
            if (entry) {
                int arg = va_arg(ap, int);
                result = fcntl(entry->fd, cmd, arg);
                __ixland_put_fd_entry(entry);
            } else {
                errno = EBADF;
            }
        } else {
            int arg = va_arg(ap, int);
            result = fcntl(fd, cmd, arg);
        }
        break;
    }

    va_end(ap);
    return result;
}

int __ixland_ioctl_impl(int fd, unsigned long request, ...) {
    if (fd < 0 || fd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    va_list ap;
    va_start(ap, request);
    void *arg = va_arg(ap, void *);
    va_end(ap);

    if (fd <= 2) {
        return ioctl(fd, request, arg);
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }

    int result = ioctl(entry->fd, request, arg);
    __ixland_put_fd_entry(entry);
    return result;
}

int __ixland_stat_impl(const char *pathname, struct stat *statbuf) {
    if (!pathname || !statbuf) {
        errno = EFAULT;
        return -1;
    }
    return ixland_vfs_stat(pathname, statbuf);
}

int __ixland_fstat_impl(int fd, struct stat *statbuf) {
    if (!statbuf) {
        errno = EFAULT;
        return -1;
    }

    if (fd < 0 || fd >= IXLAND_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    /* Standard FDs pass through directly */
    if (fd <= 2) {
        return fstat(fd, statbuf);
    }

    ixland_fd_entry_t *entry = __ixland_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }

    int result = fstat(entry->fd, statbuf);
    __ixland_put_fd_entry(entry);
    return result;
}

int __ixland_lstat_impl(const char *pathname, struct stat *statbuf) {
    if (!pathname || !statbuf) {
        errno = EFAULT;
        return -1;
    }
    return ixland_vfs_lstat(pathname, statbuf);
}

int __ixland_access_impl(const char *pathname, int mode) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }
    return ixland_vfs_access(pathname, mode);
}

int __ixland_faccessat_impl(int dirfd, const char *pathname, int mode, int flags) {
    (void)dirfd;
    (void)flags;
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }
    return ixland_vfs_access(pathname, mode);
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

int ixland_open(const char *pathname, int flags, ...) {
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return __ixland_open_impl(pathname, flags, mode);
}

int ixland_creat(const char *pathname, mode_t mode) {
    return __ixland_open_impl(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

ssize_t ixland_read(int fd, void *buf, size_t count) {
    return __ixland_read_impl(fd, buf, count);
}

ssize_t ixland_write(int fd, const void *buf, size_t count) {
    return __ixland_write_impl(fd, buf, count);
}

int ixland_close(int fd) {
    return __ixland_close_impl(fd);
}

off_t ixland_lseek(int fd, off_t offset, int whence) {
    return __ixland_lseek_impl(fd, offset, whence);
}

ssize_t ixland_pread(int fd, void *buf, size_t count, off_t offset) {
    return __ixland_pread_impl(fd, buf, count, offset);
}

ssize_t ixland_pwrite(int fd, const void *buf, size_t count, off_t offset) {
    return __ixland_pwrite_impl(fd, buf, count, offset);
}

int ixland_dup(int oldfd) {
    return __ixland_dup_impl(oldfd);
}

int ixland_dup2(int oldfd, int newfd) {
    return __ixland_dup2_impl(oldfd, newfd);
}

int ixland_dup3(int oldfd, int newfd, int flags) {
    (void)flags;
    return __ixland_dup2_impl(oldfd, newfd);
}

int ixland_fcntl(int fd, int cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    int arg = va_arg(ap, int);
    va_end(ap);
    return __ixland_fcntl_impl(fd, cmd, arg);
}

int ixland_ioctl(int fd, unsigned long request, ...) {
    va_list ap;
    va_start(ap, request);
    void *arg = va_arg(ap, void *);
    va_end(ap);
    return __ixland_ioctl_impl(fd, request, arg);
}

int ixland_stat(const char *pathname, struct stat *statbuf) {
    return __ixland_stat_impl(pathname, statbuf);
}

int ixland_fstat(int fd, struct stat *statbuf) {
    return __ixland_fstat_impl(fd, statbuf);
}

int ixland_lstat(const char *pathname, struct stat *statbuf) {
    return __ixland_lstat_impl(pathname, statbuf);
}

int ixland_access(const char *pathname, int mode) {
    return __ixland_access_impl(pathname, mode);
}

int ixland_faccessat(int dirfd, const char *pathname, int mode, int flags) {
    return __ixland_faccessat_impl(dirfd, pathname, mode, flags);
}

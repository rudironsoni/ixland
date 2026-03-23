/* iOS Subsystem for Linux - File Operations
 *
 * File descriptor and file operation syscalls
 */

#include "../internal/iox_internal.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>

/* ============================================================================
 * FILE DESCRIPTOR TABLE
 * ============================================================================ */

#define IOX_MAX_FD 256

typedef struct {
    int fd;                     /* Virtual file descriptor (index in table) */
    int real_fd;                /* Real OS file descriptor */
    int flags;                  /* Open flags */
    int mode;                   /* File mode */
    off_t offset;               /* Current offset */
    char path[IOX_MAX_PATH];    /* File path */
    bool used;                  /* Is slot used? */
    bool is_dir;                /* Is directory? */
} iox_fd_entry_t;

static iox_fd_entry_t fd_table[IOX_MAX_FD];
static pthread_mutex_t fd_table_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_int fd_table_initialized = 0;

/* Standard FDs */
#define IOX_FD_STDIN  0
#define IOX_FD_STDOUT 1
#define IOX_FD_STDERR 2

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

/* File descriptor table initialization - not static so it can be called from iox_init.c */
void __iox_file_init_impl(void) {
    pthread_mutex_lock(&fd_table_lock);
    
    fprintf(stderr, "[DEBUG] __iox_file_init_impl: initializing fd table\n");
    
    /* Clear table */
    memset(fd_table, 0, sizeof(fd_table));
    
    /* Verify table is cleared */
    int used_after_clear = 0;
    for (int i = 0; i < IOX_MAX_FD; i++) {
        if (fd_table[i].used) used_after_clear++;
    }
    fprintf(stderr, "[DEBUG] After memset: %d entries marked as used\n", used_after_clear);
    
    /* Initialize standard file descriptors */
    fd_table[IOX_FD_STDIN].fd = IOX_FD_STDIN;
    fd_table[IOX_FD_STDIN].flags = O_RDONLY;
    fd_table[IOX_FD_STDIN].used = true;
    fd_table[IOX_FD_STDIN].offset = 0;
    fd_table[IOX_FD_STDIN].real_fd = IOX_FD_STDIN;
    strcpy(fd_table[IOX_FD_STDIN].path, "/dev/stdin");
    
    fd_table[IOX_FD_STDOUT].fd = IOX_FD_STDOUT;
    fd_table[IOX_FD_STDOUT].flags = O_WRONLY;
    fd_table[IOX_FD_STDOUT].used = true;
    fd_table[IOX_FD_STDOUT].offset = 0;
    fd_table[IOX_FD_STDOUT].real_fd = IOX_FD_STDOUT;
    strcpy(fd_table[IOX_FD_STDOUT].path, "/dev/stdout");
    
    fd_table[IOX_FD_STDERR].fd = IOX_FD_STDERR;
    fd_table[IOX_FD_STDERR].flags = O_WRONLY;
    fd_table[IOX_FD_STDERR].used = true;
    fd_table[IOX_FD_STDERR].offset = 0;
    fd_table[IOX_FD_STDERR].real_fd = IOX_FD_STDERR;
    strcpy(fd_table[IOX_FD_STDERR].path, "/dev/stderr");
    
    atomic_store(&fd_table_initialized, 1);
    pthread_mutex_unlock(&fd_table_lock);
}

static void __iox_file_init(void) __attribute__((constructor(100))) __attribute__((used));

static void __iox_file_init(void) {
    __iox_file_init_impl();
}

static void __iox_ensure_file_init(void) {
    if (!atomic_load(&fd_table_initialized)) {
        __iox_file_init_impl();
    }
}

/* ============================================================================
 * INTERNAL HELPERS
 * ============================================================================ */

static int alloc_count = 0;

static int __iox_alloc_fd(void) {
    /* Ensure file descriptor table is initialized */
    __iox_ensure_file_init();
    
    pthread_mutex_lock(&fd_table_lock);
    
    /* Debug: Track allocation count */
    alloc_count++;
    if (alloc_count <= 10 || alloc_count % 50 == 0) {
        fprintf(stderr, "[DEBUG] __iox_alloc_fd called (count=%d)\n", alloc_count);
    }
    
    /* Debug: Check how many fds are used */
    int used_count = 0;
    int first_free = -1;
    for (int i = 0; i < IOX_MAX_FD; i++) {
        if (fd_table[i].used) used_count++;
        else if (first_free == -1 && i >= 3) first_free = i;
    }
    
    if (used_count >= IOX_MAX_FD - 5) {
        fprintf(stderr, "[DEBUG] FD table almost full: %d/%d used, first free=%d\n", used_count, IOX_MAX_FD, first_free);
        fprintf(stderr, "[DEBUG] Total allocations so far: %d\n", alloc_count);
    }
    
    for (int i = 3; i < IOX_MAX_FD; i++) {
        if (!fd_table[i].used) {
            fd_table[i].used = true;
            fd_table[i].fd = i;
            fd_table[i].offset = 0;
            pthread_mutex_unlock(&fd_table_lock);
            return i;
        }
    }
    
    pthread_mutex_unlock(&fd_table_lock);
    fprintf(stderr, "[DEBUG] FD table full: %d/%d used, cannot allocate\n", used_count, IOX_MAX_FD);
    errno = EMFILE;
    return -1;
}

static void __iox_free_fd(int fd) {
    if (fd < 0 || fd >= IOX_MAX_FD) return;
    
    pthread_mutex_lock(&fd_table_lock);
    if (fd_table[fd].used) {
        memset(&fd_table[fd], 0, sizeof(iox_fd_entry_t));
    }
    pthread_mutex_unlock(&fd_table_lock);
}

static iox_fd_entry_t *__iox_get_fd_entry(int fd) {
    if (fd < 0 || fd >= IOX_MAX_FD) return NULL;
    
    pthread_mutex_lock(&fd_table_lock);
    iox_fd_entry_t *entry = fd_table[fd].used ? &fd_table[fd] : NULL;
    pthread_mutex_unlock(&fd_table_lock);
    
    return entry;
}

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */

int __iox_open_impl(const char *pathname, int flags, mode_t mode) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }
    
    /* Allocate file descriptor */
    int fd = __iox_alloc_fd();
    if (fd < 0) {
        return -1;
    }
    
    /* Resolve path */
    char resolved_path[IOX_MAX_PATH];
    if (__iox_path_resolve(pathname, resolved_path, sizeof(resolved_path)) != 0) {
        __iox_free_fd(fd);
        errno = ENOENT;
        return -1;
    }
    
    /* Check sandbox */
    if (!__iox_path_in_sandbox(resolved_path)) {
        __iox_free_fd(fd);
        errno = EACCES;
        return -1;
    }
    
    /* For now, use real open (will be replaced with VFS later) */
    int real_fd = open(resolved_path, flags, mode);
    if (real_fd < 0) {
        __iox_free_fd(fd);
        return -1;
    }
    
    /* Store in table */
    pthread_mutex_lock(&fd_table_lock);
    fd_table[fd].fd = fd;
    fd_table[fd].real_fd = real_fd;
    fd_table[fd].flags = flags;
    fd_table[fd].mode = mode;
    fd_table[fd].offset = 0;
    strncpy(fd_table[fd].path, resolved_path, IOX_MAX_PATH - 1);
    
    /* Check if directory */
    struct stat st;
    if (fstat(real_fd, &st) == 0) {
        fd_table[fd].is_dir = S_ISDIR(st.st_mode);
    }
    pthread_mutex_unlock(&fd_table_lock);
    
    return fd;
}

int __iox_openat_impl(int dirfd, const char *pathname, int flags, mode_t mode) {
    (void)dirfd;
    /* For now, just use regular open */
    /* TODO: Implement relative to directory fd */
    return __iox_open_impl(pathname, flags, mode);
}

ssize_t __iox_read_impl(int fd, void *buf, size_t count) {
    if (!buf) {
        errno = EFAULT;
        return -1;
    }
    
    iox_fd_entry_t *entry = __iox_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }
    
    /* For standard FDs, use real read */
    if (fd <= 2) {
        return read(fd, buf, count);
    }
    
    /* For now, passthrough to real read */
    /* TODO: Implement with VFS */
    ssize_t bytes = read(entry->real_fd, buf, count);
    if (bytes > 0) {
        pthread_mutex_lock(&fd_table_lock);
        fd_table[fd].offset += bytes;
        pthread_mutex_unlock(&fd_table_lock);
    }
    
    return bytes;
}

ssize_t __iox_write_impl(int fd, const void *buf, size_t count) {
    if (!buf) {
        errno = EFAULT;
        return -1;
    }
    
    iox_fd_entry_t *entry = __iox_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }
    
    /* For standard FDs, use real write */
    if (fd <= 2) {
        return write(fd, buf, count);
    }
    
    /* For now, passthrough to real write */
    ssize_t bytes = write(entry->real_fd, buf, count);
    if (bytes > 0) {
        pthread_mutex_lock(&fd_table_lock);
        fd_table[fd].offset += bytes;
        pthread_mutex_unlock(&fd_table_lock);
    }
    
    return bytes;
}

int __iox_close_impl(int fd) {
    if (fd < 0 || fd >= IOX_MAX_FD) {
        errno = EBADF;
        return -1;
    }
    
    /* Don't close standard FDs */
    if (fd <= 2) {
        return 0;
    }
    
    iox_fd_entry_t *entry = __iox_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }
    
    /* Close real FD */
    if (entry->real_fd >= 0) {
        close(entry->real_fd);
    }
    
    __iox_free_fd(fd);
    return 0;
}

off_t __iox_lseek_impl(int fd, off_t offset, int whence) {
    iox_fd_entry_t *entry = __iox_get_fd_entry(fd);
    if (!entry) {
        errno = EBADF;
        return -1;
    }
    
    /* Calculate new offset */
    off_t new_offset;
    pthread_mutex_lock(&fd_table_lock);
    
    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = fd_table[fd].offset + offset;
            break;
        case SEEK_END:
            /* Would need file size - for now, just return current */
            new_offset = offset;
            break;
        default:
            pthread_mutex_unlock(&fd_table_lock);
            errno = EINVAL;
            return -1;
    }
    
    fd_table[fd].offset = new_offset;
    pthread_mutex_unlock(&fd_table_lock);
    
    return new_offset;
}

ssize_t __iox_pread_impl(int fd, void *buf, size_t count, off_t offset) {
    /* For now, just do regular read at offset */
    off_t old_offset = __iox_lseek_impl(fd, 0, SEEK_CUR);
    if (old_offset < 0) return -1;
    
    if (__iox_lseek_impl(fd, offset, SEEK_SET) < 0) return -1;
    
    ssize_t bytes = __iox_read_impl(fd, buf, count);
    
    /* Restore old offset */
    __iox_lseek_impl(fd, old_offset, SEEK_SET);
    
    return bytes;
}

ssize_t __iox_pwrite_impl(int fd, const void *buf, size_t count, off_t offset) {
    /* For now, just do regular write at offset */
    off_t old_offset = __iox_lseek_impl(fd, 0, SEEK_CUR);
    if (old_offset < 0) return -1;
    
    if (__iox_lseek_impl(fd, offset, SEEK_SET) < 0) return -1;
    
    ssize_t bytes = __iox_write_impl(fd, buf, count);
    
    /* Restore old offset */
    __iox_lseek_impl(fd, old_offset, SEEK_SET);
    
    return bytes;
}

int __iox_dup_impl(int oldfd) {
    /* For now, return same FD */
    /* TODO: Implement proper dup with reference counting */
    return oldfd;
}

int __iox_dup2_impl(int oldfd, int newfd) {
    if (oldfd < 0 || oldfd >= IOX_MAX_FD || newfd < 0 || newfd >= IOX_MAX_FD) {
        errno = EBADF;
        return -1;
    }
    
    if (oldfd == newfd) {
        return newfd;
    }
    
    /* Close newfd if open */
    iox_fd_entry_t *new_entry = __iox_get_fd_entry(newfd);
    if (new_entry) {
        __iox_close_impl(newfd);
    }
    
    /* Duplicate the real OS fd */
    int new_real_fd = dup(fd_table[oldfd].real_fd);
    if (new_real_fd < 0) {
        return -1;
    }
    
    /* Copy entry */
    pthread_mutex_lock(&fd_table_lock);
    memcpy(&fd_table[newfd], &fd_table[oldfd], sizeof(iox_fd_entry_t));
    fd_table[newfd].fd = newfd;
    fd_table[newfd].real_fd = new_real_fd;
    pthread_mutex_unlock(&fd_table_lock);
    
    return newfd;
}

int __iox_dup3_impl(int oldfd, int newfd, int flags) {
    (void)flags;
    /* Ignore flags for now */
    return __iox_dup2_impl(oldfd, newfd);
}

int __iox_fcntl_impl(int fd, int cmd, ...) {
    iox_fd_entry_t *entry = __iox_get_fd_entry(fd);
    if (!entry) {
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
            result = __iox_dup2_impl(fd, minfd);
            break;
        }
        
        case F_GETFD: {
            /* Get file descriptor flags */
            result = 0;
            break;
        }
        
        case F_SETFD: {
            /* Set file descriptor flags */
            (void)va_arg(ap, int);
            result = 0;
            break;
        }
        
        case F_GETFL: {
            /* Get file status flags */
            result = entry->flags;
            break;
        }
        
        case F_SETFL: {
            /* Set file status flags */
            int flags = va_arg(ap, int);
            pthread_mutex_lock(&fd_table_lock);
            fd_table[fd].flags = flags;
            pthread_mutex_unlock(&fd_table_lock);
            result = 0;
            break;
        }
        
        default:
            errno = EINVAL;
            result = -1;
            break;
    }
    
    va_end(ap);
    return result;
}

int __iox_ioctl_impl(int fd, unsigned long request, ...) {
    (void)fd;
    (void)request;
    /* For now, return ENOTTY (not a TTY) */
    /* TODO: Implement terminal ioctl */
    errno = ENOTTY;
    return -1;
}

int __iox_access_impl(const char *pathname, int mode) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }
    
    char resolved_path[IOX_MAX_PATH];
    if (__iox_path_resolve(pathname, resolved_path, sizeof(resolved_path)) != 0) {
        errno = ENOENT;
        return -1;
    }
    
    if (!__iox_path_in_sandbox(resolved_path)) {
        errno = EACCES;
        return -1;
    }
    
    return access(resolved_path, mode);
}

int __iox_faccessat_impl(int dirfd, const char *pathname, int mode, int flags) {
    (void)dirfd;
    (void)flags;
    /* For now, just use access */
    return __iox_access_impl(pathname, mode);
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

int iox_open(const char *pathname, int flags, ...) {
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return __iox_open_impl(pathname, flags, mode);
}

int iox_openat(int dirfd, const char *pathname, int flags, ...) {
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return __iox_openat_impl(dirfd, pathname, flags, mode);
}

int iox_creat(const char *pathname, mode_t mode) {
    return __iox_open_impl(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

ssize_t iox_read(int fd, void *buf, size_t count) {
    return __iox_read_impl(fd, buf, count);
}

ssize_t iox_write(int fd, const void *buf, size_t count) {
    return __iox_write_impl(fd, buf, count);
}

int iox_close(int fd) {
    return __iox_close_impl(fd);
}

off_t iox_lseek(int fd, off_t offset, int whence) {
    return __iox_lseek_impl(fd, offset, whence);
}

ssize_t iox_pread(int fd, void *buf, size_t count, off_t offset) {
    return __iox_pread_impl(fd, buf, count, offset);
}

ssize_t iox_pwrite(int fd, const void *buf, size_t count, off_t offset) {
    return __iox_pwrite_impl(fd, buf, count, offset);
}

int iox_dup(int oldfd) {
    return __iox_dup_impl(oldfd);
}

int iox_dup2(int oldfd, int newfd) {
    return __iox_dup2_impl(oldfd, newfd);
}

int iox_dup3(int oldfd, int newfd, int flags) {
    return __iox_dup3_impl(oldfd, newfd, flags);
}

int iox_fcntl(int fd, int cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    int arg = va_arg(ap, int);
    va_end(ap);
    return __iox_ioctl_impl(fd, cmd, arg);
}

int iox_ioctl(int fd, unsigned long request, ...) {
    va_list ap;
    va_start(ap, request);
    void *arg = va_arg(ap, void *);
    va_end(ap);
    return __iox_ioctl_impl(fd, request, arg);
}

int iox_access(const char *pathname, int mode) {
    return __iox_access_impl(pathname, mode);
}

int iox_faccessat(int dirfd, const char *pathname, int mode, int flags) {
    return __iox_faccessat_impl(dirfd, pathname, mode, flags);
}

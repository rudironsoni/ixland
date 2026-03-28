/* iOS Subsystem for Linux - Minimal Working Implementation
 *
 * Streamlined version that builds and works
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* ============================================================================
 * BASIC DEFINITIONS
 * ============================================================================ */

#define IOX_MAX_FD 256
#define IOX_MAX_PATH 1024

/* Current PID counter */
static pid_t next_pid = 1000;
static pthread_mutex_t pid_lock = PTHREAD_MUTEX_INITIALIZER;

/* Thread-local current PID */
__thread pid_t __iox_current_pid = 0;

/* ============================================================================
 * FD TABLE
 * ============================================================================ */

static struct {
    int real_fd;
    bool used;
} fd_table[IOX_MAX_FD];

static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

static int alloc_fd(void) {
    pthread_mutex_lock(&fd_lock);
    for (int i = 3; i < IOX_MAX_FD; i++) {
        if (!fd_table[i].used) {
            fd_table[i].used = true;
            pthread_mutex_unlock(&fd_lock);
            return i;
        }
    }
    pthread_mutex_unlock(&fd_lock);
    errno = EMFILE;
    return -1;
}

static void free_fd(int fd) {
    if (fd >= 3 && fd < IOX_MAX_FD) {
        pthread_mutex_lock(&fd_lock);
        fd_table[fd].used = false;
        pthread_mutex_unlock(&fd_lock);
    }
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

pid_t iox_getpid(void) {
    if (__iox_current_pid == 0) {
        pthread_mutex_lock(&pid_lock);
        __iox_current_pid = next_pid++;
        pthread_mutex_unlock(&pid_lock);
    }
    return __iox_current_pid;
}

pid_t iox_getppid(void) {
    return 0; /* No real parent in this simulation */
}

int iox_open(const char *pathname, int flags, ...) {
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }

    int fd = alloc_fd();
    if (fd < 0)
        return -1;

    int real_fd = open(pathname, flags, mode);
    if (real_fd < 0) {
        free_fd(fd);
        return -1;
    }

    pthread_mutex_lock(&fd_lock);
    fd_table[fd].real_fd = real_fd;
    pthread_mutex_unlock(&fd_lock);

    return fd;
}

ssize_t iox_read(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= IOX_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    if (fd <= 2) {
        return read(fd, buf, count);
    }

    pthread_mutex_lock(&fd_lock);
    if (!fd_table[fd].used) {
        pthread_mutex_unlock(&fd_lock);
        errno = EBADF;
        return -1;
    }
    int real_fd = fd_table[fd].real_fd;
    pthread_mutex_unlock(&fd_lock);

    return read(real_fd, buf, count);
}

ssize_t iox_write(int fd, const void *buf, size_t count) {
    if (fd < 0 || fd >= IOX_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    if (fd <= 2) {
        return write(fd, buf, count);
    }

    pthread_mutex_lock(&fd_lock);
    if (!fd_table[fd].used) {
        pthread_mutex_unlock(&fd_lock);
        errno = EBADF;
        return -1;
    }
    int real_fd = fd_table[fd].real_fd;
    pthread_mutex_unlock(&fd_lock);

    return write(real_fd, buf, count);
}

int iox_close(int fd) {
    if (fd < 0 || fd >= IOX_MAX_FD) {
        errno = EBADF;
        return -1;
    }

    if (fd <= 2) {
        return 0; /* Don't close standard FDs */
    }

    pthread_mutex_lock(&fd_lock);
    if (!fd_table[fd].used) {
        pthread_mutex_unlock(&fd_lock);
        errno = EBADF;
        return -1;
    }
    int real_fd = fd_table[fd].real_fd;
    fd_table[fd].used = false;
    pthread_mutex_unlock(&fd_lock);

    return close(real_fd);
}

/* ============================================================================
 * SYMBOL INTERPOSITION
 * ============================================================================ */

pid_t getpid(void) {
    return iox_getpid();
}
pid_t getppid(void) {
    return iox_getppid();
}

int open(const char *pathname, int flags, ...) {
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return iox_open(pathname, flags, mode);
}

ssize_t read(int fd, void *buf, size_t count) {
    return iox_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return iox_write(fd, buf, count);
}

int close(int fd) {
    return iox_close(fd);
}

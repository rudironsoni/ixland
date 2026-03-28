/* iXland - I/O Multiplexing: poll(), select(), epoll implementation
 *
 * Efficient I/O multiplexing using kqueue on iOS.
 * Provides Linux-compatible poll(), select(), and epoll interfaces.
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "../internal/iox_internal.h"

/* Include poll/epoll headers from ixland-libc boundary
 * These are public headers installed to include/linux/
 * Include paths are set by CMake: ixland-libc/include */
#define _LINUX_POLL_H
#define _LINUX_EPOLL_H
#include <linux/epoll.h>
#include <linux/poll.h>

/* ============================================================================
 * POLL IMPLEMENTATION
 * ============================================================================ */

/**
 * @brief Convert poll events to kqueue filter flags
 */
static int16_t iox_poll_events_to_kfilter(short events) {
    int16_t filter = 0;

    if (events & (IOX_POLLIN | IOX_POLLRDNORM)) {
        filter = EVFILT_READ;
    } else if (events & (IOX_POLLOUT | IOX_POLLWRNORM)) {
        filter = EVFILT_WRITE;
    }

    return filter;
}

/**
 * @brief Convert kqueue flags to poll revents
 */
static short iox_kfilter_to_poll_revents(int16_t filter, uint16_t flags) {
    short revents = 0;

    if (filter == EVFILT_READ) {
        revents |= IOX_POLLIN;
    } else if (filter == EVFILT_WRITE) {
        revents |= IOX_POLLOUT;
    }

    if (flags & EV_ERROR) {
        revents |= IOX_POLLERR;
    }

    if (flags & EV_EOF) {
        revents |= IOX_POLLHUP;
    }

    return revents;
}

/**
 * @brief Internal poll implementation using kqueue
 */
static int iox_poll_kqueue(struct linux_pollfd *fds, unsigned int nfds, int timeout_ms) {
    if (!fds) {
        errno = EFAULT;
        return -1;
    }

    if (nfds == 0) {
        /* No FDs to poll - just wait for timeout */
        if (timeout_ms > 0) {
            usleep(timeout_ms * 1000);
        } else if (timeout_ms < 0) {
            /* Wait forever with no FDs - this is a bit strange but valid */
            pause();
            errno = EINTR;
            return -1;
        }
        return 0;
    }

    /* Create kqueue */
    int kq = kqueue();
    if (kq < 0) {
        errno = ENOMEM;
        return -1;
    }

    /* Build kevent array */
    struct kevent *changelist = calloc(nfds * 2, sizeof(struct kevent));
    if (!changelist) {
        close(kq);
        errno = ENOMEM;
        return -1;
    }

    int nchanges = 0;
    for (unsigned int i = 0; i < nfds; i++) {
        /* Clear revents */
        fds[i].revents = 0;

        if (fds[i].fd < 0) {
            /* Negative fd is ignored */
            continue;
        }

        /* Check if fd is valid */
        if (fcntl(fds[i].fd, F_GETFL) < 0) {
            fds[i].revents = IOX_POLLNVAL;
            continue;
        }

        /* Add read filter if requested */
        if (fds[i].events & (IOX_POLLIN | IOX_POLLRDNORM | IOX_POLLRDBAND | IOX_POLLPRI)) {
            EV_SET(&changelist[nchanges], fds[i].fd, EVFILT_READ, EV_ADD, 0, 0,
                   (void *)(uintptr_t)i);
            nchanges++;
        }

        /* Add write filter if requested */
        if (fds[i].events & (IOX_POLLOUT | IOX_POLLWRNORM | IOX_POLLWRBAND)) {
            EV_SET(&changelist[nchanges], fds[i].fd, EVFILT_WRITE, EV_ADD, 0, 0,
                   (void *)(uintptr_t)i);
            nchanges++;
        }
    }

    /* If all FDs were invalid, return error */
    if (nchanges == 0) {
        free(changelist);
        close(kq);

        /* Check if any FD had POLLNVAL */
        for (unsigned int i = 0; i < nfds; i++) {
            if (fds[i].revents & IOX_POLLNVAL) {
                return 1; /* At least one ready (with error) */
            }
        }
        return 0;
    }

    /* Prepare timeout */
    struct timespec ts;
    struct timespec *tsp = NULL;
    if (timeout_ms >= 0) {
        ts.tv_sec = timeout_ms / 1000;
        ts.tv_nsec = (timeout_ms % 1000) * 1000000;
        tsp = &ts;
    }

    /* Event list for results */
    struct kevent *eventlist = calloc(nfds * 2, sizeof(struct kevent));
    if (!eventlist) {
        free(changelist);
        close(kq);
        errno = ENOMEM;
        return -1;
    }

    /* Wait for events */
    int nevents = kevent(kq, changelist, nchanges, eventlist, nfds * 2, tsp);

    /* Handle errors */
    if (nevents < 0) {
        int saved_errno = errno;
        free(changelist);
        free(eventlist);
        close(kq);
        errno = (saved_errno == EINTR) ? EINTR : EFAULT;
        return -1;
    }

    /* Process results */
    int ready_count = 0;
    for (int i = 0; i < nevents; i++) {
        unsigned int idx = (unsigned int)(uintptr_t)eventlist[i].udata;
        if (idx < nfds) {
            fds[idx].revents |=
                iox_kfilter_to_poll_revents(eventlist[i].filter, eventlist[i].flags);
            ready_count++;
        }
    }

    free(changelist);
    free(eventlist);
    close(kq);

    return ready_count;
}

/**
 * @brief Wait for some event on a file descriptor
 *
 * @param fds Array of pollfd structures
 * @param nfds Number of file descriptors
 * @param timeout Timeout in milliseconds (-1 = forever, 0 = non-blocking)
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int iox_poll(struct linux_pollfd *fds, unsigned int nfds, int timeout) {
    /* Validate parameters */
    if (!fds) {
        errno = EFAULT;
        return -1;
    }

    /* Use kqueue implementation */
    return iox_poll_kqueue(fds, nfds, timeout);
}

/**
 * @brief Wait for some event on a file descriptor (with signal mask)
 *
 * @param fds Array of pollfd structures
 * @param nfds Number of file descriptors
 * @param timeout Timeout (NULL = forever)
 * @param sigmask Signal mask to restore during wait
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int iox_ppoll(struct linux_pollfd *fds, unsigned int nfds, const struct linux_timespec *timeout,
              const linux_sigset_t *sigmask) {
    /* Convert timeout */
    int timeout_ms = -1;
    if (timeout) {
        timeout_ms = (int)(timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000);
    }

    /* Save current signal mask if needed */
    sigset_t oldmask;
    sigset_t newmask;
    if (sigmask) {
        /* Convert linux_sigset_t (128 bytes) to native sigset_t */
        memset(&newmask, 0, sizeof(newmask));
        memcpy(&newmask, sigmask, sizeof(newmask) < 128 ? sizeof(newmask) : 128);
        pthread_sigmask(SIG_SETMASK, &newmask, &oldmask);
    }

    /* Call poll */
    int result = iox_poll(fds, nfds, timeout_ms);

    /* Restore signal mask */
    if (sigmask) {
        pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
    }

    return result;
}

/* ============================================================================
 * SELECT IMPLEMENTATION
 * ============================================================================ */

/**
 * @brief Synchronous I/O multiplexing using kqueue
 *
 * select() monitors multiple FDs for readiness using kqueue on iOS.
 *
 * @param nfds Highest-numbered FD + 1
 * @param readfds FDs to check for reading (may be NULL)
 * @param writefds FDs to check for writing (may be NULL)
 * @param exceptfds FDs to check for exceptions (may be NULL)
 * @param timeout Timeout (NULL = wait forever, {0,0} = non-blocking)
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int iox_select(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
               linux_fd_set_t *exceptfds, struct linux_timeval *timeout) {
    /* Validate nfds */
    if (nfds < 0 || nfds > IOX_FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }

    /* Handle empty FD sets */
    if (nfds == 0 || (readfds == NULL && writefds == NULL && exceptfds == NULL)) {
        if (timeout) {
            /* Just sleep for the timeout duration */
            struct timespec ts;
            ts.tv_sec = timeout->tv_sec;
            ts.tv_nsec = timeout->tv_usec * 1000;
            nanosleep(&ts, NULL);
        }
        return 0;
    }

    /* Create kqueue */
    int kq = kqueue();
    if (kq < 0) {
        errno = ENOMEM;
        return -1;
    }

    /* Count FDs and build changelist */
    int max_changes = nfds * 3; /* read + write + except per fd */
    struct kevent *changelist = calloc(max_changes, sizeof(struct kevent));
    if (!changelist) {
        close(kq);
        errno = ENOMEM;
        return -1;
    }

    int nchanges = 0;
    int fd_count = 0;

    for (int fd = 0; fd < nfds; fd++) {
        int events_added = 0;

        /* Add read filter */
        if (readfds && IOX_FD_ISSET(fd, readfds)) {
            /* Check if fd is valid */
            if (fcntl(fd, F_GETFL) < 0) {
                free(changelist);
                close(kq);
                errno = EBADF;
                return -1;
            }
            EV_SET(&changelist[nchanges], fd, EVFILT_READ, EV_ADD, 0, 0,
                   (void *)(intptr_t)(fd + 1)); /* +1 to distinguish from NULL */
            nchanges++;
            events_added++;
        }

        /* Add write filter */
        if (writefds && IOX_FD_ISSET(fd, writefds)) {
            if (fcntl(fd, F_GETFL) < 0) {
                free(changelist);
                close(kq);
                errno = EBADF;
                return -1;
            }
            EV_SET(&changelist[nchanges], fd, EVFILT_WRITE, EV_ADD, 0, 0,
                   (void *)(intptr_t)(fd + 1));
            nchanges++;
            events_added++;
        }

        /* Add except filter (use EVFILT_READ with EXCEPT flag) */
        if (exceptfds && IOX_FD_ISSET(fd, exceptfds)) {
            if (fcntl(fd, F_GETFL) < 0) {
                free(changelist);
                close(kq);
                errno = EBADF;
                return -1;
            }
            EV_SET(&changelist[nchanges], fd, EVFILT_READ, EV_ADD | EV_OOBAND, 0, 0,
                   (void *)(intptr_t)(fd + 1));
            nchanges++;
            events_added++;
        }

        if (events_added > 0) {
            fd_count++;
        }
    }

    /* If no FDs to monitor, just handle timeout */
    if (nchanges == 0) {
        free(changelist);
        close(kq);
        if (timeout) {
            struct timespec ts;
            ts.tv_sec = timeout->tv_sec;
            ts.tv_nsec = timeout->tv_usec * 1000;
            nanosleep(&ts, NULL);
        }
        return 0;
    }

    /* Prepare timeout */
    struct timespec ts;
    struct timespec *tsp = NULL;
    if (timeout) {
        ts.tv_sec = timeout->tv_sec;
        ts.tv_nsec = timeout->tv_usec * 1000;
        tsp = &ts;
    }

    /* Event list for results */
    struct kevent *eventlist = calloc(nchanges, sizeof(struct kevent));
    if (!eventlist) {
        free(changelist);
        close(kq);
        errno = ENOMEM;
        return -1;
    }

    /* Clear output fd_sets */
    if (readfds)
        IOX_FD_ZERO(readfds);
    if (writefds)
        IOX_FD_ZERO(writefds);
    if (exceptfds)
        IOX_FD_ZERO(exceptfds);

    /* Wait for events */
    int nevents = kevent(kq, changelist, nchanges, eventlist, nchanges, tsp);

    if (nevents < 0) {
        int saved_errno = errno;
        free(changelist);
        free(eventlist);
        close(kq);
        errno = (saved_errno == EINTR) ? EINTR : EBADF;
        return -1;
    }

    /* Process results */
    int ready_count = 0;
    for (int i = 0; i < nevents; i++) {
        int fd = (int)(intptr_t)eventlist[i].udata - 1;
        if (fd < 0 || fd >= nfds)
            continue;

        if (eventlist[i].filter == EVFILT_READ) {
            if (eventlist[i].flags & EV_OOBAND) {
                /* Exception condition */
                if (exceptfds)
                    IOX_FD_SET(fd, exceptfds);
            } else {
                /* Read ready */
                if (readfds)
                    IOX_FD_SET(fd, readfds);
            }
            ready_count++;
        } else if (eventlist[i].filter == EVFILT_WRITE) {
            /* Write ready */
            if (writefds)
                IOX_FD_SET(fd, writefds);
            ready_count++;
        }
    }

    free(changelist);
    free(eventlist);
    close(kq);

    return ready_count;
}

/**
 * @brief Synchronous I/O multiplexing with signal mask
 */
int iox_pselect(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
                linux_fd_set_t *exceptfds, const struct linux_timespec *timeout,
                const linux_sigset_t *sigmask) {
    /* Convert timeout */
    struct linux_timeval tv;
    struct linux_timeval *tvp = NULL;
    if (timeout) {
        tv.tv_sec = timeout->tv_sec;
        tv.tv_usec = timeout->tv_nsec / 1000;
        tvp = &tv;
    }

    /* Save current signal mask if needed */
    sigset_t oldmask;
    sigset_t newmask;
    if (sigmask) {
        /* Convert linux_sigset_t (128 bytes) to native sigset_t */
        memset(&newmask, 0, sizeof(newmask));
        memcpy(&newmask, sigmask, sizeof(newmask) < 128 ? sizeof(newmask) : 128);
        pthread_sigmask(SIG_SETMASK, &newmask, &oldmask);
    }

    /* Call select */
    int result = iox_select(nfds, readfds, writefds, exceptfds, tvp);

    /* Restore signal mask */
    if (sigmask) {
        pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
    }

    return result;
}

/* ============================================================================
 * EPOLL IMPLEMENTATION
 * ============================================================================
 *
 * epoll is implemented using kqueue as the backend on iOS.
 * Each epoll instance creates a kqueue, and epoll_ctl operations
 * map to kevent changes. epoll_wait maps to kevent with timeout.
 */

/* Epoll item - represents a registered file descriptor */
typedef struct iox_epitem {
    int fd;                     /* Registered file descriptor */
    struct epoll_event event;   /* Registered events */
    uint32_t registered_events; /* Events currently registered with kqueue */
    bool is_registered;         /* Is this item active? */
    bool edge_triggered;        /* EPOLLET mode */
    struct iox_epitem *next;    /* Hash table chaining */
    struct iox_epitem *prev;    /* Hash table chaining */
} iox_epitem_t;

/* Epoll instance structure */
typedef struct iox_epoll_instance {
    int kq;               /* kqueue descriptor */
    uint32_t flags;       /* Creation flags (EPOLL_CLOEXEC) */
    int size;             /* Original size hint */
    atomic_int ref_count; /* Reference count */
    pthread_mutex_t lock; /* Instance lock */

    /* Hash table for fast fd lookup (size is power of 2) */
    iox_epitem_t **items; /* Hash table buckets */
    int hash_size;        /* Number of buckets */
    int item_count;       /* Number of registered items */

    /* Stats */
    uint64_t total_events; /* Total events returned */
} iox_epoll_instance_t;

/* Hash function for fd -> bucket */
static inline int iox_epoll_hash(iox_epoll_instance_t *ep, int fd) {
    return fd & (ep->hash_size - 1);
}

/* Find epitem by fd */
static iox_epitem_t *iox_epoll_find_item(iox_epoll_instance_t *ep, int fd) {
    int idx = iox_epoll_hash(ep, fd);
    iox_epitem_t *item = ep->items[idx];

    while (item) {
        if (item->fd == fd) {
            return item;
        }
        item = item->next;
    }
    return NULL;
}

/* Add item to hash table */
static void iox_epoll_add_item(iox_epoll_instance_t *ep, iox_epitem_t *item) {
    int idx = iox_epoll_hash(ep, item->fd);
    item->next = ep->items[idx];
    item->prev = NULL;
    if (ep->items[idx]) {
        ep->items[idx]->prev = item;
    }
    ep->items[idx] = item;
    ep->item_count++;
}

/* Remove item from hash table */
static void iox_epoll_remove_item(iox_epoll_instance_t *ep, iox_epitem_t *item) {
    if (item->prev) {
        item->prev->next = item->next;
    } else {
        int idx = iox_epoll_hash(ep, item->fd);
        ep->items[idx] = item->next;
    }
    if (item->next) {
        item->next->prev = item->prev;
    }
    ep->item_count--;
}

/* Map epoll events to kqueue filter flags */
static uint32_t iox_epoll_to_kqueue_flags(uint32_t epoll_events) {
    uint32_t kflags = 0;

    if (epoll_events & EPOLLET) {
        kflags |= EV_CLEAR; /* Edge-triggered */
    }
    if (epoll_events & EPOLLONESHOT) {
        kflags |= EV_ONESHOT;
    }

    return kflags;
}

/* Convert epoll events to kqueue filters and build kevent changes */
static int iox_epoll_build_kevents(struct kevent *changes, int max_changes, int fd,
                                   uint32_t epoll_events, void *udata, bool add_mode) {
    int n = 0;
    uint32_t kflags = iox_epoll_to_kqueue_flags(epoll_events);

    /* Read events */
    if (epoll_events & (EPOLLIN | EPOLLRDNORM | EPOLLRDBAND | EPOLLPRI)) {
        if (n < max_changes) {
            EV_SET(&changes[n], fd, EVFILT_READ, add_mode ? EV_ADD : EV_DELETE, 0, 0, udata);
            if (add_mode) {
                changes[n].flags |= kflags;
            }
            n++;
        }
    }

    /* Write events */
    if (epoll_events & (EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND)) {
        if (n < max_changes) {
            EV_SET(&changes[n], fd, EVFILT_WRITE, add_mode ? EV_ADD : EV_DELETE, 0, 0, udata);
            if (add_mode) {
                changes[n].flags |= kflags;
            }
            n++;
        }
    }

    return n;
}

/* Convert kqueue filter to epoll events */
static uint32_t iox_kqueue_to_epoll_events(int16_t filter, uint16_t flags, int data) {
    uint32_t events = 0;

    if (filter == EVFILT_READ) {
        events |= EPOLLIN;
        if (flags & EV_OOBAND) {
            events |= EPOLLPRI;
        }
    } else if (filter == EVFILT_WRITE) {
        events |= EPOLLOUT;
    }

    if (flags & EV_ERROR) {
        events |= EPOLLERR;
    }
    if (flags & EV_EOF) {
        events |= EPOLLHUP;
        /* For sockets, RDHUP might also be set */
        events |= EPOLLRDHUP;
    }

    return events;
}

/* Static table to track epoll instances (epfd -> instance) */
#define IOX_MAX_EPOLL_INSTANCES 256
static iox_epoll_instance_t *iox_epoll_table[IOX_MAX_EPOLL_INSTANCES];
static pthread_mutex_t iox_epoll_table_lock = PTHREAD_MUTEX_INITIALIZER;

/* Register epoll instance */
static int iox_epoll_register_instance(iox_epoll_instance_t *ep) {
    pthread_mutex_lock(&iox_epoll_table_lock);
    for (int i = 0; i < IOX_MAX_EPOLL_INSTANCES; i++) {
        if (iox_epoll_table[i] == NULL) {
            iox_epoll_table[i] = ep;
            pthread_mutex_unlock(&iox_epoll_table_lock);
            return i + IOX_MAX_FD + 100; /* Reserve range for epoll fds */
        }
    }
    pthread_mutex_unlock(&iox_epoll_table_lock);
    return -1;
}

/* Lookup epoll instance by epfd */
static iox_epoll_instance_t *iox_epoll_lookup_instance(int epfd) {
    int idx = epfd - IOX_MAX_FD - 100;
    if (idx < 0 || idx >= IOX_MAX_EPOLL_INSTANCES) {
        return NULL;
    }
    return iox_epoll_table[idx];
}

/* Unregister epoll instance */
static void iox_epoll_unregister_instance(int epfd) {
    int idx = epfd - IOX_MAX_FD - 100;
    if (idx >= 0 && idx < IOX_MAX_EPOLL_INSTANCES) {
        pthread_mutex_lock(&iox_epoll_table_lock);
        iox_epoll_table[idx] = NULL;
        pthread_mutex_unlock(&iox_epoll_table_lock);
    }
}

/**
 * @brief Create a new epoll instance
 *
 * @param size Size hint (ignored on modern kernels, but must be > 0)
 * @return int Epoll file descriptor on success, -1 on error with errno set
 */
int iox_epoll_create(int size) {
    if (size <= 0) {
        errno = EINVAL;
        return -1;
    }

    return iox_epoll_create1(0);
}

/**
 * @brief Create a new epoll instance with flags
 *
 * @param flags EPOLL_CLOEXEC or 0
 * @return int Epoll file descriptor on success, -1 on error with errno set
 */
int iox_epoll_create1(int flags) {
    /* Validate flags */
    if (flags & ~EPOLL_CLOEXEC) {
        errno = EINVAL;
        return -1;
    }

    /* Allocate instance */
    iox_epoll_instance_t *ep = calloc(1, sizeof(iox_epoll_instance_t));
    if (!ep) {
        errno = ENOMEM;
        return -1;
    }

    /* Create kqueue */
    ep->kq = kqueue();
    if (ep->kq < 0) {
        free(ep);
        errno = EMFILE; /* Or ENFILE depending on the error */
        return -1;
    }

    /* Set CLOEXEC if requested */
    if (flags & EPOLL_CLOEXEC) {
        int fd_flags = fcntl(ep->kq, F_GETFD);
        if (fd_flags >= 0) {
            fcntl(ep->kq, F_SETFD, fd_flags | FD_CLOEXEC);
        }
    }

    /* Initialize hash table (start with 16 buckets) */
    ep->hash_size = 16;
    ep->items = calloc(ep->hash_size, sizeof(iox_epitem_t *));
    if (!ep->items) {
        close(ep->kq);
        free(ep);
        errno = ENOMEM;
        return -1;
    }

    ep->flags = (uint32_t)flags;
    ep->item_count = 0;
    ep->total_events = 0;
    atomic_init(&ep->ref_count, 1);
    pthread_mutex_init(&ep->lock, NULL);

    /* Register in table and get epfd */
    int epfd = iox_epoll_register_instance(ep);
    if (epfd < 0) {
        free(ep->items);
        close(ep->kq);
        free(ep);
        errno = EMFILE;
        return -1;
    }

    return epfd;
}

/**
 * @brief Control interface for an epoll file descriptor
 *
 * @param epfd Epoll file descriptor
 * @param op Operation (EPOLL_CTL_ADD, EPOLL_CTL_DEL, EPOLL_CTL_MOD)
 * @param fd Target file descriptor
 * @param event Associated event information (can be NULL for EPOLL_CTL_DEL)
 * @return int 0 on success, -1 on error with errno set
 */
int iox_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    /* Validate epfd */
    iox_epoll_instance_t *ep = iox_epoll_lookup_instance(epfd);
    if (!ep) {
        errno = EBADF;
        return -1;
    }

    /* Validate fd */
    if (fd < 0 || fcntl(fd, F_GETFL) < 0) {
        errno = EBADF;
        return -1;
    }

    /* epfd and fd cannot be the same */
    if (epfd == fd) {
        errno = EINVAL;
        return -1;
    }

    pthread_mutex_lock(&ep->lock);

    iox_epitem_t *item = iox_epoll_find_item(ep, fd);

    switch (op) {
    case EPOLL_CTL_ADD: {
        /* Event must be provided */
        if (!event) {
            pthread_mutex_unlock(&ep->lock);
            errno = EINVAL;
            return -1;
        }

        /* Check if already registered */
        if (item != NULL) {
            pthread_mutex_unlock(&ep->lock);
            errno = EEXIST;
            return -1;
        }

        /* Create new item */
        item = calloc(1, sizeof(iox_epitem_t));
        if (!item) {
            pthread_mutex_unlock(&ep->lock);
            errno = ENOMEM;
            return -1;
        }

        item->fd = fd;
        memcpy(&item->event, event, sizeof(struct epoll_event));
        item->is_registered = true;
        item->edge_triggered = (event->events & EPOLLET) != 0;

        /* Build kevent changes */
        struct kevent changes[2];
        int nchanges =
            iox_epoll_build_kevents(changes, 2, fd, event->events & ~EPOLLONESHOT, item, true);

        if (nchanges > 0) {
            int ret = kevent(ep->kq, changes, nchanges, NULL, 0, NULL);
            if (ret < 0) {
                free(item);
                pthread_mutex_unlock(&ep->lock);
                errno = EPERM; /* FD doesn't support epoll */
                return -1;
            }
        }

        /* Add to hash table */
        iox_epoll_add_item(ep, item);
        break;
    }

    case EPOLL_CTL_DEL: {
        /* Check if registered */
        if (item == NULL) {
            pthread_mutex_unlock(&ep->lock);
            errno = ENOENT;
            return -1;
        }

        /* Remove from kqueue */
        struct kevent changes[2];
        int nchanges =
            iox_epoll_build_kevents(changes, 2, fd, item->registered_events, item, false);
        if (nchanges > 0) {
            kevent(ep->kq, changes, nchanges, NULL, 0, NULL);
        }

        /* Remove from hash table and free */
        iox_epoll_remove_item(ep, item);
        free(item);
        break;
    }

    case EPOLL_CTL_MOD: {
        /* Event must be provided */
        if (!event) {
            pthread_mutex_unlock(&ep->lock);
            errno = EINVAL;
            return -1;
        }

        /* Check if registered */
        if (item == NULL) {
            pthread_mutex_unlock(&ep->lock);
            errno = ENOENT;
            return -1;
        }

        /* Delete old registration first */
        struct kevent changes[2];
        int nchanges =
            iox_epoll_build_kevents(changes, 2, fd, item->registered_events, item, false);
        if (nchanges > 0) {
            kevent(ep->kq, changes, nchanges, NULL, 0, NULL);
        }

        /* Update item */
        memcpy(&item->event, event, sizeof(struct epoll_event));
        item->edge_triggered = (event->events & EPOLLET) != 0;

        /* Add new registration */
        nchanges =
            iox_epoll_build_kevents(changes, 2, fd, event->events & ~EPOLLONESHOT, item, true);
        if (nchanges > 0) {
            int ret = kevent(ep->kq, changes, nchanges, NULL, 0, NULL);
            if (ret < 0) {
                pthread_mutex_unlock(&ep->lock);
                errno = EPERM;
                return -1;
            }
        }
        break;
    }

    default:
        pthread_mutex_unlock(&ep->lock);
        errno = EINVAL;
        return -1;
    }

    pthread_mutex_unlock(&ep->lock);
    return 0;
}

/**
 * @brief Wait for an I/O event on an epoll file descriptor
 *
 * @param epfd Epoll file descriptor
 * @param events Array to store returned events
 * @param maxevents Maximum number of events to return
 * @param timeout Timeout in milliseconds (-1 = wait forever, 0 = non-blocking)
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int iox_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
    return iox_epoll_pwait(epfd, events, maxevents, timeout, NULL);
}

/**
 * @brief Wait for events with signal mask
 */
int iox_epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout,
                    const struct iox_sigset *sigmask) {
    /* Validate parameters */
    if (!events || maxevents <= 0) {
        errno = EINVAL;
        return -1;
    }

    /* Validate epfd */
    iox_epoll_instance_t *ep = iox_epoll_lookup_instance(epfd);
    if (!ep) {
        errno = EBADF;
        return -1;
    }

    /* Save and set signal mask if provided */
    sigset_t oldmask;
    sigset_t newmask;
    if (sigmask) {
        /* Convert linux_sigset_t (128 bytes) to native sigset_t */
        memset(&newmask, 0, sizeof(newmask));
        memcpy(&newmask, sigmask, sizeof(newmask) < 128 ? sizeof(newmask) : 128);
        pthread_sigmask(SIG_SETMASK, &newmask, &oldmask);
    }

    /* Prepare timeout */
    struct timespec ts;
    struct timespec *tsp = NULL;
    if (timeout >= 0) {
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = (timeout % 1000) * 1000000;
        tsp = &ts;
    }

    /* Allocate event list */
    struct kevent *kevents = calloc(maxevents, sizeof(struct kevent));
    if (!kevents) {
        if (sigmask) {
            pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
        }
        errno = ENOMEM;
        return -1;
    }

    /* Wait for events */
    int nevents = kevent(ep->kq, NULL, 0, kevents, maxevents, tsp);

    if (nevents < 0) {
        int saved_errno = errno;
        free(kevents);
        if (sigmask) {
            pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
        }
        errno = (saved_errno == EINTR) ? EINTR : EBADF;
        return -1;
    }

    /* Process results */
    int ready_count = 0;
    for (int i = 0; i < nevents && ready_count < maxevents; i++) {
        iox_epitem_t *item = (iox_epitem_t *)kevents[i].udata;
        if (!item)
            continue;

        events[ready_count].events =
            iox_kqueue_to_epoll_events(kevents[i].filter, kevents[i].flags, (int)kevents[i].data);

        /* Copy user data from the registered event */
        memcpy(&events[ready_count].data, &item->event.data, sizeof(epoll_data_t));

        /* Handle EPOLLONESHOT - remove after trigger */
        if (item->event.events & EPOLLONESHOT) {
            pthread_mutex_lock(&ep->lock);
            struct kevent changes[2];
            int nchanges =
                iox_epoll_build_kevents(changes, 2, item->fd, item->registered_events, item, false);
            if (nchanges > 0) {
                kevent(ep->kq, changes, nchanges, NULL, 0, NULL);
            }
            item->is_registered = false;
            pthread_mutex_unlock(&ep->lock);
        }

        ready_count++;
    }

    free(kevents);

    if (sigmask) {
        pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
    }

    ep->total_events += ready_count;
    return ready_count;
}

/**
 * @brief Wait for events with nanosecond timeout
 */
int iox_epoll_pwait2(int epfd, struct epoll_event *events, int maxevents,
                     const struct iox_timespec *timeout, const struct iox_sigset *sigmask) {
    /* Convert timeout to milliseconds */
    int timeout_ms = -1;
    if (timeout) {
        timeout_ms = (int)(timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000);
    }

    return iox_epoll_pwait(epfd, events, maxevents, timeout_ms, (const struct iox_sigset *)sigmask);
}

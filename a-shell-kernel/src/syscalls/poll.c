/*
 * poll.c - Poll and epoll syscalls
 *
 * poll() uses native poll()
 * epoll() emulated via kqueue on iOS
 */

#include <poll.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../include/linux/poll.h"
#include "../../include/linux/epoll.h"

/* ============================================================================
 * Poll Operations
 * ============================================================================ */

int a_shell_poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    return poll(fds, nfds, timeout);
}

int a_shell_ppoll(struct pollfd *fds, nfds_t nfds, 
                  const struct timespec *tmo_p, const sigset_t *sigmask) {
    /* iOS doesn't have ppoll, emulate via pselect or poll */
    int timeout = -1;
    if (tmo_p) {
        timeout = tmo_p->tv_sec * 1000 + tmo_p->tv_nsec / 1000000;
    }
    
    /* TODO: Handle sigmask if non-NULL */
    return poll(fds, nfds, timeout);
}

/* ============================================================================
 * epoll Context
 * ============================================================================ */

/* epoll is Linux-specific; on iOS we emulate it using kqueue */

typedef struct epoll_context {
    int kq;                 /* kqueue file descriptor */
    struct epoll_event *events;  /* Registered events */
    int num_events;
    int max_events;
} epoll_context_t;

static epoll_context_t *epoll_contexts[1024] = {NULL};
static pthread_mutex_t epoll_mutex = PTHREAD_MUTEX_INITIALIZER;

static epoll_context_t *get_epoll_context(int epfd) {
    if (epfd < 0 || epfd >= 1024) {
        return NULL;
    }
    return epoll_contexts[epfd];
}

static int set_epoll_context(int epfd, epoll_context_t *ctx) {
    if (epfd < 0 || epfd >= 1024) {
        return -1;
    }
    epoll_contexts[epfd] = ctx;
    return 0;
}

/* Convert epoll events to kqueue filters */
static int16_t epoll_to_kqueue_filter(uint32_t events) {
    if (events & EPOLLIN) {
        return EVFILT_READ;
    } else if (events & EPOLLOUT) {
        return EVFILT_WRITE;
    }
    return EVFILT_READ;  /* Default */
}

/* Convert kqueue filters to epoll events */
static uint32_t kqueue_to_epoll_events(int16_t filter, uint16_t flags) {
    uint32_t events = 0;
    
    if (filter == EVFILT_READ) {
        events |= EPOLLIN;
    } else if (filter == EVFILT_WRITE) {
        events |= EPOLLOUT;
    }
    
    if (flags & EV_ERROR) {
        events |= EPOLLERR;
    }
    if (flags & EV_EOF) {
        events |= EPOLLHUP;
    }
    
    return events;
}

/* ============================================================================
 * epoll Operations
 * ============================================================================ */

int a_shell_epoll_create(int size) {
    return a_shell_epoll_create1(0);
}

int a_shell_epoll_create1(int flags) {
    int kq = kqueue();
    if (kq < 0) {
        return -1;
    }
    
    /* Set close-on-exec if requested */
    if (flags & EPOLL_CLOEXEC) {
        int fd_flags = fcntl(kq, F_GETFD);
        if (fd_flags >= 0) {
            fcntl(kq, F_SETFD, fd_flags | FD_CLOEXEC);
        }
    }
    
    pthread_mutex_lock(&epoll_mutex);
    
    /* Allocate context */
    epoll_context_t *ctx = malloc(sizeof(epoll_context_t));
    if (!ctx) {
        pthread_mutex_unlock(&epoll_mutex);
        close(kq);
        errno = ENOMEM;
        return -1;
    }
    
    ctx->kq = kq;
    ctx->events = NULL;
    ctx->num_events = 0;
    ctx->max_events = 0;
    
    if (set_epoll_context(kq, ctx) < 0) {
        pthread_mutex_unlock(&epoll_mutex);
        free(ctx);
        close(kq);
        errno = EMFILE;
        return -1;
    }
    
    pthread_mutex_unlock(&epoll_mutex);
    return kq;
}

int a_shell_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    epoll_context_t *ctx = get_epoll_context(epfd);
    if (!ctx) {
        errno = EINVAL;
        return -1;
    }
    
    struct kevent kev;
    uint16_t kev_flags = 0;
    int16_t filter = 0;
    
    switch (op) {
        case EPOLL_CTL_ADD:
            kev_flags = EV_ADD;
            if (event->events & EPOLLET) {
                kev_flags |= EV_CLEAR;
            }
            if (event->events & EPOLLONESHOT) {
                kev_flags |= EV_ONESHOT;
            }
            filter = epoll_to_kqueue_filter(event->events);
            break;
            
        case EPOLL_CTL_DEL:
            /* Need to delete both read and write filters */
            EV_SET(&kev, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            kevent(ctx->kq, &kev, 1, NULL, 0, NULL);
            EV_SET(&kev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            return kevent(ctx->kq, &kev, 1, NULL, 0, NULL);
            
        case EPOLL_CTL_MOD:
            kev_flags = EV_ADD;
            if (event->events & EPOLLET) {
                kev_flags |= EV_CLEAR;
            }
            if (event->events & EPOLLONESHOT) {
                kev_flags |= EV_ONESHOT;
            }
            filter = epoll_to_kqueue_filter(event->events);
            break;
            
        default:
            errno = EINVAL;
            return -1;
    }
    
    EV_SET(&kev, fd, filter, kev_flags, 0, 0, event->data.ptr);
    return kevent(ctx->kq, &kev, 1, NULL, 0, NULL);
}

int a_shell_epoll_wait(int epfd, struct epoll_event *events, 
                       int maxevents, int timeout) {
    return a_shell_epoll_pwait(epfd, events, maxevents, timeout, NULL);
}

int a_shell_epoll_pwait(int epfd, struct epoll_event *events, 
                        int maxevents, int timeout, const sigset_t *sigmask) {
    epoll_context_t *ctx = get_epoll_context(epfd);
    if (!ctx) {
        errno = EINVAL;
        return -1;
    }
    
    struct kevent kevs[maxevents];
    struct timespec ts;
    struct timespec *tsp = NULL;
    
    if (timeout >= 0) {
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = (timeout % 1000) * 1000000;
        tsp = &ts;
    }
    
    /* TODO: Handle sigmask if non-NULL */
    
    int n = kevent(ctx->kq, NULL, 0, kevs, maxevents, tsp);
    if (n < 0) {
        return -1;
    }
    
    /* Convert kqueue events to epoll events */
    for (int i = 0; i < n; i++) {
        events[i].data.ptr = kevs[i].udata;
        events[i].events = kqueue_to_epoll_events(kevs[i].filter, kevs[i].flags);
    }
    
    return n;
}

/* iXland libc - Linux-compatible poll.h
 *
 * Polling definitions matching Linux kernel.
 * Provides poll/select interfaces for I/O multiplexing.
 */

#ifndef _LINUX_POLL_H
#define _LINUX_POLL_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * POLL EVENT CONSTANTS
 * ============================================================================ */

/* Poll events (compatible with Linux poll.h) */
#define IXLAND_POLLIN 0x001   /* There is data to read */
#define IXLAND_POLLPRI 0x002  /* There is urgent data to read */
#define IXLAND_POLLOUT 0x004  /* Writing now will not block */
#define IXLAND_POLLERR 0x008  /* Error condition */
#define IXLAND_POLLHUP 0x010  /* Hung up */
#define IXLAND_POLLNVAL 0x020 /* Invalid polling request */

/* Extended poll events */
#define IXLAND_POLLRDNORM 0x040 /* Normal data may be read */
#define IXLAND_POLLRDBAND 0x080 /* Priority data may be read */
#define IXLAND_POLLWRNORM 0x100 /* Writing now will not block */
#define IXLAND_POLLWRBAND 0x200 /* Priority data may be written */
#define IXLAND_POLLMSG 0x400    /* Signal message received */
#define IXLAND_POLLREMOVE 0x800 /* Remove from watched list (epoll internal) */
#define IXLAND_POLLRDHUP 0x2000 /* Peer closed connection */

/* Compatibility aliases (standard names) */
#define POLLIN IXLAND_POLLIN
#define POLLPRI IXLAND_POLLPRI
#define POLLOUT IXLAND_POLLOUT
#define POLLERR IXLAND_POLLERR
#define POLLHUP IXLAND_POLLHUP
#define POLLNVAL IXLAND_POLLNVAL
#define POLLRDNORM IXLAND_POLLRDNORM
#define POLLRDBAND IXLAND_POLLRDBAND
#define POLLWRNORM IXLAND_POLLWRNORM
#define POLLWRBAND IXLAND_POLLWRBAND
#define POLLMSG IXLAND_POLLMSG
#define POLLREMOVE IXLAND_POLLREMOVE
#define POLLRDHUP IXLAND_POLLRDHUP

/* Set of events that can be specified for poll() */
#define IXLAND_POLLIN_SET (IXLAND_POLLIN | IXLAND_POLLRDNORM | IXLAND_POLLRDBAND)
#define IXLAND_POLLOUT_SET (IXLAND_POLLOUT | IXLAND_POLLWRNORM | IXLAND_POLLWRBAND)
#define IXLAND_POLLERR_SET (IXLAND_POLLERR | IXLAND_POLLHUP | IXLAND_POLLNVAL)

/* Input event mask (events user can request) */
#define IXLAND_POLLIN_EVENTS                                                                   \
    (IXLAND_POLLIN | IXLAND_POLLPRI | IXLAND_POLLOUT | IXLAND_POLLRDNORM | IXLAND_POLLRDBAND | \
     IXLAND_POLLWRNORM | IXLAND_POLLWRBAND | IXLAND_POLLRDHUP)

/* Output event mask (events that can be returned) */
#define IXLAND_POLLOUT_EVENTS \
    (IXLAND_POLLIN_EVENTS | IXLAND_POLLERR | IXLAND_POLLHUP | IXLAND_POLLNVAL | IXLAND_POLLMSG)

/* ============================================================================
 * POLL STRUCTURES
 * ============================================================================ */

/* Poll file descriptor structure - Linux-compatible */
struct linux_pollfd {
    int fd;        /* File descriptor to poll */
    short events;  /* Requested events */
    short revents; /* Returned events */
};

/* Legacy BSD struct (for compatibility) - defined in linux/types.h if included first */
#ifndef IXLAND_POLLFD_DEFINED
struct ixland_pollfd {
    int fd;
    short events;
    short revents;
};
#endif

/* Compatibility alias */
#define pollfd linux_pollfd

/* Poll list structure for ppoll */
struct linux_poll_list {
    struct linux_pollfd *entries;
    unsigned int count;
};

/* ============================================================================
 * TIMEOUT VALUES
 * ============================================================================ */

/* Special timeout values */
#define IXLAND_POLL_WAIT_FOREVER (-1) /* Wait indefinitely */
#define IXLAND_POLL_WAIT_NONE 0       /* Return immediately */

/* Standard timeout */
#define IXLAND_POLL_TIMEOUT_MS 1000 /* Default timeout in milliseconds */

/* ============================================================================
 * SELECT MACROS
 * ============================================================================ */

/* FD set size for select() */
#ifndef IXLAND_FD_SETSIZE
#define IXLAND_FD_SETSIZE 1024
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE IXLAND_FD_SETSIZE
#endif

/* Calculate number of longs needed for fd_set */
#define IXLAND_NFDBITS (8 * sizeof(unsigned long))
#define IXLAND_FDELT(d) ((d) / IXLAND_NFDBITS)
#define IXLAND_FDMASK(d) (1UL << ((d) % IXLAND_NFDBITS))

/* FD set structure - Linux-compatible */
typedef struct {
    unsigned long fds_bits[IXLAND_FD_SETSIZE / IXLAND_NFDBITS];
} linux_fd_set_t;

/* Compatibility alias */
#define fd_set linux_fd_set_t

/* FD set manipulation macros - guard against ixland/sys/types.h versions */
#ifndef IXLAND_FD_ZERO
#define IXLAND_FD_ZERO(set)                                              \
    do {                                                                 \
        unsigned int __i;                                                \
        for (__i = 0; __i < (IXLAND_FD_SETSIZE / IXLAND_NFDBITS); __i++) \
            (set)->fds_bits[__i] = 0;                                    \
    } while (0)
#endif

#ifndef IXLAND_FD_SET
#define IXLAND_FD_SET(fd, set)                                      \
    do {                                                            \
        if ((unsigned int)(fd) < IXLAND_FD_SETSIZE)                 \
            (set)->fds_bits[IXLAND_FDELT(fd)] |= IXLAND_FDMASK(fd); \
    } while (0)
#endif

#ifndef IXLAND_FD_CLR
#define IXLAND_FD_CLR(fd, set)                                       \
    do {                                                             \
        if ((unsigned int)(fd) < IXLAND_FD_SETSIZE)                  \
            (set)->fds_bits[IXLAND_FDELT(fd)] &= ~IXLAND_FDMASK(fd); \
    } while (0)
#endif

#ifndef IXLAND_FD_ISSET
#define IXLAND_FD_ISSET(fd, set)                                   \
    (((unsigned int)(fd) < IXLAND_FD_SETSIZE)                      \
         ? ((set)->fds_bits[IXLAND_FDELT(fd)] & IXLAND_FDMASK(fd)) \
         : 0)
#endif

/* ============================================================================
 * TIMEOUT STRUCTURES
 * ============================================================================ */

/* Timeout for ppoll - uses linux_timespec from linux/time.h */
#ifndef _LINUX_TIMESPEC_DEFINED
#define _LINUX_TIMESPEC_DEFINED
struct linux_timespec {
    int64_t tv_sec;  /* Seconds */
    int64_t tv_nsec; /* Nanoseconds */
};

/* Timeval for select */
struct linux_timeval {
    int64_t tv_sec;  /* Seconds */
    int64_t tv_usec; /* Microseconds */
};
#endif /* _LINUX_TIMESPEC_DEFINED */

/* ============================================================================
 * SIGNAL MASK FOR PPOLL
 * ============================================================================ */

/* Signal set for ppoll */
typedef struct {
    unsigned long sig[128 / sizeof(unsigned long)];
} linux_sigset_t;

/* Compatibility alias */
#define sigset_t linux_sigset_t

/* ============================================================================
 * POLL MODES
 * ============================================================================ */

/* Poll modes for optimized implementations */
#define IXLAND_POLL_MODE_LEVEL 0 /* Level-triggered (default) */
#define IXLAND_POLL_MODE_EDGE 1  /* Edge-triggered */

/* ============================================================================
 * POLL STATISTICS
 * ============================================================================ */

/* Poll statistics structure */
struct ixland_poll_stats {
    uint64_t poll_calls;      /* Number of poll() calls */
    uint64_t poll_returns;    /* Number of returns */
    uint64_t poll_events;     /* Total events returned */
    uint64_t poll_timeouts;   /* Number of timeouts */
    uint64_t poll_interrupts; /* Number of EINTR returns */
    uint64_t poll_time_total; /* Total time waiting (ns) */
};

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Wait for some event on a file descriptor
 *
 * @param fds Array of pollfd structures
 * @param nfds Number of file descriptors
 * @param timeout Timeout in milliseconds (-1 = forever, 0 = non-blocking)
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int ixland_poll(struct linux_pollfd *fds, unsigned int nfds, int timeout);

/**
 * @brief Wait for some event on a file descriptor (with signal mask)
 *
 * @param fds Array of pollfd structures
 * @param nfds Number of file descriptors
 * @param timeout Timeout (NULL = forever)
 * @param sigmask Signal mask to restore during wait
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int ixland_ppoll(struct linux_pollfd *fds, unsigned int nfds, const struct linux_timespec *timeout,
                 const linux_sigset_t *sigmask);

/**
 * @brief Poll multiple file descriptors with advanced options
 *
 * @param fds Array of pollfd structures
 * @param nfds Number of file descriptors
 * @param timeout Timeout in milliseconds
 * @param extra_flags Additional flags (implementation-specific)
 * @return int Number of ready fds on success, -1 on error
 */
int ixland_poll_advanced(struct linux_pollfd *fds, unsigned int nfds, int timeout, int extra_flags);

/* Extra poll flags */
#define IXLAND_POLL_ADV_NONBLOCK 0x01 /* Never block */
#define IXLAND_POLL_ADV_ONESHOT 0x02  /* One-shot notification */
#define IXLAND_POLL_ADV_WAKEUP 0x04   /* Wake up waiters */

/* ============================================================================
 * SELECT FUNCTIONS
 * ============================================================================ */

/**
 * @brief Synchronous I/O multiplexing
 *
 * @param nfds Highest-numbered FD + 1
 * @param readfds FDs to check for reading (may be NULL)
 * @param writefds FDs to check for writing (may be NULL)
 * @param exceptfds FDs to check for exceptions (may be NULL)
 * @param timeout Timeout (NULL = wait forever, {0,0} = non-blocking)
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int ixland_select(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
                  linux_fd_set_t *exceptfds, struct linux_timeval *timeout);

/**
 * @brief Synchronous I/O multiplexing (with signal mask)
 *
 * @param nfds Highest-numbered FD + 1
 * @param readfds FDs to check for reading (may be NULL)
 * @param writefds FDs to check for writing (may be NULL)
 * @param exceptfds FDs to check for exceptions (may be NULL)
 * @param timeout Timeout (may be NULL)
 * @param sigmask Signal mask to restore during wait
 * @return int Number of ready fds on success, -1 on error
 */
int ixland_pselect(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
                   linux_fd_set_t *exceptfds, const struct linux_timespec *timeout,
                   const linux_sigset_t *sigmask);

/**
 * @brief Extended select with additional options
 *
 * @param nfds Highest-numbered FD + 1
 * @param readfds FDs to check for reading
 * @param writefds FDs to check for writing
 * @param exceptfds FDs to check for exceptions
 * @param timeout Timeout
 * @param sigmask Signal mask
 * @param flags Additional flags
 * @return int Number of ready fds on success, -1 on error
 */
int ixland_pselect6(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
                    linux_fd_set_t *exceptfds, const struct linux_timespec *timeout,
                    const linux_sigset_t *sigmask, void *unused);

/* ============================================================================
 * HELPER MACROS
 * ============================================================================ */

/* Check if any FD is ready in a pollfd array */
#define IXLAND_POLL_HAS_EVENTS(pollfds, nfds)  \
    ({                                         \
        unsigned int __i;                      \
        int __found = 0;                       \
        for (__i = 0; __i < (nfds); __i++) {   \
            if ((pollfds)[__i].revents != 0) { \
                __found = 1;                   \
                break;                         \
            }                                  \
        }                                      \
        __found;                               \
    })

/* Count number of ready FDs in pollfd array */
#define IXLAND_POLL_COUNT_EVENTS(pollfds, nfds) \
    ({                                          \
        unsigned int __i, __count = 0;          \
        for (__i = 0; __i < (nfds); __i++) {    \
            if ((pollfds)[__i].revents != 0)    \
                __count++;                      \
        }                                       \
        __count;                                \
    })

/* Wait for input on a single FD */
#define IXLAND_POLL_WAIT_INPUT(fd, timeout_ms)                \
    ({                                                        \
        struct linux_pollfd __pfd = {(fd), IXLAND_POLLIN, 0}; \
        ixland_poll(&__pfd, 1, (timeout_ms));                 \
    })

/* Wait for output on a single FD */
#define IXLAND_POLL_WAIT_OUTPUT(fd, timeout_ms)                \
    ({                                                         \
        struct linux_pollfd __pfd = {(fd), IXLAND_POLLOUT, 0}; \
        ixland_poll(&__pfd, 1, (timeout_ms));                  \
    })

/* Check if pollfd has error events */
#define IXLAND_POLL_HAS_ERROR(pfd) \
    ((pfd)->revents & (IXLAND_POLLERR | IXLAND_POLLHUP | IXLAND_POLLNVAL))

/* Check if pollfd has input events */
#define IXLAND_POLL_HAS_INPUT(pfd) \
    ((pfd)->revents & (IXLAND_POLLIN | IXLAND_POLLRDNORM | IXLAND_POLLRDBAND | IXLAND_POLLRDHUP))

/* Check if pollfd has output events */
#define IXLAND_POLL_HAS_OUTPUT(pfd) \
    ((pfd)->revents & (IXLAND_POLLOUT | IXLAND_POLLWRNORM | IXLAND_POLLWRBAND))

/* ============================================================================
 * INTERNAL POLLING (for kernel use)
 * ============================================================================ */

/* Poll table for internal kernel use */
struct ixland_poll_table;

/* Poll callback type */
typedef int (*ixland_poll_callback_t)(struct linux_pollfd *pfd, void *data);

/* Poll wait queue structure */
struct ixland_poll_wait_queue {
    struct ixland_poll_wait_queue *next;
    struct ixland_poll_wait_queue *prev;
    void *private; /* Private data for wait */
};

/* Initialize poll table */
void ixland_poll_table_init(struct ixland_poll_table *table);

/* Free poll table */
void ixland_poll_table_free(struct ixland_poll_table *table);

/* Add wait queue to poll table */
void ixland_poll_table_add(struct ixland_poll_table *table, struct ixland_poll_wait_queue *wq);

/* Wake up poll waiters */
void ixland_poll_wake(struct ixland_poll_wait_queue *wq, int key);

/* ============================================================================
 * POLL NOTIFY
 * ============================================================================ */

/* Poll notify structure */
struct ixland_poll_notify {
    int fd;         /* File descriptor */
    short events;   /* Events to notify */
    void *userdata; /* User data */
};

/* Request poll notification */
int ixland_poll_request_notify(const struct ixland_poll_notify *notify);

/* Cancel poll notification */
int ixland_poll_cancel_notify(int fd);

/* ============================================================================
 * POLL VECTORED (for efficient batch polling)
 * ============================================================================ */

/* Vector poll structure */
struct ixland_poll_vec {
    struct linux_pollfd *fds; /* FD array */
    unsigned int nfds;        /* Number of FDs */
    void *userdata;           /* User context */
};

/* Batch poll operation */
int ixland_poll_vec_poll(struct ixland_poll_vec *vecs, unsigned int nvec, int timeout,
                         unsigned int *results);

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_POLL_H */

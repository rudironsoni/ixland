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
#define IOX_POLLIN 0x001   /* There is data to read */
#define IOX_POLLPRI 0x002  /* There is urgent data to read */
#define IOX_POLLOUT 0x004  /* Writing now will not block */
#define IOX_POLLERR 0x008  /* Error condition */
#define IOX_POLLHUP 0x010  /* Hung up */
#define IOX_POLLNVAL 0x020 /* Invalid polling request */

/* Extended poll events */
#define IOX_POLLRDNORM 0x040 /* Normal data may be read */
#define IOX_POLLRDBAND 0x080 /* Priority data may be read */
#define IOX_POLLWRNORM 0x100 /* Writing now will not block */
#define IOX_POLLWRBAND 0x200 /* Priority data may be written */
#define IOX_POLLMSG 0x400    /* Signal message received */
#define IOX_POLLREMOVE 0x800 /* Remove from watched list (epoll internal) */
#define IOX_POLLRDHUP 0x2000 /* Peer closed connection */

/* Compatibility aliases (standard names) */
#define POLLIN IOX_POLLIN
#define POLLPRI IOX_POLLPRI
#define POLLOUT IOX_POLLOUT
#define POLLERR IOX_POLLERR
#define POLLHUP IOX_POLLHUP
#define POLLNVAL IOX_POLLNVAL
#define POLLRDNORM IOX_POLLRDNORM
#define POLLRDBAND IOX_POLLRDBAND
#define POLLWRNORM IOX_POLLWRNORM
#define POLLWRBAND IOX_POLLWRBAND
#define POLLMSG IOX_POLLMSG
#define POLLREMOVE IOX_POLLREMOVE
#define POLLRDHUP IOX_POLLRDHUP

/* Set of events that can be specified for poll() */
#define IOX_POLLIN_SET (IOX_POLLIN | IOX_POLLRDNORM | IOX_POLLRDBAND)
#define IOX_POLLOUT_SET (IOX_POLLOUT | IOX_POLLWRNORM | IOX_POLLWRBAND)
#define IOX_POLLERR_SET (IOX_POLLERR | IOX_POLLHUP | IOX_POLLNVAL)

/* Input event mask (events user can request) */
#define IOX_POLLIN_EVENTS                                                                        \
    (IOX_POLLIN | IOX_POLLPRI | IOX_POLLOUT | IOX_POLLRDNORM | IOX_POLLRDBAND | IOX_POLLWRNORM | \
     IOX_POLLWRBAND | IOX_POLLRDHUP)

/* Output event mask (events that can be returned) */
#define IOX_POLLOUT_EVENTS \
    (IOX_POLLIN_EVENTS | IOX_POLLERR | IOX_POLLHUP | IOX_POLLNVAL | IOX_POLLMSG)

/* ============================================================================
 * POLL STRUCTURES
 * ============================================================================ */

/* Poll file descriptor structure - Linux-compatible */
struct linux_pollfd {
    int fd;        /* File descriptor to poll */
    short events;  /* Requested events */
    short revents; /* Returned events */
};

/* Legacy BSD struct (for compatibility) */
struct iox_pollfd {
    int fd;
    short events;
    short revents;
};

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
#define IOX_POLL_WAIT_FOREVER (-1) /* Wait indefinitely */
#define IOX_POLL_WAIT_NONE 0       /* Return immediately */

/* Standard timeout */
#define IOX_POLL_TIMEOUT_MS 1000 /* Default timeout in milliseconds */

/* ============================================================================
 * SELECT MACROS
 * ============================================================================ */

/* FD set size for select() */
#ifndef IOX_FD_SETSIZE
#define IOX_FD_SETSIZE 1024
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE IOX_FD_SETSIZE
#endif

/* Calculate number of longs needed for fd_set */
#define IOX_NFDBITS (8 * sizeof(unsigned long))
#define IOX_FDELT(d) ((d) / IOX_NFDBITS)
#define IOX_FDMASK(d) (1UL << ((d) % IOX_NFDBITS))

/* FD set structure - Linux-compatible */
typedef struct {
    unsigned long fds_bits[IOX_FD_SETSIZE / IOX_NFDBITS];
} linux_fd_set_t;

/* Compatibility alias */
#define fd_set linux_fd_set_t

/* FD set manipulation macros */
#define IOX_FD_ZERO(set)                                           \
    do {                                                           \
        unsigned int __i;                                          \
        for (__i = 0; __i < (IOX_FD_SETSIZE / IOX_NFDBITS); __i++) \
            (set)->fds_bits[__i] = 0;                              \
    } while (0)

#define IOX_FD_SET(fd, set)                                   \
    do {                                                      \
        if ((unsigned int)(fd) < IOX_FD_SETSIZE)              \
            (set)->fds_bits[IOX_FDELT(fd)] |= IOX_FDMASK(fd); \
    } while (0)

#define IOX_FD_CLR(fd, set)                                    \
    do {                                                       \
        if ((unsigned int)(fd) < IOX_FD_SETSIZE)               \
            (set)->fds_bits[IOX_FDELT(fd)] &= ~IOX_FDMASK(fd); \
    } while (0)

#define IOX_FD_ISSET(fd, set) \
    (((unsigned int)(fd) < IOX_FD_SETSIZE) ? ((set)->fds_bits[IOX_FDELT(fd)] & IOX_FDMASK(fd)) : 0)

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
#define IOX_POLL_MODE_LEVEL 0 /* Level-triggered (default) */
#define IOX_POLL_MODE_EDGE 1  /* Edge-triggered */

/* ============================================================================
 * POLL STATISTICS
 * ============================================================================ */

/* Poll statistics structure */
struct iox_poll_stats {
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
int iox_poll(struct linux_pollfd *fds, unsigned int nfds, int timeout);

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
int iox_poll_advanced(struct linux_pollfd *fds, unsigned int nfds, int timeout, int extra_flags);

/* Extra poll flags */
#define IOX_POLL_ADV_NONBLOCK 0x01 /* Never block */
#define IOX_POLL_ADV_ONESHOT 0x02  /* One-shot notification */
#define IOX_POLL_ADV_WAKEUP 0x04   /* Wake up waiters */

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
int iox_select(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
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
int iox_pselect(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
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
int iox_pselect6(int nfds, linux_fd_set_t *readfds, linux_fd_set_t *writefds,
                 linux_fd_set_t *exceptfds, const struct linux_timespec *timeout,
                 const linux_sigset_t *sigmask, void *unused);

/* ============================================================================
 * HELPER MACROS
 * ============================================================================ */

/* Check if any FD is ready in a pollfd array */
#define IOX_POLL_HAS_EVENTS(pollfds, nfds)     \
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
#define IOX_POLL_COUNT_EVENTS(pollfds, nfds) \
    ({                                       \
        unsigned int __i, __count = 0;       \
        for (__i = 0; __i < (nfds); __i++) { \
            if ((pollfds)[__i].revents != 0) \
                __count++;                   \
        }                                    \
        __count;                             \
    })

/* Wait for input on a single FD */
#define IOX_POLL_WAIT_INPUT(fd, timeout_ms)                \
    ({                                                     \
        struct linux_pollfd __pfd = {(fd), IOX_POLLIN, 0}; \
        iox_poll(&__pfd, 1, (timeout_ms));                 \
    })

/* Wait for output on a single FD */
#define IOX_POLL_WAIT_OUTPUT(fd, timeout_ms)                \
    ({                                                      \
        struct linux_pollfd __pfd = {(fd), IOX_POLLOUT, 0}; \
        iox_poll(&__pfd, 1, (timeout_ms));                  \
    })

/* Check if pollfd has error events */
#define IOX_POLL_HAS_ERROR(pfd) ((pfd)->revents & (IOX_POLLERR | IOX_POLLHUP | IOX_POLLNVAL))

/* Check if pollfd has input events */
#define IOX_POLL_HAS_INPUT(pfd) \
    ((pfd)->revents & (IOX_POLLIN | IOX_POLLRDNORM | IOX_POLLRDBAND | IOX_POLLRDHUP))

/* Check if pollfd has output events */
#define IOX_POLL_HAS_OUTPUT(pfd) ((pfd)->revents & (IOX_POLLOUT | IOX_POLLWRNORM | IOX_POLLWRBAND))

/* ============================================================================
 * INTERNAL POLLING (for kernel use)
 * ============================================================================ */

/* Poll table for internal kernel use */
struct iox_poll_table;

/* Poll callback type */
typedef int (*iox_poll_callback_t)(struct linux_pollfd *pfd, void *data);

/* Poll wait queue structure */
struct iox_poll_wait_queue {
    struct iox_poll_wait_queue *next;
    struct iox_poll_wait_queue *prev;
    void *private; /* Private data for wait */
};

/* Initialize poll table */
void iox_poll_table_init(struct iox_poll_table *table);

/* Free poll table */
void iox_poll_table_free(struct iox_poll_table *table);

/* Add wait queue to poll table */
void iox_poll_table_add(struct iox_poll_table *table, struct iox_poll_wait_queue *wq);

/* Wake up poll waiters */
void iox_poll_wake(struct iox_poll_wait_queue *wq, int key);

/* ============================================================================
 * POLL NOTIFY
 * ============================================================================ */

/* Poll notify structure */
struct iox_poll_notify {
    int fd;         /* File descriptor */
    short events;   /* Events to notify */
    void *userdata; /* User data */
};

/* Request poll notification */
int iox_poll_request_notify(const struct iox_poll_notify *notify);

/* Cancel poll notification */
int iox_poll_cancel_notify(int fd);

/* ============================================================================
 * POLL VECTORED (for efficient batch polling)
 * ============================================================================ */

/* Vector poll structure */
struct iox_poll_vec {
    struct linux_pollfd *fds; /* FD array */
    unsigned int nfds;        /* Number of FDs */
    void *userdata;           /* User context */
};

/* Batch poll operation */
int iox_poll_vec_poll(struct iox_poll_vec *vecs, unsigned int nvec, int timeout,
                      unsigned int *results);

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_POLL_H */

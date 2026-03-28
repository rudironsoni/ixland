/* iXland libc - Linux-compatible epoll.h
 *
 * epoll definitions matching Linux kernel.
 * Provides scalable I/O event notification mechanism.
 */

#ifndef _LINUX_EPOLL_H
#define _LINUX_EPOLL_H

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * EPOLL CONSTANTS
 * ============================================================================ */

/* epoll_ctl operations */
#define EPOLL_CTL_ADD   1   /* Add a file descriptor to the interface */
#define EPOLL_CTL_DEL   2   /* Remove a file descriptor from the interface */
#define EPOLL_CTL_MOD   3   /* Change file descriptor epoll_event structure */

/* iXland aliases */
#define IOX_EPOLL_CTL_ADD   EPOLL_CTL_ADD
#define IOX_EPOLL_CTL_DEL   EPOLL_CTL_DEL
#define IOX_EPOLL_CTL_MOD   EPOLL_CTL_MOD

/* epoll events (bitmask) - These MUST match Linux kernel values exactly */
#define EPOLLIN     0x001     /* The associated file is available for read */
#define EPOLLPRI    0x002     /* There is urgent data available for read */
#define EPOLLOUT    0x004     /* The associated file is available for write */
#define EPOLLERR    0x008     /* Error condition happened */
#define EPOLLHUP    0x010     /* Hang up happened */
#define EPOLLNVAL   0x020     /* Invalid file descriptor */
#define EPOLLRDNORM 0x040     /* Equivalent to POLLIN */
#define EPOLLRDBAND 0x080     /* Priority data can be read */
#define EPOLLWRNORM 0x100     /* Equivalent to EPOLLOUT */
#define EPOLLWRBAND 0x200     /* Priority data can be written */
#define EPOLLMSG    0x400     /* A message is available */
#define EPOLLRDHUP  0x2000    /* Stream socket peer closed connection */

/* iXland aliases */
#define IOX_EPOLLIN     EPOLLIN
#define IOX_EPOLLPRI    EPOLLPRI
#define IOX_EPOLLOUT    EPOLLOUT
#define IOX_EPOLLERR    EPOLLERR
#define IOX_EPOLLHUP    EPOLLHUP
#define IOX_EPOLLNVAL   EPOLLNVAL
#define IOX_EPOLLRDNORM EPOLLRDNORM
#define IOX_EPOLLRDBAND EPOLLRDBAND
#define IOX_EPOLLWRNORM EPOLLWRNORM
#define IOX_EPOLLWRBAND EPOLLWRBAND
#define IOX_EPOLLMSG    EPOLLMSG
#define IOX_EPOLLRDHUP  EPOLLRDHUP

/* epoll_create flags */
#define EPOLL_CLOEXEC   02000000 /* Close-on-exec flag */

/* iXland aliases */
#define IOX_EPOLL_CLOEXEC   EPOLL_CLOEXEC

/* Set exclusive wakeup mode for epoll_wait */
#define EPOLL_EXCLUSIVE 0x10000000  /* Wakeup only one epoll */
#define EPOLLWAKEUP     0x20000000  /* Wakeup system on event (disabled on iOS) */
#define EPOLLONESHOT    0x40000000  /* One-shot notification */
#define EPOLLET         0x80000000  /* Edge-triggered mode */

/* iXland aliases */
#define IOX_EPOLL_EXCLUSIVE EPOLL_EXCLUSIVE
#define IOX_EPOLLWAKEUP     EPOLLWAKEUP
#define IOX_EPOLLONESHOT    EPOLLONESHOT
#define IOX_EPOLLET         EPOLLET

/* Input event mask (events that can be registered) */
#define IOX_EPOLL_INPUT_EVENTS \
    (IOX_EPOLLIN | IOX_EPOLLPRI | IOX_EPOLLOUT | \
     IOX_EPOLLRDNORM | IOX_EPOLLRDBAND | \
     IOX_EPOLLWRNORM | IOX_EPOLLWRBAND | IOX_EPOLLRDHUP)

/* Output event mask (events that can be returned) */
#define IOX_EPOLL_OUTPUT_EVENTS \
    (IOX_EPOLL_INPUT_EVENTS | IOX_EPOLLERR | IOX_EPOLLHUP | IOX_EPOLLNVAL)

/* Maximum events per epoll_wait call */
#define IOX_EPOLL_MAX_EVENTS    1024

/* Default timeout values */
#define IOX_EPOLL_WAIT_FOREVER  (-1)    /* Wait indefinitely */
#define IOX_EPOLL_WAIT_NONE     0       /* Return immediately */

/* ============================================================================
 * EPOLL EVENT STRUCTURE
 * ============================================================================
 *
 * This structure MUST match the Linux kernel definition exactly for
 * binary compatibility. The epoll_event structure uses a union for the data
 * field and is packed to ensure consistent layout across platforms.
 */

typedef union epoll_data {
    void    *ptr;       /* Pointer to user data */
    int      fd;        /* File descriptor */
    uint32_t u32;       /* 32-bit integer value */
    uint64_t u64;       /* 64-bit integer value */
} epoll_data_t;

/* Linux epoll_event structure - packed to match kernel layout exactly */
struct epoll_event {
    uint32_t     events;    /* Epoll events (bitmask) */
    epoll_data_t data;      /* User data variable */
} __attribute__((packed));

/* iXland compatibility types */
typedef union iox_epoll_data {
    void    *ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
} iox_epoll_data_t;

struct iox_epoll_event {
    uint32_t     events;
    iox_epoll_data_t data;
} __attribute__((packed));

/* ============================================================================
 * EPOLL INFO STRUCTURE (for epoll_pwait2)
 * ============================================================================ */

/* Timeout specification for epoll_pwait2 */
struct iox_timespec {
    int64_t tv_sec;     /* Seconds */
    int64_t tv_nsec;    /* Nanoseconds */
};

/* Signal set for pwait variants */
struct iox_sigset {
    unsigned long sig[128 / sizeof(unsigned long)];
};

/* ============================================================================
 * EPOLL STATISTICS
 * ============================================================================ */

/* epoll statistics structure */
struct iox_epoll_stats {
    uint64_t epoll_create_calls;    /* Number of epoll_create calls */
    uint64_t epoll_ctl_calls;       /* Number of epoll_ctl calls */
    uint64_t epoll_wait_calls;      /* Number of epoll_wait calls */
    uint64_t epoll_wait_returns;    /* Returns from epoll_wait */
    uint64_t epoll_wait_events;     /* Total events returned */
    uint64_t epoll_wait_timeouts;   /* Number of timeouts */
    uint64_t epoll_wait_interrupts; /* Number of EINTR returns */
    uint64_t registered_fds;        /* Currently registered FDs */
    uint64_t total_registrations;   /* Total registrations ever */
};

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Create a new epoll instance
 *
 * Creates a new epoll instance with initial size hint (ignored on modern kernels).
 *
 * @param size Size hint (deprecated but required for compatibility)
 * @return int Epoll file descriptor on success, -1 on error with errno set
 */
int iox_epoll_create(int size);

/**
 * @brief Create a new epoll instance with flags
 *
 * Creates a new epoll instance with optional flags.
 *
 * @param flags EPOLL_CLOEXEC or 0
 * @return int Epoll file descriptor on success, -1 on error with errno set
 */
int iox_epoll_create1(int flags);

/**
 * @brief Control interface for an epoll file descriptor
 *
 * Performs control operations on the epoll instance.
 *
 * @param epfd Epoll file descriptor
 * @param op Operation (IOX_EPOLL_CTL_ADD, IOX_EPOLL_CTL_DEL, IOX_EPOLL_CTL_MOD)
 * @param fd Target file descriptor
 * @param event Associated event information (can be NULL for EPOLL_CTL_DEL)
 * @return int 0 on success, -1 on error with errno set
 */
int iox_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

/**
 * @brief Wait for an I/O event on an epoll file descriptor
 *
 * Waits for events on the epoll instance.
 *
 * @param epfd Epoll file descriptor
 * @param events Array to store returned events
 * @param maxevents Maximum number of events to return
 * @param timeout Timeout in milliseconds (-1 = wait forever, 0 = non-blocking)
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int iox_epoll_wait(int epfd, struct epoll_event *events,
                   int maxevents, int timeout);

/**
 * @brief Wait for events with signal mask
 *
 * Same as epoll_wait but allows atomically setting signal mask.
 *
 * @param epfd Epoll file descriptor
 * @param events Array to store returned events
 * @param maxevents Maximum number of events
 * @param timeout Timeout in milliseconds
 * @param sigmask Signal mask to restore during wait (NULL = no change)
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int iox_epoll_pwait(int epfd, struct epoll_event *events,
                    int maxevents, int timeout, const struct iox_sigset *sigmask);

/**
 * @brief Wait for events with nanosecond timeout
 *
 * Same as epoll_pwait but with nanosecond timeout precision.
 *
 * @param epfd Epoll file descriptor
 * @param events Array to store returned events
 * @param maxevents Maximum number of events
 * @param timeout Timeout specification (NULL = wait forever)
 * @param sigmask Signal mask to restore during wait (NULL = no change)
 * @return int Number of ready fds on success, -1 on error with errno set
 */
int iox_epoll_pwait2(int epfd, struct epoll_event *events,
                     int maxevents, const struct iox_timespec *timeout,
                     const struct iox_sigset *sigmask);

/* ============================================================================
 * EPOLL INSTANCE INFO
 * ============================================================================ */

/* Epoll info structure */
struct iox_epoll_info {
    int epfd;                   /* Epoll file descriptor */
    uint32_t flags;             /* Creation flags */
    uint32_t registered_count;  /* Number of registered FDs */
    uint32_t waiting_count;     /* Number of threads waiting */
    uint64_t total_events;      /* Total events reported */
};

/**
 * @brief Get epoll instance information
 *
 * Retrieves information about an epoll instance.
 *
 * @param epfd Epoll file descriptor
 * @param info Pointer to info structure
 * @return int 0 on success, -1 on error
 */
int iox_epoll_get_info(int epfd, struct iox_epoll_info *info);

/**
 * @brief Check if file descriptor is registered with epoll
 *
 * @param epfd Epoll file descriptor
 * @param fd File descriptor to check
 * @return int 1 if registered, 0 if not, -1 on error
 */
int iox_epoll_is_registered(int epfd, int fd);

/**
 * @brief Get registered events for a file descriptor
 *
 * @param epfd Epoll file descriptor
 * @param fd File descriptor to query
 * @param event Where to store current registration
 * @return int 0 on success, -1 on error (ENOENT if not registered)
 */
int iox_epoll_get_registration(int epfd, int fd, struct epoll_event *event);

/* ============================================================================
 * EPOLL EVENT BATCH OPERATIONS
 * ============================================================================ */

/* Batch epoll_ctl operation */
struct iox_epoll_ctl_batch {
    int op;                     /* Operation (ADD/DEL/MOD) */
    int fd;                     /* File descriptor */
    struct epoll_event event;   /* Event specification */
};

/**
 * @brief Batch epoll_ctl operations
 *
 * Performs multiple epoll_ctl operations atomically.
 *
 * @param epfd Epoll file descriptor
 * @param ops Array of operations
 * @param nops Number of operations
 * @return int 0 on success, -1 on error (all operations failed)
 */
int iox_epoll_ctl_batch(int epfd, const struct iox_epoll_ctl_batch *ops, int nops);

/* ============================================================================
 * EPOLL INTERNAL STRUCTURES (for kernel use)
 * ============================================================================ */

/* Epoll item (file registration) */
struct iox_epitem;

/* Epoll ready list item */
struct iox_epoll_ready {
    struct iox_epoll_ready *next;
    struct iox_epoll_ready *prev;
    struct iox_epitem *epi;     /* Associated epoll item */
};

/* Epoll wait queue */
struct iox_epoll_wait_queue {
    struct iox_epoll_wait_queue *next;
    struct iox_epoll_wait_queue *prev;
    int waiting;                /* Is thread waiting */
};

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================ */

/* Check if event has error condition */
#define IOX_EPOLL_HAS_ERROR(ev) \
    ((ev)->events & (IOX_EPOLLERR | IOX_EPOLLHUP | IOX_EPOLLNVAL))

/* Check if event indicates readable data */
#define IOX_EPOLL_HAS_READ(ev) \
    ((ev)->events & (IOX_EPOLLIN | IOX_EPOLLRDNORM | IOX_EPOLLRDBAND | IOX_EPOLLRDHUP))

/* Check if event indicates writable */
#define IOX_EPOLL_HAS_WRITE(ev) \
    ((ev)->events & (IOX_EPOLLOUT | IOX_EPOLLWRNORM | IOX_EPOLLWRBAND))

/* Check if event indicates urgent data */
#define IOX_EPOLL_HAS_URGENT(ev) \
    ((ev)->events & IOX_EPOLLPRI)

/* Check if edge-triggered mode is set */
#define IOX_EPOLL_IS_EDGE_TRIGGERED(ev) \
    ((ev)->events & IOX_EPOLLET)

/* Check if one-shot mode is set */
#define IOX_EPOLL_IS_ONESHOT(ev) \
    ((ev)->events & IOX_EPOLLONESHOT)

/* Create event with data as file descriptor */
#define IOX_EPOLL_EVENT_FD(events, fd_val) \
    (struct epoll_event) { .events = (events), .data = { .fd = (fd_val) } }

/* Create event with data as pointer */
#define IOX_EPOLL_EVENT_PTR(events, ptr_val) \
    (struct epoll_event) { .events = (events), .data = { .ptr = (ptr_val) } }

/* Create event with data as 32-bit integer */
#define IOX_EPOLL_EVENT_U32(events, u32_val) \
    (struct epoll_event) { .events = (events), .data = { .u32 = (u32_val) } }

/* Create event with data as 64-bit integer */
#define IOX_EPOLL_EVENT_U64(events, u64_val) \
    (struct epoll_event) { .events = (events), .data = { .u64 = (u64_val) } }

/* ============================================================================
 * EPOLL OPTIMIZATION HINTS
 * ============================================================================ */

/* Epoll hint structure for advanced usage */
struct iox_epoll_hint {
    uint32_t expected_events;   /* Expected event frequency */
    uint32_t burst_size;      /* Expected burst size */
    uint32_t latency_target;  /* Latency target in microseconds */
};

/**
 * @brief Set optimization hints for an epoll registration
 *
 * @param epfd Epoll file descriptor
 * @param fd Target file descriptor
 * @param hint Optimization hints
 * @return int 0 on success, -1 on error
 */
int iox_epoll_set_hint(int epfd, int fd, const struct iox_epoll_hint *hint);

/* ============================================================================
 * EPOLL ERROR CODES
 * ============================================================================
 *
 * epoll functions can set the following errno values:
 *
 * epoll_create:
 *   EINVAL - size is not positive (historical)
 *   EMFILE - per-process limit on open FDs reached
 *   ENFILE - system limit on total open FDs reached
 *   ENOMEM - insufficient kernel memory
 *
 * epoll_ctl:
 *   EBADF  - epfd or fd is not a valid FD
 *   EEXIST - op was EPOLL_CTL_ADD and fd already registered
 *   EINVAL - epfd is not an epoll FD, or fd is the same as epfd
 *   ENOENT - op was EPOLL_CTL_MOD/DEL and fd not registered
 *   ENOMEM - insufficient memory
 *   EPERM  - target FD does not support epoll
 *
 * epoll_wait/epoll_pwait:
 *   EBADF  - epfd is not a valid FD
 *   EFAULT - events array is not accessible
 *   EINTR  - signal interrupted call
 *   EINVAL - epfd is not an epoll FD, or maxevents <= 0
 */

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_EPOLL_H */

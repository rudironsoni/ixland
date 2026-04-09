/* iXland libc - Linux-compatible time.h
 *
 * Time functions and structures matching Linux kernel definitions.
 * Provides clock and timer interfaces for syscall compatibility.
 */

#ifndef _LINUX_TIME_H
#define _LINUX_TIME_H

#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TIME CONSTANTS
 * ============================================================================ */

/* Clock IDs for clock_gettime/clock_settime/clock_getres
 * These are only defined if not already defined by system headers */
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0           /* System-wide realtime clock */
#define CLOCK_MONOTONIC 1          /* Monotonic clock (not affected by time changes) */
#define CLOCK_PROCESS_CPUTIME_ID 2 /* Per-process CPU-time clock */
#define CLOCK_THREAD_CPUTIME_ID 3  /* Per-thread CPU-time clock */
#define CLOCK_MONOTONIC_RAW 4      /* Hardware-based monotonic clock */
#define CLOCK_REALTIME_COARSE 5    /* Fast but less accurate realtime */
#define CLOCK_MONOTONIC_COARSE 6   /* Fast but less accurate monotonic */
#define CLOCK_BOOTTIME 7           /* Monotonic with suspend time */
#define CLOCK_REALTIME_ALARM 8     /* Alarm-style realtime clock */
#define CLOCK_BOOTTIME_ALARM 9     /* Alarm-style boottime clock */
#endif                             /* CLOCK_REALTIME */

/* iXland-specific clock extensions */
#define CLOCK_IXLAND_VIRTUAL 128 /* Virtual process time */
#define CLOCK_IXLAND_HOST 129    /* Host system time */

/* Timer types for timer_create */
#define TIMER_ABSTIME 0x01 /* Absolute time for timer_settime */

/* Time conversion constants */
#define IXLAND_NSEC_PER_SEC 1000000000L
#define IXLAND_NSEC_PER_MSEC 1000000L
#define IXLAND_USEC_PER_SEC 1000000L
#define IXLAND_MSEC_PER_SEC 1000L

/* Maximum time values */
#define IXLAND_TIME_MAX_SEC 9223372036L /* Max seconds before overflow */

/* ============================================================================
 * TIME STRUCTURES
 * ============================================================================ */

/* Linux kernel timespec structure */
#ifndef _LINUX_TIMESPEC_DEFINED
#define _LINUX_TIMESPEC_DEFINED
struct linux_timespec {
    int64_t tv_sec;  /* Seconds */
    int64_t tv_nsec; /* Nanoseconds */
};

/* Linux kernel timeval structure */
struct linux_timeval {
    int64_t tv_sec;  /* Seconds */
    int64_t tv_usec; /* Microseconds */
};
#endif /* _LINUX_TIMESPEC_DEFINED */

/* Linux kernel timezone structure (obsolete but required) */
struct linux_timezone {
    int tz_minuteswest; /* Minutes west of Greenwich */
    int tz_dsttime;     /* Type of DST correction */
};

/* Linux kernel itimerspec structure */
struct linux_itimerspec {
    struct linux_timespec it_interval; /* Timer interval */
    struct linux_timespec it_value;    /* Initial expiration */
};

/* Linux kernel itimerval structure */
struct linux_itimerval {
    struct linux_timeval it_interval; /* Timer interval */
    struct linux_timeval it_value;    /* Initial expiration */
};

/* Timer info structure for timerfd */
struct linux_itimerspec64 {
    struct linux_timespec it_interval;
    struct linux_timespec it_value;
};

/* ============================================================================
 * TIMER TYPES
 * ============================================================================ */

/* Timer notification types */
typedef int linux_timer_t;   /* POSIX timer ID */
typedef int linux_clockid_t; /* Clock ID type */

/* Timer event notification structure */
struct linux_sigevent;

/* ============================================================================
 * TIME HELPER MACROS
 * ============================================================================ */

/* Convert timespec to milliseconds */
#define IXLAND_TIMESPEC_TO_MS(ts) \
    ((ts).tv_sec * IXLAND_MSEC_PER_SEC + (ts).tv_nsec / IXLAND_NSEC_PER_MSEC)

/* Convert timespec to microseconds */
#define IXLAND_TIMESPEC_TO_US(ts) ((ts).tv_sec * IXLAND_USEC_PER_SEC + (ts).tv_nsec / 1000)

/* Convert milliseconds to timespec */
#define IXLAND_MS_TO_TIMESPEC(ms, ts)                                       \
    do {                                                                    \
        (ts).tv_sec = (ms) / IXLAND_MSEC_PER_SEC;                           \
        (ts).tv_nsec = ((ms) % IXLAND_MSEC_PER_SEC) * IXLAND_NSEC_PER_MSEC; \
    } while (0)

/* Normalize a timespec structure */
#define IXLAND_TIMESPEC_NORMALIZE(ts)                 \
    do {                                              \
        while ((ts).tv_nsec >= IXLAND_NSEC_PER_SEC) { \
            (ts).tv_sec++;                            \
            (ts).tv_nsec -= IXLAND_NSEC_PER_SEC;      \
        }                                             \
        while ((ts).tv_nsec < 0) {                    \
            (ts).tv_sec--;                            \
            (ts).tv_nsec += IXLAND_NSEC_PER_SEC;      \
        }                                             \
    } while (0)

/* Add two timespec values: result = a + b */
#define IXLAND_TIMESPEC_ADD(a, b, result)             \
    do {                                              \
        (result).tv_sec = (a).tv_sec + (b).tv_sec;    \
        (result).tv_nsec = (a).tv_nsec + (b).tv_nsec; \
        IXLAND_TIMESPEC_NORMALIZE(result);            \
    } while (0)

/* Subtract two timespec values: result = a - b */
#define IXLAND_TIMESPEC_SUB(a, b, result)             \
    do {                                              \
        (result).tv_sec = (a).tv_sec - (b).tv_sec;    \
        (result).tv_nsec = (a).tv_nsec - (b).tv_nsec; \
        IXLAND_TIMESPEC_NORMALIZE(result);            \
    } while (0)

/* Compare two timespec values: -1 if a < b, 0 if equal, 1 if a > b */
#define IXLAND_TIMESPEC_CMP(a, b)       \
    (((a).tv_sec < (b).tv_sec)     ? -1 \
     : ((a).tv_sec > (b).tv_sec)   ? 1  \
     : ((a).tv_nsec < (b).tv_nsec) ? -1 \
     : ((a).tv_nsec > (b).tv_nsec) ? 1  \
                                   : 0)

/* Check if timespec is zero */
#define IXLAND_TIMESPEC_IS_ZERO(ts) ((ts).tv_sec == 0 && (ts).tv_nsec == 0)

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Get time from specified clock
 *
 * @param clockid Clock to query
 * @param tp Where to store the time
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_clock_gettime(linux_clockid_t clockid, struct linux_timespec *tp);

/**
 * @brief Set time on specified clock
 *
 * @param clockid Clock to set
 * @param tp New time value
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_clock_settime(linux_clockid_t clockid, const struct linux_timespec *tp);

/**
 * @brief Get clock resolution
 *
 * @param clockid Clock to query
 * @param res Where to store the resolution
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_clock_getres(linux_clockid_t clockid, struct linux_timespec *res);

/**
 * @brief Sleep on specified clock with optional flags
 *
 * @param clockid Clock to use
 * @param flags TIMER_ABSTIME or 0
 * @param request Requested sleep time
 * @param remain Remaining time (if interrupted)
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_clock_nanosleep(linux_clockid_t clockid, int flags, const struct linux_timespec *request,
                           struct linux_timespec *remain);

/**
 * @brief Get current time (legacy)
 *
 * @param tv Where to store the time
 * @param tz Timezone (should be NULL, obsolete)
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_gettimeofday(struct linux_timeval *tv, struct linux_timezone *tz);

/**
 * @brief Set current time (legacy, requires privileges)
 *
 * @param tv New time value
 * @param tz Timezone (should be NULL, obsolete)
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_settimeofday(const struct linux_timeval *tv, const struct linux_timezone *tz);

/**
 * @brief Sleep for specified time
 *
 * @param req Requested sleep time
 * @param rem Remaining time (if interrupted)
 * @return int 0 on success, -1 on error with errno set (EINTR)
 */
int ixland_nanosleep(const struct linux_timespec *req, struct linux_timespec *rem);

/**
 * @brief Legacy sleep function
 *
 * @param seconds Seconds to sleep
 * @return unsigned int Remaining seconds (if interrupted)
 */
unsigned int ixland_sleep(unsigned int seconds);

/**
 * @brief High-resolution sleep
 *
 * @param seconds Seconds to sleep
 * @param nanoseconds Nanoseconds to sleep
 * @return int 0 on success, -1 on error
 */
int ixland_usleep(unsigned long usec);

/**
 * @brief Set an alarm (signal-based timer)
 *
 * @param seconds Seconds until SIGALRM, or 0 to cancel
 * @return unsigned int Previous alarm value
 */
unsigned int ixland_alarm(unsigned int seconds);

/**
 * @brief Set an interval timer
 *
 * @param which Which timer to set (ITIMER_REAL, etc.)
 * @param new_value New timer value
 * @param old_value Where to store previous value
 * @return int 0 on success, -1 on error
 */
int ixland_setitimer(int which, const struct linux_itimerval *new_value,
                     struct linux_itimerval *old_value);

/**
 * @brief Get an interval timer
 *
 * @param which Which timer to get
 * @param curr_value Where to store current value
 * @return int 0 on success, -1 on error
 */
int ixland_getitimer(int which, struct linux_itimerval *curr_value);

/* ============================================================================
 * TIMERFD DECLARATIONS
 * ============================================================================ */

/**
 * @brief Create a timer file descriptor
 *
 * @param clockid Clock to use
 * @param flags TFD_CLOEXEC, TFD_NONBLOCK
 * @return int Timer FD on success, -1 on error
 */
int ixland_timerfd_create(linux_clockid_t clockid, int flags);

/**
 * @brief Set timerfd expiration
 *
 * @param fd Timer file descriptor
 * @param flags TFD_TIMER_ABSTIME
 * @param new_value New timer specification
 * @param old_value Where to store old value (may be NULL)
 * @return int 0 on success, -1 on error
 */
int ixland_timerfd_settime(int fd, int flags, const struct linux_itimerspec *new_value,
                           struct linux_itimerspec *old_value);

/**
 * @brief Get timerfd current value
 *
 * @param fd Timer file descriptor
 * @param curr_value Where to store current value
 * @return int 0 on success, -1 on error
 */
int ixland_timerfd_gettime(int fd, struct linux_itimerspec *curr_value);

/* Timerfd flags */
#define IXLAND_TFD_CLOEXEC 02000000  /* Close on exec */
#define IXLAND_TFD_NONBLOCK 00004000 /* Non-blocking I/O */
#define IXLAND_TFD_TIMER_ABSTIME 1   /* Absolute time */

/* ============================================================================
 * TIME CONVERSION FUNCTIONS
 * ============================================================================ */

/**
 * @brief Convert time_t to struct tm (GMT)
 *
 * @param timep Pointer to time_t
 * @return struct tm* Pointer to broken-down time
 */
struct tm *ixland_gmtime(const time_t *timep);

/**
 * @brief Convert time_t to struct tm (local time)
 *
 * @param timep Pointer to time_t
 * @return struct tm* Pointer to broken-down time
 */
struct tm *ixland_localtime(const time_t *timep);

/**
 * @brief Convert struct tm to time_t
 *
 * @param tp Pointer to broken-down time
 * @return time_t Calendar time
 */
time_t ixland_mktime(struct tm *tp);

/**
 * @brief Format time as string
 *
 * @param buf Output buffer
 * @param size Buffer size
 * @param format Format string
 * @param tm Time structure
 * @return size_t Number of characters written
 */
size_t ixland_strftime(char *buf, size_t size, const char *format, const struct tm *tm);

/**
 * @brief Parse time string
 *
 * @param buf Input string
 * @param format Format string
 * @param tm Output time structure
 * @return char* Pointer to first character not processed
 */
char *ixland_strptime(const char *buf, const char *format, struct tm *tm);

/* ============================================================================
 * ADJTIME/ADJTIMEX
 * ============================================================================ */

/* Timex modes for adjtimex */
#define IXLAND_ADJ_OFFSET 0x0001            /* Time offset */
#define IXLAND_ADJ_FREQUENCY 0x0002         /* Frequency offset */
#define IXLAND_ADJ_MAXERROR 0x0004          /* Maximum time error */
#define IXLAND_ADJ_ESTERROR 0x0008          /* Estimated time error */
#define IXLAND_ADJ_STATUS 0x0010            /* Clock status */
#define IXLAND_ADJ_TIMECONST 0x0020         /* PLL time constant */
#define IXLAND_ADJ_TICK 0x4000              /* Tick value */
#define IXLAND_ADJ_OFFSET_SINGLESHOT 0x8001 /* Old-fashioned adjtime */

/* Clock status bits */
#define IXLAND_STA_PLL 0x0001     /* Enable PLL updates */
#define IXLAND_STA_PPSFREQ 0x0002 /* Enable PPS freq discipline */
#define IXLAND_STA_PPSTIME 0x0004 /* Enable PPS time discipline */
#define IXLAND_STA_FLL 0x0008     /* Enable FLL mode */

/* Timex structure */
struct ixland_timex {
    unsigned int modes;        /* Mode selector */
    long offset;               /* Time offset (usec) */
    long freq;                 /* Frequency offset (scaled ppm) */
    long maxerror;             /* Maximum error (usec) */
    long esterror;             /* Estimated error (usec) */
    int status;                /* Clock command/status */
    long constant;             /* PLL time constant */
    long precision;            /* Clock precision (usec) */
    long tolerance;            /* Clock freq tolerance (ppm) */
    struct linux_timeval time; /* Current time */
    long tick;                 /* Clock tick (usec) */
    long ppsfreq;              /* PPS frequency (scaled ppm) */
    long jitter;               /* PPS jitter (usec) */
    int shift;                 /* Interval duration (s) */
    long stabil;               /* PPS stability (scaled ppm) */
    long jitcnt;               /* Jitter limit exceeded count */
    long calcnt;               /* Calibration intervals */
    long errcnt;               /* Calibration errors */
    long stbcnt;               /* Stability limit exceeded count */
    int tai;                   /* TAI offset */
};

/**
 * @brief Adjust system clock
 *
 * @param delta Time adjustment
 * @param olddelta Previous adjustment (may be NULL)
 * @return int 0 on success, -1 on error
 */
int ixland_adjtime(const struct linux_timeval *delta, struct linux_timeval *olddelta);

/**
 * @brief Extended clock adjustment (Linux-specific)
 *
 * @param buf Timex structure
 * @return int Clock state
 */
int ixland_adjtimex(struct ixland_timex *buf);

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_TIME_H */

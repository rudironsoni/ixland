/*
 * time.c - Time-related syscalls
 */

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "../../include/linux/time.h"

/* ============================================================================
 * Interval Timer Operations
 * ============================================================================ */

unsigned int a_shell_alarm(unsigned int seconds) {
    return alarm(seconds);
}

int a_shell_setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value) {
    return setitimer(which, new_value, old_value);
}

int a_shell_getitimer(int which, struct itimerval *curr_value) {
    return getitimer(which, curr_value);
}

/* ============================================================================
 * POSIX Timers (not fully supported on macOS/iOS)
 * ============================================================================ */

/* Simple timer structure for emulation */
typedef struct a_shell_timer {
    timer_t id;
    struct sigevent sev;
    struct itimerspec value;
    int active;
} a_shell_timer_t;

static a_shell_timer_t timers[32];
static int timer_count = 0;

int a_shell_timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid) {
    /* POSIX timers not fully supported on macOS/iOS */
    /* Return ENOSYS to indicate unimplemented */
    errno = ENOSYS;
    return -1;
}

int a_shell_timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value) {
    errno = ENOSYS;
    return -1;
}

int a_shell_timer_gettime(timer_t timerid, struct itimerspec *curr_value) {
    errno = ENOSYS;
    return -1;
}

int a_shell_timer_getoverrun(timer_t timerid) {
    errno = ENOSYS;
    return -1;
}

int a_shell_timer_delete(timer_t timerid) {
    errno = ENOSYS;
    return -1;
}

/* ============================================================================
 * Process Times
 * ============================================================================ */

clock_t a_shell_times(struct tms *buf) {
    return times(buf);
}

/* ============================================================================
 * Time of Day
 * ============================================================================ */

int a_shell_gettimeofday(struct timeval *tv, struct timezone *tz) {
    return gettimeofday(tv, tz);
}

int a_shell_settimeofday(const struct timeval *tv, const struct timezone *tz) {
    return settimeofday(tv, tz);
}

int a_shell_adjtime(const struct timeval *delta, struct timeval *olddelta) {
    return adjtime(delta, olddelta);
}

/* ============================================================================
 * High Resolution Sleep
 * ============================================================================ */

int a_shell_nanosleep(const struct timespec *req, struct timespec *rem) {
    return nanosleep(req, rem);
}

/* ============================================================================
 * Clock Operations
 * ============================================================================ */

int a_shell_clock_gettime(clockid_t clk_id, struct timespec *tp) {
    return clock_gettime(clk_id, tp);
}

int a_shell_clock_settime(clockid_t clk_id, const struct timespec *tp) {
    return clock_settime(clk_id, tp);
}

int a_shell_clock_getres(clockid_t clk_id, struct timespec *res) {
    return clock_getres(clk_id, res);
}

int a_shell_clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *request, struct timespec *remain) {
    /* clock_nanosleep not available on macOS/iOS */
    /* Fall back to nanosleep for CLOCK_REALTIME */
    if (clock_id == CLOCK_REALTIME) {
        if (flags & TIMER_ABSTIME) {
            /* Would need to calculate relative time from absolute */
            errno = ENOSYS;
            return -1;
        }
        return nanosleep(request, remain);
    }
    errno = EINVAL;
    return -1;
}

int a_shell_clock_adjtime(clockid_t clk_id, struct timex *buf) {
    /* Not available on macOS/iOS */
    errno = ENOSYS;
    return -1;
}

/* ============================================================================
 * Time Conversion
 * ============================================================================ */

time_t a_shell_time(time_t *tloc) {
    return time(tloc);
}

double a_shell_difftime(time_t time1, time_t time0) {
    return difftime(time1, time0);
}

time_t a_shell_mktime(struct tm *tm) {
    return mktime(tm);
}

struct tm *a_shell_localtime(const time_t *timep) {
    return localtime(timep);
}

struct tm *a_shell_gmtime(const time_t *timep) {
    return gmtime(timep);
}

size_t a_shell_strftime(char *s, size_t max, const char *format, const struct tm *tm) {
    return strftime(s, max, format, tm);
}

char *a_shell_ctime(const time_t *timep) {
    return ctime(timep);
}

char *a_shell_asctime(const struct tm *tm) {
    return asctime(tm);
}

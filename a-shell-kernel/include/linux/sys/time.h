/*
 * linux/sys/time.h - Time types and functions (BSD/SVID)
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: setitimer, getitimer, gettimeofday
 * NOTE: alarm is in unistd.h, nanosleep is in time.h
 */

#ifndef _LINUX_SYS_TIME_H
#define _LINUX_SYS_TIME_H

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Interval Timers (BSD/SVID - in sys/time.h)
 * ============================================================================ */

extern int a_shell_getitimer(int which, struct itimerval *curr_value);
extern int a_shell_setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);

/* ============================================================================
 * Time of Day (BSD/SVID - in sys/time.h)
 * ============================================================================ */

extern int a_shell_gettimeofday(struct timeval *tv, struct timezone *tz);
extern int a_shell_settimeofday(const struct timeval *tv, const struct timezone *tz);
extern int a_shell_adjtime(const struct timeval *delta, struct timeval *olddelta);

/* ============================================================================
 * Legacy Compatibility Macros
 * Map standard names to a_shell_* implementations
 * NOTE: alarm() is in unistd.h, nanosleep() is in time.h
 * ============================================================================ */

#define getitimer(which, curr_val) \
                                a_shell_getitimer(which, curr_val)
#define setitimer(which, new_val, old_val) \
                                a_shell_setitimer(which, new_val, old_val)

#define gettimeofday(tv, tz)    a_shell_gettimeofday(tv, tz)
#define settimeofday(tv, tz)    a_shell_settimeofday(tv, tz)
#define adjtime(delta, olddelta) \
                                a_shell_adjtime(delta, olddelta)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_SYS_TIME_H */

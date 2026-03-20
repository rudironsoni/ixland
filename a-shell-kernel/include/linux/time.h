/*
 * linux/time.h - Time types and functions
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: nanosleep, clock operations
 * NOTE: alarm is in unistd.h, setitimer is in sys/time.h
 */

#ifndef _LINUX_TIME_H
#define _LINUX_TIME_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Process Times (in time.h per POSIX)
 * ============================================================================ */

extern clock_t a_shell_times(struct tms *buf);

/* ============================================================================
 * High Resolution Sleep (in time.h per POSIX)
 * ============================================================================ */

extern int a_shell_nanosleep(const struct timespec *req, struct timespec *rem);

/* ============================================================================
 * Clock Operations (in time.h per POSIX)
 * ============================================================================ */

extern int a_shell_clock_gettime(clockid_t clk_id, struct timespec *tp);
extern int a_shell_clock_settime(clockid_t clk_id, const struct timespec *tp);
extern int a_shell_clock_getres(clockid_t clk_id, struct timespec *res);
extern int a_shell_clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *request, struct timespec *remain);

/* Note: clock_adjtime and POSIX timers not available on Darwin */

/* ============================================================================
 * Legacy Compatibility Macros
 * Map standard names to a_shell_* implementations
 * NOTE: alarm() macro is in unistd.h, NOT here
 * ============================================================================ */

#define times(buf)              a_shell_times(buf)

#define nanosleep(req, rem)     a_shell_nanosleep(req, rem)

#define clock_gettime(clk_id, tp) \
                                a_shell_clock_gettime(clk_id, tp)
#define clock_settime(clk_id, tp) \
                                a_shell_clock_settime(clk_id, tp)
#define clock_getres(clk_id, tp) \
                                a_shell_clock_getres(clk_id, tp)
#define clock_nanosleep(clock_id, flags, req, rem) \
                                a_shell_clock_nanosleep(clock_id, flags, req, rem)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_TIME_H */

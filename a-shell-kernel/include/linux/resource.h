/*
 * linux/resource.h - Resource usage and limits
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: getrlimit, setrlimit, getrusage
 * Uses system RLIMIT_* constants from <sys/resource.h>
 */

#ifndef _LINUX_RESOURCE_H
#define _LINUX_RESOURCE_H

#include <sys/types.h>
#include <sys/resource.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Resource Limits (uses system RLIMIT_* constants)
 * ============================================================================ */

extern int a_shell_getrlimit(int resource, struct rlimit *rlim);
extern int a_shell_setrlimit(int resource, const struct rlimit *rlim);
extern int a_shell_prlimit(pid_t pid, int resource, 
                           const struct rlimit *new_limit, struct rlimit *old_limit);

/* ============================================================================
 * Resource Usage
 * ============================================================================ */

extern int a_shell_getrusage(int who, struct rusage *usage);

/* ============================================================================
 * Process Priority
 * ============================================================================ */

extern int a_shell_getpriority(int which, id_t who);
extern int a_shell_setpriority(int which, id_t who, int prio);
extern int a_shell_nice(int inc);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define getrlimit(resource, rlim) \
                                a_shell_getrlimit(resource, rlim)
#define setrlimit(resource, rlim) \
                                a_shell_setrlimit(resource, rlim)
#define prlimit(pid, resource, new_lim, old_lim) \
                                a_shell_prlimit(pid, resource, new_lim, old_lim)

#define getrusage(who, usage)   a_shell_getrusage(who, usage)

#define getpriority(which, who) a_shell_getpriority(which, who)
#define setpriority(which, who, prio) \
                                a_shell_setpriority(which, who, prio)
#define nice(inc)               a_shell_nice(inc)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_RESOURCE_H */

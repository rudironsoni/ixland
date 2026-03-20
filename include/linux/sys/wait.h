/*
 * linux/sys/wait.h - Declarations for waiting
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: wait, waitpid, wait3, wait4, status macros
 */

#ifndef _LINUX_SYS_WAIT_H
#define _LINUX_SYS_WAIT_H

#include <sys/types.h>
#include <sys/resource.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ID Types for waitid()
 * ============================================================================ */

#ifndef P_ALL
#define P_ALL           0
#define P_PID           1
#define P_PGID          2
#endif

/* ============================================================================
 * Wait for Process State Changes
 * ============================================================================ */

extern pid_t a_shell_wait(int *wstatus);
extern pid_t a_shell_waitpid(pid_t pid, int *wstatus, int options);
extern pid_t a_shell_wait3(int *wstatus, int options, struct rusage *rusage);
extern pid_t a_shell_wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage);

/* Note: waitid uses system types */
extern pid_t a_shell_waitid(int idtype, pid_t id, void *infop, int options);

/* ============================================================================
 * Process Status Macros
 * Use system definitions from <sys/wait.h> - do NOT redefine here
 * ============================================================================ */

/* WIFEXITED, WEXITSTATUS, WIFSIGNALED, etc. are provided by system headers */

/* ============================================================================
 * Wait Options
 * Standard POSIX wait options
 * ============================================================================ */

#ifndef WNOHANG
#define WNOHANG         0x00000001
#endif

#ifndef WUNTRACED
#define WUNTRACED       0x00000002
#endif

#ifndef WCONTINUED
#define WCONTINUED      0x00000008
#endif

/* ============================================================================
 * Legacy Compatibility Macros
 * Map standard names to a_shell_* implementations
 * NOTE: These are ONLY defined here, NOT in unistd.h
 * ============================================================================ */

#define wait(status)            a_shell_wait(status)
#define waitpid(pid, status, options) \
                                a_shell_waitpid(pid, status, options)
#define wait3(status, options, rusage) \
                                a_shell_wait3(status, options, rusage)
#define wait4(pid, status, options, rusage) \
                                a_shell_wait4(pid, status, options, rusage)
#define waitid(idtype, id, infop, options) \
                                a_shell_waitid(idtype, id, infop, options)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_SYS_WAIT_H */

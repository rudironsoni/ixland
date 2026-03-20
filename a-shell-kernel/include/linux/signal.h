/*
 * linux/signal.h - Signal handling
 *
 * Linux-compatible header for a-Shell kernel
 * Uses Darwin signal numbers, provides a_shell_* wrappers
 */

#ifndef _LINUX_SIGNAL_H
#define _LINUX_SIGNAL_H

/* Include Darwin signal.h first to get signal numbers (SIGUSR1=30, etc.) */
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Signal Handler Type
 * ============================================================================ */

typedef void (*a_shell_sighandler_t)(int);

/* ============================================================================
 * Signal Handling Functions
 * ============================================================================ */

extern a_shell_sighandler_t a_shell_signal(int signum, a_shell_sighandler_t handler);
extern int a_shell_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

/* ============================================================================
 * Send Signals
 * ============================================================================ */

extern int a_shell_kill(pid_t pid, int sig);
extern int a_shell_killpg(pid_t pgrp, int sig);
extern int a_shell_raise(int sig);

/* ============================================================================
 * Signal Sets
 * ============================================================================ */

extern int a_shell_sigemptyset(sigset_t *set);
extern int a_shell_sigfillset(sigset_t *set);
extern int a_shell_sigaddset(sigset_t *set, int signum);
extern int a_shell_sigdelset(sigset_t *set, int signum);
extern int a_shell_sigismember(const sigset_t *set, int signum);

/* ============================================================================
 * Signal Masks
 * ============================================================================ */

extern int a_shell_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
extern int a_shell_sigpending(sigset_t *set);
extern int a_shell_sigsuspend(const sigset_t *mask);

/* ============================================================================
 * Signal Actions
 * ============================================================================ */

extern int a_shell_siginterrupt(int sig, int flag);

/* ============================================================================
 * Signal File Descriptors
 * ============================================================================ */

extern int a_shell_signalfd(int fd, const sigset_t *mask, int flags);

/* ============================================================================
 * Signal Information
 * ============================================================================ */

extern int a_shell_sigwait(const sigset_t *set, int *sig);
extern int a_shell_sigwaitinfo(const sigset_t *set, siginfo_t *info);
extern int a_shell_sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout);

/* ============================================================================
 * Signal Queue
 * ============================================================================ */

extern int a_shell_sigqueue(pid_t pid, int sig, const union sigval value);

/* ============================================================================
 * Legacy Compatibility Macros
 * Map standard names to a_shell_* implementations
 * ============================================================================ */

#define signal(signum, handler) \
                                a_shell_signal(signum, handler)
#define sigaction(signum, act, oldact) \
                                a_shell_sigaction(signum, act, oldact)

#define kill(pid, sig)          a_shell_kill(pid, sig)
#define killpg(pgrp, sig)       a_shell_killpg(pgrp, sig)
#define raise(sig)              a_shell_raise(sig)

#define sigemptyset(set)        a_shell_sigemptyset(set)
#define sigfillset(set)         a_shell_sigfillset(set)
#define sigaddset(set, signum)  a_shell_sigaddset(set, signum)
#define sigdelset(set, signum)  a_shell_sigdelset(set, signum)
#define sigismember(set, signum) \
                                a_shell_sigismember(set, signum)

#define sigprocmask(how, set, oldset) \
                                a_shell_sigprocmask(how, set, oldset)
#define sigpending(set)         a_shell_sigpending(set)
#define sigsuspend(mask)        a_shell_sigsuspend(mask)

#define siginterrupt(sig, flag) a_shell_siginterrupt(sig, flag)
#define signalfd(fd, mask, flags) \
                                a_shell_signalfd(fd, mask, flags)

#define sigwait(set, sig)       a_shell_sigwait(set, sig)
#define sigwaitinfo(set, info)  a_shell_sigwaitinfo(set, info)
#define sigtimedwait(set, info, timeout) \
                                a_shell_sigtimedwait(set, info, timeout)

#define sigqueue(pid, sig, value) \
                                a_shell_sigqueue(pid, sig, value)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_SIGNAL_H */

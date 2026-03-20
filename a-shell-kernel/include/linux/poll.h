/*
 * linux/poll.h - Poll interface
 *
 * Linux-compatible header for a-Shell kernel
 * Primary location for: poll, ppoll
 */

#ifndef _LINUX_POLL_H
#define _LINUX_POLL_H

#include <poll.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Poll Operations
 * ============================================================================ */

extern int a_shell_poll(struct pollfd *fds, nfds_t nfds, int timeout);
extern int a_shell_ppoll(struct pollfd *fds, nfds_t nfds, 
                         const struct timespec *tmo_p, const sigset_t *sigmask);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define poll(fds, nfds, timeout) \
                                a_shell_poll(fds, nfds, timeout)
#define ppoll(fds, nfds, tmo_p, sigmask) \
                                a_shell_ppoll(fds, nfds, tmo_p, sigmask)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_POLL_H */

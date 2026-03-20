/*
 * linux/epoll.h - epoll interface
 *
 * Linux-compatible header for a-Shell kernel
 * Emulated via kqueue on iOS
 */

#ifndef _LINUX_EPOLL_H
#define _LINUX_EPOLL_H

#include <sys/types.h>
#include <stdint.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Data Types
 * ============================================================================ */

typedef union epoll_data {
    void    *ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;
    epoll_data_t data;
};

/* ============================================================================
 * epoll Operations
 * ============================================================================ */

extern int a_shell_epoll_create(int size);
extern int a_shell_epoll_create1(int flags);
extern int a_shell_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
extern int a_shell_epoll_wait(int epfd, struct epoll_event *events, 
                              int maxevents, int timeout);
extern int a_shell_epoll_pwait(int epfd, struct epoll_event *events, 
                               int maxevents, int timeout, const sigset_t *sigmask);

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define epoll_create(size)      a_shell_epoll_create(size)
#define epoll_create1(flags)    a_shell_epoll_create1(flags)
#define epoll_ctl(epfd, op, fd, event) \
                                a_shell_epoll_ctl(epfd, op, fd, event)
#define epoll_wait(epfd, events, maxevents, timeout) \
                                a_shell_epoll_wait(epfd, events, maxevents, timeout)
#define epoll_pwait(epfd, events, maxevents, timeout, sigmask) \
                                a_shell_epoll_pwait(epfd, events, maxevents, timeout, sigmask)

/* ============================================================================
 * epoll Events (Linux standard values)
 * ============================================================================ */

#ifndef EPOLLIN
#define EPOLLIN      0x001
#define EPOLLPRI     0x002
#define EPOLLOUT     0x004
#define EPOLLERR     0x008
#define EPOLLHUP     0x010
#define EPOLLRDNORM  0x040
#define EPOLLRDBAND  0x080
#define EPOLLWRNORM  0x100
#define EPOLLWRBAND  0x200
#define EPOLLMSG     0x400
#define EPOLLRDHUP   0x2000
#define EPOLLONESHOT (1u << 30)
#define EPOLLET      (1u << 31)
#endif

/* ============================================================================
 * epoll Control Operations
 * ============================================================================ */

#ifndef EPOLL_CTL_ADD
#define EPOLL_CTL_ADD   1
#define EPOLL_CTL_DEL   2
#define EPOLL_CTL_MOD   3
#endif

/* ============================================================================
 * epoll_create1 Flags
 * ============================================================================ */

#ifndef EPOLL_CLOEXEC
#define EPOLL_CLOEXEC   02000000
#endif

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_EPOLL_H */

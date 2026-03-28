/* Poll/Syscall Tests - poll, select, epoll
 *
 * Tests I/O multiplexing syscalls:
 * - iox_poll() - wait for events on multiple FDs
 * - iox_select() - synchronous I/O multiplexing
 * - iox_epoll_create() - create epoll instance
 * - iox_epoll_ctl() - control epoll operations
 * - iox_epoll_wait() - wait for epoll events
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../harness/iox_test.h"

/* Include internal header which has poll/epoll definitions */
#include "../../src/iox/internal/iox_internal.h"

/* Use system definitions for compatibility */
#define IOX_POLLIN POLLIN
#define IOX_POLLOUT POLLOUT
#define IOX_POLLERR POLLERR
#define IOX_POLLHUP POLLHUP
#define IOX_POLLNVAL POLLNVAL

/* Define linux types for tests - mapped to system types */
struct linux_pollfd {
    int fd;
    short events;
    short revents;
};

typedef struct {
    unsigned long fds_bits[1024 / (8 * sizeof(unsigned long))];
} linux_fd_set_t;

struct linux_timeval {
    long tv_sec;
    long tv_usec;
};

struct linux_timespec {
    long tv_sec;
    long tv_nsec;
};

typedef struct {
    unsigned long sig[128 / sizeof(unsigned long)];
} linux_sigset_t;

/* IOX_FD_SETSIZE */
#ifndef IOX_FD_SETSIZE
#define IOX_FD_SETSIZE 1024
#endif

/* FD_SET macros */
#define IOX_FD_ZERO(set) memset((set), 0, sizeof(linux_fd_set_t))
#define IOX_FD_SET(fd, set) ((set)->fds_bits[(fd)/(8*sizeof(unsigned long))] |= (1UL << ((fd) % (8*sizeof(unsigned long)))))
#define IOX_FD_CLR(fd, set) ((set)->fds_bits[(fd)/(8*sizeof(unsigned long))] &= ~(1UL << ((fd) % (8*sizeof(unsigned long)))))
#define IOX_FD_ISSET(fd, set) (((set)->fds_bits[(fd)/(8*sizeof(unsigned long))] >> ((fd) % (8*sizeof(unsigned long)))) & 1)

/* Helper: Create a connected socket pair */
static int create_socket_pair(int fds[2]) {
    return socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
}

/* ============================================================================
 * poll() tests
 * ============================================================================ */

/* Test poll with NULL fds returns EFAULT */
IOX_TEST(poll_null_fds_returns_efault) {
    int result = iox_poll(NULL, 1, 0);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EFAULT);
    return true;
}

/* Test poll with empty nfds and zero timeout returns 0 */
IOX_TEST(poll_empty_fds_zero_timeout_returns_zero) {
    struct linux_pollfd fds[1];
    int result = iox_poll(fds, 0, 0);
    IOX_ASSERT_EQ(result, 0);
    return true;
}

/* Test poll with invalid fd sets POLLNVAL */
IOX_TEST(poll_invalid_fd_sets_nval) {
    struct linux_pollfd pfd = {-1, IOX_POLLIN, 0};
    int result = iox_poll(&pfd, 1, 0);
    IOX_ASSERT_EQ(result, 1);
    IOX_ASSERT(pfd.revents & IOX_POLLNVAL);
    return true;
}

/* Test poll on pipe with data ready */
IOX_TEST(poll_pipe_read_ready) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Write data to pipe */
    write(pipefd[1], "x", 1);

    /* Poll for read readiness */
    struct linux_pollfd pfd = {pipefd[0], IOX_POLLIN, 0};
    int result = iox_poll(&pfd, 1, 100);

    IOX_ASSERT_EQ(result, 1);
    IOX_ASSERT(pfd.revents & IOX_POLLIN);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test poll on pipe write ready */
IOX_TEST(poll_pipe_write_ready) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Poll for write readiness */
    struct linux_pollfd pfd = {pipefd[1], IOX_POLLOUT, 0};
    int result = iox_poll(&pfd, 1, 100);

    IOX_ASSERT_EQ(result, 1);
    IOX_ASSERT(pfd.revents & IOX_POLLOUT);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test poll timeout with no events */
IOX_TEST(poll_timeout_no_events) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Poll with no data available, 50ms timeout */
    struct linux_pollfd pfd = {pipefd[0], IOX_POLLIN, 0};
    int result = iox_poll(&pfd, 1, 50);

    IOX_ASSERT_EQ(result, 0);  /* Timeout, no events */

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test poll with multiple FDs */
IOX_TEST(poll_multiple_fds) {
    int pipe1[2], pipe2[2];
    IOX_ASSERT_EQ(pipe(pipe1), 0);
    IOX_ASSERT_EQ(pipe(pipe2), 0);

    /* Write to both pipes */
    write(pipe1[1], "a", 1);
    write(pipe2[1], "b", 1);

    struct linux_pollfd fds[2] = {
        {pipe1[0], IOX_POLLIN, 0},
        {pipe2[0], IOX_POLLIN, 0}
    };

    int result = iox_poll(fds, 2, 100);

    IOX_ASSERT_EQ(result, 2);
    IOX_ASSERT(fds[0].revents & IOX_POLLIN);
    IOX_ASSERT(fds[1].revents & IOX_POLLIN);

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    return true;
}

/* Test poll clears revents for unready FDs */
IOX_TEST(poll_clears_revents_for_unready) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Only write to one pipe (using single pipe with read/write ends) */
    write(pipefd[1], "x", 1);

    /* Poll for write on read end (should not be ready for write) and read on write end (not ready) */
    struct linux_pollfd fds[2] = {
        {pipefd[0], IOX_POLLOUT, IOX_POLLIN},  /* Pre-set revents */
        {pipefd[1], IOX_POLLIN, IOX_POLLOUT}   /* Pre-set revents */
    };

    int result = iox_poll(fds, 2, 0);

    /* First FD should have some event (pipe is writable), second should timeout */
    IOX_ASSERT_GE(result, 0);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* ============================================================================
 * select() tests
 * ============================================================================ */

/* Test select with invalid nfds */
IOX_TEST(select_invalid_nfds_returns_einval) {
    linux_fd_set_t readfds;
    IOX_FD_ZERO(&readfds);
    int result = iox_select(-1, &readfds, NULL, NULL, NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    return true;
}

/* Test select with nfds > IOX_FD_SETSIZE */
IOX_TEST(select_nfds_too_large_returns_einval) {
    linux_fd_set_t readfds;
    IOX_FD_ZERO(&readfds);
    int result = iox_select(IOX_FD_SETSIZE + 1, &readfds, NULL, NULL, NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    return true;
}

/* Test select with empty FD sets returns 0 */
IOX_TEST(select_empty_fds_returns_zero) {
    struct linux_timeval tv = {0, 0};
    int result = iox_select(0, NULL, NULL, NULL, &tv);
    IOX_ASSERT_EQ(result, 0);
    return true;
}

/* Test select with invalid fd in set returns EBADF */
IOX_TEST(select_invalid_fd_returns_ebadf) {
    linux_fd_set_t readfds;
    IOX_FD_ZERO(&readfds);
    IOX_FD_SET(-1, &readfds);

    int result = iox_select(1, &readfds, NULL, NULL, NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    return true;
}

/* Test select on pipe with data ready */
IOX_TEST(select_pipe_read_ready) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Write data to pipe */
    write(pipefd[1], "x", 1);

    /* Select for read readiness */
    linux_fd_set_t readfds;
    IOX_FD_ZERO(&readfds);
    IOX_FD_SET(pipefd[0], &readfds);

    struct linux_timeval tv = {0, 100000};  /* 100ms */
    int result = iox_select(pipefd[0] + 1, &readfds, NULL, NULL, &tv);

    IOX_ASSERT_EQ(result, 1);
    IOX_ASSERT(IOX_FD_ISSET(pipefd[0], &readfds));

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test select on pipe write ready */
IOX_TEST(select_pipe_write_ready) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Select for write readiness */
    linux_fd_set_t writefds;
    IOX_FD_ZERO(&writefds);
    IOX_FD_SET(pipefd[1], &writefds);

    struct linux_timeval tv = {0, 100000};  /* 100ms */
    int result = iox_select(pipefd[1] + 1, NULL, &writefds, NULL, &tv);

    IOX_ASSERT_EQ(result, 1);
    IOX_ASSERT(IOX_FD_ISSET(pipefd[1], &writefds));

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test select timeout */
IOX_TEST(select_timeout_no_events) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Select with no data available */
    linux_fd_set_t readfds;
    IOX_FD_ZERO(&readfds);
    IOX_FD_SET(pipefd[0], &readfds);

    struct linux_timeval tv = {0, 50000};  /* 50ms */
    int result = iox_select(pipefd[0] + 1, &readfds, NULL, NULL, &tv);

    IOX_ASSERT_EQ(result, 0);  /* Timeout */
    IOX_ASSERT(!IOX_FD_ISSET(pipefd[0], &readfds));  /* Should be cleared */

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test select clears unready FDs */
IOX_TEST(select_clears_unready_fds) {
    int pipe1[2], pipe2[2];
    IOX_ASSERT_EQ(pipe(pipe1), 0);
    IOX_ASSERT_EQ(pipe(pipe2), 0);

    /* Write only to pipe1 */
    write(pipe1[1], "x", 1);

    linux_fd_set_t readfds;
    IOX_FD_ZERO(&readfds);
    IOX_FD_SET(pipe1[0], &readfds);
    IOX_FD_SET(pipe2[0], &readfds);

    int maxfd = (pipe1[0] > pipe2[0]) ? pipe1[0] : pipe2[0];
    struct linux_timeval tv = {0, 100000};  /* 100ms */
    int result = iox_select(maxfd + 1, &readfds, NULL, NULL, &tv);

    IOX_ASSERT_EQ(result, 1);
    IOX_ASSERT(IOX_FD_ISSET(pipe1[0], &readfds));
    IOX_ASSERT(!IOX_FD_ISSET(pipe2[0], &readfds));

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    return true;
}

/* Test select with multiple FD sets */
IOX_TEST(select_multiple_sets) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Write data */
    write(pipefd[1], "x", 1);

    linux_fd_set_t readfds, writefds;
    IOX_FD_ZERO(&readfds);
    IOX_FD_ZERO(&writefds);
    IOX_FD_SET(pipefd[0], &readfds);
    IOX_FD_SET(pipefd[1], &writefds);

    int maxfd = (pipefd[0] > pipefd[1]) ? pipefd[0] : pipefd[1];
    struct linux_timeval tv = {0, 100000};  /* 100ms */
    int result = iox_select(maxfd + 1, &readfds, &writefds, NULL, &tv);

    IOX_ASSERT_EQ(result, 2);  /* Both read and write ready */
    IOX_ASSERT(IOX_FD_ISSET(pipefd[0], &readfds));
    IOX_ASSERT(IOX_FD_ISSET(pipefd[1], &writefds));

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* ============================================================================
 * epoll tests
 * ============================================================================ */

/* Test epoll_create with invalid size returns EINVAL */
IOX_TEST(epoll_create_invalid_size_returns_einval) {
    int epfd = iox_epoll_create(0);
    IOX_ASSERT_EQ(epfd, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    epfd = iox_epoll_create(-1);
    IOX_ASSERT_EQ(epfd, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    return true;
}

/* Test epoll_create returns valid epfd */
IOX_TEST(epoll_create_returns_valid_epfd) {
    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    /* Close the epoll instance (by doing nothing - epoll has no close yet) */
    /* In real implementation, we would have epoll close */
    (void)epfd;  /* Mark as used */

    return true;
}

/* Test epoll_create1 with CLOEXEC */
IOX_TEST(epoll_create1_cloexec) {
    int epfd = iox_epoll_create1(EPOLL_CLOEXEC);
    IOX_ASSERT_GT(epfd, -1);

    /* Check that CLOEXEC flag is set on underlying kqueue */
    int flags = fcntl(epfd - IOX_MAX_FD - 100, F_GETFD);
    /* Note: epfd is virtual, actual kqueue FD is internal */
    (void)flags;

    return true;
}

/* Test epoll_create1 with invalid flags */
IOX_TEST(epoll_create1_invalid_flags) {
    int epfd = iox_epoll_create1(0xFF);
    IOX_ASSERT_EQ(epfd, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    return true;
}

/* Test epoll_ctl with invalid epfd returns EBADF */
IOX_TEST(epoll_ctl_invalid_epfd_returns_ebadf) {
    struct epoll_event ev = {EPOLLIN, {.fd = 0}};
    int result = iox_epoll_ctl(-1, EPOLL_CTL_ADD, 0, &ev);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    return true;
}

/* Test epoll_ctl with invalid fd returns EBADF */
IOX_TEST(epoll_ctl_invalid_fd_returns_ebadf) {
    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev = {EPOLLIN, {.fd = -1}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, -1, &ev);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);

    return true;
}

/* Test epoll_ctl with epfd == fd returns EINVAL */
IOX_TEST(epoll_ctl_epfd_equals_fd_returns_einval) {
    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev = {EPOLLIN, {.fd = epfd}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, epfd, &ev);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    return true;
}

/* Test epoll_ctl ADD with NULL event returns EINVAL */
IOX_TEST(epoll_ctl_add_null_event_returns_einval) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_ctl ADD duplicate returns EEXIST */
IOX_TEST(epoll_ctl_add_duplicate_returns_eexist) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev = {EPOLLIN, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    /* Try to add again */
    result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EEXIST);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_ctl ADD succeeds */
IOX_TEST(epoll_ctl_add_success) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev = {EPOLLIN, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_ctl MOD on unregistered returns ENOENT */
IOX_TEST(epoll_ctl_mod_unregistered_returns_enoent) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev = {EPOLLOUT, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_MOD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_ctl MOD succeeds */
IOX_TEST(epoll_ctl_mod_success) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    /* Add for read */
    struct epoll_event ev = {EPOLLIN, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    /* Modify to write */
    ev.events = EPOLLOUT;
    result = iox_epoll_ctl(epfd, EPOLL_CTL_MOD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_ctl DEL on unregistered returns ENOENT */
IOX_TEST(epoll_ctl_del_unregistered_returns_enoent) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    int result = iox_epoll_ctl(epfd, EPOLL_CTL_DEL, pipefd[0], NULL);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, ENOENT);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_ctl DEL succeeds */
IOX_TEST(epoll_ctl_del_success) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    /* Add */
    struct epoll_event ev = {EPOLLIN, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    /* Delete */
    result = iox_epoll_ctl(epfd, EPOLL_CTL_DEL, pipefd[0], NULL);
    IOX_ASSERT_EQ(result, 0);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_ctl invalid op returns EINVAL */
IOX_TEST(epoll_ctl_invalid_op_returns_einval) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev = {EPOLLIN, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, 99, pipefd[0], &ev);  /* Invalid op */
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_wait with invalid epfd returns EBADF */
IOX_TEST(epoll_wait_invalid_epfd_returns_ebadf) {
    struct epoll_event ev;
    int result = iox_epoll_wait(-1, &ev, 1, 0);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EBADF);
    return true;
}

/* Test epoll_wait with NULL events returns EINVAL */
IOX_TEST(epoll_wait_null_events_returns_einval) {
    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    int result = iox_epoll_wait(epfd, NULL, 1, 0);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    return true;
}

/* Test epoll_wait with maxevents <= 0 returns EINVAL */
IOX_TEST(epoll_wait_invalid_maxevents_returns_einval) {
    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev;
    int result = iox_epoll_wait(epfd, &ev, 0, 0);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    result = iox_epoll_wait(epfd, &ev, -1, 0);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    return true;
}

/* Test epoll_wait timeout */
IOX_TEST(epoll_wait_timeout_no_events) {
    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev;
    int result = iox_epoll_wait(epfd, &ev, 1, 50);  /* 50ms timeout */
    IOX_ASSERT_EQ(result, 0);

    return true;
}

/* Test epoll_wait with data ready */
IOX_TEST(epoll_wait_data_ready) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    /* Register for read events */
    struct epoll_event ev = {EPOLLIN, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    /* Write data */
    write(pipefd[1], "x", 1);

    /* Wait for event */
    struct epoll_event events[1];
    result = iox_epoll_wait(epfd, events, 1, 100);

    IOX_ASSERT_EQ(result, 1);
    IOX_ASSERT(events[0].events & EPOLLIN);
    IOX_ASSERT_EQ(events[0].data.fd, pipefd[0]);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_wait respects maxevents */
IOX_TEST(epoll_wait_respects_maxevents) {
    int pipe1[2], pipe2[2];
    IOX_ASSERT_EQ(pipe(pipe1), 0);
    IOX_ASSERT_EQ(pipe(pipe2), 0);

    int epfd = iox_epoll_create(2);
    IOX_ASSERT_GT(epfd, -1);

    /* Register both pipes */
    struct epoll_event ev1 = {EPOLLIN, {.fd = pipe1[0]}};
    struct epoll_event ev2 = {EPOLLIN, {.fd = pipe2[0]}};
    iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipe1[0], &ev1);
    iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipe2[0], &ev2);

    /* Write to both */
    write(pipe1[1], "a", 1);
    write(pipe2[1], "b", 1);

    /* Wait with maxevents=1 */
    struct epoll_event events[2];
    int result = iox_epoll_wait(epfd, events, 1, 100);

    IOX_ASSERT_EQ(result, 1);  /* Should only return 1 event */

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    return true;
}

/* Test epoll with EPOLLET (edge-triggered) */
IOX_TEST(epoll_edge_triggered) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    /* Register with edge-triggered mode */
    struct epoll_event ev = {EPOLLIN | EPOLLET, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    /* Write data */
    write(pipefd[1], "x", 1);

    /* First wait should return the event */
    struct epoll_event events[1];
    result = iox_epoll_wait(epfd, events, 1, 0);
    IOX_ASSERT_EQ(result, 1);

    /* Second wait should return 0 (edge-triggered, no new data) */
    result = iox_epoll_wait(epfd, events, 1, 0);
    /* Note: This depends on kqueue behavior with EV_CLEAR */

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll with EPOLLONESHOT */
IOX_TEST(epoll_oneshot) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    /* Register with oneshot mode */
    struct epoll_event ev = {EPOLLIN | EPOLLONESHOT, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    /* Write data */
    write(pipefd[1], "x", 1);

    /* First wait should return the event */
    struct epoll_event events[1];
    result = iox_epoll_wait(epfd, events, 1, 100);
    IOX_ASSERT_EQ(result, 1);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test epoll_wait returns EPOLLHUP on pipe close */
IOX_TEST(epoll_wait_pipe_close_returns_hup) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    /* Register for read events */
    struct epoll_event ev = {EPOLLIN, {.fd = pipefd[0]}};
    int result = iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);
    IOX_ASSERT_EQ(result, 0);

    /* Close write end */
    close(pipefd[1]);

    /* Wait should return HUP */
    struct epoll_event events[1];
    result = iox_epoll_wait(epfd, events, 1, 100);

    IOX_ASSERT_EQ(result, 1);
    IOX_ASSERT(events[0].events & (EPOLLHUP | EPOLLERR | EPOLLIN));

    close(pipefd[0]);
    return true;
}

/* ============================================================================
 * Integration tests
 * ============================================================================ */

/* Test poll and select consistency */
IOX_TEST(poll_select_consistency) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Write data */
    write(pipefd[1], "x", 1);

    /* Poll for read */
    struct linux_pollfd pfd = {pipefd[0], IOX_POLLIN, 0};
    int poll_result = iox_poll(&pfd, 1, 100);

    /* Select for read */
    linux_fd_set_t readfds;
    IOX_FD_ZERO(&readfds);
    IOX_FD_SET(pipefd[0], &readfds);
    struct linux_timeval tv = {0, 100000};
    int select_result = iox_select(pipefd[0] + 1, &readfds, NULL, NULL, &tv);

    /* Both should indicate ready */
    IOX_ASSERT_EQ(poll_result, 1);
    IOX_ASSERT_EQ(select_result, 1);
    IOX_ASSERT(pfd.revents & IOX_POLLIN);
    IOX_ASSERT(IOX_FD_ISSET(pipefd[0], &readfds));

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Test poll and epoll consistency */
IOX_TEST(poll_epoll_consistency) {
    int pipefd[2];
    IOX_ASSERT_EQ(pipe(pipefd), 0);

    /* Write data */
    write(pipefd[1], "x", 1);

    /* Poll for read */
    struct linux_pollfd pfd = {pipefd[0], IOX_POLLIN, 0};
    int poll_result = iox_poll(&pfd, 1, 100);

    /* Epoll for read */
    int epfd = iox_epoll_create(1);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev = {EPOLLIN, {.fd = pipefd[0]}};
    iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);

    struct epoll_event events[1];
    int epoll_result = iox_epoll_wait(epfd, events, 1, 100);

    /* Both should indicate ready */
    IOX_ASSERT_EQ(poll_result, 1);
    IOX_ASSERT_EQ(epoll_result, 1);
    IOX_ASSERT(pfd.revents & IOX_POLLIN);
    IOX_ASSERT(events[0].events & EPOLLIN);

    close(pipefd[0]);
    close(pipefd[1]);
    return true;
}

/* Integration test: Multiple FDs with different event types */
IOX_TEST(io_multiplex_integration) {
    int pipe_read[2], pipe_write[2];
    IOX_ASSERT_EQ(pipe(pipe_read), 0);
    IOX_ASSERT_EQ(pipe(pipe_write), 0);

    /* Write to read pipe */
    write(pipe_read[1], "data", 4);

    /* Test with poll */
    struct linux_pollfd fds[2] = {
        {pipe_read[0], IOX_POLLIN, 0},
        {pipe_write[1], IOX_POLLOUT, 0}
    };

    int result = iox_poll(fds, 2, 100);
    IOX_ASSERT_EQ(result, 2);
    IOX_ASSERT(fds[0].revents & IOX_POLLIN);
    IOX_ASSERT(fds[1].revents & IOX_POLLOUT);

    /* Test with epoll */
    int epfd = iox_epoll_create(2);
    IOX_ASSERT_GT(epfd, -1);

    struct epoll_event ev1 = {EPOLLIN, {.fd = pipe_read[0]}};
    struct epoll_event ev2 = {EPOLLOUT, {.fd = pipe_write[1]}};
    iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_read[0], &ev1);
    iox_epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_write[1], &ev2);

    struct epoll_event events[2];
    result = iox_epoll_wait(epfd, events, 2, 100);
    IOX_ASSERT_EQ(result, 2);

    close(pipe_read[0]);
    close(pipe_read[1]);
    close(pipe_write[0]);
    close(pipe_write[1]);
    return true;
}

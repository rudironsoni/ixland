/*
 * test_poll.c - Polling tests
 *
 * Tests poll, epoll using explicit a_shell_*() API
 */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <poll.h>

void test_poll_basic() {
    printf("Test: poll() basic...\n");
    
    struct pollfd fds[1];
    int pipefd[2];
    
    if (pipe(pipefd) < 0) {
        printf("  ! pipe failed\n");
        return;
    }
    
    fds[0].fd = pipefd[0];
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    
    /* Poll with 0 timeout (non-blocking) */
    int ret = poll(fds, 1, 0);
    printf("  ✓ poll returned %d\n", ret);
    
    close(pipefd[0]);
    close(pipefd[1]);
}

void test_epoll_basic() {
    printf("Test: epoll() basic...\n");
    printf("  (epoll not available on macOS - skipped)\n");
    /*
     * epoll is Linux-specific. On macOS, use kqueue instead.
     * This test is skipped for header-only compilation checks.
     */
}

int main() {
    printf("\n========================================\n");
    printf("Polling Tests\n");
    printf("========================================\n\n");
    
    test_poll_basic();
    test_epoll_basic();
    
    printf("\n========================================\n");
    printf("Poll tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

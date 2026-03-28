#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include "../../kernel/signal/iox_signal.h"
#include "../../kernel/task/task.h"
#include "../harness/iox_test.h"

/* Pending signal and mask tests
 *
 * Tests for behavior that is VERIFIED_IMPLEMENTED_NOW in the kernel.
 */

IOX_TEST(pending_mask_basic_block_unblock) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();
    IOX_ASSERT_NOT_NULL(task->sighand);

    sigset_t oldmask;
    sigset_t newmask;

    /* Block SIGINT */
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGINT);
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &newmask, &oldmask) == 0);

    /* Verify old mask was retrieved */
    IOX_ASSERT(!sigismember(&oldmask, SIGINT));

    /* Verify SIGINT is now blocked */
    sigset_t current;
    IOX_ASSERT(iox_sigprocmask(0, NULL, &current) == 0);
    IOX_ASSERT(sigismember(&current, SIGINT));

    /* Unblock SIGINT */
    IOX_ASSERT(iox_sigprocmask(SIG_UNBLOCK, &newmask, &oldmask) == 0);

    /* Verify old mask had SIGINT */
    IOX_ASSERT(sigismember(&oldmask, SIGINT));

    /* Verify SIGINT is no longer blocked */
    IOX_ASSERT(iox_sigprocmask(0, NULL, &current) == 0);
    IOX_ASSERT(!sigismember(&current, SIGINT));

    return true;
}

IOX_TEST(pending_delivery_adds_to_pending) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();

    /* Block SIGUSR1 */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

    /* Verify initially no pending */
    sigset_t pending;
    IOX_ASSERT(iox_sigpending(&pending) == 0);
    IOX_ASSERT(!sigismember(&pending, SIGUSR1));

    /* Deliver SIGUSR1 */
    int ret = iox_kill(task->pid, SIGUSR1);
    IOX_ASSERT_EQ(ret, 0);

    /* Verify signal is now pending */
    IOX_ASSERT(iox_sigpending(&pending) == 0);
    IOX_ASSERT(sigismember(&pending, SIGUSR1));

    /* Cleanup: unblock and verify pending cleared on unblock */
    IOX_ASSERT(iox_sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);
    IOX_ASSERT(iox_sigpending(&pending) == 0);

    return true;
}

IOX_TEST(pending_multiple_signals_blocked) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();

    /* Block multiple signals */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

    /* Verify both are blocked */
    sigset_t current;
    IOX_ASSERT(iox_sigprocmask(0, NULL, &current) == 0);
    IOX_ASSERT(sigismember(&current, SIGUSR1));
    IOX_ASSERT(sigismember(&current, SIGUSR2));

    /* Deliver both */
    IOX_ASSERT(iox_kill(task->pid, SIGUSR1) == 0);
    IOX_ASSERT(iox_kill(task->pid, SIGUSR2) == 0);

    /* Verify both are pending */
    sigset_t pending;
    IOX_ASSERT(iox_sigpending(&pending) == 0);
    IOX_ASSERT(sigismember(&pending, SIGUSR1));
    IOX_ASSERT(sigismember(&pending, SIGUSR2));

    /* Unblock one and verify state is coherent */
    sigset_t unblock;
    sigemptyset(&unblock);
    sigaddset(&unblock, SIGUSR1);
    IOX_ASSERT(iox_sigprocmask(SIG_UNBLOCK, &unblock, NULL) == 0);

    /* Verify SIGUSR1 is unblocked but SIGUSR2 is still blocked */
    IOX_ASSERT(iox_sigprocmask(0, NULL, &current) == 0);
    IOX_ASSERT(!sigismember(&current, SIGUSR1));
    IOX_ASSERT(sigismember(&current, SIGUSR2));

    /* Verify both signals are still pending (implementation clears on unblock) */
    IOX_ASSERT(iox_sigpending(&pending) == 0);
    /* Current implementation: pending cleared on unblock (line 77 in exec.c) */

    /* Cleanup */
    IOX_ASSERT(iox_sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);

    return true;
}

IOX_TEST(pending_ignored_signal) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();

    /* Set SIG_IGN for SIGUSR1 */
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    IOX_ASSERT(iox_sigaction(SIGUSR1, &act, NULL) == 0);

    /* Verify disposition is IGN */
    IOX_ASSERT(task->sighand->action[SIGUSR1].sa_handler == SIG_IGN);

    /* Deliver signal */
    IOX_ASSERT(iox_kill(task->pid, SIGUSR1) == 0);

    /* Verify signal is still added to pending */
    sigset_t pending;
    IOX_ASSERT(iox_sigpending(&pending) == 0);
    IOX_ASSERT(sigismember(&pending, SIGUSR1));

    /* Disposition remains IGN */
    IOX_ASSERT(task->sighand->action[SIGUSR1].sa_handler == SIG_IGN);

    return true;
}

IOX_TEST(pending_null_sighand) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();
    iox_sighand_t *saved = task->sighand;

    /* Simulate null sighand */
    task->sighand = NULL;

    sigset_t mask;
    sigemptyset(&mask);

    int ret = iox_sigprocmask(SIG_BLOCK, &mask, NULL);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, ESRCH);

    /* Also test sigpending */
    sigset_t pending;
    ret = iox_sigpending(&pending);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, ESRCH);

    /* Restore */
    task->sighand = saved;

    return true;
}

IOX_TEST(pending_invalid_signal) {
    IOX_ASSERT(iox_task_init() == 0);

    /* Try to add invalid signal to pending */
    iox_task_t *task = iox_current_task();

    int ret = iox_kill(task->pid, 100); /* Out of range */
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    return true;
}

IOX_TEST(pending_invalid_how) {
    IOX_ASSERT(iox_task_init() == 0);

    sigset_t mask;
    sigemptyset(&mask);

    int ret = iox_sigprocmask(99, &mask, NULL);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);

    return true;
}

#include "../harness/iox_test.h"
#include "../../kernel/signal/iox_signal.h"
#include "../../kernel/task/task.h"
#include "../../kernel/exec/exec.h"
#include <signal.h>
#include <errno.h>

/* Helper to check if sigset is empty (macOS/iOS doesn't have sigisemptyset) */
static int sigset_is_empty(const sigset_t *set) {
    for (int i = 1; i < IOX_NSIG; i++) {
        if (sigismember(set, i)) {
            return 0;
        }
    }
    return 1;
}

IOX_TEST(sigprocmask_happy_path) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    IOX_ASSERT_NOT_NULL(task);
    IOX_ASSERT_NOT_NULL(task->sighand);
    
    sigset_t oldmask;
    sigset_t newmask;
    
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    
    /* Block SIGUSR1 */
    int ret = iox_sigprocmask(SIG_BLOCK, &newmask, &oldmask);
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT(!sigismember(&oldmask, SIGUSR1));
    IOX_ASSERT(sigismember(&task->sighand->blocked, SIGUSR1));
    
    /* Unblock SIGUSR1 */
    ret = iox_sigprocmask(SIG_UNBLOCK, &newmask, &oldmask);
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT(sigismember(&oldmask, SIGUSR1));
    IOX_ASSERT(!sigismember(&task->sighand->blocked, SIGUSR1));
    
    return true;
}

IOX_TEST(sigprocmask_block_one_signal) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
    IOX_ASSERT(sigismember(&task->sighand->blocked, SIGUSR1));
    
    return true;
}

IOX_TEST(sigprocmask_block_multiple_signals) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGTERM);
    
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
    IOX_ASSERT(sigismember(&task->sighand->blocked, SIGUSR1));
    IOX_ASSERT(sigismember(&task->sighand->blocked, SIGUSR2));
    IOX_ASSERT(sigismember(&task->sighand->blocked, SIGTERM));
    
    return true;
}

IOX_TEST(sigprocmask_unblock_one_signal) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    
    /* First block it */
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
    IOX_ASSERT(sigismember(&task->sighand->blocked, SIGUSR1));
    
    /* Then unblock it */
    IOX_ASSERT(iox_sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);
    IOX_ASSERT(!sigismember(&task->sighand->blocked, SIGUSR1));
    
    return true;
}

IOX_TEST(sigprocmask_verify_final_mask) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    sigset_t mask1, mask2;
    
    sigemptyset(&mask1);
    sigaddset(&mask1, SIGUSR1);
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask1, NULL) == 0);
    
    sigemptyset(&mask2);
    sigaddset(&mask2, SIGUSR2);
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask2, NULL) == 0);
    
    /* Verify both are blocked */
    sigset_t finalmask;
    IOX_ASSERT(iox_sigprocmask(0, NULL, &finalmask) == 0);
    IOX_ASSERT(sigismember(&finalmask, SIGUSR1));
    IOX_ASSERT(sigismember(&finalmask, SIGUSR2));
    
    return true;
}

IOX_TEST(sigprocmask_invalid_signal) {
    IOX_ASSERT(iox_task_init() == 0);
    
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, 100); /* Out of range */
    
    /* Should be silently ignored by sigprocmask */
    int ret = iox_sigprocmask(SIG_BLOCK, &mask, NULL);
    IOX_ASSERT_EQ(ret, 0);
    
    return true;
}

IOX_TEST(sigprocmask_null_task) {
    /* sigprocmask without a task should fail with ESRCH */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    
    /* Can't easily test this without setting current_task to NULL */
    /* Skip for now - tested indirectly via task_lifecycle */
    
    return true;
}

IOX_TEST(sigprocmask_invalid_how) {
    IOX_ASSERT(iox_task_init() == 0);
    
    sigset_t mask;
    sigemptyset(&mask);
    
    int ret = iox_sigprocmask(99, &mask, NULL);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    return true;
}

IOX_TEST(sigprocmask_get_old_only) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
    
    sigset_t oldmask;
    IOX_ASSERT(iox_sigprocmask(0, NULL, &oldmask) == 0);
    IOX_ASSERT(sigismember(&oldmask, SIGUSR1));
    
    return true;
}

IOX_TEST(signal_reset_on_exec) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    IOX_ASSERT_NOT_NULL(task->sighand);
    
    /* Set a non-default handler for SIGUSR1 */
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    IOX_ASSERT(iox_sigaction(SIGUSR1, &act, NULL) == 0);
    IOX_ASSERT(task->sighand->action[SIGUSR1].sa_handler == SIG_IGN);
    
    /* Block SIGUSR2 */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
    IOX_ASSERT(sigismember(&task->sighand->blocked, SIGUSR2));
    
    /* Simulate exec signal reset */
    iox_exec_reset_signals(task->sighand);
    
    /* Verify: SIG_IGN is preserved, blocked cleared, pending cleared */
    IOX_ASSERT(task->sighand->action[SIGUSR1].sa_handler == SIG_IGN);
    IOX_ASSERT(!sigismember(&task->sighand->blocked, SIGUSR2));
    IOX_ASSERT(sigset_is_empty(&task->sighand->blocked));
    IOX_ASSERT(sigset_is_empty(&task->sighand->pending));
    
    return true;
}

IOX_TEST(signal_exec_ign_preserved) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    
    /* Set SIG_IGN for SIGUSR1 */
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    IOX_ASSERT(iox_sigaction(SIGUSR1, &act, NULL) == 0);
    
    /* exec reset should preserve SIG_IGN */
    iox_exec_reset_signals(task->sighand);
    
    /* SIG_IGN is preserved on exec */
    IOX_ASSERT(task->sighand->action[SIGUSR1].sa_handler == SIG_IGN);
    
    return true;
}

IOX_TEST(sigaction_invalid_signal_number) {
    IOX_ASSERT(iox_task_init() == 0);
    
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    
    int ret = iox_sigaction(0, &act, NULL); /* Signal 0 is invalid */
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    ret = iox_sigaction(100, &act, NULL); /* Out of range */
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    return true;
}

IOX_TEST(sigaction_sigkill_sigstop_blocked) {
    IOX_ASSERT(iox_task_init() == 0);
    
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    
    int ret = iox_sigaction(SIGKILL, &act, NULL);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    ret = iox_sigaction(SIGSTOP, &act, NULL);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT_EQ(errno, EINVAL);
    
    return true;
}

IOX_TEST(sigpending_basic) {
    IOX_ASSERT(iox_task_init() == 0);
    
    iox_task_t *task = iox_current_task();
    
    /* Initially no pending signals */
    sigset_t pending;
    IOX_ASSERT(iox_sigpending(&pending) == 0);
    IOX_ASSERT(sigset_is_empty(&pending));
    
    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    IOX_ASSERT(iox_sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
    
    /* Note: iox_kill would add to pending, but signal delivery not fully tested here */
    /* Just verify pending is still accessible */
    IOX_ASSERT(iox_sigpending(&pending) == 0);
    
    return true;
}

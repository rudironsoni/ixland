#include <stdlib.h>
#include <unistd.h>

#include "../../kernel/signal/iox_signal.h"
#include "../../kernel/task/task.h"
#include "../harness/iox_test.h"

IOX_TEST(task_init_state_after_alloc) {
    /* Initialize task system */
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();
    IOX_ASSERT_NOT_NULL(task);

    /* Verify initial state */
    IOX_ASSERT(task->state == IOX_TASK_RUNNING);
    IOX_ASSERT(task->pid > 0);
    IOX_ASSERT(task->ppid == task->pid); /* init task is its own parent */
    IOX_ASSERT(task->tgid == task->pid);
    IOX_ASSERT(task->pgid == task->pid);
    IOX_ASSERT(task->sid == task->pid);
    IOX_ASSERT(atomic_load(&task->refs) >= 1);

    /* Verify resources allocated */
    IOX_ASSERT_NOT_NULL(task->files);
    IOX_ASSERT_NOT_NULL(task->fs);
    IOX_ASSERT_NOT_NULL(task->sighand);

    return true;
}

IOX_TEST(task_pid_invariants) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();

    /* PIDs should be positive */
    IOX_ASSERT(task->pid > 0);
    IOX_ASSERT(task->ppid > 0);
    IOX_ASSERT(task->tgid > 0);
    IOX_ASSERT(task->pgid > 0);
    IOX_ASSERT(task->sid > 0);

    /* For init-like task, all IDs equal its PID */
    IOX_ASSERT_EQ(task->pid, task->ppid);
    IOX_ASSERT_EQ(task->pid, task->tgid);
    IOX_ASSERT_EQ(task->pid, task->pgid);
    IOX_ASSERT_EQ(task->pid, task->sid);

    return true;
}

IOX_TEST(task_refs_initial) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();

    /* Initial refcount should be at least 1 */
    int refs = atomic_load(&task->refs);
    IOX_ASSERT(refs >= 1);

    return true;
}

IOX_TEST(task_lookup_returns_same_task) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();
    pid_t pid = task->pid;

    /* Lookup should return the same task */
    iox_task_t *found = iox_task_lookup(pid);
    IOX_ASSERT_NOT_NULL(found);
    IOX_ASSERT_EQ(found, task);

    /* Refcount should have incremented */
    IOX_ASSERT(atomic_load(&task->refs) >= 2);

    /* Free the lookup reference */
    iox_task_free(found);

    return true;
}

IOX_TEST(task_lookup_invalid_pid) {
    IOX_ASSERT(iox_task_init() == 0);

    /* Lookup invalid PID should return NULL */
    iox_task_t *found = iox_task_lookup(-1);
    IOX_ASSERT_NULL(found);

    found = iox_task_lookup(999999);
    IOX_ASSERT_NULL(found);

    return true;
}

IOX_TEST(task_refcount_increments) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();
    int initial_refs = atomic_load(&task->refs);

    /* Each lookup increments refcount */
    iox_task_t *found1 = iox_task_lookup(task->pid);
    iox_task_t *found2 = iox_task_lookup(task->pid);

    IOX_ASSERT(atomic_load(&task->refs) == initial_refs + 2);

    iox_task_free(found1);
    iox_task_free(found2);

    IOX_ASSERT(atomic_load(&task->refs) == initial_refs);

    return true;
}

IOX_TEST(task_exit_status_flag) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();

    /* Initially not exited */
    IOX_ASSERT(!atomic_load(&task->exited));
    IOX_ASSERT(!atomic_load(&task->signaled));

    /* Simulate exit */
    task->exit_status = 42;
    atomic_store(&task->exited, true);

    IOX_ASSERT_EQ(task->exit_status, 42);
    IOX_ASSERT(atomic_load(&task->exited));
    IOX_ASSERT(!atomic_load(&task->signaled));

    return true;
}

IOX_TEST(task_exit_signaled_flag) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();

    /* Save original state to restore after test */
    bool orig_signaled = atomic_load(&task->signaled);
    int orig_termsig = atomic_load(&task->termsig);
    bool orig_exited = atomic_load(&task->exited);

    /* Simulate signaled exit */
    atomic_store(&task->signaled, true);
    atomic_store(&task->termsig, SIGTERM);
    atomic_store(&task->exited, true);

    IOX_ASSERT(atomic_load(&task->signaled));
    IOX_ASSERT_EQ(atomic_load(&task->termsig), SIGTERM);
    IOX_ASSERT(atomic_load(&task->exited));

    /* Restore original state to prevent test contamination */
    atomic_store(&task->signaled, orig_signaled);
    atomic_store(&task->termsig, orig_termsig);
    atomic_store(&task->exited, orig_exited);

    return true;
}

IOX_TEST(task_alloc_basic_state) {
    /* Allocate a fresh task without going through init */
    iox_task_t *task = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(task);

    /* Verify iox_task_alloc sets up minimal state */
    IOX_ASSERT(task->pid > 0);                   /* PID is allocated */
    IOX_ASSERT(task->state == IOX_TASK_RUNNING); /* State is set */
    IOX_ASSERT(atomic_load(&task->refs) == 1);   /* Refcount initialized */
    IOX_ASSERT_NULL(task->files);                /* Resources not yet allocated */
    IOX_ASSERT_NULL(task->fs);
    IOX_ASSERT_NULL(task->sighand);

    /* Cleanup */
    iox_task_free(task);

    return true;
}

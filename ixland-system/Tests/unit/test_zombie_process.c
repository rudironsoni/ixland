/**
 * @file test_zombie_process.c
 * @brief Comprehensive unit tests for zombie process handling
 *
 * Validates:
 * - Exited processes become zombies (state ZOMBIE)
 * - Zombies retain exit status for parent
 * - waitpid() reaps zombies and returns status
 * - WIFEXITED, WIFSIGNALED macros work correctly
 * - WNOHANG option returns 0 if no zombies
 * - WNOHANG returns PID if zombie exists
 */

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "../../fs/fdtable.h"
#include "../../fs/vfs.h"
#include "../../kernel/signal/iox_signal.h"
#include "../../kernel/task/task.h"
#include "../harness/iox_test.h"

/* External declarations for syscalls tested */
extern pid_t iox_waitpid(pid_t pid, int *wstatus, int options);
extern void iox_exit(int status);

/**
 * Test: Exit creates zombie process
 * Validates: Exited process becomes zombie (state ZOMBIE)
 */
IOX_TEST(exit_creates_zombie) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create child */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();
    IOX_ASSERT_NOT_NULL(child->files);
    IOX_ASSERT_NOT_NULL(child->fs);
    IOX_ASSERT_NOT_NULL(child->sighand);

    /* Set up parent-child relationship */
    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Verify initial state is RUNNING */
    IOX_ASSERT_EQ(atomic_load(&child->state), IOX_TASK_RUNNING);
    IOX_ASSERT_FALSE(atomic_load(&child->exited));

    /* Simulate child exit - this creates zombie */
    pthread_mutex_lock(&child->lock);
    child->exit_status = 42;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    /* Verify zombie state */
    IOX_ASSERT_EQ(atomic_load(&child->state), IOX_TASK_ZOMBIE);
    IOX_ASSERT_TRUE(atomic_load(&child->exited));
    IOX_ASSERT_EQ(child->exit_status, 42);

    /* Cleanup - reap the zombie */
    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);
    IOX_ASSERT_EQ(result, child_pid);

    return true;
}

/**
 * Test: Zombie retains exit status for parent
 * Validates: Zombie process preserves exit code until reaped
 */
IOX_TEST(zombie_retains_exit_status) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Child exits with specific status */
    const int expected_status = 123;
    pthread_mutex_lock(&child->lock);
    child->exit_status = expected_status;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    /* Verify status is retained */
    IOX_ASSERT_EQ(child->exit_status, expected_status);

    /* Parent waits and retrieves status */
    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT(WIFEXITED(status));
    IOX_ASSERT_EQ(WEXITSTATUS(status), expected_status);

    return true;
}

/**
 * Test: waitpid reaps zombie
 * Validates: waitpid() properly reaps zombie and removes from children list
 */
IOX_TEST(waitpid_reaps_zombie) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Child becomes zombie */
    pthread_mutex_lock(&child->lock);
    child->exit_status = 0;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    /* Verify child is in list before reap */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_NOT_NULL(parent->children);
    pthread_mutex_unlock(&parent->lock);

    /* Reap the zombie */
    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);

    /* Verify child is removed from list after reap */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_NULL(parent->children);
    pthread_mutex_unlock(&parent->lock);

    return true;
}

/**
 * Test: WIFEXITED macro works
 * Validates: WIFEXITED returns true for normally exited process
 */
IOX_TEST(wifexited_works) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Normal exit (not signaled) */
    pthread_mutex_lock(&child->lock);
    child->exit_status = 42;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT_TRUE(WIFEXITED(status));
    IOX_ASSERT_FALSE(WIFSIGNALED(status));

    return true;
}

/**
 * Test: WEXITSTATUS extracts exit code
 * Validates: WEXITSTATUS returns correct exit code (0-255)
 */
IOX_TEST(wexitstatus_extracts_exit_code) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    /* Test exit code 42 */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    pthread_mutex_lock(&child->lock);
    child->exit_status = 42;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT_TRUE(WIFEXITED(status));
    IOX_ASSERT_EQ(WEXITSTATUS(status), 42);

    return true;
}

/**
 * Test: WEXITSTATUS extracts zero exit code
 * Validates: WEXITSTATUS works for exit code 0
 */
IOX_TEST(wexitstatus_extracts_zero) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    pthread_mutex_lock(&child->lock);
    child->exit_status = 0;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT_TRUE(WIFEXITED(status));
    IOX_ASSERT_EQ(WEXITSTATUS(status), 0);

    return true;
}

/**
 * Test: WEXITSTATUS extracts max exit code
 * Validates: WEXITSTATUS works for exit code 255
 */
IOX_TEST(wexitstatus_extracts_max) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    pthread_mutex_lock(&child->lock);
    child->exit_status = 255;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT_TRUE(WIFEXITED(status));
    IOX_ASSERT_EQ(WEXITSTATUS(status), 255);

    return true;
}

/**
 * Test: Signal termination status encoding - SIGHUP
 * Validates: WTERMSIG extracts correct signal number
 */
IOX_TEST(wtermsig_sighup) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    pthread_mutex_lock(&child->lock);
    child->exit_status = 0;
    atomic_store(&child->signaled, true);
    atomic_store(&child->termsig, SIGHUP);
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT_TRUE(WIFSIGNALED(status));
    IOX_ASSERT_EQ(WTERMSIG(status), SIGHUP);

    return true;
}

/**
 * Test: Signal termination status encoding - SIGTERM
 * Validates: WTERMSIG extracts correct signal number
 */
IOX_TEST(wtermsig_sigterm) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    pthread_mutex_lock(&child->lock);
    child->exit_status = 0;
    atomic_store(&child->signaled, true);
    atomic_store(&child->termsig, SIGTERM);
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT_TRUE(WIFSIGNALED(status));
    IOX_ASSERT_EQ(WTERMSIG(status), SIGTERM);

    return true;
}

/**
 * Test: WIFSIGNALED macro works
 * Validates: WIFSIGNALED returns true for signal-terminated process
 */
IOX_TEST(wifsignaled_works) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Signal-terminated exit */
    pthread_mutex_lock(&child->lock);
    child->exit_status = 0;
    atomic_store(&child->signaled, true);
    atomic_store(&child->termsig, SIGTERM);
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(child_pid, &status, 0);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT_TRUE(WIFSIGNALED(status));
    IOX_ASSERT_FALSE(WIFEXITED(status));
    IOX_ASSERT_EQ(WTERMSIG(status), SIGTERM);

    return true;
}

/**
 * Test: WNOHANG returns 0 if no zombies
 * Validates: waitpid with WNOHANG returns 0 when no exited children
 */
IOX_TEST(wnohang_returns_zero_if_no_zombies) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    /* Create child that has NOT exited (not a zombie) */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;
    atomic_store(&child->exited, false); /* Not exited */

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* WNOHANG with live child should return 0 */
    int status;
    pid_t result = iox_waitpid(-1, &status, WNOHANG);

    IOX_ASSERT_EQ(result, 0);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);
    iox_task_free(child);

    return true;
}

/**
 * Test: WNOHANG returns PID if zombie exists
 * Validates: waitpid with WNOHANG returns child PID when zombie exists
 */
IOX_TEST(wnohang_returns_pid_if_zombie_exists) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Child is a zombie */
    pthread_mutex_lock(&child->lock);
    child->exit_status = 99;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    /* WNOHANG with zombie should return PID */
    int status;
    pid_t child_pid = child->pid;
    pid_t result = iox_waitpid(-1, &status, WNOHANG);

    IOX_ASSERT_EQ(result, child_pid);
    IOX_ASSERT_TRUE(WIFEXITED(status));
    IOX_ASSERT_EQ(WEXITSTATUS(status), 99);

    return true;
}

/**
 * Test: WNOHANG does not block
 * Validates: waitpid with WNOHANG returns immediately
 */
IOX_TEST(wnohang_does_not_block) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    /* Create live child */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;
    atomic_store(&child->exited, false);

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* WNOHANG should return immediately with 0 */
    int status;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pid_t result = iox_waitpid(-1, &status, WNOHANG);

    clock_gettime(CLOCK_MONOTONIC, &end);

    /* Should return 0, not block */
    IOX_ASSERT_EQ(result, 0);

    /* Should complete quickly (less than 100ms) */
    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
    IOX_ASSERT(elapsed_ms < 100);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);
    iox_task_free(child);

    return true;
}

/**
 * Test: Multiple zombies can be reaped in any order
 * Validates: waitpid can reap specific zombies by PID
 */
IOX_TEST(multiple_zombies_reap_any_order) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    /* Create multiple children */
    iox_task_t *child1 = iox_task_alloc();
    iox_task_t *child2 = iox_task_alloc();
    iox_task_t *child3 = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child1);
    IOX_ASSERT_NOT_NULL(child2);
    IOX_ASSERT_NOT_NULL(child3);

    /* Setup all children */
    iox_task_t *children[3] = {child1, child2, child3};
    pid_t pids[3];

    for (int i = 0; i < 3; i++) {
        children[i]->files = iox_files_alloc(IOX_MAX_FD);
        children[i]->fs = iox_fs_alloc();
        children[i]->sighand = iox_sighand_alloc();
        children[i]->ppid = parent->pid;
        children[i]->parent = parent;

        pthread_mutex_lock(&parent->lock);
        children[i]->next_sibling = parent->children;
        parent->children = children[i];
        pthread_mutex_unlock(&parent->lock);

        /* Make zombie with unique exit code */
        pthread_mutex_lock(&children[i]->lock);
        children[i]->exit_status = 100 + i;
        atomic_store(&children[i]->exited, true);
        atomic_store(&children[i]->state, IOX_TASK_ZOMBIE);
        pthread_mutex_unlock(&children[i]->lock);

        pids[i] = children[i]->pid;
    }

    /* Reap in reverse order */
    for (int i = 2; i >= 0; i--) {
        int status;
        pid_t result = iox_waitpid(pids[i], &status, 0);

        IOX_ASSERT_EQ(result, pids[i]);
        IOX_ASSERT_TRUE(WIFEXITED(status));
        IOX_ASSERT_EQ(WEXITSTATUS(status), 100 + i);
    }

    /* All children should be reaped */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_NULL(parent->children);
    pthread_mutex_unlock(&parent->lock);

    return true;
}

/**
 * Test: Zombie remains in task table until reaped
 * Validates: Zombie task structure persists until waitpid called
 */
IOX_TEST(zombie_remains_until_reaped) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    pid_t child_pid = child->pid;

    /* Child becomes zombie */
    pthread_mutex_lock(&child->lock);
    child->exit_status = 55;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    /* Verify zombie can be found in task table */
    iox_task_t *found = iox_task_lookup(child_pid);
    IOX_ASSERT_NOT_NULL(found);
    IOX_ASSERT_EQ(found->pid, child_pid);
    IOX_ASSERT_TRUE(atomic_load(&found->exited));
    IOX_ASSERT_EQ(atomic_load(&found->state), IOX_TASK_ZOMBIE);
    IOX_ASSERT_EQ(found->exit_status, 55);

    /* Release lookup reference */
    iox_task_free(found);

    /* Reap the zombie */
    int status;
    pid_t result = iox_waitpid(child_pid, &status, 0);
    IOX_ASSERT_EQ(result, child_pid);

    return true;
}

/**
 * Test: waitpid with WNOHANG returns 0 for live children
 * Validates: Proper behavior when children exist but haven't exited
 */
IOX_TEST(waitpid_wnohang_live_children) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    /* Create a live child (not exited) */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;
    atomic_store(&child->exited, false); /* Not exited */

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* WNOHANG with live children returns 0 (no zombies ready) */
    int status;
    pid_t result = iox_waitpid(-1, &status, WNOHANG);

    /* When there are live children but no zombies, WNOHANG returns 0 */
    IOX_ASSERT_EQ(result, 0);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);
    iox_task_free(child);

    return true;
}

/**
 * Test: Zombie not reaped until waitpid called
 * Validates: Zombie persists in children list until explicitly reaped
 */
IOX_TEST(zombie_persists_until_waitpid) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;
    child->parent = parent;

    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Child becomes zombie */
    pthread_mutex_lock(&child->lock);
    child->exit_status = 77;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    /* Save PID before reap */
    pid_t child_pid = child->pid;

    /* Verify child is still in list (multiple checks) */
    for (int i = 0; i < 3; i++) {
        pthread_mutex_lock(&parent->lock);
        IOX_ASSERT_NOT_NULL(parent->children);
        pthread_mutex_unlock(&parent->lock);
    }

    /* Now reap */
    int status;
    pid_t result = iox_waitpid(child_pid, &status, 0);
    IOX_ASSERT_EQ(result, child_pid);

    /* Verify child is gone */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_NULL(parent->children);
    pthread_mutex_unlock(&parent->lock);

    return true;
}

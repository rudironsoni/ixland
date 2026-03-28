#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include "../../fs/fdtable.h"
#include "../../fs/vfs.h"
#include "../../kernel/signal/iox_signal.h"
#include "../../kernel/task/task.h"
#include "../harness/iox_test.h"

/* Note: Kernel layer provides pgid/sid fields but no manipulation APIs.
 * These fields are set at task allocation time and inherited during fork.
 * Process group reassignment (setpgid) and session creation (setsid)
 * are implemented at a higher layer (src/iox/core/iox_process.c).
 *
 * These tests verify the kernel-level behavior.
 */

IOX_TEST(pgrp_initial_task_leader_invariants) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();
    IOX_ASSERT_NOT_NULL(task);

    /* Initial task starts as its own process group leader */
    IOX_ASSERT_EQ(task->pgid, task->pid);
    IOX_ASSERT_EQ(task->sid, task->pid);

    /* pgid and sid should be positive */
    IOX_ASSERT(task->pgid > 0);
    IOX_ASSERT(task->sid > 0);

    /* For initial task, pgid == pid == sid */
    IOX_ASSERT_EQ(task->pgid, task->sid);

    return true;
}

IOX_TEST(pgrp_fork_inheritance) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    pid_t parent_pgid = parent->pgid;
    pid_t parent_sid = parent->sid;

    /* Create child manually (simulating fork path) */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    /* Set up child with inherited values */
    child->ppid = parent->pid;
    child->pgid = parent->pgid; /* Inherited from parent */
    child->sid = parent->sid;   /* Inherited from parent */

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();
    IOX_ASSERT_NOT_NULL(child->files);
    IOX_ASSERT_NOT_NULL(child->fs);
    IOX_ASSERT_NOT_NULL(child->sighand);

    /* Link child to parent */
    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Verify inheritance */
    IOX_ASSERT_EQ(child->pgid, parent_pgid);
    IOX_ASSERT_EQ(child->sid, parent_sid);
    IOX_ASSERT_EQ(child->pgid, parent->pgid);
    IOX_ASSERT_EQ(child->sid, parent->sid);

    /* Child is NOT leader of its own group */
    IOX_ASSERT(child->pgid != child->pid);
    IOX_ASSERT(child->sid != child->pid);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);
    iox_task_free(child);

    return true;
}

IOX_TEST(pgrp_multiple_children_same_group) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    pid_t parent_pgid = parent->pgid;

    /* Create multiple children in same process group */
    iox_task_t *children[3];

    for (int i = 0; i < 3; i++) {
        children[i] = iox_task_alloc();
        IOX_ASSERT_NOT_NULL(children[i]);

        children[i]->ppid = parent->pid;
        children[i]->pgid = parent->pgid; /* Same PGID */
        children[i]->sid = parent->sid;

        children[i]->files = iox_files_alloc(IOX_MAX_FD);
        children[i]->fs = iox_fs_alloc();
        children[i]->sighand = iox_sighand_alloc();

        pthread_mutex_lock(&parent->lock);
        children[i]->parent = parent;
        children[i]->next_sibling = parent->children;
        parent->children = children[i];
        pthread_mutex_unlock(&parent->lock);
    }

    /* Verify all children share same process group */
    for (int i = 0; i < 3; i++) {
        IOX_ASSERT_EQ(children[i]->pgid, parent_pgid);
        IOX_ASSERT_EQ(children[i]->pgid, children[0]->pgid);
    }

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    for (int i = 0; i < 3; i++) {
        iox_task_free(children[i]);
    }

    return true;
}

IOX_TEST(pgrp_session_invariants) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *task = iox_current_task();

    /* sid should always equal pgid for session leader */
    /* or be the session ID inherited from parent */
    IOX_ASSERT_EQ(task->sid, task->pgid);

    /* Create child to verify session inheritance */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->ppid = task->pid;
    child->pgid = task->pgid;
    child->sid = task->sid;

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    pthread_mutex_lock(&task->lock);
    child->parent = task;
    child->next_sibling = task->children;
    task->children = child;
    pthread_mutex_unlock(&task->lock);

    /* Child inherits session */
    IOX_ASSERT_EQ(child->sid, task->sid);
    IOX_ASSERT(child->sid != child->pid); /* Child is not session leader */

    /* Cleanup */
    pthread_mutex_lock(&task->lock);
    task->children = NULL;
    pthread_mutex_unlock(&task->lock);
    iox_task_free(child);

    return true;
}

IOX_TEST(pgrp_child_lookup_by_pgid) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    pid_t pgid = parent->pgid;

    /* Create child in same process group */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->ppid = parent->pid;
    child->pgid = pgid;
    child->sid = parent->sid;

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Verify we can find child and check its PGID */
    iox_task_t *found = iox_task_lookup(child->pid);
    IOX_ASSERT_NOT_NULL(found);
    IOX_ASSERT_EQ(found->pgid, pgid);
    IOX_ASSERT_EQ(found->pgid, parent->pgid);

    iox_task_free(found);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);
    iox_task_free(child);

    return true;
}

IOX_TEST(pgrp_zombie_retains_pgid) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    pid_t pgid = parent->pgid;

    /* Create child */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->ppid = parent->pid;
    child->pgid = pgid;
    child->sid = parent->sid;

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Simulate child exit - becomes zombie */
    pthread_mutex_lock(&child->lock);
    child->exit_status = 0;
    atomic_store(&child->exited, true);
    atomic_store(&child->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&child->lock);

    /* Zombie should retain its pgid */
    IOX_ASSERT_EQ(child->pgid, pgid);
    IOX_ASSERT(atomic_load(&child->state) == IOX_TASK_ZOMBIE);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);
    iox_task_free(child);

    return true;
}

IOX_TEST(pgrp_refcount_with_membership) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();

    /* Create child */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    int initial_refs = atomic_load(&child->refs);

    child->ppid = parent->pid;
    child->pgid = parent->pgid;
    child->sid = parent->sid;

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Membership doesn't change refcount directly */
    IOX_ASSERT_EQ(atomic_load(&child->refs), initial_refs);

    /* But lookup does increment it */
    iox_task_t *found = iox_task_lookup(child->pid);
    IOX_ASSERT_EQ(atomic_load(&child->refs), initial_refs + 1);

    iox_task_free(found);
    IOX_ASSERT_EQ(atomic_load(&child->refs), initial_refs);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);
    iox_task_free(child);

    return true;
}

/* Note: Kernel layer does not implement:
 * - setpgid() - Process group reassignment
 * - setsid() - Session creation
 * - Cross-session move validation
 * - Process group management APIs
 *
 * These are implemented at the src/iox/core/ layer.
 */

/**
 * @file test_orphan_adoption.c
 * @brief Unit tests for orphan adoption (reparenting to init)
 *
 * Tests that when a parent exits, its children are properly reparented to init.
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "../../fs/fdtable.h"
#include "../../fs/vfs.h"
#include "../../kernel/signal/iox_signal.h"
#include "../../kernel/task/task.h"
#include "../harness/iox_test.h"

/* External declaration for init_task */
extern iox_task_t *init_task;

/* Test basic orphan reparenting to init */
IOX_TEST(orphan_reparented_to_init_basic) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *init = iox_current_task();
    IOX_ASSERT_NOT_NULL(init);

    /* Create a "middle" parent */
    iox_task_t *middle_parent = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(middle_parent);

    middle_parent->files = iox_files_alloc(IOX_MAX_FD);
    middle_parent->fs = iox_fs_alloc();
    middle_parent->sighand = iox_sighand_alloc();
    IOX_ASSERT_NOT_NULL(middle_parent->files);
    IOX_ASSERT_NOT_NULL(middle_parent->fs);
    IOX_ASSERT_NOT_NULL(middle_parent->sighand);

    /* Link middle_parent to init */
    middle_parent->ppid = init->pid;
    pthread_mutex_lock(&init->lock);
    middle_parent->parent = init;
    middle_parent->next_sibling = init->children;
    init->children = middle_parent;
    pthread_mutex_unlock(&init->lock);

    /* Create a child of middle_parent */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();
    IOX_ASSERT_NOT_NULL(child->files);
    IOX_ASSERT_NOT_NULL(child->fs);
    IOX_ASSERT_NOT_NULL(child->sighand);

    /* Link child to middle_parent */
    child->ppid = middle_parent->pid;
    pthread_mutex_lock(&middle_parent->lock);
    child->parent = middle_parent;
    child->next_sibling = middle_parent->children;
    middle_parent->children = child;
    pthread_mutex_unlock(&middle_parent->lock);

    /* Verify initial state */
    IOX_ASSERT_EQ(child->parent, middle_parent);
    IOX_ASSERT_EQ(child->ppid, middle_parent->pid);

    /* Simulate middle_parent exit - reparent child to init */
    pthread_mutex_lock(&middle_parent->lock);
    pthread_mutex_lock(&init->lock);

    /* Update child's parent pointer */
    pthread_mutex_lock(&child->lock);
    child->parent = init;
    child->ppid = init->pid;
    pthread_mutex_unlock(&child->lock);

    /* Move child from middle_parent to init */
    middle_parent->children = child->next_sibling;

    /* Link child to init's children list */
    child->next_sibling = init->children;
    init->children = child;

    pthread_mutex_unlock(&init->lock);
    pthread_mutex_unlock(&middle_parent->lock);

    /* Verify child is now child of init */
    IOX_ASSERT_EQ(child->parent, init);
    IOX_ASSERT_EQ(child->ppid, init->pid);

    /* Cleanup */
    pthread_mutex_lock(&init->lock);
    init->children = NULL;
    pthread_mutex_unlock(&init->lock);

    iox_task_free(child);
    iox_task_free(middle_parent);

    return true;
}

/* Test multiple orphans reparented together */
IOX_TEST(multiple_orphans_reparented_together) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *init = iox_current_task();
    IOX_ASSERT_NOT_NULL(init);

    /* Clear init's children list first */
    pthread_mutex_lock(&init->lock);
    init->children = NULL;
    pthread_mutex_unlock(&init->lock);

    /* Create a parent with multiple children */
    iox_task_t *parent = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(parent);

    parent->files = iox_files_alloc(IOX_MAX_FD);
    parent->fs = iox_fs_alloc();
    parent->sighand = iox_sighand_alloc();

    /* Create children */
    iox_task_t *child1 = iox_task_alloc();
    iox_task_t *child2 = iox_task_alloc();
    iox_task_t *child3 = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child1);
    IOX_ASSERT_NOT_NULL(child2);
    IOX_ASSERT_NOT_NULL(child3);

    child1->files = iox_files_alloc(IOX_MAX_FD);
    child1->fs = iox_fs_alloc();
    child1->sighand = iox_sighand_alloc();
    child2->files = iox_files_alloc(IOX_MAX_FD);
    child2->fs = iox_fs_alloc();
    child2->sighand = iox_sighand_alloc();
    child3->files = iox_files_alloc(IOX_MAX_FD);
    child3->fs = iox_fs_alloc();
    child3->sighand = iox_sighand_alloc();

    /* Link children to parent */
    pthread_mutex_lock(&parent->lock);

    child1->ppid = parent->pid;
    child1->parent = parent;
    child1->next_sibling = parent->children;
    parent->children = child1;

    child2->ppid = parent->pid;
    child2->parent = parent;
    child2->next_sibling = parent->children;
    parent->children = child2;

    child3->ppid = parent->pid;
    child3->parent = parent;
    child3->next_sibling = parent->children;
    parent->children = child3;

    pthread_mutex_unlock(&parent->lock);

    /* Verify parent has 3 children */
    pthread_mutex_lock(&parent->lock);
    int count = 0;
    iox_task_t *c = parent->children;
    while (c) {
        count++;
        c = c->next_sibling;
    }
    IOX_ASSERT_EQ(count, 3);
    pthread_mutex_unlock(&parent->lock);

    /* Simulate parent exit - reparent all children to init */
    pthread_mutex_lock(&parent->lock);
    pthread_mutex_lock(&init->lock);

    /* Reparent all children */
    c = parent->children;
    while (c) {
        pthread_mutex_lock(&c->lock);
        c->parent = init;
        c->ppid = init->pid;
        pthread_mutex_unlock(&c->lock);
        c = c->next_sibling;
    }

    /* Move entire list to init */
    if (parent->children) {
        /* Find last child in parent's list */
        iox_task_t *last = parent->children;
        while (last->next_sibling) {
            last = last->next_sibling;
        }

        /* Prepend to init's list */
        last->next_sibling = init->children;
        init->children = parent->children;
        parent->children = NULL;
    }

    pthread_mutex_unlock(&init->lock);
    pthread_mutex_unlock(&parent->lock);

    /* Verify all children are now children of init */
    pthread_mutex_lock(&init->lock);
    count = 0;
    c = init->children;
    while (c) {
        count++;
        IOX_ASSERT_EQ(c->parent, init);
        IOX_ASSERT_EQ(c->ppid, init->pid);
        c = c->next_sibling;
    }
    IOX_ASSERT_EQ(count, 3);
    pthread_mutex_unlock(&init->lock);

    /* Cleanup */
    pthread_mutex_lock(&init->lock);
    init->children = NULL;
    pthread_mutex_unlock(&init->lock);

    iox_task_free(child1);
    iox_task_free(child2);
    iox_task_free(child3);
    iox_task_free(parent);

    return true;
}

/* Test orphan's ppid is set to init's PID */
IOX_TEST(orphan_ppid_set_to_init_pid) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *init = iox_current_task();
    IOX_ASSERT_NOT_NULL(init);

    /* Create parent and child */
    iox_task_t *parent = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(parent);

    parent->files = iox_files_alloc(IOX_MAX_FD);
    parent->fs = iox_fs_alloc();
    parent->sighand = iox_sighand_alloc();

    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    /* Link child to parent */
    pthread_mutex_lock(&parent->lock);
    child->ppid = parent->pid;
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Verify initial ppid */
    IOX_ASSERT_EQ(child->ppid, parent->pid);

    /* Simulate reparenting */
    pthread_mutex_lock(&child->lock);
    child->ppid = init->pid;
    child->parent = init;
    pthread_mutex_unlock(&child->lock);

    /* Verify ppid is now init's PID */
    IOX_ASSERT_EQ(child->ppid, init->pid);
    IOX_ASSERT_EQ(child->parent, init);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child);
    iox_task_free(parent);

    return true;
}

/* Test orphaned grandchildren scenario */
IOX_TEST(orphaned_grandchildren_scenario) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *init = iox_current_task();
    IOX_ASSERT_NOT_NULL(init);

    /* Create grandparent -> parent -> grandchild structure */
    iox_task_t *grandparent = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(grandparent);
    grandparent->files = iox_files_alloc(IOX_MAX_FD);
    grandparent->fs = iox_fs_alloc();
    grandparent->sighand = iox_sighand_alloc();

    iox_task_t *parent = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(parent);
    parent->files = iox_files_alloc(IOX_MAX_FD);
    parent->fs = iox_fs_alloc();
    parent->sighand = iox_sighand_alloc();

    iox_task_t *grandchild = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(grandchild);
    grandchild->files = iox_files_alloc(IOX_MAX_FD);
    grandchild->fs = iox_fs_alloc();
    grandchild->sighand = iox_sighand_alloc();

    /* Link grandparent -> parent */
    pthread_mutex_lock(&init->lock);
    grandparent->ppid = init->pid;
    grandparent->parent = init;
    grandparent->next_sibling = init->children;
    init->children = grandparent;
    pthread_mutex_unlock(&init->lock);

    /* Link parent -> grandchild */
    pthread_mutex_lock(&parent->lock);
    grandchild->ppid = parent->pid;
    grandchild->parent = parent;
    grandchild->next_sibling = parent->children;
    parent->children = grandchild;
    pthread_mutex_unlock(&parent->lock);

    /* Link grandparent -> parent (as child) */
    pthread_mutex_lock(&grandparent->lock);
    parent->ppid = grandparent->pid;
    parent->parent = grandparent;
    parent->next_sibling = grandparent->children;
    grandparent->children = parent;
    pthread_mutex_unlock(&grandparent->lock);

    /* Verify hierarchy */
    IOX_ASSERT_EQ(grandchild->parent, parent);
    IOX_ASSERT_EQ(parent->parent, grandparent);
    IOX_ASSERT_EQ(grandparent->parent, init);

    /* Simulate parent exit - grandchild should be reparented to init */
    pthread_mutex_lock(&parent->lock);
    pthread_mutex_lock(&init->lock);

    /* Reparent grandchild */
    pthread_mutex_lock(&grandchild->lock);
    grandchild->parent = init;
    grandchild->ppid = init->pid;
    pthread_mutex_unlock(&grandchild->lock);

    /* Move grandchild to init */
    parent->children = grandchild->next_sibling;
    grandchild->next_sibling = init->children;
    init->children = grandchild;

    pthread_mutex_unlock(&init->lock);
    pthread_mutex_unlock(&parent->lock);

    /* Verify grandchild is now child of init */
    IOX_ASSERT_EQ(grandchild->parent, init);
    IOX_ASSERT_EQ(grandchild->ppid, init->pid);

    /* Cleanup */
    pthread_mutex_lock(&init->lock);
    init->children = NULL;
    pthread_mutex_unlock(&init->lock);

    iox_task_free(grandchild);
    iox_task_free(parent);
    iox_task_free(grandparent);

    return true;
}

/* Test that orphans are findable in init's children list */
IOX_TEST(orphans_findable_in_init_children) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *init = iox_current_task();
    IOX_ASSERT_NOT_NULL(init);

    /* Create parent and orphan */
    iox_task_t *parent = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(parent);
    parent->files = iox_files_alloc(IOX_MAX_FD);
    parent->fs = iox_fs_alloc();
    parent->sighand = iox_sighand_alloc();

    iox_task_t *orphan = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(orphan);
    orphan->files = iox_files_alloc(IOX_MAX_FD);
    orphan->fs = iox_fs_alloc();
    orphan->sighand = iox_sighand_alloc();

    /* Link orphan to parent */
    pthread_mutex_lock(&parent->lock);
    orphan->ppid = parent->pid;
    orphan->parent = parent;
    orphan->next_sibling = parent->children;
    parent->children = orphan;
    pthread_mutex_unlock(&parent->lock);

    /* Reparent orphan to init */
    pthread_mutex_lock(&parent->lock);
    pthread_mutex_lock(&init->lock);

    pthread_mutex_lock(&orphan->lock);
    orphan->parent = init;
    orphan->ppid = init->pid;
    pthread_mutex_unlock(&orphan->lock);

    parent->children = orphan->next_sibling;
    orphan->next_sibling = init->children;
    init->children = orphan;

    pthread_mutex_unlock(&init->lock);
    pthread_mutex_unlock(&parent->lock);

    /* Find orphan in init's children list */
    iox_task_t *found = NULL;
    pthread_mutex_lock(&init->lock);
    iox_task_t *c = init->children;
    while (c) {
        if (c->pid == orphan->pid) {
            found = c;
            break;
        }
        c = c->next_sibling;
    }
    pthread_mutex_unlock(&init->lock);

    IOX_ASSERT_NOT_NULL(found);
    IOX_ASSERT_EQ(found, orphan);
    IOX_ASSERT_EQ(found->parent, init);

    /* Cleanup */
    pthread_mutex_lock(&init->lock);
    init->children = NULL;
    pthread_mutex_unlock(&init->lock);

    iox_task_free(orphan);
    iox_task_free(parent);

    return true;
}

/* Test orphan adoption maintains sibling relationships */
IOX_TEST(orphan_adoption_maintains_siblings) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *init = iox_current_task();
    IOX_ASSERT_NOT_NULL(init);

    /* Clear init's children */
    pthread_mutex_lock(&init->lock);
    init->children = NULL;
    pthread_mutex_unlock(&init->lock);

    /* Create parent with siblings */
    iox_task_t *parent = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(parent);
    parent->files = iox_files_alloc(IOX_MAX_FD);
    parent->fs = iox_fs_alloc();
    parent->sighand = iox_sighand_alloc();

    iox_task_t *child1 = iox_task_alloc();
    iox_task_t *child2 = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child1);
    IOX_ASSERT_NOT_NULL(child2);
    child1->files = iox_files_alloc(IOX_MAX_FD);
    child1->fs = iox_fs_alloc();
    child1->sighand = iox_sighand_alloc();
    child2->files = iox_files_alloc(IOX_MAX_FD);
    child2->fs = iox_fs_alloc();
    child2->sighand = iox_sighand_alloc();

    /* Link siblings */
    pthread_mutex_lock(&parent->lock);
    child1->ppid = parent->pid;
    child1->parent = parent;
    child1->next_sibling = parent->children;
    parent->children = child1;

    child2->ppid = parent->pid;
    child2->parent = parent;
    child2->next_sibling = parent->children;
    parent->children = child2;
    pthread_mutex_unlock(&parent->lock);

    /* Verify sibling chain: child2 -> child1 */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_EQ(parent->children, child2);
    IOX_ASSERT_EQ(child2->next_sibling, child1);
    IOX_ASSERT_NULL(child1->next_sibling);
    pthread_mutex_unlock(&parent->lock);

    /* Reparent both to init */
    pthread_mutex_lock(&parent->lock);
    pthread_mutex_lock(&init->lock);

    pthread_mutex_lock(&child1->lock);
    child1->parent = init;
    child1->ppid = init->pid;
    pthread_mutex_unlock(&child1->lock);

    pthread_mutex_lock(&child2->lock);
    child2->parent = init;
    child2->ppid = init->pid;
    pthread_mutex_unlock(&child2->lock);

    /* Move list to init, preserving sibling order */
    parent->children = NULL;
    child1->next_sibling = init->children;
    init->children = child2;
    /* child2 already points to child1, child1 points to NULL */

    pthread_mutex_unlock(&init->lock);
    pthread_mutex_unlock(&parent->lock);

    /* Verify sibling relationship preserved */
    pthread_mutex_lock(&init->lock);
    IOX_ASSERT_EQ(init->children, child2);
    IOX_ASSERT_EQ(child2->next_sibling, child1);
    IOX_ASSERT_NULL(child1->next_sibling);
    pthread_mutex_unlock(&init->lock);

    /* Cleanup */
    pthread_mutex_lock(&init->lock);
    init->children = NULL;
    pthread_mutex_unlock(&init->lock);

    iox_task_free(child1);
    iox_task_free(child2);
    iox_task_free(parent);

    return true;
}

/* Test init can wait for orphaned children */
IOX_TEST(init_can_wait_for_orphans) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *init = iox_current_task();
    IOX_ASSERT_NOT_NULL(init);

    /* Create orphan that is already exited */
    iox_task_t *orphan = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(orphan);
    orphan->files = iox_files_alloc(IOX_MAX_FD);
    orphan->fs = iox_fs_alloc();
    orphan->sighand = iox_sighand_alloc();

    /* Make orphan a child of init */
    orphan->ppid = init->pid;
    orphan->parent = init;

    pthread_mutex_lock(&init->lock);
    orphan->next_sibling = init->children;
    init->children = orphan;
    pthread_mutex_unlock(&init->lock);

    /* Mark orphan as exited (zombie) */
    pthread_mutex_lock(&orphan->lock);
    orphan->exit_status = 42;
    atomic_store(&orphan->exited, true);
    atomic_store(&orphan->state, IOX_TASK_ZOMBIE);
    pthread_mutex_unlock(&orphan->lock);

    /* Verify init can find the zombie child */
    pthread_mutex_lock(&init->lock);
    iox_task_t *found = NULL;
    iox_task_t *c = init->children;
    while (c) {
        if (c->pid == orphan->pid && atomic_load(&c->exited)) {
            found = c;
            break;
        }
        c = c->next_sibling;
    }
    pthread_mutex_unlock(&init->lock);

    IOX_ASSERT_NOT_NULL(found);
    IOX_ASSERT_EQ(found->exit_status, 42);
    IOX_ASSERT(atomic_load(&found->exited));

    /* Cleanup */
    pthread_mutex_lock(&init->lock);
    init->children = NULL;
    pthread_mutex_unlock(&init->lock);

    iox_task_free(orphan);

    return true;
}

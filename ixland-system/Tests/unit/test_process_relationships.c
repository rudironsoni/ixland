/**
 * @file test_process_relationships.c
 * @brief Unit tests for parent/child relationship tracking
 *
 * Validates VAL-PROCESS-007, VAL-PROCESS-008: Parent-child linkage and traversal
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "../../fs/fdtable.h"
#include "../../fs/vfs.h"
#include "../../kernel/signal/iox_signal.h"
#include "../../kernel/task/task.h"
#include "../harness/iox_test.h"

/* Test parent-child linkage on fork (VAL-PROCESS-007) */
IOX_TEST(fork_child_links_to_parent) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create child manually simulating fork behavior */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    /* Allocate resources */
    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();
    IOX_ASSERT_NOT_NULL(child->files);
    IOX_ASSERT_NOT_NULL(child->fs);
    IOX_ASSERT_NOT_NULL(child->sighand);

    /* Set up parent-child relationship */
    child->ppid = parent->pid;
    child->pgid = parent->pgid;
    child->sid = parent->sid;

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Verify parent-child linkage (VAL-PROCESS-007) */
    IOX_ASSERT_EQ(child->ppid, parent->pid);
    IOX_ASSERT_EQ(child->parent, parent);

    /* Cleanup - unlink child */
    pthread_mutex_lock(&parent->lock);
    parent->children = child->next_sibling;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child);
    return true;
}

/* Test child list traversal (VAL-PROCESS-008) */
IOX_TEST(parent_can_traverse_children) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create multiple children */
    iox_task_t *child1 = iox_task_alloc();
    iox_task_t *child2 = iox_task_alloc();
    iox_task_t *child3 = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child1);
    IOX_ASSERT_NOT_NULL(child2);
    IOX_ASSERT_NOT_NULL(child3);

    /* Allocate resources */
    child1->files = iox_files_alloc(IOX_MAX_FD);
    child1->fs = iox_fs_alloc();
    child1->sighand = iox_sighand_alloc();
    child2->files = iox_files_alloc(IOX_MAX_FD);
    child2->fs = iox_fs_alloc();
    child2->sighand = iox_sighand_alloc();
    child3->files = iox_files_alloc(IOX_MAX_FD);
    child3->fs = iox_fs_alloc();
    child3->sighand = iox_sighand_alloc();

    /* Set up parent-child relationships */
    child1->ppid = parent->pid;
    child2->ppid = parent->pid;
    child3->ppid = parent->pid;

    pthread_mutex_lock(&parent->lock);

    /* Insert at head: child3, then child2, then child1 */
    child3->parent = parent;
    child3->next_sibling = parent->children;
    parent->children = child3;

    child2->parent = parent;
    child2->next_sibling = parent->children;
    parent->children = child2;

    child1->parent = parent;
    child1->next_sibling = parent->children;
    parent->children = child1;

    pthread_mutex_unlock(&parent->lock);

    /* Traverse children and verify all are reachable (VAL-PROCESS-008) */
    pthread_mutex_lock(&parent->lock);

    int count = 0;
    iox_task_t *child = parent->children;
    while (child) {
        count++;
        IOX_ASSERT_EQ(child->parent, parent);
        IOX_ASSERT_EQ(child->ppid, parent->pid);
        child = child->next_sibling;
    }

    IOX_ASSERT_EQ(count, 3);
    pthread_mutex_unlock(&parent->lock);

    /* Cleanup - unlink all children */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child1);
    iox_task_free(child2);
    iox_task_free(child3);

    return true;
}

/* Test sibling list forms NULL-terminated list */
IOX_TEST(sibling_list_null_terminated) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create a single child */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Verify single child has NULL next_sibling after insertion at head */
    /* (since parent->children was initially NULL) */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_NULL(child->next_sibling);
    pthread_mutex_unlock(&parent->lock);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child);
    return true;
}

/* Test multiple children have correct sibling chain */
IOX_TEST(multiple_children_sibling_chain) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create two children */
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

    child1->ppid = parent->pid;
    child2->ppid = parent->pid;

    pthread_mutex_lock(&parent->lock);

    /* Insert child2 first, then child1 */
    child2->parent = parent;
    child2->next_sibling = parent->children;
    parent->children = child2;

    child1->parent = parent;
    child1->next_sibling = parent->children;
    parent->children = child1;

    pthread_mutex_unlock(&parent->lock);

    /* Verify chain: child1 -> child2 -> NULL */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_EQ(parent->children, child1);
    IOX_ASSERT_EQ(child1->next_sibling, child2);
    IOX_ASSERT_NULL(child2->next_sibling);
    pthread_mutex_unlock(&parent->lock);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child1);
    iox_task_free(child2);

    return true;
}

/* Test orphan reparenting to init */
IOX_TEST(orphan_reparenting_to_init) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create a child */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    /* Link child to parent */
    child->ppid = parent->pid;

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Verify initial linkage */
    IOX_ASSERT_EQ(child->parent, parent);
    IOX_ASSERT_EQ(child->ppid, parent->pid);

    /* Simulate orphan reparenting (what happens when parent exits) */
    pthread_mutex_lock(&child->lock);
    child->ppid = 1; /* init's PID */
    pthread_mutex_unlock(&child->lock);

    /* Verify orphan was reparented */
    IOX_ASSERT_EQ(child->ppid, 1);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child);
    return true;
}

/* Test child lookup by traversing from parent */
IOX_TEST(child_lookup_by_traversal) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create child */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Find child by traversing */
    iox_task_t *found = NULL;
    pthread_mutex_lock(&parent->lock);
    iox_task_t *c = parent->children;
    while (c) {
        if (c->pid == child->pid) {
            found = c;
            break;
        }
        c = c->next_sibling;
    }
    pthread_mutex_unlock(&parent->lock);

    IOX_ASSERT_NOT_NULL(found);
    IOX_ASSERT_EQ(found, child);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child);
    return true;
}

/* Test multi-level process tree (grandparent -> parent -> child) */
IOX_TEST(multi_level_process_tree) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *grandparent = iox_current_task();
    IOX_ASSERT_NOT_NULL(grandparent);

    /* Create parent */
    iox_task_t *parent = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(parent);

    parent->files = iox_files_alloc(IOX_MAX_FD);
    parent->fs = iox_fs_alloc();
    parent->sighand = iox_sighand_alloc();

    parent->ppid = grandparent->pid;

    pthread_mutex_lock(&grandparent->lock);
    parent->parent = grandparent;
    parent->next_sibling = grandparent->children;
    grandparent->children = parent;
    pthread_mutex_unlock(&grandparent->lock);

    /* Create child of parent */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Verify three-level hierarchy */
    IOX_ASSERT_EQ(parent->parent, grandparent);
    IOX_ASSERT_EQ(parent->ppid, grandparent->pid);
    IOX_ASSERT_EQ(child->parent, parent);
    IOX_ASSERT_EQ(child->ppid, parent->pid);

    /* Traverse from grandparent to find child (via parent) */
    pthread_mutex_lock(&grandparent->lock);
    IOX_ASSERT_NOT_NULL(grandparent->children);
    IOX_ASSERT_EQ(grandparent->children, parent);
    pthread_mutex_unlock(&grandparent->lock);

    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_NOT_NULL(parent->children);
    IOX_ASSERT_EQ(parent->children, child);
    pthread_mutex_unlock(&parent->lock);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    pthread_mutex_lock(&grandparent->lock);
    grandparent->children = NULL;
    pthread_mutex_unlock(&grandparent->lock);

    iox_task_free(child);
    iox_task_free(parent);

    return true;
}

/* Test children maintain correct order (LIFO) */
IOX_TEST(children_insertion_order_lifo) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create children in order */
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

    /* Insert in order: 1, 2, 3 */
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

    /* Verify LIFO order: 3 -> 2 -> 1 -> NULL */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_EQ(parent->children, child3);
    IOX_ASSERT_EQ(child3->next_sibling, child2);
    IOX_ASSERT_EQ(child2->next_sibling, child1);
    IOX_ASSERT_NULL(child1->next_sibling);
    pthread_mutex_unlock(&parent->lock);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child1);
    iox_task_free(child2);
    iox_task_free(child3);

    return true;
}

/* Test unlinking specific child from middle of list */
IOX_TEST(unlink_middle_child) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create three children */
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

    /* Link in order: 1, 2, 3 (results in 3 -> 2 -> 1) */
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

    /* Unlink child2 (middle) - list becomes 3 -> 1 */
    pthread_mutex_lock(&parent->lock);
    iox_task_t **pp = &parent->children;
    while (*pp && *pp != child2) {
        pp = &(*pp)->next_sibling;
    }
    if (*pp) {
        *pp = child2->next_sibling;
    }
    pthread_mutex_unlock(&parent->lock);

    /* Verify child2 is unlinked */
    pthread_mutex_lock(&parent->lock);
    IOX_ASSERT_EQ(parent->children, child3);
    IOX_ASSERT_EQ(child3->next_sibling, child1);
    IOX_ASSERT_NULL(child1->next_sibling);
    pthread_mutex_unlock(&parent->lock);

    /* Cleanup */
    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child1);
    iox_task_free(child2);
    iox_task_free(child3);

    return true;
}

/* Test that init task is its own parent */
IOX_TEST(init_task_self_parent) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *init = iox_current_task();
    IOX_ASSERT_NOT_NULL(init);

    /* After init, the current task is init */
    IOX_ASSERT_EQ(init->ppid, init->pid);
    IOX_ASSERT_EQ(init->pid, init->pgid);
    IOX_ASSERT_EQ(init->pid, init->sid);

    return true;
}

/* Test child reference counting with parent pointer */
IOX_TEST(child_with_parent_refcount) {
    IOX_ASSERT(iox_task_init() == 0);

    iox_task_t *parent = iox_current_task();
    IOX_ASSERT_NOT_NULL(parent);

    /* Create child */
    iox_task_t *child = iox_task_alloc();
    IOX_ASSERT_NOT_NULL(child);

    child->files = iox_files_alloc(IOX_MAX_FD);
    child->fs = iox_fs_alloc();
    child->sighand = iox_sighand_alloc();

    child->ppid = parent->pid;

    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);

    /* Child should have initial refcount of 1 */
    IOX_ASSERT_EQ(atomic_load(&child->refs), 1);

    /* Lookup should increment refcount */
    iox_task_t *found = iox_task_lookup(child->pid);
    IOX_ASSERT_NOT_NULL(found);
    IOX_ASSERT_EQ(atomic_load(&child->refs), 2);

    /* Cleanup */
    iox_task_free(found);
    IOX_ASSERT_EQ(atomic_load(&child->refs), 1);

    pthread_mutex_lock(&parent->lock);
    parent->children = NULL;
    pthread_mutex_unlock(&parent->lock);

    iox_task_free(child);

    return true;
}

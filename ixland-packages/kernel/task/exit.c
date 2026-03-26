#include "task.h"
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

void iox_exit(int status) {
    iox_task_t *task = iox_current_task();
    if (!task) {
        _Exit(status);
    }
    
    pthread_mutex_lock(&task->lock);
    
    /* Set exit status */
    task->exit_status = status;
    atomic_store(&task->exited, true);
    atomic_store(&task->state, IOX_TASK_ZOMBIE);
    
    /* Reparent children to init */
    iox_task_t *child = task->children;
    while (child) {
        pthread_mutex_lock(&child->lock);
        child->ppid = 1;  /* init process */
        pthread_mutex_unlock(&child->lock);
        child = child->next_sibling;
    }
    
    /* Wake up any waiters */
    if (task->waiters > 0) {
        pthread_cond_broadcast(&task->wait_cond);
    }
    
    pthread_mutex_unlock(&task->lock);
    
    /* Notify parent via SIGCHLD */
    if (task->parent) {
        /* In real implementation: kill(task->parent->pid, SIGCHLD); */
        pthread_mutex_lock(&task->parent->lock);
        if (task->parent->waiters > 0) {
            pthread_cond_broadcast(&task->parent->wait_cond);
        }
        pthread_mutex_unlock(&task->parent->lock);
    }
    
    /* Terminate thread but keep task until parent waits */
    pthread_exit(NULL);
}

void iox__exit(int status) {
    /* Immediate exit without cleanup */
    _Exit(status);
}

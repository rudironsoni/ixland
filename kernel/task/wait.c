#include "task.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

static int task_to_status(iox_task_t *task) {
    int status = 0;
    
    if (atomic_load(&task->signaled)) {
        status = task->termsig;
    } else {
        status = (task->exit_status & 0xFF) << 8;
    }
    
    return status;
}

pid_t iox_waitpid(pid_t pid, int *wstatus, int options) {
    iox_task_t *parent = iox_current_task();
    if (!parent) {
        errno = ESRCH;
        return -1;
    }
    
    iox_task_t *child = NULL;
    
    pthread_mutex_lock(&parent->lock);
    parent->waiters++;
    
    while (1) {
        /* Find matching child */
        if (pid > 0) {
            /* Wait for specific child */
            child = parent->children;
            while (child && child->pid != pid) {
                child = child->next_sibling;
            }
        } else if (pid == -1) {
            /* Wait for any child */
            child = parent->children;
        } else if (pid == 0) {
            /* Wait for any child in same process group */
            child = parent->children;
            while (child && child->pgid != parent->pgid) {
                child = child->next_sibling;
            }
        } else {
            /* pid < -1: Wait for any child in process group |pid| */
            pid_t pgid = -pid;
            child = parent->children;
            while (child && child->pgid != pgid) {
                child = child->next_sibling;
            }
        }
        
        if (child && atomic_load(&child->exited)) {
            /* Found exited child */
            break;
        }
        
        /* No matching exited child */
        if (options & WNOHANG) {
            parent->waiters--;
            pthread_mutex_unlock(&parent->lock);
            return 0;
        }
        
        if (!parent->children) {
            /* No children at all */
            parent->waiters--;
            pthread_mutex_unlock(&parent->lock);
            errno = ECHILD;
            return -1;
        }
        
        /* Wait for child to exit */
        struct timespec timeout;
        if (options & WNOHANG) {
            /* Non-blocking: just check once */
            parent->waiters--;
            pthread_mutex_unlock(&parent->lock);
            return 0;
        }
        
        /* Block waiting for child */
        pthread_cond_wait(&parent->wait_cond, &parent->lock);
    }
    
    /* Remove child from parent's list */
    iox_task_t **pp = &parent->children;
    while (*pp && *pp != child) {
        pp = &(*pp)->next_sibling;
    }
    if (*pp) {
        *pp = child->next_sibling;
    }
    
    parent->waiters--;
    pthread_mutex_unlock(&parent->lock);
    
    /* Return status */
    if (wstatus) {
        *wstatus = task_to_status(child);
    }
    
    pid_t child_pid = child->pid;
    iox_task_free(child);
    
    return child_pid;
}

pid_t iox_wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage) {
    /* rusage not implemented yet - just call waitpid */
    (void)rusage;
    return iox_waitpid(pid, wstatus, options);
}

pid_t iox_wait(int *wstatus) {
    return iox_waitpid(-1, wstatus, 0);
}

pid_t iox_wait3(int *wstatus, int options, struct rusage *rusage) {
    return iox_wait4(-1, wstatus, options, rusage);
}

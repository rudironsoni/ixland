#include "iox_signal.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

iox_sighand_t *iox_sighand_alloc(void) {
    iox_sighand_t *sighand = calloc(1, sizeof(iox_sighand_t));
    if (!sighand) return NULL;
    
    atomic_init(&sighand->refs, 1);
    pthread_mutex_init(&sighand->queue.lock, NULL);
    
    /* Initialize default handlers */
    for (int i = 0; i < IOX_NSIG; i++) {
        sighand->action[i].sa_handler = SIG_DFL;
        sigemptyset(&sighand->action[i].sa_mask);
        sighand->action[i].sa_flags = 0;
    }
    
    sigemptyset(&sighand->blocked);
    sigemptyset(&sighand->pending);
    
    return sighand;
}

void iox_sighand_free(iox_sighand_t *sighand) {
    if (!sighand) return;
    if (atomic_fetch_sub(&sighand->refs, 1) > 1) return;
    
    /* Free queued signals */
    pthread_mutex_lock(&sighand->queue.lock);
    iox_sigqueue_entry_t *entry = sighand->queue.head;
    while (entry) {
        iox_sigqueue_entry_t *next = entry->next;
        free(entry);
        entry = next;
    }
    pthread_mutex_unlock(&sighand->queue.lock);
    
    pthread_mutex_destroy(&sighand->queue.lock);
    free(sighand);
}

iox_sighand_t *iox_sighand_dup(iox_sighand_t *parent) {
    if (!parent) return NULL;
    
    iox_sighand_t *child = iox_sighand_alloc();
    if (!child) return NULL;
    
    /* Copy signal handlers */
    memcpy(child->action, parent->action, sizeof(child->action));
    
    /* Child inherits parent's signal mask */
    child->blocked = parent->blocked;
    
    /* But pending signals are cleared */
    sigemptyset(&child->pending);
    
    return child;
}

int iox_sigaction(int sig, const struct sigaction *act, struct sigaction *oldact) {
    if (sig < 1 || sig >= IOX_NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    if (sig == SIGKILL || sig == SIGSTOP) {
        errno = EINVAL;
        return -1;
    }
    
    iox_task_t *task = iox_current_task();
    if (!task || !task->sighand) {
        errno = ESRCH;
        return -1;
    }
    
    if (oldact) {
        *oldact = task->sighand->action[sig];
    }
    
    if (act) {
        task->sighand->action[sig] = *act;
    }
    
    return 0;
}

/* Apply signal to a single task with state transitions and parent notification.
 * Must be called with task lock held. Does NOT release task reference.
 */
static void __iox_apply_signal_to_task(iox_task_t *task, int sig) {
    int terminating = (sig == SIGTERM || sig == SIGKILL || sig == SIGINT);
    /* Add signal to pending */
    sigaddset(&task->sighand->pending, sig);
    
    /* Handle SIGSTOP: transition to STOPPED state (not reaped) */
    if (sig == SIGSTOP) {
        atomic_store(&task->state, IOX_TASK_STOPPED);
        atomic_store(&task->stopped, true);
        atomic_store(&task->stopsig, SIGSTOP);
    }
    
    /* Handle SIGCONT: transition back to RUNNING (not reaped) */
    if (sig == SIGCONT) {
        atomic_store(&task->state, IOX_TASK_RUNNING);
        atomic_store(&task->stopped, false);
        atomic_store(&task->continued, true);
    }
    
    /* Handle terminating signals: mark for reap on wait (SIGKILL, SIGTERM, SIGINT) */
    if (terminating) {
        atomic_store(&task->signaled, true);
        atomic_store(&task->termsig, sig);
    }
    
    /* Notify parent if this child state is wait-visible */
    int state_changed = (sig == SIGSTOP) || (sig == SIGCONT) || atomic_load(&task->signaled);
    if (state_changed && task->parent) {
        pthread_mutex_lock(&task->parent->lock);
        if (task->parent->waiters > 0) {
            pthread_cond_broadcast(&task->parent->wait_cond);
        }
        pthread_mutex_unlock(&task->parent->lock);
    }
    
    /* Wake up this task if waiting */
    if (task->waiters > 0) {
        pthread_cond_broadcast(&task->wait_cond);
    }
}

int iox_kill(pid_t pid, int sig) {
    if (sig < 0 || sig >= IOX_NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    if (pid <= 0) {
        /* Process group handling */
        if (pid == 0) {
            /* Current process group */
            iox_task_t *task = iox_current_task();
            if (!task) {
                errno = ESRCH;
                return -1;
            }
            return iox_killpg(task->pgid, sig);
        } else if (pid == -1) {
            /* All processes (privileged) */
            errno = EPERM;
            return -1;
        } else {
            /* Process group |pid| */
            return iox_killpg(-pid, sig);
        }
    }
    
    iox_task_t *task = iox_task_lookup(pid);
    if (!task) {
        errno = ESRCH;
        return -1;
    }
    
    if (sig == 0) {
        /* Just check if process exists */
        iox_task_free(task);
        return 0;
    }
    
    /* Apply signal with state transitions and notifications */
    pthread_mutex_lock(&task->lock);
    __iox_apply_signal_to_task(task, sig);
    pthread_mutex_unlock(&task->lock);
    
    iox_task_free(task);
    return 0;
}

int iox_killpg(pid_t pgrp, int sig) {
    /* Contract:
     * - pgrp <= 0: return -1, errno = EINVAL
     * - no matching tasks: return -1, errno = ESRCH
     * - valid PGID with matches: deliver to all, return 0
     */
    
    /* Validate signal number */
    if (sig < 0 || sig >= IOX_NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    /* Validate process group ID (must be positive) */
    if (pgrp <= 0) {
        errno = EINVAL;
        return -1;
    }
    
    /* Signal 0 is used to check if process group exists */
    int check_only = (sig == 0);
    
    /* Collect matching tasks under lock */
    #define IOX_KILLPG_MAX_MATCHES 256
    iox_task_t *matches[IOX_KILLPG_MAX_MATCHES];
    int match_count = 0;
    
    extern pthread_mutex_t task_table_lock;
    extern iox_task_t *task_table[];
    extern size_t task_hash(pid_t pid);
    
    pthread_mutex_lock(&task_table_lock);
    
    /* Iterate all buckets in task table */
    for (int i = 0; i < IOX_MAX_TASKS; i++) {
        iox_task_t *task = task_table[i];
        while (task) {
            if (task->pgid == pgrp) {
                if (match_count < IOX_KILLPG_MAX_MATCHES) {
                    atomic_fetch_add(&task->refs, 1);
                    matches[match_count++] = task;
                }
                /* If we hit the limit, continue counting but don't store */
            }
            task = task->hash_next;
        }
    }
    
    pthread_mutex_unlock(&task_table_lock);
    
    /* No matching tasks found */
    if (match_count == 0) {
        errno = ESRCH;
        return -1;
    }
    
    /* Signal 0: just checking existence, don't actually deliver */
    if (check_only) {
        for (int i = 0; i < match_count; i++) {
            iox_task_free(matches[i]);
        }
        return 0;
    }
    
    /* Deliver signal to each matched task using shared helper */
    for (int i = 0; i < match_count; i++) {
        iox_task_t *task = matches[i];
        
        pthread_mutex_lock(&task->lock);
        __iox_apply_signal_to_task(task, sig);
        pthread_mutex_unlock(&task->lock);
        
        iox_task_free(matches[i]);
    }
    
    return 0;
}

int iox_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    iox_task_t *task = iox_current_task();
    if (!task || !task->sighand) {
        errno = ESRCH;
        return -1;
    }
    
    if (oldset) {
        *oldset = task->sighand->blocked;
    }
    
    if (set) {
        switch (how) {
        case SIG_BLOCK:
            for (int i = 1; i < IOX_NSIG; i++) {
                if (sigismember(set, i)) {
                    sigaddset(&task->sighand->blocked, i);
                }
            }
            break;
        case SIG_UNBLOCK:
            for (int i = 1; i < IOX_NSIG; i++) {
                if (sigismember(set, i)) {
                    sigdelset(&task->sighand->blocked, i);
                }
            }
            break;
        case SIG_SETMASK:
            task->sighand->blocked = *set;
            break;
        default:
            errno = EINVAL;
            return -1;
        }
    }
    
    return 0;
}

int iox_sigpending(sigset_t *set) {
    if (!set) {
        errno = EFAULT;
        return -1;
    }
    
    iox_task_t *task = iox_current_task();
    if (!task || !task->sighand) {
        errno = ESRCH;
        return -1;
    }
    
    *set = task->sighand->pending;
    return 0;
}

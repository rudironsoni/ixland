#include "signal.h"
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
    
    /* Add signal to pending */
    pthread_mutex_lock(&task->lock);
    sigaddset(&task->sighand->pending, sig);
    
    /* Wake up if waiting */
    if (task->waiters > 0) {
        pthread_cond_broadcast(&task->wait_cond);
    }
    pthread_mutex_unlock(&task->lock);
    
    iox_task_free(task);
    return 0;
}

int iox_killpg(pid_t pgrp, int sig) {
    /* Find all tasks in process group and send signal */
    /* Simplified: iterate through all tasks */
    /* In production: maintain per-pgrp hash table */
    (void)pgrp;
    (void)sig;
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
            sigorset(&task->sighand->blocked, &task->sighand->blocked, set);
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

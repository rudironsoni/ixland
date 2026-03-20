/*
 * signal.c - Signal handling syscalls
 */

#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "../../include/linux/signal.h"
#include "../../include/linux/unistd.h"

/* ============================================================================
 * Signal Handler Table
 * ============================================================================ */

static struct sigaction signal_handlers[NSIG];
static pthread_mutex_t signal_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ============================================================================
 * Signal Handling
 * ============================================================================ */

a_shell_sighandler_t a_shell_signal(int signum, a_shell_sighandler_t handler) {
    if (signum < 1 || signum >= NSIG) {
        errno = EINVAL;
        return SIG_ERR;
    }
    
    pthread_mutex_lock(&signal_mutex);
    
    a_shell_sighandler_t old_handler = (a_shell_sighandler_t)signal_handlers[signum].sa_handler;
    
    /* Update our table */
    memset(&signal_handlers[signum], 0, sizeof(struct sigaction));
    signal_handlers[signum].sa_handler = (void (*)(int))handler;
    sigemptyset(&signal_handlers[signum].sa_mask);
    signal_handlers[signum].sa_flags = 0;
    
    /* Install actual signal handler */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = (void (*)(int))handler;
    sigemptyset(&sa.sa_mask);
    
    int ret = sigaction(signum, &sa, NULL);
    
    pthread_mutex_unlock(&signal_mutex);
    
    if (ret < 0) {
        return SIG_ERR;
    }
    
    return old_handler;
}

int a_shell_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    if (signum < 1 || signum >= NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    pthread_mutex_lock(&signal_mutex);
    
    if (oldact) {
        *oldact = signal_handlers[signum];
    }
    
    if (act) {
        signal_handlers[signum] = *act;
        
        /* Install actual signal handler */
        struct sigaction sa = *act;
        int ret = sigaction(signum, &sa, NULL);
        
        pthread_mutex_unlock(&signal_mutex);
        return ret;
    }
    
    pthread_mutex_unlock(&signal_mutex);
    return 0;
}

/* ============================================================================
 * Send Signals
 * ============================================================================ */

int a_shell_kill(pid_t pid, int sig) {
    if (pid == a_shell_getpid()) {
        return raise(sig);
    }
    
    /* For virtual PIDs, would need to signal the thread */
    /* For now, return success for signal 0 (check process exists) */
    if (sig == 0) {
        return 0;
    }
    
    errno = ESRCH;
    return -1;
}

int a_shell_killpg(pid_t pgrp, int sig) {
    /* Process groups not fully supported on iOS */
    if (pgrp == 0 || pgrp == a_shell_getpgrp()) {
        return a_shell_kill(-a_shell_getpgrp(), sig);
    }
    
    errno = ESRCH;
    return -1;
}

int a_shell_raise(int sig) {
    return raise(sig);
}

/* ============================================================================
 * Signal Sets
 * ============================================================================ */

int a_shell_sigemptyset(sigset_t *set) {
    return sigemptyset(set);
}

int a_shell_sigfillset(sigset_t *set) {
    return sigfillset(set);
}

int a_shell_sigaddset(sigset_t *set, int signum) {
    return sigaddset(set, signum);
}

int a_shell_sigdelset(sigset_t *set, int signum) {
    return sigdelset(set, signum);
}

int a_shell_sigismember(const sigset_t *set, int signum) {
    return sigismember(set, signum);
}

/* ============================================================================
 * Signal Masks
 * ============================================================================ */

int a_shell_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    return sigprocmask(how, set, oldset);
}

int a_shell_sigpending(sigset_t *set) {
    return sigpending(set);
}

int a_shell_sigsuspend(const sigset_t *mask) {
    return sigsuspend(mask);
}

/* ============================================================================
 * Signal Actions
 * ============================================================================ */

int a_shell_siginterrupt(int sig, int flag) {
    /* On modern systems, this is equivalent to setting/clearing SA_RESTART */
    struct sigaction sa;
    
    if (sigaction(sig, NULL, &sa) < 0) {
        return -1;
    }
    
    if (flag) {
        sa.sa_flags &= ~SA_RESTART;
    } else {
        sa.sa_flags |= SA_RESTART;
    }
    
    return sigaction(sig, &sa, NULL);
}

/* ============================================================================
 * Signal File Descriptors
 * ============================================================================ */

int a_shell_signalfd(int fd, const sigset_t *mask, int flags) {
    /* signalfd not available on macOS/iOS */
    errno = ENOSYS;
    return -1;
}

/* ============================================================================
 * Signal Information
 * ============================================================================ */

int a_shell_sigwait(const sigset_t *set, int *sig) {
    return sigwait(set, sig);
}

int a_shell_sigwaitinfo(const sigset_t *set, siginfo_t *info) {
    /* sigwaitinfo not available on macOS/iOS, use sigwait */
    int sig;
    int ret = sigwait(set, &sig);
    if (ret == 0 && info) {
        memset(info, 0, sizeof(*info));
        info->si_signo = sig;
    }
    return ret;
}

int a_shell_sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout) {
    /* sigtimedwait not available on macOS/iOS */
    errno = ENOSYS;
    return -1;
}

/* ============================================================================
 * Signal Queue
 * ============================================================================ */

int a_shell_sigqueue(pid_t pid, int sig, const union sigval value) {
    /* sigqueue not available on macOS/iOS */
    /* Fall back to kill */
    return a_shell_kill(pid, sig);
}

#ifndef IOX_SIGNAL_H
#define IOX_SIGNAL_H

#include <signal.h>
#include <stdbool.h>
#include "../task/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Signal queue entry */
typedef struct iox_sigqueue_entry {
    int sig;
    siginfo_t info;
    struct iox_sigqueue_entry *next;
} iox_sigqueue_entry_t;

/* Signal queue */
typedef struct iox_sigqueue {
    iox_sigqueue_entry_t *head;
    iox_sigqueue_entry_t *tail;
    int count;
    pthread_mutex_t lock;
} iox_sigqueue_t;

/* Signal handling state */
struct iox_sighand {
    struct sigaction action[IOX_NSIG];
    sigset_t blocked;
    sigset_t pending;
    iox_sigqueue_t queue;
    atomic_int refs;
};

iox_sighand_t *iox_sighand_alloc(void);
void iox_sighand_free(iox_sighand_t *sighand);
iox_sighand_t *iox_sighand_dup(iox_sighand_t *parent);

int iox_sigaction(int sig, const struct sigaction *act, struct sigaction *oldact);
int iox_kill(pid_t pid, int sig);
int iox_killpg(pid_t pgrp, int sig);
int iox_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int iox_sigpending(sigset_t *set);
int iox_sigsuspend(const sigset_t *mask);
int iox_sigqueue(pid_t pid, int sig, const union sigval value);
int iox_sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout);

void iox_signal_init(void);
void iox_signal_deinit(void);

#ifdef __cplusplus
}
#endif

#endif

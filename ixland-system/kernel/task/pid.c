#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"

#define IOX_MIN_PID 1000
#define IOX_MAX_PID 65535

static _Atomic pid_t next_pid = IOX_MIN_PID;
static pthread_mutex_t pid_lock = PTHREAD_MUTEX_INITIALIZER;

pid_t iox_alloc_pid(void) {
    pthread_mutex_lock(&pid_lock);
    pid_t pid = atomic_fetch_add(&next_pid, 1);
    if (pid >= IOX_MAX_PID) {
        pid = IOX_MIN_PID;
        atomic_store(&next_pid, IOX_MIN_PID + 1);
    }
    pthread_mutex_unlock(&pid_lock);
    return pid;
}

void iox_free_pid(pid_t pid) {
    (void)pid;
}

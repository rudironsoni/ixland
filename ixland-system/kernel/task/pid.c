#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"

#define IOX_MIN_PID 1000
#define IOX_MAX_PID 65535
#define IOX_PID_COUNT (IOX_MAX_PID - IOX_MIN_PID + 1)

/* Free list stack for O(1) PID allocation/reuse */
static pid_t pid_free_stack[IOX_PID_COUNT];
static _Atomic int pid_stack_top = 0;
static pthread_mutex_t pid_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_bool pid_initialized = false;

/**
 * @brief Initialize the PID allocator with all available PIDs.
 *
 * Populates the free stack with PIDs from IOX_MAX_PID down to IOX_MIN_PID
 * so that sequential allocation starts from IOX_MIN_PID.
 */
void iox_pid_init(void) {
    if (atomic_load(&pid_initialized)) {
        return;
    }

    pthread_mutex_lock(&pid_lock);
    if (atomic_load(&pid_initialized)) {
        pthread_mutex_unlock(&pid_lock);
        return;
    }

    /* Push PIDs in reverse order so IOX_MIN_PID is popped first */
    int idx = 0;
    for (pid_t pid = IOX_MAX_PID; pid >= IOX_MIN_PID; pid--) {
        pid_free_stack[idx++] = pid;
    }
    atomic_store(&pid_stack_top, IOX_PID_COUNT);
    atomic_store(&pid_initialized, true);

    pthread_mutex_unlock(&pid_lock);
}

/**
 * @brief Allocate a unique PID in O(1) time.
 *
 * Uses atomic stack pop for lock-free allocation. If the free list is empty,
 * returns -1 (equivalent to EAGAIN).
 *
 * @return Allocated PID (>= IOX_MIN_PID) or -1 if no PIDs available
 */
pid_t iox_alloc_pid(void) {
    /* Ensure initialized (thread-safe via atomic flag) */
    if (!atomic_load(&pid_initialized)) {
        iox_pid_init();
    }

    pthread_mutex_lock(&pid_lock);
    int top = atomic_load(&pid_stack_top);
    if (top <= 0) {
        pthread_mutex_unlock(&pid_lock);
        return -1; /* No PIDs available */
    }

    top--;
    pid_t pid = pid_free_stack[top];
    atomic_store(&pid_stack_top, top);
    pthread_mutex_unlock(&pid_lock);

    return pid;
}

/**
 * @brief Free a PID for immediate reuse.
 *
 * Returns the PID to the free list for O(1) reuse. Invalid PIDs are silently
 * ignored. The freed PID becomes immediately available for allocation.
 *
 * @param pid The PID to free
 */
void iox_free_pid(pid_t pid) {
    /* Validate PID range */
    if (pid < IOX_MIN_PID || pid > IOX_MAX_PID) {
        return;
    }

    /* Ensure initialized */
    if (!atomic_load(&pid_initialized)) {
        iox_pid_init();
    }

    pthread_mutex_lock(&pid_lock);
    int top = atomic_load(&pid_stack_top);

    /* Defensive: check for stack overflow (shouldn't happen with correct usage) */
    if (top >= IOX_PID_COUNT) {
        pthread_mutex_unlock(&pid_lock);
        return;
    }

    pid_free_stack[top] = pid;
    atomic_store(&pid_stack_top, top + 1);
    pthread_mutex_unlock(&pid_lock);
}

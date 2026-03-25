#include "task.h"
#include "../../fs/fdtable.h"
#include "../../fs/vfs.h"
#include "../signal/iox_signal.h"
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <pthread.h>

/* Note: IOX_MAX_TASKS defined in task.h for cross-module access */

static __thread iox_task_t *current_task = NULL;
iox_task_t *init_task = NULL;

/* Task table - accessible to signal.c for iox_killpg */
pthread_mutex_t task_table_lock = PTHREAD_MUTEX_INITIALIZER;
iox_task_t *task_table[IOX_MAX_TASKS] = {NULL};

size_t task_hash(pid_t pid) {
    return (size_t)(pid % IOX_MAX_TASKS);
}

iox_task_t *iox_current_task(void) {
    return current_task;
}

void iox_set_current_task(iox_task_t *task) {
    current_task = task;
}

iox_task_t *iox_task_alloc(void) {
    iox_task_t *task = calloc(1, sizeof(iox_task_t));
    if (!task) return NULL;
    
    task->pid = iox_alloc_pid();
    task->tgid = task->pid;
    task->pgid = task->pid;
    task->sid = task->pid;
    
    atomic_init(&task->state, IOX_TASK_RUNNING);
    atomic_init(&task->refs, 1);
    atomic_init(&task->exited, false);
    atomic_init(&task->signaled, false);
    
    pthread_mutex_init(&task->lock, NULL);
    pthread_cond_init(&task->wait_cond, NULL);
    pthread_mutex_init(&task->wait_lock, NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &task->start_time);
    
    int idx = task_hash(task->pid);
    pthread_mutex_lock(&task_table_lock);
    task->hash_next = task_table[idx];
    task_table[idx] = task;
    pthread_mutex_unlock(&task_table_lock);
    
    return task;
}

void iox_task_free(iox_task_t *task) {
    if (!task) return;
    
    if (atomic_fetch_sub(&task->refs, 1) > 1) return;
    
    int idx = task_hash(task->pid);
    pthread_mutex_lock(&task_table_lock);
    iox_task_t **pp = &task_table[idx];
    while (*pp && *pp != task) {
        pp = &(*pp)->hash_next;
    }
    if (*pp) {
        *pp = task->hash_next;
    }
    pthread_mutex_unlock(&task_table_lock);
    
    if (task->files) iox_files_free(task->files);
    if (task->fs) free(task->fs);
    if (task->sighand) free(task->sighand);
    if (task->tty) atomic_fetch_sub(&task->tty->refs, 1);
    if (task->mm) free(task->mm);
    if (task->exec_image) free(task->exec_image);
    
    pthread_cond_destroy(&task->wait_cond);
    pthread_mutex_destroy(&task->wait_lock);
    pthread_mutex_destroy(&task->lock);
    
    iox_free_pid(task->pid);
    free(task);
}

iox_task_t *iox_task_lookup(pid_t pid) {
    int idx = task_hash(pid);
    pthread_mutex_lock(&task_table_lock);
    iox_task_t *task = task_table[idx];
    while (task && task->pid != pid) {
        task = task->hash_next;
    }
    if (task) {
        atomic_fetch_add(&task->refs, 1);
    }
    pthread_mutex_unlock(&task_table_lock);
    return task;
}

static void iox_task_init_once(void) {
    init_task = iox_task_alloc();
    if (!init_task) return;
    
    init_task->ppid = init_task->pid;
    strncpy(init_task->comm, "init", sizeof(init_task->comm));
    
    init_task->files = iox_files_alloc(IOX_MAX_FD);
    if (!init_task->files) {
        iox_task_free(init_task);
        init_task = NULL;
        return;
    }
    
    init_task->fs = iox_fs_alloc();
    if (!init_task->fs) {
        iox_task_free(init_task);
        init_task = NULL;
        return;
    }
    
    init_task->sighand = iox_sighand_alloc();
    if (!init_task->sighand) {
        iox_task_free(init_task);
        init_task = NULL;
        return;
    }
    
    current_task = init_task;
}

int iox_task_init(void) {
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    
    pthread_once(&once, iox_task_init_once);
    
    return init_task ? 0 : -1;
}

void iox_task_deinit(void) {
    if (init_task) {
        iox_task_free(init_task);
        init_task = NULL;
    }
}

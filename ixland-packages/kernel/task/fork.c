#include "task.h"
#include "../../fs/fdtable.h"
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <pthread.h>
#include <errno.h>

/* Fork context for child continuation */
typedef struct {
    iox_task_t *parent;
    iox_task_t *child;
    jmp_buf parent_jmp;
    jmp_buf child_jmp;
    volatile int child_ready;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} fork_ctx_t;

static void *fork_child_trampoline(void *arg) {
    fork_ctx_t *ctx = (fork_ctx_t *)arg;
    
    /* Set child as current task */
    iox_set_current_task(ctx->child);
    
    /* Signal parent that child is ready */
    pthread_mutex_lock(&ctx->lock);
    ctx->child_ready = 1;
    pthread_cond_signal(&ctx->cond);
    pthread_mutex_unlock(&ctx->lock);
    
    /* Jump to child continuation */
    longjmp(ctx->child_jmp, 1);
    
    /* NOTREACHED */
    return NULL;
}

pid_t iox_fork(void) {
    iox_task_t *parent = iox_current_task();
    if (!parent) {
        errno = ESRCH;
        return -1;
    }
    
    /* Allocate child task */
    iox_task_t *child = iox_task_alloc();
    if (!child) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Set up parent-child relationship */
    child->ppid = parent->pid;
    child->pgid = parent->pgid;
    child->sid = parent->sid;
    
    /* Copy subsystems with proper semantics */
    child->files = iox_files_dup(parent->files);
    if (!child->files) {
        iox_task_free(child);
        errno = ENOMEM;
        return -1;
    }
    
    /* Reference TTY (not copy) */
    if (parent->tty) {
        child->tty = parent->tty;
        atomic_fetch_add(&child->tty->refs, 1);
    }
    
    /* Link into parent's children list */
    pthread_mutex_lock(&parent->lock);
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);
    
    /* Set up fork context */
    fork_ctx_t ctx;
    ctx.parent = parent;
    ctx.child = child;
    ctx.child_ready = 0;
    pthread_mutex_init(&ctx.lock, NULL);
    pthread_cond_init(&ctx.cond, NULL);
    
    /* Parent: Set up jump point */
    if (setjmp(ctx.parent_jmp) == 0) {
        /* Create child thread */
        pthread_t child_thread;
        int rc = pthread_create(&child_thread, NULL, fork_child_trampoline, &ctx);
        if (rc != 0) {
            /* Cleanup on failure */
            pthread_mutex_lock(&parent->lock);
            parent->children = child->next_sibling;
            pthread_mutex_unlock(&parent->lock);
            iox_task_free(child);
            errno = rc;
            return -1;
        }
        
        /* Wait for child to initialize */
        pthread_mutex_lock(&ctx.lock);
        while (!ctx.child_ready) {
            pthread_cond_wait(&ctx.cond, &ctx.lock);
        }
        pthread_mutex_unlock(&ctx.lock);
        
        /* Detach child thread - it will exit via longjmp */
        pthread_detach(child_thread);
        
        /* Parent returns child PID */
        return child->pid;
    }
    
    /* Parent continuation after fork */
    pthread_mutex_destroy(&ctx.lock);
    pthread_cond_destroy(&ctx.cond);
    
    return child->pid;
}

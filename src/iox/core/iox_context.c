/* iOS Subsystem for Linux - Thread Context and Process Simulation
 * 
 * Maximum fidelity process simulation within iOS constraints.
 * Captures and isolates: memory, file descriptors, signals, environment, working directory
 */

#include "../internal/iox_internal.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* Forward declarations */
pid_t iox_getpid(void);
pid_t iox_getppid(void);
int iox_kill(pid_t pid, int sig);
int iox_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int iox_open(const char *pathname, int flags, ...);
ssize_t iox_read(int fd, void *buf, size_t count);
ssize_t iox_write(int fd, const void *buf, size_t count);
int iox_close(int fd);
off_t iox_lseek(int fd, off_t offset, int whence);
pid_t iox_waitpid(pid_t pid, int *stat_loc, int options);
void iox_exit(int status);

/* ============================================================================
 * PROCESS CONTEXT
 * ============================================================================ */
 
/* iox_context_t is defined in iox_internal.h as struct iox_process_context */
/* It is used for thread-based process simulation */
/* See iox_internal.h for the full definition */

/* Context-local current context */
extern __thread iox_context_t *__iox_current_ctx;

/* ============================================================================
 * GLOBAL STATE
 * ============================================================================ */

static pthread_mutex_t ctx_table_lock = PTHREAD_MUTEX_INITIALIZER;
static iox_context_t *ctx_table[IOX_MAX_PROCESSES] = {NULL};
static atomic_pid_t next_pid = IOX_MIN_PID;

/* Thread-local current context */
__thread iox_context_t *__iox_current_ctx = NULL;

/* ============================================================================
 * CONTEXT MANAGEMENT
 * ============================================================================ */

iox_context_t *iox_ctx_alloc(pid_t pid, iox_context_t *parent) {
    iox_context_t *ctx = calloc(1, sizeof(iox_context_t));
    if (!ctx) {
        errno = ENOMEM;
        return NULL;
    }
    
    ctx->pid = pid;
    ctx->ppid = parent ? parent->pid : pid;
    ctx->pgid = parent ? parent->pgid : pid;
    ctx->sid = parent ? parent->sid : pid;
    
    atomic_init(&ctx->state, IOX_PROC_RUNNING);
    atomic_init(&ctx->refs, 1);
    
    pthread_mutex_init(&ctx->lock, NULL);
    pthread_mutex_init(&ctx->fd_lock, NULL);
    pthread_mutex_init(&ctx->env_lock, NULL);
    pthread_mutex_init(&ctx->sig_lock, NULL);
    pthread_cond_init(&ctx->wait_cond, NULL);
    
    /* Set default resource limits */
    ctx->rlimits[RLIMIT_NOFILE].rlim_cur = 256;
    ctx->rlimits[RLIMIT_NOFILE].rlim_max = 1024;
    ctx->rlimits[RLIMIT_NPROC].rlim_cur = 1024;
    ctx->rlimits[RLIMIT_NPROC].rlim_max = 1024;
    
    /* Insert into table */
    int idx = (pid - IOX_MIN_PID) % IOX_MAX_PROCESSES;
    pthread_mutex_lock(&ctx_table_lock);
    ctx_table[idx] = ctx;
    pthread_mutex_unlock(&ctx_table_lock);
    
    return ctx;
}

void iox_ctx_free(iox_context_t *ctx) {
    if (!ctx) return;
    
    /* Remove from table */
    int idx = (ctx->pid - IOX_MIN_PID) % IOX_MAX_PROCESSES;
    pthread_mutex_lock(&ctx_table_lock);
    if (ctx_table[idx] == ctx) {
        ctx_table[idx] = NULL;
    }
    pthread_mutex_unlock(&ctx_table_lock);
    
    /* Close all FDs */
    pthread_mutex_lock(&ctx->fd_lock);
    for (int i = 0; i < IOX_MAX_FD; i++) {
        if (ctx->fd_table[i].used && ctx->fd_table[i].real_fd >= 0) {
            close(ctx->fd_table[i].real_fd);
        }
    }
    pthread_mutex_unlock(&ctx->fd_lock);
    
    /* Free environment */
    pthread_mutex_lock(&ctx->env_lock);
    if (ctx->env) {
        for (int i = 0; i < ctx->env_count; i++) {
            free(ctx->env[i]);
        }
        free(ctx->env);
    }
    pthread_mutex_unlock(&ctx->env_lock);
    
    pthread_mutex_destroy(&ctx->lock);
    pthread_mutex_destroy(&ctx->fd_lock);
    pthread_mutex_destroy(&ctx->env_lock);
    pthread_mutex_destroy(&ctx->sig_lock);
    pthread_cond_destroy(&ctx->wait_cond);
    
    free(ctx);
}

iox_context_t *iox_ctx_get(pid_t pid) {
    int idx = (pid - IOX_MIN_PID) % IOX_MAX_PROCESSES;
    pthread_mutex_lock(&ctx_table_lock);
    iox_context_t *ctx = ctx_table[idx];
    if (ctx) {
        atomic_fetch_add(&ctx->refs, 1);
    }
    pthread_mutex_unlock(&ctx_table_lock);
    return ctx;
}

void iox_ctx_put(iox_context_t *ctx) {
    if (!ctx) return;
    if (atomic_fetch_sub(&ctx->refs, 1) == 1) {
        iox_ctx_free(ctx);
    }
}

/* ============================================================================
 * FORK - Maximum Fidelity Within iOS Constraints
 * ============================================================================ */

/* What fork() needs to do:
 * 1. Create new thread
 * 2. Copy FD table (duplicating all open FDs)
 * 3. Copy environment
 * 4. Copy signal handlers and masks
 * 5. Copy working directory
 * 6. Copy resource limits
 * 7. Set up parent-child relationship
 */

static void *iox_child_thread(void *arg) {
    iox_context_t *ctx = arg;
    
    /* Set thread-local context */
    __iox_current_ctx = ctx;
    
    /* Copy parent's working directory */
    pthread_mutex_lock(&ctx->lock);
    if (ctx->cwd[0]) {
        chdir(ctx->cwd);
    }
    pthread_mutex_unlock(&ctx->lock);
    
    /* Restore signal mask */
    pthread_sigmask(SIG_SETMASK, &ctx->sig_mask, NULL);
    
    /* Return to fork() caller */
    return (void *)(intptr_t)0;  /* Child returns 0 */
}

pid_t iox_fork_full(void) {
    iox_context_t *parent = __iox_current_ctx;
    if (!parent) {
        /* Called outside process context - use main thread */
        parent = iox_ctx_get(iox_getpid());
    }
    
    /* Allocate new PID */
    pid_t pid = atomic_fetch_add(&next_pid, 1);
    
    /* Create child context */
    iox_context_t *child = iox_ctx_alloc(pid, parent);
    if (!child) {
        return -1;
    }
    
    /* COPY: File descriptors (FULL COPY, not shared) */
    pthread_mutex_lock(&parent->fd_lock);
    pthread_mutex_lock(&child->fd_lock);
    
    for (int i = 0; i < IOX_MAX_FD; i++) {
        if (parent->fd_table[i].used && parent->fd_table[i].real_fd >= 0) {
            /* Duplicate the file descriptor */
            int new_fd = dup(parent->fd_table[i].real_fd);
            if (new_fd >= 0) {
                child->fd_table[i] = parent->fd_table[i];
                child->fd_table[i].real_fd = new_fd;
                /* Reset offset tracking - each process has independent offset */
                child->fd_table[i].offset = parent->fd_table[i].offset;
            }
        }
    }
    
    pthread_mutex_unlock(&child->fd_lock);
    pthread_mutex_unlock(&parent->fd_lock);
    
    /* COPY: Environment */
    pthread_mutex_lock(&parent->env_lock);
    pthread_mutex_lock(&child->env_lock);
    
    if (parent->env && parent->env_count > 0) {
        child->env = calloc(parent->env_count + 1, sizeof(char *));
        child->env_count = parent->env_count;
        
        for (int i = 0; i < parent->env_count; i++) {
            if (parent->env[i]) {
                child->env[i] = strdup(parent->env[i]);
            }
        }
    }
    
    pthread_mutex_unlock(&child->env_lock);
    pthread_mutex_unlock(&parent->env_lock);
    
    /* COPY: Signal handlers and masks */
    pthread_mutex_lock(&parent->sig_lock);
    pthread_mutex_lock(&child->sig_lock);
    
    memcpy(child->sig_actions, parent->sig_actions, sizeof(child->sig_actions));
    child->sig_mask = parent->sig_mask;
    child->sig_pending = parent->sig_pending;
    
    pthread_mutex_unlock(&child->sig_lock);
    pthread_mutex_unlock(&parent->sig_lock);
    
    /* COPY: Working directory */
    pthread_mutex_lock(&parent->lock);
    strncpy(child->cwd, parent->cwd, IOX_MAX_PATH - 1);
    pthread_mutex_unlock(&parent->lock);
    
    /* COPY: Resource limits */
    memcpy(child->rlimits, parent->rlimits, sizeof(child->rlimits));
    
    /* COPY: Name */
    strncpy(child->name, parent->name, IOX_MAX_NAME - 1);
    
    /* Set up parent-child relationship */
    pthread_mutex_lock(&parent->lock);
    child->next_sibling = parent->children;
    parent->children = child;
    pthread_mutex_unlock(&parent->lock);
    
    /* Create child thread */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    /* Try to create thread with larger stack like a real process would have */
    size_t stacksize = 8 * 1024 * 1024;  /* 8MB like Linux default */
    pthread_attr_setstacksize(&attr, stacksize);
    
    if (pthread_create(&child->thread, &attr, iox_child_thread, child) != 0) {
        iox_ctx_free(child);
        pthread_attr_destroy(&attr);
        return -1;
    }
    pthread_attr_destroy(&attr);
    
    /* Parent returns child's PID */
    return pid;
}

/* ============================================================================
 * EXEC - Replace process image
 * ============================================================================ */

int iox_execve_full(const char *pathname, char *const argv[], char *const envp[]) {
    iox_context_t *ctx = __iox_current_ctx;
    if (!ctx) {
        errno = ENOENT;
        return -1;
    }
    
    /* In iOS, we cannot truly exec. Best we can do:
     * 1. Update process name
     * 2. Replace environment
     * 3. Set up argv[0]
     * 4. Return to caller (they must handle the "exec")
     * 
     * For a real implementation, we'd load a WASM binary or
     * dynamically loaded library and jump to it.
     */
    
    pthread_mutex_lock(&ctx->lock);
    
    /* Update process name from argv[0] */
    if (argv && argv[0]) {
        strncpy(ctx->name, argv[0], IOX_MAX_NAME - 1);
    } else if (pathname) {
        strncpy(ctx->name, pathname, IOX_MAX_NAME - 1);
    }
    
    pthread_mutex_unlock(&ctx->lock);
    
    /* Replace environment if provided */
    if (envp) {
        pthread_mutex_lock(&ctx->env_lock);
        
        /* Free old environment */
        if (ctx->env) {
            for (int i = 0; i < ctx->env_count; i++) {
                free(ctx->env[i]);
            }
            free(ctx->env);
        }
        
        /* Count new environment entries */
        int count = 0;
        while (envp[count]) count++;
        
        /* Allocate and copy */
        ctx->env = calloc(count + 1, sizeof(char *));
        ctx->env_count = count;
        
        for (int i = 0; i < count; i++) {
            ctx->env[i] = strdup(envp[i]);
        }
        
        pthread_mutex_unlock(&ctx->env_lock);
    }
    
    /* Return "success" - caller must actually perform the execution
     * In a real implementation with WAMR, we'd:
     * 1. Load the WASM binary
     * 2. Set up memory
     * 3. Transfer control to WASM runtime
     */
    
    return 0;
}

/* ============================================================================
 * WAIT - Parent waits for child
 * ============================================================================ */

pid_t iox_waitpid_full(pid_t pid, int *wstatus, int options) {
    iox_context_t *parent = __iox_current_ctx;
    if (!parent) {
        errno = ECHILD;
        return -1;
    }
    
    if (pid == -1) {
        /* Wait for any child */
        pthread_mutex_lock(&parent->lock);
        
        iox_context_t *child = parent->children;
        while (child) {
            int state = atomic_load(&child->state);
            if (state == IOX_PROC_ZOMBIE) {
                /* Found zombie child */
                pid_t child_pid = child->pid;
                if (wstatus) {
                    *wstatus = child->exit_status;
                }
                
                /* Remove from parent's children list */
                parent->children = child->next_sibling;
                pthread_mutex_unlock(&parent->lock);
                
                /* Release our reference */
                iox_ctx_put(child);
                return child_pid;
            }
            child = child->next_sibling;
        }
        
        /* No zombie children found */
        if (options & WNOHANG) {
            pthread_mutex_unlock(&parent->lock);
            return 0;
        }
        
        /* Block waiting for child */
        parent->waited_on = true;
        pthread_cond_wait(&parent->wait_cond, &parent->lock);
        parent->waited_on = false;
        pthread_mutex_unlock(&parent->lock);
        
        /* Try again */
        return iox_waitpid_full(-1, wstatus, options);
        
    } else {
        /* Wait for specific child */
        iox_context_t *child = iox_ctx_get(pid);
        if (!child || child->ppid != parent->pid) {
            errno = ECHILD;
            return -1;
        }
        
        /* Check if already exited */
        int state = atomic_load(&child->state);
        if (state == IOX_PROC_ZOMBIE) {
            if (wstatus) {
                *wstatus = child->exit_status;
            }
            iox_ctx_put(child);
            return pid;
        }
        
        if (options & WNOHANG) {
            iox_ctx_put(child);
            return 0;
        }
        
        /* Block waiting for specific child */
        pthread_mutex_lock(&child->lock);
        while (atomic_load(&child->state) != IOX_PROC_ZOMBIE) {
            pthread_cond_wait(&child->wait_cond, &child->lock);
        }
        pthread_mutex_unlock(&child->lock);
        
        if (wstatus) {
            *wstatus = child->exit_status;
        }
        
        iox_ctx_put(child);
        return pid;
    }
}

/* ============================================================================
 * EXIT - Process termination
 * ============================================================================ */

void iox_exit_full(int status) {
    iox_context_t *ctx = __iox_current_ctx;
    if (!ctx) {
        /* No context - just exit thread */
        pthread_exit((void *)(intptr_t)status);
    }
    
    pthread_mutex_lock(&ctx->lock);
    
    /* Store exit status */
    ctx->exit_status = W_EXITCODE(status & 0xFF, 0);
    atomic_store(&ctx->state, IOX_PROC_ZOMBIE);
    
    /* Notify parent */
    iox_context_t *parent = iox_ctx_get(ctx->ppid);
    if (parent) {
        pthread_mutex_lock(&parent->lock);
        if (parent->waited_on) {
            pthread_cond_signal(&parent->wait_cond);
        }
        pthread_mutex_unlock(&parent->lock);
        iox_ctx_put(parent);
    }
    
    /* Signal anyone waiting on this process */
    pthread_cond_broadcast(&ctx->wait_cond);
    pthread_mutex_unlock(&ctx->lock);
    
    /* Release our reference (context will be freed when parent waits) */
    iox_ctx_put(ctx);
    __iox_current_ctx = NULL;
    
    /* Exit thread */
    pthread_exit((void *)(intptr_t)status);
}

/* ============================================================================
 * FD OPERATIONS - Per-process FD table
 * ============================================================================ */

int iox_open_in_ctx(const char *pathname, int flags, mode_t mode) {
    iox_context_t *ctx = __iox_current_ctx;
    if (!ctx) {
        return open(pathname, flags, mode);
    }
    
    /* Allocate FD slot */
    pthread_mutex_lock(&ctx->fd_lock);
    
    int fd = -1;
    for (int i = 3; i < IOX_MAX_FD; i++) {
        if (!ctx->fd_table[i].used) {
            fd = i;
            break;
        }
    }
    
    if (fd < 0) {
        pthread_mutex_unlock(&ctx->fd_lock);
        errno = EMFILE;
        return -1;
    }
    
    /* Open through VFS */
    int real_fd = iox_vfs_open(pathname, flags, mode);
    if (real_fd < 0) {
        pthread_mutex_unlock(&ctx->fd_lock);
        return -1;
    }
    
    /* Store in table */
    ctx->fd_table[fd].used = true;
    ctx->fd_table[fd].real_fd = real_fd;
    ctx->fd_table[fd].flags = flags;
    ctx->fd_table[fd].offset = 0;
    strncpy(ctx->fd_table[fd].path, pathname, IOX_MAX_PATH - 1);
    
    pthread_mutex_unlock(&ctx->fd_lock);
    return fd;
}

ssize_t iox_read_in_ctx(int fd, void *buf, size_t count) {
    iox_context_t *ctx = __iox_current_ctx;
    if (!ctx || fd < 0 || fd >= IOX_MAX_FD) {
        return read(fd, buf, count);
    }
    
    pthread_mutex_lock(&ctx->fd_lock);
    
    if (!ctx->fd_table[fd].used) {
        pthread_mutex_unlock(&ctx->fd_lock);
        errno = EBADF;
        return -1;
    }
    
    int real_fd = ctx->fd_table[fd].real_fd;
    off_t *offset = &ctx->fd_table[fd].offset;
    
    pthread_mutex_unlock(&ctx->fd_lock);
    
    /* Read at current offset if regular file */
    ssize_t bytes;
    if (ctx->fd_table[fd].flags & O_DIRECTORY) {
        bytes = read(real_fd, buf, count);
    } else {
        bytes = pread(real_fd, buf, count, *offset);
        if (bytes > 0) {
            *offset += bytes;
        }
    }
    
    return bytes;
}

ssize_t iox_write_in_ctx(int fd, const void *buf, size_t count) {
    iox_context_t *ctx = __iox_current_ctx;
    if (!ctx || fd < 0 || fd >= IOX_MAX_FD) {
        return write(fd, buf, count);
    }
    
    pthread_mutex_lock(&ctx->fd_lock);
    
    if (!ctx->fd_table[fd].used) {
        pthread_mutex_unlock(&ctx->fd_lock);
        errno = EBADF;
        return -1;
    }
    
    int real_fd = ctx->fd_table[fd].real_fd;
    off_t *offset = &ctx->fd_table[fd].offset;
    
    pthread_mutex_unlock(&ctx->fd_lock);
    
    /* Write at current offset */
    ssize_t bytes = pwrite(real_fd, buf, count, *offset);
    if (bytes > 0) {
        *offset += bytes;
    }
    
    return bytes;
}

int iox_close_in_ctx(int fd) {
    iox_context_t *ctx = __iox_current_ctx;
    if (!ctx || fd < 0 || fd >= IOX_MAX_FD) {
        return close(fd);
    }
    
    pthread_mutex_lock(&ctx->fd_lock);
    
    if (!ctx->fd_table[fd].used) {
        pthread_mutex_unlock(&ctx->fd_lock);
        errno = EBADF;
        return -1;
    }
    
    int real_fd = ctx->fd_table[fd].real_fd;
    ctx->fd_table[fd].used = false;
    
    pthread_mutex_unlock(&ctx->fd_lock);
    
    return close(real_fd);
}

off_t iox_lseek_in_ctx(int fd, off_t offset, int whence) {
    iox_context_t *ctx = __iox_current_ctx;
    if (!ctx || fd < 0 || fd >= IOX_MAX_FD) {
        return lseek(fd, offset, whence);
    }
    
    pthread_mutex_lock(&ctx->fd_lock);
    
    if (!ctx->fd_table[fd].used) {
        pthread_mutex_unlock(&ctx->fd_lock);
        errno = EBADF;
        return -1;
    }
    
    off_t *current_offset = &ctx->fd_table[fd].offset;
    off_t new_offset;
    
    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = *current_offset + offset;
            break;
        case SEEK_END: {
            /* Get file size */
            struct stat st;
            if (fstat(ctx->fd_table[fd].real_fd, &st) < 0) {
                pthread_mutex_unlock(&ctx->fd_lock);
                return -1;
            }
            new_offset = st.st_size + offset;
            break;
        }
        default:
            pthread_mutex_unlock(&ctx->fd_lock);
            errno = EINVAL;
            return -1;
    }
    
    if (new_offset < 0) {
        pthread_mutex_unlock(&ctx->fd_lock);
        errno = EINVAL;
        return -1;
    }
    
    *current_offset = new_offset;
    pthread_mutex_unlock(&ctx->fd_lock);
    
    return new_offset;
}

/* ============================================================================
 * SIGNALS - Queue and deliver to threads
 * ============================================================================ */

int iox_kill_full(pid_t pid, int sig) {
    if (sig < 0 || sig >= IOX_NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    if (sig == 0) {
        /* Just check if process exists */
        iox_context_t *ctx = iox_ctx_get(pid);
        if (ctx) {
            iox_ctx_put(ctx);
            return 0;
        }
        errno = ESRCH;
        return -1;
    }
    
    iox_context_t *target = iox_ctx_get(pid);
    if (!target) {
        errno = ESRCH;
        return -1;
    }
    
    pthread_mutex_lock(&target->sig_lock);
    
    /* Check if signal is blocked */
    if (sigismember(&target->sig_mask, sig)) {
        /* Signal blocked - add to pending */
        sigaddset(&target->sig_pending, sig);
        pthread_mutex_unlock(&target->sig_lock);
        iox_ctx_put(target);
        return 0;
    }
    
    pthread_mutex_unlock(&target->sig_lock);
    
    /* Deliver signal to thread */
    pthread_kill(target->thread, sig);
    
    iox_ctx_put(target);
    return 0;
}

int iox_sigaction_full(int sig, const struct sigaction *act, struct sigaction *oldact) {
    if (sig < 1 || sig >= IOX_NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    iox_context_t *ctx = __iox_current_ctx;
    if (!ctx) {
        /* No context - pass through to real sigaction */
        return sigaction(sig, act, oldact);
    }
    
    pthread_mutex_lock(&ctx->sig_lock);
    
    if (oldact) {
        *oldact = ctx->sig_actions[sig];
    }
    
    if (act) {
        ctx->sig_actions[sig] = *act;
    }
    
    pthread_mutex_unlock(&ctx->sig_lock);
    return 0;
}

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

int iox_context_init(void) {
    static atomic_int initialized = 0;
    if (atomic_exchange(&initialized, 1) == 1) {
        return 0;  /* Already initialized */
    }
    
    /* Create init process context for main thread */
    pid_t init_pid = atomic_fetch_add(&next_pid, 1);
    iox_context_t *init = iox_ctx_alloc(init_pid, NULL);
    if (!init) {
        return -1;
    }
    
    strncpy(init->name, "init", IOX_MAX_NAME - 1);
    init->name[IOX_MAX_NAME - 1] = '\0';
    
    /* Use default cwd instead of getcwd to avoid circular dependency */
    strncpy(init->cwd, "/", IOX_MAX_PATH - 1);
    init->cwd[IOX_MAX_PATH - 1] = '\0';
    
    init->thread = pthread_self();
    
    /* Set as current context */
    __iox_current_ctx = init;
    
    return 0;
}

void iox_context_deinit(void) {
    /* Cleanup all contexts */
    for (int i = 0; i < IOX_MAX_PROCESSES; i++) {
        if (ctx_table[i]) {
            iox_ctx_free(ctx_table[i]);
            ctx_table[i] = NULL;
        }
    }
}

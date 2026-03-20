/*
 * process.m - Process management syscalls
 *
 * Thread-based process simulation for iOS (no real fork)
 */

#import <Foundation/Foundation.h>
#import <pthread.h>
#import <errno.h>
#import <unistd.h>
#import <sys/wait.h>
#import <sys/resource.h>
#import <signal.h>

#import "../../include/linux/unistd.h"
#import "../../include/linux/wait.h"
#import "../../include/linux/signal.h"

/* ============================================================================
 * Process Table
 * ============================================================================ */

#define MAX_PROCESSES 1024
#define FIRST_VIRTUAL_PID 1000

typedef enum {
    PROCESS_RUNNING,
    PROCESS_STOPPED,
    PROCESS_ZOMBIE,
    PROCESS_DEAD
} process_state_t;

typedef struct process_entry {
    pid_t pid;
    pid_t ppid;
    pthread_t thread;
    process_state_t state;
    int exit_status;
    int exit_signal;
    bool exited;
    sigset_t sigmask;
    sigset_t sigpending;
    struct sigaction sigactions[NSIG];
    void *(*start_routine)(void *);
    void *arg;
} process_entry_t;

static process_entry_t process_table[MAX_PROCESSES];
static pthread_mutex_t process_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t process_table_cond = PTHREAD_COND_INITIALIZER;
static pid_t next_pid = FIRST_VIRTUAL_PID;
static pid_t current_pid = 0;
static __thread pid_t thread_pid = 0;

/* ============================================================================
 * Process Table Management
 * ============================================================================ */

static process_entry_t *find_process(pid_t pid) {
    if (pid < FIRST_VIRTUAL_PID || pid >= next_pid) {
        return NULL;
    }
    return &process_table[pid - FIRST_VIRTUAL_PID];
}

static process_entry_t *alloc_process(void) {
    pthread_mutex_lock(&process_table_mutex);
    
    if (next_pid >= FIRST_VIRTUAL_PID + MAX_PROCESSES) {
        pthread_mutex_unlock(&process_table_mutex);
        errno = EAGAIN;
        return NULL;
    }
    
    process_entry_t *entry = &process_table[next_pid - FIRST_VIRTUAL_PID];
    memset(entry, 0, sizeof(*entry));
    entry->pid = next_pid++;
    entry->state = PROCESS_RUNNING;
    entry->exited = false;
    sigemptyset(&entry->sigmask);
    sigemptyset(&entry->sigpending);
    
    pthread_mutex_unlock(&process_table_mutex);
    return entry;
}

static void free_process(process_entry_t *entry) {
    if (!entry) return;
    entry->state = PROCESS_DEAD;
    entry->pid = 0;
}

/* ============================================================================
 * Thread Entry Point
 * ============================================================================ */

static void *process_trampoline(void *arg) {
    process_entry_t *entry = (process_entry_t *)arg;
    
    /* Set thread-local PID */
    thread_pid = entry->pid;
    
    /* Execute the actual function */
    void *result = entry->start_routine(entry->arg);
    
    /* Mark as exited */
    pthread_mutex_lock(&process_table_mutex);
    entry->exited = true;
    entry->state = PROCESS_ZOMBIE;
    pthread_cond_broadcast(&process_table_cond);
    pthread_mutex_unlock(&process_table_mutex);
    
    return result;
}

/* ============================================================================
 * Process Creation
 * ============================================================================ */

pid_t a_shell_fork(void) {
    /* iOS forbids real fork - create virtual PID */
    process_entry_t *parent = find_process(a_shell_getpid());
    
    process_entry_t *child = alloc_process();
    if (!child) {
        return -1;
    }
    
    child->ppid = a_shell_getpid();
    
    /* In a real implementation, we'd need to:
     * 1. Save parent's context
     * 2. Create thread
     * 3. Child returns 0, parent returns pid
     * 
     * For now, return ENOSYS as true fork requires more machinery
     */
    errno = ENOSYS;
    return -1;
}

pid_t a_shell_vfork(void) {
    /* Same as fork for iOS (no copy-on-write available) */
    return a_shell_fork();
}

/* ============================================================================
 * Process Termination
 * ============================================================================ */

void a_shell_exit(int status) {
    pid_t pid = a_shell_getpid();
    process_entry_t *entry = find_process(pid);
    
    if (entry) {
        pthread_mutex_lock(&process_table_mutex);
        entry->exit_status = status;
        entry->exited = true;
        entry->state = PROCESS_ZOMBIE;
        pthread_cond_broadcast(&process_table_cond);
        pthread_mutex_unlock(&process_table_mutex);
    }
    
    pthread_exit((void *)(intptr_t)status);
}

void a_shell__exit(int status) {
    /* _exit() shouldn't flush stdio, but on iOS we don't have much choice */
    a_shell_exit(status);
}

/* ============================================================================
 * Process Execution
 * ============================================================================ */

int a_shell_execve(const char *pathname, char *const argv[], char *const envp[]) {
    /* On iOS, we can't truly replace the process image.
     * This would need integration with dynamic loading.
     * For now, return ENOSYS.
     */
    errno = ENOSYS;
    return -1;
}

int a_shell_execv(const char *pathname, char *const argv[]) {
    return a_shell_execve(pathname, argv, NULL);
}

int a_shell_execvp(const char *file, char *const argv[]) {
    /* Search PATH for executable */
    const char *path = getenv("PATH");
    if (!path) {
        path = "/usr/bin:/bin";
    }
    
    /* Try to execute directly if path contains / */
    if (strchr(file, '/')) {
        return a_shell_execv(file, argv);
    }
    
    /* Search in PATH */
    char *path_copy = strdup(path);
    char *dir = strtok(path_copy, ":");
    
    while (dir) {
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, file);
        
        if (access(fullpath, X_OK) == 0) {
            free(path_copy);
            return a_shell_execv(fullpath, argv);
        }
        
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    errno = ENOENT;
    return -1;
}

/* ============================================================================
 * Process Waiting
 * ============================================================================ */

pid_t a_shell_waitpid(pid_t pid, int *wstatus, int options) {
    pthread_mutex_lock(&process_table_mutex);
    
    pid_t self = a_shell_getpid();
    pid_t result_pid = -1;
    int status = 0;
    
    if (pid == -1) {
        /* Wait for any child */
        while (1) {
            for (int i = 0; i < MAX_PROCESSES; i++) {
                process_entry_t *child = &process_table[i];
                if (child->pid > 0 && child->ppid == self && child->exited) {
                    result_pid = child->pid;
                    status = child->exit_status;
                    child->state = PROCESS_DEAD;
                    pthread_mutex_unlock(&process_table_mutex);
                    
                    if (wstatus) {
                        *wstatus = (status & 0xFF) << 8;
                    }
                    return result_pid;
                }
            }
            
            if (options & WNOHANG) {
                pthread_mutex_unlock(&process_table_mutex);
                return 0;
            }
            
            /* Wait for a child to exit */
            pthread_cond_wait(&process_table_cond, &process_table_mutex);
        }
    } else if (pid > 0) {
        /* Wait for specific child */
        process_entry_t *child = find_process(pid);
        
        if (!child || child->ppid != self) {
            pthread_mutex_unlock(&process_table_mutex);
            errno = ECHILD;
            return -1;
        }
        
        while (!child->exited) {
            if (options & WNOHANG) {
                pthread_mutex_unlock(&process_table_mutex);
                return 0;
            }
            pthread_cond_wait(&process_table_cond, &process_table_mutex);
        }
        
        result_pid = child->pid;
        status = child->exit_status;
        child->state = PROCESS_DEAD;
        pthread_mutex_unlock(&process_table_mutex);
        
        if (wstatus) {
            *wstatus = (status & 0xFF) << 8;
        }
        return result_pid;
    } else if (pid == 0) {
        /* Wait for any child in same process group - not implemented */
        pthread_mutex_unlock(&process_table_mutex);
        errno = ECHILD;
        return -1;
    } else {
        /* pid < -1: wait for any child in process group -pgrp */
        pthread_mutex_unlock(&process_table_mutex);
        errno = ECHILD;
        return -1;
    }
    
    pthread_mutex_unlock(&process_table_mutex);
    errno = ECHILD;
    return -1;
}

pid_t a_shell_wait(int *wstatus) {
    return a_shell_waitpid(-1, wstatus, 0);
}

pid_t a_shell_wait3(int *wstatus, int options, struct rusage *rusage) {
    /* rusage not implemented */
    return a_shell_waitpid(-1, wstatus, options);
}

pid_t a_shell_wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage) {
    /* rusage not implemented */
    return a_shell_waitpid(pid, wstatus, options);
}

/* ============================================================================
 * Process IDs
 * ============================================================================ */

pid_t a_shell_getpid(void) {
    if (thread_pid > 0) {
        return thread_pid;
    }
    /* Main thread */
    return current_pid > 0 ? current_pid : getpid();
}

pid_t a_shell_getppid(void) {
    process_entry_t *entry = find_process(a_shell_getpid());
    if (entry) {
        return entry->ppid;
    }
    return 1; /* Init */
}

pid_t a_shell_getpgrp(void) {
    /* Process group = session leader PID on iOS */
    return a_shell_getpid();
}

pid_t a_shell_getpgid(pid_t pid) {
    return a_shell_getpgrp();
}

int a_shell_setpgid(pid_t pid, pid_t pgid) {
    /* Not supported on iOS */
    errno = EPERM;
    return -1;
}

pid_t a_shell_getsid(pid_t pid) {
    return a_shell_getpid();
}

pid_t a_shell_setsid(void) {
    /* Always succeeds - we're the session leader */
    return a_shell_getpid();
}

/* ============================================================================
 * Process Groups
 * ============================================================================ */

int a_shell_setpgrp(void) {
    return a_shell_setpgid(0, 0);
}

/* ============================================================================
 * Kernel Initialization
 * ============================================================================ */

__attribute__((constructor))
static void process_init(void) {
    /* Initialize main process */
    current_pid = getpid();
    thread_pid = current_pid;
    
    process_entry_t *main = &process_table[0];
    memset(main, 0, sizeof(*main));
    main->pid = current_pid;
    main->ppid = 0;
    main->state = PROCESS_RUNNING;
    main->exited = false;
    sigemptyset(&main->sigmask);
    sigemptyset(&main->sigpending);
    
    next_pid = FIRST_VIRTUAL_PID;
}

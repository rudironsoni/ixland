//
//  a_shell_process.c
//  a-shell-kernel
//
//  Extended process table with parent/child tracking
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#include "a_shell_kernel.h"

// Extended process table: 1024 processes (up from 128)
#define A_SHELL_MAX_PROCESSES 1024

typedef struct a_shell_process {
    pid_t pid;                      // Virtual process ID
    pid_t ppid;                     // Parent process ID
    pthread_t thread;               // Thread handle
    char name[64];                  // Process name
    int status;                     // Exit status
    volatile int running;           // Running flag
    time_t start_time;              // Process start time
    
    // Signal handling
    struct sigaction sig_handlers[NSIG];
    sigjmp_buf *signal_env;         // For signal emulation
    volatile int pending_signals;
    
    // Environment (per-process)
    char **environment;
    int env_count;
    char *cwd;                      // Current working directory
    
    // Cleanup tracking
    volatile int marked_for_cleanup;
    pthread_mutex_t cleanup_lock;
} a_shell_process_t;

// Process table
static a_shell_process_t *process_table[A_SHELL_MAX_PROCESSES];
static pthread_mutex_t process_table_lock = PTHREAD_MUTEX_INITIALIZER;
static pid_t next_vpid = 1000;      // Start at 1000 to avoid conflicts
static _Thread_local pid_t current_vpid = 0;

// Process limit
static int process_limit = A_SHELL_MAX_PROCESSES;

// Initialize process table entry
static a_shell_process_t* alloc_process_entry(void) {
    a_shell_process_t *proc = calloc(1, sizeof(a_shell_process_t));
    if (proc) {
        pthread_mutex_init(&proc->cleanup_lock, NULL);
        proc->running = 0;
        proc->marked_for_cleanup = 0;
    }
    return proc;
}

// Find process by PID
a_shell_process_t* find_process(pid_t pid) {
    if (pid < 0 || pid >= A_SHELL_MAX_PROCESSES) {
        return NULL;
    }
    pthread_mutex_lock(&process_table_lock);
    a_shell_process_t *proc = process_table[pid % A_SHELL_MAX_PROCESSES];
    // Verify PID matches (handles hash collisions)
    if (proc && proc->pid != pid) {
        proc = NULL;
    }
    pthread_mutex_unlock(&process_table_lock);
    return proc;
}

// Allocate new virtual PID
pid_t a_shell_vfork(void) {
    pthread_mutex_lock(&process_table_lock);
    
    // Find available slot
    pid_t vpid = next_vpid++;
    int slot = vpid % A_SHELL_MAX_PROCESSES;
    
    // Clean up old entry if exists
    if (process_table[slot]) {
        a_shell_process_t *old = process_table[slot];
        if (old->marked_for_cleanup) {
            free(old);
            process_table[slot] = NULL;
        }
    }
    
    // Allocate new process entry
    a_shell_process_t *proc = alloc_process_entry();
    if (!proc) {
        pthread_mutex_unlock(&process_table_lock);
        errno = EAGAIN;
        return -1;
    }
    
    proc->pid = vpid;
    proc->ppid = current_vpid ? current_vpid : 1; // 1 is init
    proc->running = 0;  // Not running yet (vfork semantics)
    proc->start_time = time(NULL);
    strncpy(proc->name, "vfork-child", 63);
    
    process_table[slot] = proc;
    current_vpid = vpid;
    
    pthread_mutex_unlock(&process_table_lock);
    
    return vpid;
}

// Fork (returns ENOSYS since real fork not available on iOS)
pid_t a_shell_fork(void) {
    // On iOS, fork() returns ENOSYS
    errno = ENOSYS;
    return -1;
}

// Get current PID
pid_t a_shell_getpid(void) {
    return current_vpid ? current_vpid : getpid();
}

// Get parent PID
pid_t a_shell_getppid(void) {
    a_shell_process_t *proc = find_process(current_vpid);
    if (proc) {
        return proc->ppid;
    }
    return 1; // init
}

// Wait for child process
pid_t a_shell_waitpid(pid_t pid, int *stat_loc, int options) {
    if (pid < -1) {
        // Wait for any child in process group
        // TODO: Implement process groups
        errno = ECHILD;
        return -1;
    }
    
    if (pid == -1) {
        // Wait for any child
        pthread_mutex_lock(&process_table_lock);
        
        while (1) {
            // Search for children of current process
            int found = 0;
            for (int i = 0; i < A_SHELL_MAX_PROCESSES; i++) {
                a_shell_process_t *proc = process_table[i];
                if (proc && proc->ppid == current_vpid && !proc->running) {
                    // Found exited child
                    if (stat_loc) {
                        *stat_loc = proc->status;
                    }
                    pid_t child_pid = proc->pid;
                    
                    // Mark for cleanup
                    proc->marked_for_cleanup = 1;
                    
                    pthread_mutex_unlock(&process_table_lock);
                    return child_pid;
                }
            }
            
            pthread_mutex_unlock(&process_table_lock);
            
            if (options & WNOHANG) {
                return 0; // No children ready
            }
            
            // Wait a bit and retry
            usleep(10000); // 10ms
            pthread_mutex_lock(&process_table_lock);
        }
    }
    
    // Wait for specific PID
    a_shell_process_t *proc = find_process(pid);
    if (!proc) {
        errno = ECHILD;
        return -1;
    }
    
    if (proc->ppid != current_vpid) {
        errno = ECHILD;
        return -1;
    }
    
    // Wait for process to exit
    while (proc->running) {
        usleep(10000);
    }
    
    if (stat_loc) {
        *stat_loc = proc->status;
    }
    
    pthread_mutex_lock(&process_table_lock);
    proc->marked_for_cleanup = 1;
    pthread_mutex_unlock(&process_table_lock);
    
    return pid;
}

// Simple wait (wait for any child)
pid_t a_shell_wait(int *stat_loc) {
    return a_shell_waitpid(-1, stat_loc, 0);
}

// Exit process
void a_shell_exit(int status) {
    a_shell_process_t *proc = find_process(current_vpid);
    if (proc) {
        pthread_mutex_lock(&proc->cleanup_lock);
        proc->status = status;
        proc->running = 0;
        pthread_mutex_unlock(&proc->cleanup_lock);
    }
    
    // Exit the thread
    pthread_exit((void*)(intptr_t)status);
}

// Cleanup thread - periodically removes exited processes
static void* cleanup_thread_func(void* arg) {
    while (1) {
        sleep(1); // Run every second
        
        pthread_mutex_lock(&process_table_lock);
        
        for (int i = 0; i < A_SHELL_MAX_PROCESSES; i++) {
            a_shell_process_t *proc = process_table[i];
            if (proc && proc->marked_for_cleanup && !proc->running) {
                // Free environment
                if (proc->environment) {
                    for (int j = 0; j < proc->env_count; j++) {
                        free(proc->environment[j]);
                    }
                    free(proc->environment);
                }
                if (proc->cwd) {
                    free(proc->cwd);
                }
                
                pthread_mutex_destroy(&proc->cleanup_lock);
                free(proc);
                process_table[i] = NULL;
            }
        }
        
        pthread_mutex_unlock(&process_table_lock);
    }
    
    return NULL;
}

// Initialize cleanup thread
__attribute__((constructor))
static void init_cleanup_thread(void) {
    pthread_t cleanup_thread;
    pthread_create(&cleanup_thread, NULL, cleanup_thread_func, NULL);
    pthread_detach(cleanup_thread);
}

// Signal handling stubs (for future implementation)
sig_t a_shell_signal(int sig, sig_t func) {
    // TODO: Implement signal handling
    return SIG_DFL;
}

int a_shell_killpid(pid_t pid, int sig) {
    a_shell_process_t *proc = find_process(pid);
    if (!proc) {
        errno = ESRCH;
        return -1;
    }
    
    // TODO: Deliver signal
    return 0;
}

// Process tree helpers
pid_t a_shell_getpgrp(void) {
    // TODO: Implement process groups
    return a_shell_getpid();
}

int a_shell_setpgid(pid_t pid, pid_t pgid) {
    // TODO: Implement process groups
    return 0;
}

pid_t a_shell_setsid(void) {
    // TODO: Implement sessions
    return a_shell_getpid();
}

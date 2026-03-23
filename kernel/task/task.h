#ifndef IOX_TASK_H
#define IOX_TASK_H

#include <sys/types.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <sys/resource.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOX_MAX_PATH 1024
#define IOX_MAX_NAME 256
#define IOX_NSIG 64
#define IOX_MAX_FD 256
#define IOX_MAX_ARGS 256

#ifndef RLIMIT_NLIMITS
#define RLIMIT_NLIMITS 16
#endif

typedef _Atomic pid_t atomic_pid_t;

typedef struct iox_task iox_task_t;
typedef struct iox_files iox_files_t;
typedef struct iox_fs iox_fs_t;
typedef struct iox_sighand iox_sighand_t;
typedef struct iox_tty {
    int tty_id;
    pid_t foreground_pgrp;
    atomic_int refs;
} iox_tty_t;

typedef struct iox_mm_emu {
    void *exec_image_base;
    size_t exec_image_size;
} iox_mm_emu_t;

typedef enum {
    IOX_IMAGE_NONE = 0,
    IOX_IMAGE_NATIVE,
    IOX_IMAGE_WASI,
    IOX_IMAGE_SCRIPT
} iox_image_type_t;

typedef int (*iox_native_entry_t)(struct iox_task *task, int argc, char **argv, char **envp);

typedef struct iox_exec_image {
    iox_image_type_t type;
    char path[IOX_MAX_PATH];
    char interpreter[IOX_MAX_PATH];
    
    union {
        struct {
            iox_native_entry_t entry;
        } native;
        struct {
            void *module;
            void *instance;
        } wasi;
        struct {
            char *interp_argv[IOX_MAX_ARGS];
            int interp_argc;
        } script;
    } u;
} iox_exec_image_t;

struct iox_task {
    pid_t pid;
    pid_t ppid;
    pid_t tgid;
    pid_t pgid;
    pid_t sid;
    
    atomic_int state;
    int exit_status;
    atomic_bool exited;
    atomic_bool signaled;
    atomic_int termsig;
    
    pthread_t thread;
    char comm[IOX_MAX_NAME];
    char exe[IOX_MAX_PATH];
    
    iox_files_t *files;
    iox_fs_t *fs;
    iox_sighand_t *sighand;
    iox_tty_t *tty;
    iox_mm_emu_t *mm;
    iox_exec_image_t *exec_image;
    
    struct iox_task *parent;
    struct iox_task *children;
    struct iox_task *next_sibling;
    struct iox_task *hash_next;
    
    pthread_cond_t wait_cond;
    pthread_mutex_t wait_lock;
    int waiters;
    
    struct rlimit rlimits[RLIMIT_NLIMITS];
    
    struct timespec start_time;
    
    atomic_int refs;
    pthread_mutex_t lock;
};

iox_task_t *iox_task_alloc(void);
void iox_task_free(iox_task_t *task);
iox_task_t *iox_current_task(void);
void iox_set_current_task(iox_task_t *task);

pid_t iox_alloc_pid(void);
void iox_free_pid(pid_t pid);

int iox_task_init(void);
void iox_task_deinit(void);

#ifdef __cplusplus
}
#endif

#endif

#ifndef IOX_TYPES_H
#define IOX_TYPES_H

/* iXland libc - Public Type Definitions
 *
 * This header defines the public type definitions for the iox API.
 * Extracted from ixland-system as the first step in libc boundary formation.
 */

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * VERSION
 * ============================================================================ */

#define IOX_VERSION_MAJOR   1
#define IOX_VERSION_MINOR   0
#define IOX_VERSION_PATCH   0

#define IOX_VERSION_STRING  "1.0.0"

/* ============================================================================
 * COMMON CONSTANTS
 * ============================================================================ */

#define IOX_MAX_PATH 1024

/* ============================================================================
 * ERROR CODES (iox-specific)
 * ============================================================================ */

typedef enum {
    IOX_OK              = 0,    /* Success */
    IOX_ERROR           = -1,   /* Generic error */
    IOX_EINVAL          = -22,  /* Invalid argument */
    IOX_ENOMEM          = -12,  /* Out of memory */
    IOX_EACCES          = -13,  /* Permission denied */
    IOX_EEXIST          = -17,  /* File exists */
    IOX_ENOENT          = -2,   /* No such file or directory */
    IOX_EAGAIN          = -11,  /* Resource temporarily unavailable */
    IOX_EBUSY           = -16,  /* Device or resource busy */
    IOX_ECHILD          = -10,  /* No child processes */
    IOX_EINTR           = -4,   /* Interrupted system call */
    IOX_EIO             = -5,   /* I/O error */
    IOX_EISDIR          = -21,  /* Is a directory */
    IOX_EMFILE          = -24,  /* Too many open files */
    IOX_ENAMETOOLONG    = -63,  /* File name too long */
    IOX_ENOTDIR         = -20,  /* Not a directory */
    IOX_ENOTEMPTY       = -39,  /* Directory not empty */
    IOX_EOPNOTSUPP      = -95,  /* Operation not supported */
    IOX_EOVERFLOW       = -75,  /* Value too large */
    IOX_EPERM           = -1,   /* Operation not permitted */
    IOX_ERANGE          = -34,  /* Result too large */
    IOX_ESPIPE          = -29,  /* Invalid seek */
    IOX_ESRCH           = -3,   /* No such process */
    IOX_ETIMEDOUT       = -60,  /* Connection timed out */
} iox_error_t;

/* ============================================================================
 * FLAGS
 * ============================================================================ */

/* File open flags - additional to standard fcntl.h */
#define IOX_O_CLOEXEC       0x80000     /* Close on exec */
#define IOX_O_DIRECTORY     0x20000     /* Must be directory */
#define IOX_O_NOFOLLOW      0x40000     /* Don't follow symlinks */
#define IOX_O_PATH          0x2000000   /* Path only, no I/O */
#define IOX_O_TMPFILE       0x404000    /* Create unnamed temp file */

/* Memory protection flags - additional to standard mman.h */
#define IOX_PROT_GROWSDOWN  0x01000000  /* Stack grows down */
#define IOX_PROT_GROWSUP    0x02000000  /* Stack grows up */
#define IOX_PROT_EXEC_STACK 0x04000000  /* Executable stack (denied on iOS) */

/* Signal flags */
#define IOX_SA_RESTORER     0x04000000  /* Restore handler (Linux-specific) */

/* Process flags */
#define IOX_CLONE_VM        0x00000100  /* Share memory space */
#define IOX_CLONE_FS        0x00000200  /* Share filesystem info */
#define IOX_CLONE_FILES     0x00000400  /* Share file descriptors */
#define IOX_CLONE_SIGHAND   0x00000800  /* Share signal handlers */
#define IOX_CLONE_PIDFD     0x00001000  /* Return PID file descriptor */
#define IOX_CLONE_THREAD    0x00010000  /* Same thread group */
#define IOX_CLONE_NEWNS     0x00020000  /* New mount namespace */
#define IOX_CLONE_SYSVSEM   0x00040000  /* Share System V SEM_UNDO */
#define IOX_CLONE_NEWNET    0x40000000  /* New network namespace */

/* ============================================================================
 * STRUCTURES
 * ============================================================================ */

/* iox library configuration */
typedef struct {
    bool        debug_enabled;      /* Enable debug logging */
    bool        trace_syscalls;   /* Trace all syscalls */
    bool        check_sandbox;    /* Validate all paths */
    size_t      max_processes;    /* Maximum virtual processes */
    size_t      max_threads;      /* Maximum threads */
    size_t      max_memory;       /* Maximum memory per process */
    const char *home_directory;   /* Home directory path */
    const char *tmp_directory;    /* Temporary directory path */
} iox_config_t;

/* Process information */
typedef struct {
    pid_t       pid;              /* Process ID */
    pid_t       ppid;             /* Parent process ID */
    pid_t       pgid;             /* Process group ID */
    uid_t       uid;              /* User ID */
    gid_t       gid;              /* Group ID */
    char        name[256];        /* Process name */
    char        state;            /* Process state (R, S, D, T, Z) */
    uint64_t    memory_rss;       /* Resident memory (bytes) */
    uint64_t    memory_vms;       /* Virtual memory (bytes) */
    uint64_t    cpu_time;         /* CPU time (nanoseconds) */
    uint64_t    start_time;       /* Start time (nanoseconds since epoch) */
} iox_proc_info_t;

/* Thread information */
typedef struct {
    pthread_t   thread_id;        /* Thread ID */
    pid_t       pid;              /* Associated process ID */
    char        state;            /* Thread state */
    uint64_t    cpu_time;         /* CPU time (nanoseconds) */
    void       *stack_base;       /* Stack base address */
    size_t      stack_size;       /* Stack size */
} iox_thread_info_t;

/* System information */
typedef struct {
    char        sysname[256];     /* Operating system name (iOS) */
    char        nodename[256];    /* Node name */
    char        release[256];     /* OS release */
    char        version[256];     /* OS version */
    char        machine[256];     /* Hardware identifier (arm64) */

    uint64_t    total_memory;     /* Total physical memory (bytes) */
    uint64_t    free_memory;      /* Free memory (bytes) */
    uint64_t    avail_memory;     /* Available memory (bytes) */
    uint64_t    shared_memory;    /* Shared memory (bytes) */
    uint64_t    buffer_memory;    /* Buffer cache (bytes) */

    uint32_t    cpu_count;        /* Number of CPUs */
    uint32_t    cpu_online;       /* Online CPUs */
    uint64_t    cpu_freq;         /* CPU frequency (Hz) */
    char        cpu_arch[64];     /* CPU architecture (arm64) */
} iox_sys_info_t;

/* ============================================================================
 * CALLBACK TYPES
 * ============================================================================ */

/* Process callback - called when process state changes */
typedef void (*iox_proc_callback_t)(pid_t pid, int event, void *userdata);

/* Thread callback - called when thread state changes */
typedef void (*iox_thread_callback_t)(pthread_t tid, int event, void *userdata);

/* Signal callback - called when signal delivered */
typedef void (*iox_signal_callback_t)(pid_t pid, int sig, void *userdata);

/* File event callback - called on file operations */
typedef void (*iox_file_callback_t)(const char *path, int op, void *userdata);

/* ============================================================================
 * CALLBACK EVENTS
 * ============================================================================ */

/* Process events */
#define IOX_PROC_CREATE     0x01    /* Process created */
#define IOX_PROC_START      0x02    /* Process started */
#define IOX_PROC_EXIT       0x04    /* Process exited */
#define IOX_PROC_SIGNAL     0x08    /* Signal received */
#define IOX_PROC_STOP       0x10    /* Process stopped */
#define IOX_PROC_CONTINUE   0x20    /* Process continued */

/* Thread events */
#define IOX_THREAD_CREATE   0x01    /* Thread created */
#define IOX_THREAD_START    0x02    /* Thread started */
#define IOX_THREAD_EXIT     0x04    /* Thread exited */
#define IOX_THREAD_DETACH   0x08    /* Thread detached */

/* File operations */
#define IOX_FILE_OPEN       0x01    /* File opened */
#define IOX_FILE_CLOSE      0x02    /* File closed */
#define IOX_FILE_READ       0x04    /* File read */
#define IOX_FILE_WRITE      0x08    /* File written */
#define IOX_FILE_TRUNCATE   0x10    /* File truncated */
#define IOX_FILE_DELETE     0x20    /* File deleted */
#define IOX_FILE_RENAME     0x40    /* File renamed */
#define IOX_FILE_CHMOD      0x80    /* Permissions changed */

/* ============================================================================
 * LIMITS
 * ============================================================================ */

/* Resource limits */
#define IOX_RLIM_INFINITY   ((uint64_t) -1)
#define IOX_RLIM_SAVED_MAX  IOX_RLIM_INFINITY
#define IOX_RLIM_SAVED_CUR  IOX_RLIM_INFINITY

typedef enum {
    IOX_RLIMIT_CPU      = 0,    /* CPU time in seconds */
    IOX_RLIMIT_FSIZE    = 1,    /* Maximum file size */
    IOX_RLIMIT_DATA     = 2,    /* Maximum data size */
    IOX_RLIMIT_STACK    = 3,    /* Maximum stack size */
    IOX_RLIMIT_CORE     = 4,    /* Maximum core file size */
    IOX_RLIMIT_RSS      = 5,    /* Maximum resident set size */
    IOX_RLIMIT_NPROC    = 6,    /* Maximum number of processes */
    IOX_RLIMIT_NOFILE   = 7,    /* Maximum number of open files */
    IOX_RLIMIT_MEMLOCK  = 8,    /* Maximum locked-in-memory address space */
    IOX_RLIMIT_AS       = 9,    /* Maximum address space */
    IOX_RLIMIT_LOCKS    = 10,   /* Maximum file locks */
    IOX_RLIMIT_SIGPENDING = 11, /* Maximum queued signals */
    IOX_RLIMIT_MSGQUEUE = 12,   /* Maximum bytes in POSIX message queues */
    IOX_RLIMIT_NICE     = 13,   /* Maximum nice priority */
    IOX_RLIMIT_RTPRIO   = 14,   /* Maximum real-time priority */
    IOX_RLIMIT_RTTIME   = 15,   /* Maximum real-time timeout */
    IOX_RLIMIT_NLIMITS  = 16
} iox_rlimit_resource_t;

typedef struct {
    uint64_t rlim_cur;  /* Current (soft) limit */
    uint64_t rlim_max;  /* Hard limit */
} iox_rlimit_t;

/* ============================================================================
 * STATISTICS
 * ============================================================================ */

/* Syscall statistics */
typedef struct {
    uint64_t    calls;            /* Total syscalls */
    uint64_t    errors;           /* Failed syscalls */
    uint64_t    time_total;       /* Total time (ns) */
    uint64_t    time_max;         /* Max time (ns) */
    uint64_t    time_min;         /* Min time (ns) */
} iox_syscall_stat_t;

/* Memory statistics */
typedef struct {
    uint64_t    allocations;      /* Total allocations */
    uint64_t    deallocations;    /* Total deallocations */
    uint64_t    bytes_allocated;  /* Current allocated bytes */
    uint64_t    bytes_total;      /* Total allocated ever */
    uint64_t    peak_allocated;   /* Peak allocated bytes */
    uint64_t    failed_allocs;    /* Failed allocations */
} iox_memory_stat_t;

/* Process statistics */
typedef struct {
    uint32_t    active_processes;   /* Currently active */
    uint32_t    total_created;      /* Total created */
    uint32_t    total_exited;       /* Total exited */
    uint32_t    zombie_processes;   /* Zombie processes */
    uint32_t    max_concurrent;     /* Max concurrent ever */
} iox_process_stat_t;

#ifdef __cplusplus
}
#endif

#endif /* IOX_TYPES_H */

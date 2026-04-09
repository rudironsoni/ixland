#ifndef IXLAND_TYPES_H
#define IXLAND_TYPES_H

/* iXland libc - Public Type Definitions
 *
 * This header defines the public type definitions for the ixland API.
 * Extracted from ixland-system as the first step in libc boundary formation.
 */

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * VERSION
 * ============================================================================ */

#define IXLAND_VERSION_MAJOR 1
#define IXLAND_VERSION_MINOR 0
#define IXLAND_VERSION_PATCH 0

#define IXLAND_VERSION_STRING "1.0.0"

/* ============================================================================
 * COMMON CONSTANTS
 * ============================================================================ */

/** Maximum path length */
#define IXLAND_MAX_PATH 1024

/* ============================================================================
 * ERROR CODES (ixland-specific)
 * ============================================================================ */

/**
 * @brief ixland error codes enumeration
 *
 * These error codes match Linux errno values for compatibility.
 * All error codes are negative values matching standard Linux errno conventions.
 */
typedef enum {
    IXLAND_OK = 0,             /**< Success */
    IXLAND_ERROR = -1,         /**< Generic error */
    IXLAND_EINVAL = -22,       /**< Invalid argument */
    IXLAND_ENOMEM = -12,       /**< Out of memory */
    IXLAND_EACCES = -13,       /**< Permission denied */
    IXLAND_EEXIST = -17,       /**< File exists */
    IXLAND_ENOENT = -2,        /**< No such file or directory */
    IXLAND_EAGAIN = -11,       /**< Resource temporarily unavailable */
    IXLAND_EBUSY = -16,        /**< Device or resource busy */
    IXLAND_ECHILD = -10,       /**< No child processes */
    IXLAND_EINTR = -4,         /**< Interrupted system call */
    IXLAND_EIO = -5,           /**< I/O error */
    IXLAND_EISDIR = -21,       /**< Is a directory */
    IXLAND_EMFILE = -24,       /**< Too many open files */
    IXLAND_ENAMETOOLONG = -63, /**< File name too long */
    IXLAND_ENOTDIR = -20,      /**< Not a directory */
    IXLAND_ENOTEMPTY = -39,    /**< Directory not empty */
    IXLAND_EOPNOTSUPP = -95,   /**< Operation not supported */
    IXLAND_EOVERFLOW = -75,    /**< Value too large */
    IXLAND_EPERM = -1,         /**< Operation not permitted */
    IXLAND_ERANGE = -34,       /**< Result too large */
    IXLAND_ESPIPE = -29,       /**< Invalid seek */
    IXLAND_ESRCH = -3,         /**< No such process */
    IXLAND_ETIMEDOUT = -60,    /**< Connection timed out */
} ixland_error_t;

/* ============================================================================
 * FLAGS
 * ============================================================================ */

/* File open flags - additional to standard fcntl.h
 * Note: These may be defined in linux/fcntl.h as well, so we guard them */
#ifndef IXLAND_O_CLOEXEC_DEFINED
#define IXLAND_O_CLOEXEC_DEFINED
#define IXLAND_O_CLOEXEC 0x80000   /* Close on exec */
#define IXLAND_O_DIRECTORY 0x20000 /* Must be directory */
#define IXLAND_O_NOFOLLOW 0x40000  /* Don't follow symlinks */
#define IXLAND_O_PATH 0x2000000    /* Path only, no I/O */
#define IXLAND_O_TMPFILE 0x404000  /* Create unnamed temp file */
#endif

/* Memory protection flags - additional to standard mman.h */
#define IXLAND_PROT_GROWSDOWN 0x01000000  /* Stack grows down */
#define IXLAND_PROT_GROWSUP 0x02000000    /* Stack grows up */
#define IXLAND_PROT_EXEC_STACK 0x04000000 /* Executable stack (denied on iOS) */

/* Signal flags */
#define IXLAND_SA_RESTORER 0x04000000 /* Restore handler (Linux-specific) */

/* Process flags */
#define IXLAND_CLONE_VM 0x00000100      /* Share memory space */
#define IXLAND_CLONE_FS 0x00000200      /* Share filesystem info */
#define IXLAND_CLONE_FILES 0x00000400   /* Share file descriptors */
#define IXLAND_CLONE_SIGHAND 0x00000800 /* Share signal handlers */
#define IXLAND_CLONE_PIDFD 0x00001000   /* Return PID file descriptor */
#define IXLAND_CLONE_THREAD 0x00010000  /* Same thread group */
#define IXLAND_CLONE_NEWNS 0x00020000   /* New mount namespace */
#define IXLAND_CLONE_SYSVSEM 0x00040000 /* Share System V SEM_UNDO */
#define IXLAND_CLONE_NEWNET 0x40000000  /* New network namespace */

/* ============================================================================
 * STRUCTURES
 * ============================================================================ */

/**
 * @brief ixland library configuration structure
 *
 * Used to configure the ixland library behavior on initialization.
 * Pass a pointer to this structure to ixland_init().
 */
typedef struct {
    bool debug_enabled;         /**< Enable debug logging */
    bool trace_syscalls;        /**< Trace all syscalls */
    bool check_sandbox;         /**< Validate all paths */
    size_t max_processes;       /**< Maximum virtual processes */
    size_t max_threads;         /**< Maximum threads */
    size_t max_memory;          /**< Maximum memory per process */
    const char *home_directory; /**< Home directory path */
    const char *tmp_directory;  /**< Temporary directory path */
} ixland_config_t;

/**
 * @brief Process information structure
 *
 * Contains information about a process in the ixland system.
 * Use ixland_get_proc_info() to retrieve this information.
 */
typedef struct {
    pid_t pid;           /**< Process ID */
    pid_t ppid;          /**< Parent process ID */
    pid_t pgid;          /**< Process group ID */
    uid_t uid;           /**< User ID */
    gid_t gid;           /**< Group ID */
    char name[256];      /**< Process name */
    char state;          /**< Process state (R, S, D, T, Z) */
    uint64_t memory_rss; /**< Resident memory (bytes) */
    uint64_t memory_vms; /**< Virtual memory (bytes) */
    uint64_t cpu_time;   /**< CPU time (nanoseconds) */
    uint64_t start_time; /**< Start time (nanoseconds since epoch) */
} ixland_proc_info_t;

/**
 * @brief Thread information structure
 *
 * Contains information about a thread in the ixland system.
 * Use ixland_get_thread_info() to retrieve this information.
 */
typedef struct {
    pthread_t thread_id; /**< Thread ID */
    pid_t pid;           /**< Associated process ID */
    char state;          /**< Thread state */
    uint64_t cpu_time;   /**< CPU time (nanoseconds) */
    void *stack_base;    /**< Stack base address */
    size_t stack_size;   /**< Stack size */
} ixland_thread_info_t;

/**
 * @brief System information structure
 *
 * Contains system-wide information about the host.
 * Use ixland_get_sys_info() to retrieve this information.
 */
typedef struct {
    char sysname[256];  /**< Operating system name (iOS) */
    char nodename[256]; /**< Node name */
    char release[256];  /**< OS release */
    char version[256];  /**< OS version */
    char machine[256];  /**< Hardware identifier (arm64) */

    uint64_t total_memory;  /**< Total physical memory (bytes) */
    uint64_t free_memory;   /**< Free memory (bytes) */
    uint64_t avail_memory;  /**< Available memory (bytes) */
    uint64_t shared_memory; /**< Shared memory (bytes) */
    uint64_t buffer_memory; /**< Buffer cache (bytes) */

    uint32_t cpu_count;  /**< Number of CPUs */
    uint32_t cpu_online; /**< Online CPUs */
    uint64_t cpu_freq;   /**< CPU frequency (Hz) */
    char cpu_arch[64];   /**< CPU architecture (arm64) */
} ixland_sys_info_t;

/* ============================================================================
 * CALLBACK TYPES
 * ============================================================================ */

/**
 * @brief Process state change callback
 * @param pid Process ID that changed state
 * @param event Event type (IXLAND_PROC_*)
 * @param userdata User data passed when registering callback
 */
typedef void (*ixland_proc_callback_t)(pid_t pid, int event, void *userdata);

/**
 * @brief Thread state change callback
 * @param tid Thread ID that changed state
 * @param event Event type (IXLAND_THREAD_*)
 * @param userdata User data passed when registering callback
 */
typedef void (*ixland_thread_callback_t)(pthread_t tid, int event, void *userdata);

/**
 * @brief Signal delivery callback
 * @param pid Process ID that received signal
 * @param sig Signal number
 * @param userdata User data passed when registering callback
 */
typedef void (*ixland_signal_callback_t)(pid_t pid, int sig, void *userdata);

/**
 * @brief File operation callback
 * @param path Path of file operated on
 * @param op Operation type (IXLAND_FILE_*)
 * @param userdata User data passed when registering callback
 */
typedef void (*ixland_file_callback_t)(const char *path, int op, void *userdata);

/* ============================================================================
 * CALLBACK EVENTS
 * ============================================================================ */

/** @name Process events */
/** @{ */
#define IXLAND_PROC_CREATE 0x01   /**< Process created */
#define IXLAND_PROC_START 0x02    /**< Process started */
#define IXLAND_PROC_EXIT 0x04     /**< Process exited */
#define IXLAND_PROC_SIGNAL 0x08   /**< Signal received */
#define IXLAND_PROC_STOP 0x10     /**< Process stopped */
#define IXLAND_PROC_CONTINUE 0x20 /**< Process continued */
/** @} */

/** @name Thread events */
/** @{ */
#define IXLAND_THREAD_CREATE 0x01 /**< Thread created */
#define IXLAND_THREAD_START 0x02  /**< Thread started */
#define IXLAND_THREAD_EXIT 0x04   /**< Thread exited */
#define IXLAND_THREAD_DETACH 0x08 /**< Thread detached */
/** @} */

/** @name File operation events */
/** @{ */
#define IXLAND_FILE_OPEN 0x01     /**< File opened */
#define IXLAND_FILE_CLOSE 0x02    /**< File closed */
#define IXLAND_FILE_READ 0x04     /**< File read */
#define IXLAND_FILE_WRITE 0x08    /**< File written */
#define IXLAND_FILE_TRUNCATE 0x10 /**< File truncated */
#define IXLAND_FILE_DELETE 0x20   /**< File deleted */
#define IXLAND_FILE_RENAME 0x40   /**< File renamed */
#define IXLAND_FILE_CHMOD 0x80    /**< Permissions changed */
/** @} */

/* ============================================================================
 * LIMITS
 * ============================================================================ */

/** Resource limit value indicating infinity */
#define IXLAND_RLIM_INFINITY ((uint64_t)-1)
/** Saved maximum resource limit */
#define IXLAND_RLIM_SAVED_MAX IXLAND_RLIM_INFINITY
/** Saved current resource limit */
#define IXLAND_RLIM_SAVED_CUR IXLAND_RLIM_INFINITY

/**
 * @brief Resource limit identifiers
 *
 * Identifiers for getrlimit/setrlimit functions.
 */
typedef enum {
    IXLAND_RLIMIT_CPU = 0,         /**< CPU time in seconds */
    IXLAND_RLIMIT_FSIZE = 1,       /**< Maximum file size */
    IXLAND_RLIMIT_DATA = 2,        /**< Maximum data size */
    IXLAND_RLIMIT_STACK = 3,       /**< Maximum stack size */
    IXLAND_RLIMIT_CORE = 4,        /**< Maximum core file size */
    IXLAND_RLIMIT_RSS = 5,         /**< Maximum resident set size */
    IXLAND_RLIMIT_NPROC = 6,       /**< Maximum number of processes */
    IXLAND_RLIMIT_NOFILE = 7,      /**< Maximum number of open files */
    IXLAND_RLIMIT_MEMLOCK = 8,     /**< Maximum locked-in-memory address space */
    IXLAND_RLIMIT_AS = 9,          /**< Maximum address space */
    IXLAND_RLIMIT_LOCKS = 10,      /**< Maximum file locks */
    IXLAND_RLIMIT_SIGPENDING = 11, /**< Maximum queued signals */
    IXLAND_RLIMIT_MSGQUEUE = 12,   /**< Maximum bytes in POSIX message queues */
    IXLAND_RLIMIT_NICE = 13,       /**< Maximum nice priority */
    IXLAND_RLIMIT_RTPRIO = 14,     /**< Maximum real-time priority */
    IXLAND_RLIMIT_RTTIME = 15,     /**< Maximum real-time timeout */
    IXLAND_RLIMIT_NLIMITS = 16     /**< Number of resource limits */
} ixland_rlimit_resource_t;

/**
 * @brief Resource limit structure
 *
 * Used with getrlimit() and setrlimit() to query/set resource limits.
 */
typedef struct {
    uint64_t rlim_cur; /**< Current (soft) limit */
    uint64_t rlim_max; /**< Hard limit */
} ixland_rlimit_t;

/* ============================================================================
 * STATISTICS
 * ============================================================================ */

/* Syscall statistics */
typedef struct {
    uint64_t calls;      /* Total syscalls */
    uint64_t errors;     /* Failed syscalls */
    uint64_t time_total; /* Total time (ns) */
    uint64_t time_max;   /* Max time (ns) */
    uint64_t time_min;   /* Min time (ns) */
} ixland_syscall_stat_t;

/* Memory statistics */
typedef struct {
    uint64_t allocations;     /* Total allocations */
    uint64_t deallocations;   /* Total deallocations */
    uint64_t bytes_allocated; /* Current allocated bytes */
    uint64_t bytes_total;     /* Total allocated ever */
    uint64_t peak_allocated;  /* Peak allocated bytes */
    uint64_t failed_allocs;   /* Failed allocations */
} ixland_memory_stat_t;

/* Process statistics */
typedef struct {
    uint32_t active_processes; /* Currently active */
    uint32_t total_created;    /* Total created */
    uint32_t total_exited;     /* Total exited */
    uint32_t zombie_processes; /* Zombie processes */
    uint32_t max_concurrent;   /* Max concurrent ever */
} ixland_process_stat_t;

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_TYPES_H */

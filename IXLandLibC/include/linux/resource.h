/* iXland libc - Linux-compatible resource.h
 *
 * Resource limit and usage definitions matching Linux kernel.
 * Provides process resource control interfaces.
 */

#ifndef _LINUX_RESOURCE_H
#define _LINUX_RESOURCE_H

#include <stdint.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * RESOURCE LIMIT TYPES
 * ============================================================================ */

/* Resource limit values */
#define IXLAND_RLIM_INFINITY ((uint64_t)-1)        /* No limit */
#define IXLAND_RLIM_SAVED_MAX IXLAND_RLIM_INFINITY /* Saved max value */
#define IXLAND_RLIM_SAVED_CUR IXLAND_RLIM_INFINITY /* Saved current value */

/* Resource limit structure (64-bit) */
struct linux_rlimit {
    uint64_t rlim_cur; /* Soft limit (current limit) */
    uint64_t rlim_max; /* Hard limit (maximum allowed) */
};

/* Resource limit structure (64-bit for new interfaces) */
struct linux_rlimit64 {
    uint64_t rlim_cur; /* Soft limit */
    uint64_t rlim_max; /* Hard limit */
};

/* ============================================================================
 * RESOURCE LIMIT IDENTIFIERS
 * ============================================================================ */

/* Resource numbers for getrlimit/setrlimit/prlimit */
#define IXLAND_RLIMIT_CPU 0         /* CPU time in seconds */
#define IXLAND_RLIMIT_FSIZE 1       /* Maximum file size */
#define IXLAND_RLIMIT_DATA 2        /* Maximum data size */
#define IXLAND_RLIMIT_STACK 3       /* Maximum stack size */
#define IXLAND_RLIMIT_CORE 4        /* Maximum core file size */
#define IXLAND_RLIMIT_RSS 5         /* Maximum resident set size */
#define IXLAND_RLIMIT_NPROC 6       /* Maximum number of processes */
#define IXLAND_RLIMIT_NOFILE 7      /* Maximum number of open files */
#define IXLAND_RLIMIT_MEMLOCK 8     /* Maximum locked-in-memory address space */
#define IXLAND_RLIMIT_AS 9          /* Maximum address space */
#define IXLAND_RLIMIT_LOCKS 10      /* Maximum file locks */
#define IXLAND_RLIMIT_SIGPENDING 11 /* Maximum queued signals */
#define IXLAND_RLIMIT_MSGQUEUE 12   /* Maximum bytes in POSIX message queues */
#define IXLAND_RLIMIT_NICE 13       /* Maximum nice priority */
#define IXLAND_RLIMIT_RTPRIO 14     /* Maximum real-time priority */
#define IXLAND_RLIMIT_RTTIME 15     /* Maximum real-time timeout */
#define IXLAND_RLIMIT_NLIMITS 16    /* Number of resource limits */

/* ============================================================================
 * RESOURCE USAGE
 * ============================================================================ */

/* Resource usage structure (64-bit for Linux compatibility) */
struct linux_rusage {
    struct timeval ru_utime; /* User time used */
    struct timeval ru_stime; /* System time used */
    long ru_maxrss;          /* Maximum resident set size */
    long ru_ixrss;           /* Integral shared memory size */
    long ru_idrss;           /* Integral unshared data size */
    long ru_isrss;           /* Integral unshared stack size */
    long ru_minflt;          /* Page reclaims (soft page faults) */
    long ru_majflt;          /* Page faults (hard page faults) */
    long ru_nswap;           /* Swaps */
    long ru_inblock;         /* Block input operations */
    long ru_oublock;         /* Block output operations */
    long ru_msgsnd;          /* Messages sent */
    long ru_msgrcv;          /* Messages received */
    long ru_nsignals;        /* Signals received */
    long ru_nvcsw;           /* Voluntary context switches */
    long ru_nivcsw;          /* Involuntary context switches */
};

/* Extended resource usage structure */
struct linux_rusage_64 {
    struct timeval ru_utime; /* User time used */
    struct timeval ru_stime; /* System time used */
    int64_t ru_maxrss;       /* Maximum resident set size */
    int64_t ru_ixrss;        /* Integral shared memory size */
    int64_t ru_idrss;        /* Integral unshared data size */
    int64_t ru_isrss;        /* Integral unshared stack size */
    int64_t ru_minflt;       /* Page reclaims */
    int64_t ru_majflt;       /* Page faults */
    int64_t ru_nswap;        /* Swaps */
    int64_t ru_inblock;      /* Block input operations */
    int64_t ru_oublock;      /* Block output operations */
    int64_t ru_msgsnd;       /* Messages sent */
    int64_t ru_msgrcv;       /* Messages received */
    int64_t ru_nsignals;     /* Signals received */
    int64_t ru_nvcsw;        /* Voluntary context switches */
    int64_t ru_nivcsw;       /* Involuntary context switches */
};

/* Who parameter for getrusage */
#define IXLAND_RUSAGE_SELF 0      /* Calling process */
#define IXLAND_RUSAGE_CHILDREN -1 /* Terminated children */
#define IXLAND_RUSAGE_THREAD 1    /* Calling thread (Linux-specific) */

/* ============================================================================
 * PRUSAGE (Process Resource Usage) - Linux extension
 * ============================================================================ */

/* Process resource usage info structure */
struct linux_prusage {
    uint64_t prutime;    /* User time */
    uint64_t prstime;    /* System time */
    uint64_t prcutime;   /* Cumulative user time */
    uint64_t prcstime;   /* Cumulative system time */
    uint64_t prbread;    /* Bytes read */
    uint64_t prbwritten; /* Bytes written */
    uint64_t prsyscr;    /* System calls read */
    uint64_t prsyscw;    /* System calls write */
};

/* ============================================================================
 * SCHEDULING PRIORITY
 * ============================================================================ */

/* Priority ranges */
#define IXLAND_PRIO_MIN (-20) /* Highest priority */
#define IXLAND_PRIO_MAX 19    /* Lowest priority */
#define IXLAND_NICE_WIDTH 40  /* Nice width (MAX - MIN + 1) */

/* getpriority/setpriority which parameter */
#define IXLAND_PRIO_PROCESS 0 /* Process */
#define IXLAND_PRIO_PGRP 1    /* Process group */
#define IXLAND_PRIO_USER 2    /* User */

/* ============================================================================
 * VM STATISTICS
 * ============================================================================ */

/* VM statistics structure */
struct linux_vmstat {
    uint64_t minflt;  /* Minor faults */
    uint64_t majflt;  /* Major faults */
    uint64_t nswap;   /* Swaps */
    uint64_t inblock; /* Input blocks */
    uint64_t oublock; /* Output blocks */
};

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Get resource limits
 *
 * @param resource Resource to query (IXLAND_RLIMIT_*)
 * @param rlim Where to store the limits
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_getrlimit(int resource, struct linux_rlimit *rlim);

/**
 * @brief Get resource limits (64-bit version)
 *
 * @param resource Resource to query
 * @param rlim Where to store the limits
 * @return int 0 on success, -1 on error
 */
int ixland_getrlimit64(int resource, struct linux_rlimit64 *rlim);

/**
 * @brief Set resource limits
 *
 * @param resource Resource to set
 * @param rlim New limits (rlim_cur <= rlim_max)
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_setrlimit(int resource, const struct linux_rlimit *rlim);

/**
 * @brief Set resource limits (64-bit version)
 *
 * @param resource Resource to set
 * @param rlim New limits
 * @return int 0 on success, -1 on error
 */
int ixland_setrlimit64(int resource, const struct linux_rlimit64 *rlim);

/**
 * @brief Extended get/set resource limits (Linux-specific)
 *
 * @param pid Process ID (0 for current process)
 * @param resource Resource to query/set
 * @param new_limit New limits (NULL to not change)
 * @param old_limit Where to store old limits (NULL to not retrieve)
 * @return int 0 on success, -1 on error
 */
int ixland_prlimit(pid_t pid, int resource, const struct linux_rlimit *new_limit,
                   struct linux_rlimit *old_limit);

/**
 * @brief Extended get/set resource limits (64-bit)
 *
 * @param pid Process ID (0 for current process)
 * @param resource Resource to query/set
 * @param new_limit New limits (NULL to not change)
 * @param old_limit Where to store old limits (NULL to not retrieve)
 * @return int 0 on success, -1 on error
 */
int ixland_prlimit64(pid_t pid, int resource, const struct linux_rlimit64 *new_limit,
                     struct linux_rlimit64 *old_limit);

/**
 * @brief Get resource usage
 *
 * @param who IXLAND_RUSAGE_SELF, IXLAND_RUSAGE_CHILDREN, or IXLAND_RUSAGE_THREAD
 * @param usage Where to store usage statistics
 * @return int 0 on success, -1 on error with errno set
 */
int ixland_getrusage(int who, struct linux_rusage *usage);

/**
 * @brief Get resource usage (64-bit version)
 *
 * @param who Who to query
 * @param usage Where to store usage statistics
 * @return int 0 on success, -1 on error
 */
int ixland_getrusage64(int who, struct linux_rusage_64 *usage);

/**
 * @brief Get scheduling priority
 *
 * @param which IXLAND_PRIO_PROCESS, IXLAND_PRIO_PGRP, or IXLAND_PRIO_USER
 * @param who Process/pgrp/user ID (0 for current)
 * @return int Priority value on success, -1 on error
 */
int ixland_getpriority(int which, id_t who);

/**
 * @brief Set scheduling priority
 *
 * @param which IXLAND_PRIO_PROCESS, IXLAND_PRIO_PGRP, or IXLAND_PRIO_USER
 * @param who Process/pgrp/user ID (0 for current)
 * @param prio New priority (IXLAND_PRIO_MIN to IXLAND_PRIO_MAX)
 * @return int 0 on success, -1 on error
 */
int ixland_setpriority(int which, id_t who, int prio);

/* ============================================================================
 * TIMES STRUCTURE
 * ============================================================================ */

/* Process times structure */
struct linux_tms {
    clock_t tms_utime;  /* User time */
    clock_t tms_stime;  /* System time */
    clock_t tms_cutime; /* Children user time */
    clock_t tms_cstime; /* Children system time */
};

/**
 * @brief Get process times
 *
 * @param tms Where to store times
 * @return clock_t Elapsed real time in clock ticks
 */
clock_t ixland_times(struct linux_tms *tms);

/* ============================================================================
 * IOPRIO (I/O Priority) - Linux extension
 * ============================================================================ */

/* I/O priority classes */
#define IXLAND_IOPRIO_CLASS_NONE 0 /* No priority */
#define IXLAND_IOPRIO_CLASS_RT 1   /* Real-time class */
#define IXLAND_IOPRIO_CLASS_BE 2   /* Best-effort class */
#define IXLAND_IOPRIO_CLASS_IDLE 3 /* Idle class */

/* I/O priority levels */
#define IXLAND_IOPRIO_NCLASS 4
#define IXLAND_IOPRIO_BITS 13
#define IXLAND_IOPRIO_MASK ((1UL << IXLAND_IOPRIO_BITS) - 1)
#define IXLAND_IOPRIO_PRIO_MASK IXLAND_IOPRIO_MASK
#define IXLAND_IOPRIO_PRIO_CLASS_MASK (~IXLAND_IOPRIO_MASK)

/* I/O priority shift */
#define IXLAND_IOPRIO_CLASS_SHIFT 13

/* I/O priority helpers */
#define IXLAND_IOPRIO_PRIO_NUMBITS 13
#define IXLAND_IOPRIO_PRIO_CLASS(x) ((x) >> IXLAND_IOPRIO_CLASS_SHIFT)
#define IXLAND_IOPRIO_PRIO_DATA(x) ((x) & IXLAND_IOPRIO_PRIO_MASK)
#define IXLAND_IOPRIO_PRIO_VALUE(class, data) (((class) << IXLAND_IOPRIO_CLASS_SHIFT) | (data))

/* I/O priority which values */
#define IXLAND_IOPRIO_WHO_PROCESS 0 /* Process */
#define IXLAND_IOPRIO_WHO_PGRP 1    /* Process group */
#define IXLAND_IOPRIO_WHO_USER 2    /* User */

/**
 * @brief Get I/O priority
 *
 * @param which IXLAND_IOPRIO_WHO_*
 * @param who PID/PGID/UID (0 for current)
 * @return int I/O priority on success, -1 on error
 */
int ixland_ioprio_get(int which, int who);

/**
 * @brief Set I/O priority
 *
 * @param which IXLAND_IOPRIO_WHO_*
 * @param who PID/PGID/UID (0 for current)
 * @param ioprio New I/O priority
 * @return int 0 on success, -1 on error
 */
int ixland_ioprio_set(int which, int who, int ioprio);

/* ============================================================================
 * GETRLIMIT HELPER MACROS
 * ============================================================================ */

/* Helper to get soft limit */
#define IXLAND_RLIMIT_SOFT(res)                       \
    ({                                                \
        struct linux_rlimit __rlim;                   \
        int __ret = ixland_getrlimit((res), &__rlim); \
        (__ret == 0 ? __rlim.rlim_cur : 0);           \
    })

/* Helper to get hard limit */
#define IXLAND_RLIMIT_HARD(res)                       \
    ({                                                \
        struct linux_rlimit __rlim;                   \
        int __ret = ixland_getrlimit((res), &__rlim); \
        (__ret == 0 ? __rlim.rlim_max : 0);           \
    })

/* Check if limit is infinite */
#define IXLAND_RLIMIT_IS_INFINITY(lim) ((lim) == IXLAND_RLIM_INFINITY)

/* ============================================================================
 * DEFAULT LIMITS
 * ============================================================================ */

/* iXland default limits */
#define IXLAND_DEFAULT_RLIMIT_CPU IXLAND_RLIM_INFINITY
#define IXLAND_DEFAULT_RLIMIT_FSIZE IXLAND_RLIM_INFINITY
#define IXLAND_DEFAULT_RLIMIT_DATA (512 * 1024 * 1024) /* 512 MB */
#define IXLAND_DEFAULT_RLIMIT_STACK (8 * 1024 * 1024)  /* 8 MB */
#define IXLAND_DEFAULT_RLIMIT_CORE 0                   /* No core dumps */
#define IXLAND_DEFAULT_RLIMIT_RSS IXLAND_RLIM_INFINITY
#define IXLAND_DEFAULT_RLIMIT_NPROC 1024                 /* Max processes */
#define IXLAND_DEFAULT_RLIMIT_NOFILE 256                 /* Max open files */
#define IXLAND_DEFAULT_RLIMIT_MEMLOCK (64 * 1024 * 1024) /* 64 MB */
#define IXLAND_DEFAULT_RLIMIT_AS IXLAND_RLIM_INFINITY
#define IXLAND_DEFAULT_RLIMIT_LOCKS 256
#define IXLAND_DEFAULT_RLIMIT_SIGPENDING 128
#define IXLAND_DEFAULT_RLIMIT_MSGQUEUE (8192 * 1024) /* 8 MB */
#define IXLAND_DEFAULT_RLIMIT_NICE 0
#define IXLAND_DEFAULT_RLIMIT_RTPRIO 0
#define IXLAND_DEFAULT_RLIMIT_RTTIME IXLAND_RLIM_INFINITY

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_RESOURCE_H */

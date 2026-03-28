/* iXland libc - Linux-compatible wait.h
 *
 * Process waiting operations and status macros.
 * These match Linux syscall signatures with iox_ prefix.
 */

#ifndef IOX_LINUX_WAIT_H
#define IOX_LINUX_WAIT_H

#include <sys/resource.h>
#include <sys/types.h>

/* Forward declaration - iox_siginfo is defined in linux/signal.h */
struct iox_siginfo;

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * WAIT OPTIONS
 * ============================================================================ */

/**
 * @brief Wait options for waitpid, waitid, and wait3/4
 */
#define IOX_WNOHANG 0x00000001    /* Return immediately if no child exited */
#define IOX_WUNTRACED 0x00000002  /* Report stopped children */
#define IOX_WCONTINUED 0x00000008 /* Report continued children */
#define IOX_WEXITED 0x00000004    /* Wait for exited children (waitid) */
#define IOX_WSTOPPED 0x00000002   /* Wait for stopped children (waitid) */
#define IOX_WNOWAIT 0x01000000    /* Leave child in waitable state */

/**
 * @brief Wait for all children (waitid only)
 */
#define IOX_P_ALL 0   /* Wait for any child */
#define IOX_P_PID 1   /* Wait for specific PID */
#define IOX_P_PGID 2  /* Wait for any child in process group */
#define IOX_P_PIDFD 3 /* Wait for child referred to by PID file descriptor */

/* ============================================================================
 * PROCESS STATUS MACROS
 * ============================================================================ */

/**
 * @brief Macros for analyzing process status
 *
 * These macros extract information from the status value returned by
 * wait, waitpid, waitid, etc.
 */

/* Normal exit */
#define IOX_WIFEXITED(status) (((status) & 0x7f) == 0)
#define IOX_WEXITSTATUS(status) (((status) >> 8) & 0xff)

/* Signal termination */
#define IOX_WIFSIGNALED(status) (((signed char)(((status) & 0x7f) + 1) >> 1) > 0)
#define IOX_WTERMSIG(status) ((status) & 0x7f)
#define IOX_WCOREDUMP(status) ((status) & 0x80)

/* Stopped processes */
#define IOX_WIFSTOPPED(status) (((status) & 0xff) == 0x7f)
#define IOX_WSTOPSIG(status) IOX_WEXITSTATUS(status)

/* Continued processes */
#define IOX_WIFCONTINUED(status) ((status) == 0xffff)

/* Construct status values (for testing) */
#define IOX_W_EXITCODE(ret, sig) ((ret) << 8 | (sig))
#define IOX_W_STOPCODE(sig) ((sig) << 8 | 0x7f)

/* ============================================================================
 * IDTYPE VALUES
 * ============================================================================ */

typedef enum {
    IOX_P_ALL_ID = 0, /* Wait for any child */
    IOX_P_PID_ID = 1, /* Wait for child with specific PID */
    IOX_P_PGID_ID = 2 /* Wait for child in specific process group */
} iox_idtype_t;

/* Note: struct iox_siginfo is defined in linux/signal.h */
/* Reuse the signal.h definition for consistency */

/* Signal codes for SIGCHLD */
#define IOX_CLD_EXITED 1    /* Child has exited */
#define IOX_CLD_KILLED 2    /* Child was killed */
#define IOX_CLD_DUMPED 3    /* Child terminated abnormally */
#define IOX_CLD_TRAPPED 4   /* Traced child has trapped */
#define IOX_CLD_STOPPED 5   /* Child has stopped */
#define IOX_CLD_CONTINUED 6 /* Stopped child has continued */

/* ============================================================================
 * RUSAGE STRUCTURE
 * ============================================================================ */

/**
 * @brief Resource usage structure
 *
 * Used by wait3 and wait4 to return resource usage information.
 * Mirrors struct rusage from sys/resource.h
 */
struct iox_rusage {
    struct timeval ru_utime; /* User CPU time */
    struct timeval ru_stime; /* System CPU time */
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

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

/**
 * @brief Wait for any child process
 *
 * Suspends execution until any child process terminates.
 *
 * @param stat_loc Pointer to store exit status (can be NULL)
 * @return pid_t Child PID on success, -1 on error
 */
pid_t iox_wait(int *stat_loc);

/**
 * @brief Wait for specific child process
 *
 * Suspends execution until specified child state changes.
 *
 * @param pid Process ID to wait for:
 *            -1: Wait for any child
 *             0: Wait for any child in same process group
 *            >0: Wait for specific PID
 *            <-1: Wait for any child in process group -pid
 * @param stat_loc Pointer to store exit status
 * @param options Options (WNOHANG, WUNTRACED, WCONTINUED)
 * @return pid_t Child PID on success, 0 with WNOHANG if no status, -1 on error
 */
pid_t iox_waitpid(pid_t pid, int *stat_loc, int options);

/**
 * @brief Wait for child with detailed info
 *
 * Provides more detailed information about child state change.
 *
 * @param idtype Type of ID (P_PID, P_PGID, P_ALL)
 * @param id Process or group ID (interpretation depends on idtype)
 * @param infop Pointer to siginfo structure to fill
 * @param options Options (WEXITED, WSTOPPED, WCONTINUED, WNOHANG, WNOWAIT)
 * @return int 0 on success, -1 on error
 */
int iox_waitid(int idtype, id_t id, struct iox_siginfo *infop, int options);

/**
 * @brief Wait for child with resource usage
 *
 * Like wait() but also returns resource usage information.
 *
 * @param stat_loc Pointer to store exit status (can be NULL)
 * @param options Options (WNOHANG, WUNTRACED)
 * @param rusage Pointer to store resource usage (can be NULL)
 * @return pid_t Child PID on success, 0 with WNOHANG, -1 on error
 */
pid_t iox_wait3(int *stat_loc, int options, struct rusage *rusage);

/**
 * @brief Wait for specific child with resource usage
 *
 * Like waitpid() but also returns resource usage information.
 *
 * @param pid Process ID (see waitpid)
 * @param stat_loc Pointer to store exit status
 * @param options Options (WNOHANG, WUNTRACED, WCONTINUED)
 * @param rusage Pointer to store resource usage (can be NULL)
 * @return pid_t Child PID on success, 0 with WNOHANG, -1 on error
 */
pid_t iox_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage);

/* ============================================================================
 * PROCESS GROUP FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get process group ID
 *
 * @return pid_t Process group ID
 */
pid_t iox_getpgrp(void);

/**
 * @brief Set process group
 *
 * Sets process group ID to current process ID.
 *
 * @return int 0 on success, -1 on error
 */
int iox_setpgrp(void);

/**
 * @brief Get process group ID for specific process
 *
 * @param pid Process ID (0 for current)
 * @return pid_t Process group ID, -1 on error
 */
pid_t iox_getpgid(pid_t pid);

/**
 * @brief Set process group ID
 *
 * @param pid Process ID (0 for current)
 * @param pgid Process group ID (0 to use pid)
 * @return int 0 on success, -1 on error
 */
int iox_setpgid(pid_t pid, pid_t pgid);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Check if process exited normally
 *
 * @param status Status value from wait/waitpid
 * @return int Non-zero if process exited normally
 */
static inline int iox_wifexited(int status) {
    return IOX_WIFEXITED(status);
}

/**
 * @brief Get exit status from normally terminated process
 *
 * @param status Status value from wait/waitpid
 * @return int Exit status (0-255)
 */
static inline int iox_wexitstatus(int status) {
    return IOX_WEXITSTATUS(status);
}

/**
 * @brief Check if process was terminated by signal
 *
 * @param status Status value from wait/waitpid
 * @return int Non-zero if process was signaled
 */
static inline int iox_wifsignaled(int status) {
    return IOX_WIFSIGNALED(status);
}

/**
 * @brief Get termination signal
 *
 * @param status Status value from wait/waitpid
 * @return int Signal number
 */
static inline int iox_wtermsig(int status) {
    return IOX_WTERMSIG(status);
}

/**
 * @brief Check if process dumped core
 *
 * @param status Status value from wait/waitpid
 * @return int Non-zero if core was dumped
 */
static inline int iox_wcoredump(int status) {
    return IOX_WCOREDUMP(status);
}

/**
 * @brief Check if process was stopped
 *
 * @param status Status value from wait/waitpid
 * @return int Non-zero if process was stopped
 */
static inline int iox_wifstopped(int status) {
    return IOX_WIFSTOPPED(status);
}

/**
 * @brief Get stop signal
 *
 * @param status Status value from wait/waitpid
 * @return int Signal number
 */
static inline int iox_wstopsig(int status) {
    return IOX_WSTOPSIG(status);
}

/**
 * @brief Check if process was continued
 *
 * @param status Status value from wait/waitpid
 * @return int Non-zero if process was continued
 */
static inline int iox_wifcontinued(int status) {
    return IOX_WIFCONTINUED(status);
}

#ifdef __cplusplus
}
#endif

#endif /* IOX_LINUX_WAIT_H */

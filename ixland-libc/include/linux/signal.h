/* iXland libc - Linux-compatible signal.h
 *
 * Signal handling operations matching Linux syscall signatures.
 * These are iXland-compatible wrappers with iox_ prefix.
 */

#ifndef IOX_LINUX_SIGNAL_H
#define IOX_LINUX_SIGNAL_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration - iox_siginfo is defined in this header */
struct iox_siginfo;

/* ============================================================================
 * SIGNAL NUMBERS
 * ============================================================================ */

/* Standard POSIX signals */
#define IOX_SIGHUP 1     /* Hangup */
#define IOX_SIGINT 2     /* Interrupt (Ctrl+C) */
#define IOX_SIGQUIT 3    /* Quit (Ctrl+\) */
#define IOX_SIGILL 4     /* Illegal instruction */
#define IOX_SIGTRAP 5    /* Trace/breakpoint trap */
#define IOX_SIGABRT 6    /* Abort */
#define IOX_SIGIOT 6     /* IOT trap (synonym for SIGABRT) */
#define IOX_SIGBUS 7     /* Bus error */
#define IOX_SIGFPE 8     /* Floating point exception */
#define IOX_SIGKILL 9    /* Kill (cannot be caught or ignored) */
#define IOX_SIGUSR1 10   /* User-defined signal 1 */
#define IOX_SIGSEGV 11   /* Segmentation fault */
#define IOX_SIGUSR2 12   /* User-defined signal 2 */
#define IOX_SIGPIPE 13   /* Broken pipe */
#define IOX_SIGALRM 14   /* Alarm clock */
#define IOX_SIGTERM 15   /* Termination */
#define IOX_SIGSTKFLT 16 /* Stack fault */
#define IOX_SIGCHLD 17   /* Child stopped or terminated */
#define IOX_SIGCLD 17    /* Child (synonym for SIGCHLD) */
#define IOX_SIGCONT 18   /* Continue */
#define IOX_SIGSTOP 19   /* Stop (cannot be caught or ignored) */
#define IOX_SIGTSTP 20   /* Terminal stop (Ctrl+Z) */
#define IOX_SIGTTIN 21   /* Terminal input for background process */
#define IOX_SIGTTOU 22   /* Terminal output for background process */
#define IOX_SIGURG 23    /* Urgent condition on socket */
#define IOX_SIGXCPU 24   /* CPU time limit exceeded */
#define IOX_SIGXFSZ 25   /* File size limit exceeded */
#define IOX_SIGVTALRM 26 /* Virtual alarm clock */
#define IOX_SIGPROF 27   /* Profiling timer expired */
#define IOX_SIGWINCH 28  /* Window resize signal */
#define IOX_SIGIO 29     /* I/O now possible */
#define IOX_SIGPOLL 29   /* Pollable event (synonym for SIGIO) */
#define IOX_SIGPWR 30    /* Power failure */
#define IOX_SIGSYS 31    /* Bad system call */
#define IOX_SIGUNUSED 31 /* Unused (synonym for SIGSYS) */

/* Real-time signals */
#define IOX_SIGRTMIN 32
#define IOX_SIGRTMAX 64

/* Number of signals */
#define IOX_NSIG 65

/* ============================================================================
 * SPECIAL SIGNAL VALUES
 * ============================================================================ */

#define IOX_SIG_ERR ((iox_sighandler_t)(intptr_t)-1) /* Error return */
#define IOX_SIG_DFL ((iox_sighandler_t)0)            /* Default action */
#define IOX_SIG_IGN ((iox_sighandler_t)1)            /* Ignore signal */
#define IOX_SIG_HOLD ((iox_sighandler_t)2)           /* Hold signal */

/* ============================================================================
 * SIGNAL ACTIONS
 * ============================================================================ */

#define IOX_SIG_BLOCK 0   /* Block signals */
#define IOX_SIG_UNBLOCK 1 /* Unblock signals */
#define IOX_SIG_SETMASK 2 /* Set signal mask */

/* ============================================================================
 * SIGNAL HANDLER TYPE
 * ============================================================================ */

/**
 * @brief Signal handler function type
 */
typedef void (*iox_sighandler_t)(int);

/**
 * @brief Signal action function type (sa_sigaction)
 */
typedef void (*iox_sigaction_t)(int, struct iox_siginfo *, void *);

/* ============================================================================
 * SIGSET_T
 * ============================================================================ */

/**
 * @brief Signal set type
 *
 * Represents a set of signals for masking operations.
 */
typedef struct {
    unsigned long sig[IOX_NSIG / (8 * sizeof(unsigned long)) + 1];
} iox_sigset_t;

/* Sigset operations */
#define IOX_SIGEMPTYSET(set) memset((set), 0, sizeof(iox_sigset_t))
#define IOX_SIGFILLSET(set) memset((set), 0xff, sizeof(iox_sigset_t))
#define IOX_SIGADDSET(set, signum)                         \
    ((set)->sig[(signum) / (8 * sizeof(unsigned long))] |= \
     (1UL << ((signum) % (8 * sizeof(unsigned long)))))
#define IOX_SIGDELSET(set, signum)                         \
    ((set)->sig[(signum) / (8 * sizeof(unsigned long))] &= \
     ~(1UL << ((signum) % (8 * sizeof(unsigned long)))))
#define IOX_SIGISMEMBER(set, signum)                       \
    (((set)->sig[(signum) / (8 * sizeof(unsigned long))] & \
      (1UL << ((signum) % (8 * sizeof(unsigned long))))) != 0)

/* ============================================================================
 * SIGACTION STRUCTURE
 * ============================================================================ */

/**
 * @brief Signal action structure
 *
 * Used by sigaction() to specify signal handling behavior.
 */
struct iox_sigaction {
    /**
     * @brief Signal handler
     *
     * Either sa_handler (simple) or sa_sigaction (extended with info).
     * Only one should be used depending on SA_SIGINFO flag.
     */
    union {
        iox_sighandler_t sa_handler;
        iox_sigaction_t sa_sigaction;
    } __sigaction_handler;

#define sa_handler __sigaction_handler.sa_handler
#define sa_sigaction __sigaction_handler.sa_sigaction

    iox_sigset_t sa_mask;      /* Signals to block during handler */
    int sa_flags;              /* Signal action flags */
    void (*sa_restorer)(void); /* Obsolete/restorer function */
};

/* Signal action flags */
#define IOX_SA_NOCLDSTOP 0x00000001 /* Don't notify on SIGCHLD stop */
#define IOX_SA_NOCLDWAIT 0x00000002 /* Don't create zombies on SIGCHLD */
#define IOX_SA_SIGINFO 0x00000004   /* Use sa_sigaction instead of sa_handler */
#define IOX_SA_ONSTACK 0x08000000   /* Use alternate signal stack */
#define IOX_SA_RESTART 0x10000000   /* Restart system calls */
#define IOX_SA_NODEFER 0x40000000   /* Don't block signal during handler */
#define IOX_SA_RESETHAND 0x80000000 /* Reset to default after handler */

/* ============================================================================
 * SIGINFO STRUCTURE
 * ============================================================================ */

/**
 * @brief Signal information structure
 *
 * Provides detailed information about signal delivery.
 */
struct iox_siginfo {
    int32_t si_signo; /* Signal number */
    int32_t si_errno; /* Error number */
    int32_t si_code;  /* Signal code */

    union {
        int32_t _pad[29]; /* Padding to match kernel size */

        /* kill() */
        struct {
            pid_t _pid; /* Sending process ID */
            uid_t _uid; /* Real user ID of sending process */
        } _kill;

        /* POSIX.1b timers */
        struct {
            uint32_t _tid;                   /* Timer ID */
            uint32_t _overrun;               /* Overrun count */
            char _pad[sizeof(int32_t) * 27]; /* Padding */
        } _timer;

        /* SIGCHLD */
        struct {
            pid_t _pid;      /* Child process ID */
            uid_t _uid;      /* Real user ID of child */
            int32_t _status; /* Exit value or signal */
            clock_t _utime;  /* User CPU time */
            clock_t _stime;  /* System CPU time */
        } _sigchld;

        /* SIGILL, SIGFPE, SIGSEGV, SIGBUS */
        struct {
            void *_addr;       /* Faulting address */
            int16_t _addr_lsb; /* LSB of faulting address */
        } _sigfault;

        /* SIGPOLL/SIGIO */
        struct {
            int64_t _band; /* Band event for SIGPOLL */
            int32_t _fd;   /* File descriptor */
        } _sigpoll;
    } _sifields;
};

/* Convenience accessors for siginfo - use iox_ prefix to avoid conflicts */
#define iox_si_pid _sifields._kill._pid
#define iox_si_uid _sifields._kill._uid
#define iox_si_status _sifields._sigchld._status
#define iox_si_utime _sifields._sigchld._utime
#define iox_si_stime _sifields._sigchld._stime
#define iox_si_addr _sifields._sigfault._addr
#define iox_si_band _sifields._sigpoll._band
#define iox_si_fd _sifields._sigpoll._fd
#define iox_si_timerid _sifields._timer._tid
#define iox_si_overrun _sifields._timer._overrun

/* Signal codes for si_code */
/* Sent by kill() */
#define IOX_SI_USER 0
#define IOX_SI_KERNEL 0x80
#define IOX_SI_QUEUE -1
#define IOX_SI_TIMER -2
#define IOX_SI_MESGQ -3
#define IOX_SI_ASYNCIO -4
#define IOX_SI_SIGIO -5
#define IOX_SI_TKILL -6
#define IOX_SI_ASYNCNL -60

/* SIGILL codes */
#define IOX_ILL_ILLOPC 1 /* Illegal opcode */
#define IOX_ILL_ILLOPN 2 /* Illegal operand */
#define IOX_ILL_ILLADR 3 /* Illegal addressing mode */
#define IOX_ILL_ILLTRP 4 /* Illegal trap */
#define IOX_ILL_PRVOPC 5 /* Privileged opcode */
#define IOX_ILL_PRVREG 6 /* Privileged register */
#define IOX_ILL_COPROC 7 /* Coprocessor error */
#define IOX_ILL_BADSTK 8 /* Internal stack error */

/* SIGFPE codes */
#define IOX_FPE_INTDIV 1 /* Integer divide by zero */
#define IOX_FPE_INTOVF 2 /* Integer overflow */
#define IOX_FPE_FLTDIV 3 /* Floating point divide by zero */
#define IOX_FPE_FLTOVF 4 /* Floating point overflow */
#define IOX_FPE_FLTUND 5 /* Floating point underflow */
#define IOX_FPE_FLTRES 6 /* Floating point inexact result */
#define IOX_FPE_FLTINV 7 /* Floating point invalid operation */
#define IOX_FPE_FLTSUB 8 /* Subscript out of range */

/* SIGSEGV codes */
#define IOX_SEGV_MAPERR 1 /* Address not mapped to object */
#define IOX_SEGV_ACCERR 2 /* Access permissions error */
#define IOX_SEGV_BNDERR 3 /* Bounds checking failure */
#define IOX_SEGV_PKUERR 4 /* Protection key checking failure */

/* SIGBUS codes */
#define IOX_BUS_ADRALN 1 /* Invalid address alignment */
#define IOX_BUS_ADRERR 2 /* Non-existent physical address */
#define IOX_BUS_OBJERR 3 /* Object specific hardware error */

/* SIGTRAP codes */
#define IOX_TRAP_BRKPT 1 /* Process breakpoint */
#define IOX_TRAP_TRACE 2 /* Process trace trap */

/* SIGCHLD codes */
#define IOX_CLD_EXITED 1    /* Child has exited */
#define IOX_CLD_KILLED 2    /* Child was killed */
#define IOX_CLD_DUMPED 3    /* Child terminated abnormally */
#define IOX_CLD_TRAPPED 4   /* Traced child has trapped */
#define IOX_CLD_STOPPED 5   /* Child has stopped */
#define IOX_CLD_CONTINUED 6 /* Stopped child has continued */

/* SIGPOLL codes */
#define IOX_POLL_IN 1  /* Data input available */
#define IOX_POLL_OUT 2 /* Output buffers available */
#define IOX_POLL_MSG 3 /* Input message available */
#define IOX_POLL_ERR 4 /* I/O error */
#define IOX_POLL_PRI 5 /* High priority input available */
#define IOX_POLL_HUP 6 /* Device disconnected */

/* ============================================================================
 * STACK STRUCTURE
 * ============================================================================ */

/**
 * @brief Alternate signal stack structure
 */
struct iox_sigaltstack {
    void *ss_sp;    /* Stack base or pointer */
    int ss_flags;   /* Flags */
    size_t ss_size; /* Stack size */
};

/* Stack flags */
#define IOX_SS_ONSTACK 1 /* Process is on alternate stack */
#define IOX_SS_DISABLE 4 /* Alternate stack is disabled */

/* Minimum stack size */
#define IOX_MINSIGSTKSZ 2048
#define IOX_SIGSTKSZ 8192

/* ============================================================================
 * SIGEVENT STRUCTURE
 * ============================================================================ */

/**
 * @brief Signal notification structure
 *
 * Note: Uses iox_ prefix for member macros to avoid conflicts with system headers.
 */
struct iox_sigevent {
    int32_t sigev_value;  /* Signal value */
    int32_t sigev_signo;  /* Signal number */
    int32_t sigev_notify; /* Notification method */

    union {
        int32_t _pad[12];

        struct {
            void (*_function)(int32_t); /* Notification function */
            void *_attribute;           /* Notification attributes */
        } _sigev_thread;
    } _sigev_un;
};

/* Accessors with iox_ prefix to avoid conflicts */
#define iox_sigev_notify_function _sigev_un._sigev_thread._function
#define iox_sigev_notify_attributes _sigev_un._sigev_thread._attribute

/* Notification methods */
#define IOX_SIGEV_SIGNAL 0 /* Notify via signal */
#define IOX_SIGEV_NONE 1   /* No notification */
#define IOX_SIGEV_THREAD 2 /* Notify via thread */

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

/**
 * @brief Install signal handler (simple interface)
 *
 * @param signum Signal number
 * @param handler Handler function (SIG_DFL, SIG_IGN, or custom)
 * @return iox_sighandler_t Previous handler, SIG_ERR on error
 */
iox_sighandler_t iox_signal(int signum, iox_sighandler_t handler);

/**
 * @brief Install signal handler (POSIX interface)
 *
 * @param signum Signal number
 * @param act New action (can be NULL)
 * @param oldact Previous action (can be NULL)
 * @return int 0 on success, -1 on error
 */
int iox_sigaction(int signum, const struct iox_sigaction *act, struct iox_sigaction *oldact);

/**
 * @brief Send signal to process
 *
 * @param pid Process ID
 * @param sig Signal number
 * @return int 0 on success, -1 on error
 */
int iox_kill(pid_t pid, int sig);

/**
 * @brief Send signal to process group
 *
 * @param pgrp Process group ID
 * @param sig Signal number
 * @return int 0 on success, -1 on error
 */
int iox_killpg(pid_t pgrp, int sig);

/**
 * @brief Send signal to current process
 *
 * @param sig Signal number
 * @return int 0 on success, -1 on error
 */
int iox_raise(int sig);

/**
 * @brief Send signal to thread
 *
 * @param tid Thread ID
 * @param sig Signal number
 * @return int 0 on success, -1 on error
 */
int iox_tkill(pid_t tid, int sig);

/**
 * @brief Send signal to thread in specific process
 *
 * @param tgid Thread group ID
 * @param tid Thread ID
 * @param sig Signal number
 * @return int 0 on success, -1 on error
 */
int iox_tgkill(pid_t tgid, pid_t tid, int sig);

/**
 * @brief Examine and change blocked signals
 *
 * @param how Action (SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK)
 * @param set New signal set (can be NULL)
 * @param oldset Previous signal set (can be NULL)
 * @return int 0 on success, -1 on error
 */
int iox_sigprocmask(int how, const iox_sigset_t *set, iox_sigset_t *oldset);

/**
 * @brief Get set of pending signals
 *
 * @param set Pointer to store pending signals
 * @return int 0 on success, -1 on error
 */
int iox_sigpending(iox_sigset_t *set);

/**
 * @brief Wait for signal
 *
 * Atomically replace signal mask and wait for signal.
 *
 * @param mask Signal mask to use while waiting
 * @return int -1 with errno=EINTR when signal caught
 */
int iox_sigsuspend(const iox_sigset_t *mask);

/**
 * @brief Wait for signal with timeout
 *
 * @param set Signal set to wait for
 * @param info Pointer to store signal info (can be NULL)
 * @param timeout Timeout (can be NULL for indefinite)
 * @return int 0 on success, -1 on error or timeout
 */
int iox_sigtimedwait(const iox_sigset_t *set, struct iox_siginfo *info,
                     const struct timespec *timeout);

/**
 * @brief Wait for signal
 *
 * @param set Signal set to wait for
 * @param info Pointer to store signal info (can be NULL)
 * @return int Signal number on success, -1 on error
 */
int iox_sigwaitinfo(const iox_sigset_t *set, struct iox_siginfo *info);

/**
 * @brief Simplified wait for signal
 *
 * @param set Signal set to wait for
 * @param sig Pointer to store signal number
 * @return int 0 on success, -1 on error
 */
int iox_sigwait(const iox_sigset_t *set, int *sig);

/**
 * @brief Initialize empty signal set
 *
 * @param set Signal set to initialize
 * @return int 0 on success, -1 on error
 */
int iox_sigemptyset(iox_sigset_t *set);

/**
 * @brief Initialize full signal set
 *
 * @param set Signal set to initialize
 * @return int 0 on success, -1 on error
 */
int iox_sigfillset(iox_sigset_t *set);

/**
 * @brief Add signal to set
 *
 * @param set Signal set
 * @param signum Signal number
 * @return int 0 on success, -1 on error
 */
int iox_sigaddset(iox_sigset_t *set, int signum);

/**
 * @brief Remove signal from set
 *
 * @param set Signal set
 * @param signum Signal number
 * @return int 0 on success, -1 on error
 */
int iox_sigdelset(iox_sigset_t *set, int signum);

/**
 * @brief Test if signal is in set
 *
 * @param set Signal set
 * @param signum Signal number
 * @return int 1 if member, 0 if not, -1 on error
 */
int iox_sigismember(const iox_sigset_t *set, int signum);

/**
 * @brief Get/set signal stack context
 *
 * @param ss New stack (can be NULL)
 * @param oldss Previous stack (can be NULL)
 * @return int 0 on success, -1 on error
 */
int iox_sigaltstack(const struct iox_sigaltstack *ss, struct iox_sigaltstack *oldss);

/**
 * @brief Set alarm
 *
 * @param seconds Seconds until SIGALRM
 * @return unsigned int Remaining seconds from previous alarm
 */
unsigned int iox_alarm(unsigned int seconds);

/**
 * @brief Set high-resolution alarm
 *
 * @param usecs Microseconds until SIGALRM
 * @return useconds_t Remaining microseconds from previous alarm
 */
useconds_t iox_ualarm(useconds_t usecs, useconds_t interval);

/**
 * @brief Pause execution until signal
 *
 * @return int -1 with errno=EINTR when signal caught
 */
int iox_pause(void);

/**
 * @brief Return from signal handler and cleanup stack frame
 *
 * @param __unused Not used
 */
void iox_sigreturn(void *__unused);

/**
 * @brief Get signal description string
 *
 * @param sig Signal number
 * @return const char* Description of signal
 */
const char *iox_strsignal(int sig);

/**
 * @brief Copy signal set
 *
 * @param dest Destination set
 * @param src Source set
 * @return int 0 on success, -1 on error
 */
int iox_sigorset(iox_sigset_t *dest, const iox_sigset_t *left, const iox_sigset_t *right);

/**
 * @brief Compute intersection of signal sets
 *
 * @param dest Destination set
 * @param left Left operand
 * @param right Right operand
 * @return int 0 on success, -1 on error
 */
int iox_sigandset(iox_sigset_t *dest, const iox_sigset_t *left, const iox_sigset_t *right);

#ifdef __cplusplus
}
#endif

#endif /* IOX_LINUX_SIGNAL_H */

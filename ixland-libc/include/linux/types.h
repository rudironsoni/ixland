/* iXland libc - Linux-compatible types.h
 *
 * Linux-specific type definitions for iXland kernel compatibility.
 * These types match Linux kernel definitions for syscall compatibility.
 */

#ifndef IOX_LINUX_TYPES_H
#define IOX_LINUX_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FIXED-WIDTH INTEGER TYPES
 * ============================================================================
 * Linux kernel uses specific type names that differ from standard C types.
 * These provide exact compatibility with Linux kernel headers.
 */

#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__

typedef int8_t __s8;
typedef uint8_t __u8;
typedef int16_t __s16;
typedef uint16_t __u16;
typedef int32_t __s32;
typedef uint32_t __u32;
typedef int64_t __s64;
typedef uint64_t __u64;

#endif /* __BIT_TYPES_DEFINED__ */

/* ============================================================================
 * LINUX KERNEL TYPES
 * ============================================================================
 * These types match Linux kernel definitions for ABI compatibility.
 */

typedef uint32_t __le32; /* Little-endian 32-bit */
typedef uint32_t __be32; /* Big-endian 32-bit */
typedef uint16_t __le16; /* Little-endian 16-bit */
typedef uint16_t __be16; /* Big-endian 16-bit */
typedef uint64_t __le64; /* Little-endian 64-bit */
typedef uint64_t __be64; /* Big-endian 64-bit */

/* Physical/virtual addresses */
typedef uintptr_t phys_addr_t;
typedef void *virt_addr_t;

/* Process/thread identifiers - forward declarations */
typedef pid_t __kernel_pid_t;
typedef pid_t __kernel_tid_t;

/* User/group identifiers */
typedef uid_t __kernel_uid_t;
typedef gid_t __kernel_gid_t;
typedef uint32_t __kernel_uid32_t;
typedef uint32_t __kernel_gid32_t;

/* Device numbers */
typedef uint32_t __kernel_dev_t;
typedef uint32_t __kernel_major_t;
typedef uint32_t __kernel_minor_t;

/* File offset and size */
typedef off_t __kernel_off_t;
typedef off_t __kernel_loff_t;     /* 64-bit offset */
typedef uint64_t __kernel_ino_t;   /* Inode number */
typedef uint32_t __kernel_mode_t;  /* File mode */
typedef uint64_t __kernel_nlink_t; /* Link count */

/* Time types */
typedef int64_t __kernel_time_t;
typedef int64_t __kernel_suseconds_t;
typedef uint32_t __kernel_clock_t;
typedef int32_t __kernel_timer_t;
typedef int32_t __kernel_clockid_t;

/* Size types */
typedef size_t __kernel_size_t;
typedef ssize_t __kernel_ssize_t;
typedef ptrdiff_t __kernel_ptrdiff_t;

/* File descriptor */
typedef int32_t __kernel_fd_t;

/* Signal */
typedef int32_t __kernel_sigset_t;

typedef uint32_t __kernel_old_dev_t; /* Old 16-bit device number */

/* ============================================================================
 * FORWARD DECLARATIONS FOR IXLAND TYPES
 * ============================================================================
 * These are forward declarations for iXland-specific structures.
 * Full definitions are in internal headers.
 */

/* Forward declaration for task structure */
struct iox_task;
struct iox_task_struct;
typedef struct iox_task *iox_task_t;
typedef struct iox_task_struct *iox_task_struct_t;

/* Forward declaration for file descriptor table */
struct iox_files;
struct iox_files_struct;
typedef struct iox_files *iox_files_t;
typedef struct iox_files_struct *iox_files_struct_t;

/* Forward declaration for signal handling */
struct iox_sigpending;
typedef struct iox_sigpending *iox_sigpending_t;

/* Forward declaration for memory management */
struct iox_mm_struct;
typedef struct iox_mm_struct *iox_mm_struct_t;

/* Forward declaration for namespace */
struct iox_nsproxy;
typedef struct iox_nsproxy *iox_nsproxy_t;

/* Forward declaration for filesystem */
struct iox_fs_struct;
typedef struct iox_fs_struct *iox_fs_struct_t;

/* Forward declaration for credentials */
struct iox_cred;
struct iox_real_cred;
typedef struct iox_cred *iox_cred_t;
typedef struct iox_real_cred *iox_real_cred_t;

/* Forward declaration for signal handling state */
struct iox_sighand_struct;
typedef struct iox_sighand_struct *iox_sighand_t;

/* Forward declaration for thread group */
struct iox_thread_group;
typedef struct iox_thread_group *iox_thread_group_t;

/* Forward declaration for wait queue */
struct iox_wait_queue;
typedef struct iox_wait_queue *iox_wait_queue_t;

/* Forward declaration for completion */
struct iox_completion;
typedef struct iox_completion *iox_completion_t;

/* Forward declaration for work queue */
struct iox_work_struct;
typedef struct iox_work_struct *iox_work_struct_t;

/* ============================================================================
 * KERNEL OBJECT TYPES
 * ============================================================================
 * Types for kernel object handles.
 */

/* File handle */
typedef uint64_t iox_handle_t;
typedef uint64_t iox_file_handle_t;

/* Event/notification */
typedef uint32_t iox_event_t;
typedef uint32_t iox_notification_t;

/* Resource handle */
typedef uint64_t iox_resource_t;

/* Capability */
typedef uint64_t iox_cap_t;
typedef uint64_t iox_cap_user_header_t;
typedef uint64_t iox_cap_user_data_t;

/* ============================================================================
 * ALIGNMENT AND ATOMIC TYPES
 * ============================================================================
 */

/* Aligned types for DMA/cache operations */
typedef uint64_t __aligned_u64 __attribute__((aligned(8)));
typedef uint32_t __aligned_u32 __attribute__((aligned(4)));
typedef uint16_t __aligned_u16 __attribute__((aligned(2)));

/* ============================================================================
 * IOVEC TYPES
 * ============================================================================
 */

/* IO vector for scatter/gather I/O */
struct iox_iovec {
    void *iov_base; /* Starting address */
    size_t iov_len; /* Number of bytes */
};

typedef struct iox_iovec iox_iovec_t;

/* Kernel IO vector (64-bit clean) */
struct iox_kiovec {
    uint64_t iov_base; /* Starting address (as 64-bit) */
    uint64_t iov_len;  /* Number of bytes */
};

typedef struct iox_kiovec iox_kiovec_t;

/* ============================================================================
 * IOCTL TYPES
 * ============================================================================
 */

/* IOCTL command encoding */
typedef uint32_t iox_ioctl_cmd_t;

/* IOCTL argument */
typedef uint64_t iox_ioctl_arg_t;

/* ============================================================================
 * SYSTEM CALL TYPES
 * ============================================================================
 */

/* Syscall number */
typedef uint64_t iox_syscall_num_t;

/* Syscall return value */
typedef int64_t iox_syscall_ret_t;

/* Syscall argument */
typedef uint64_t iox_syscall_arg_t;

/* ============================================================================
 * SIGNAL TYPES
 * ============================================================================
 */

/* Signal set (simplified) */
typedef struct {
    unsigned long sig[128 / sizeof(unsigned long)];
} iox_kernel_sigset_t;

/* Signal info (forward) */
struct iox_kernel_siginfo;
typedef struct iox_kernel_siginfo iox_kernel_siginfo_t;

/* ============================================================================
 * TIMER TYPES
 * ============================================================================
 */

/* Timer ID */
typedef int32_t iox_timer_t;

/* Interval timer value */
struct iox_itimerval {
    struct timeval it_interval; /* Timer interval */
    struct timeval it_value;    /* Current value */
};

typedef struct iox_itimerval iox_itimerval_t;

/* High-resolution timer */
struct iox_itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};

typedef struct iox_itimerspec iox_itimerspec_t;

/* ============================================================================
 * POLL TYPES
 * ============================================================================
 */

/* Poll file descriptor */
struct iox_pollfd {
    int32_t fd;      /* File descriptor */
    int16_t events;  /* Requested events */
    int16_t revents; /* Returned events */
};

typedef struct iox_pollfd iox_pollfd_t;

/* ============================================================================
 * EPOLL TYPES
 * ============================================================================
 */

/* Epoll data union */
union iox_epoll_data {
    void *ptr;
    int32_t fd;
    uint32_t u32;
    uint64_t u64;
};

typedef union iox_epoll_data iox_epoll_data_t;

/* Epoll event structure */
struct iox_epoll_event {
    uint32_t events;       /* Epoll events */
    iox_epoll_data_t data; /* User data variable */
} __attribute__((packed));

typedef struct iox_epoll_event iox_epoll_event_t;

/* ============================================================================
 * SOCKET TYPES
 * ============================================================================
 */

/* Socket address (forward declaration) */
struct iox_sockaddr;
typedef struct iox_sockaddr iox_sockaddr_t;

/* Socket address storage */
struct iox_sockaddr_storage {
    uint16_t ss_family; /* Address family */
    char ss_padding[128 - sizeof(uint16_t)];
};

typedef struct iox_sockaddr_storage iox_sockaddr_storage_t;

/* Socket length */
typedef uint32_t iox_socklen_t;

/* Message header for sendmsg/recvmsg */
struct iox_msghdr {
    void *msg_name;            /* Address to send to/receive from */
    iox_socklen_t msg_namelen; /* Length of address */
    struct iox_iovec *msg_iov; /* Vector of data to send/receive */
    size_t msg_iovlen;         /* Number of elements in vector */
    void *msg_control;         /* Ancillary data */
    size_t msg_controllen;     /* Ancillary data buffer length */
    int32_t msg_flags;         /* Flags on received message */
};

typedef struct iox_msghdr iox_msghdr_t;

/* ============================================================================
 * USER/GROUP DATABASE TYPES
 * ============================================================================
 */

/* User entry (re-declared for Linux compatibility) */
struct iox_passwd {
    char *pw_name;   /* Username */
    char *pw_passwd; /* Password */
    __kernel_uid32_t pw_uid;
    __kernel_gid32_t pw_gid;
    char *pw_gecos; /* Real name */
    char *pw_dir;   /* Home directory */
    char *pw_shell; /* Shell program */
};

typedef struct iox_passwd iox_passwd_t;

/* Group entry (re-declared for Linux compatibility) */
struct iox_group {
    char *gr_name;   /* Group name */
    char *gr_passwd; /* Group password */
    __kernel_gid32_t gr_gid;
    char **gr_mem; /* Member list */
};

typedef struct iox_group iox_group_t;

/* ============================================================================
 * LIMIT TYPES
 * ============================================================================
 */

/* Resource limit (64-bit) */
struct iox_rlimit64 {
    uint64_t rlim_cur; /* Soft limit */
    uint64_t rlim_max; /* Hard limit */
};

typedef struct iox_rlimit64 iox_rlimit64_t;

/* Resource usage */
struct iox_rusage_64 {
    struct timeval ru_utime; /* User time used */
    struct timeval ru_stime; /* System time used */
    int64_t ru_maxrss;       /* Maximum resident set size */
    int64_t ru_ixrss;        /* Integral shared memory size */
    int64_t ru_idrss;        /* Integral unshared data size */
    int64_t ru_isrss;        /* Integral unshared stack size */
    int64_t ru_minflt;       /* Page reclaims (soft page faults) */
    int64_t ru_majflt;       /* Page faults (hard page faults) */
    int64_t ru_nswap;        /* Swaps */
    int64_t ru_inblock;      /* Block input operations */
    int64_t ru_oublock;      /* Block output operations */
    int64_t ru_msgsnd;       /* Messages sent */
    int64_t ru_msgrcv;       /* Messages received */
    int64_t ru_nsignals;     /* Signals received */
    int64_t ru_nvcsw;        /* Voluntary context switches */
    int64_t ru_nivcsw;       /* Involuntary context switches */
};

typedef struct iox_rusage_64 iox_rusage_64_t;

/* ============================================================================
 * STATFS TYPES
 * ============================================================================
 */

/* File system statistics */
struct iox_statfs_64 {
    int64_t f_type;     /* Filesystem type */
    int64_t f_bsize;    /* Optimal transfer block size */
    int64_t f_blocks;   /* Total data blocks */
    int64_t f_bfree;    /* Free blocks */
    int64_t f_bavail;   /* Free blocks for unprivileged users */
    int64_t f_files;    /* Total file nodes */
    int64_t f_ffree;    /* Free file nodes */
    int64_t f_fsid;     /* Filesystem ID */
    int64_t f_namelen;  /* Maximum filename length */
    int64_t f_frsize;   /* Fragment size */
    int64_t f_flags;    /* Mount flags */
    int64_t f_spare[4]; /* Padding */
};

typedef struct iox_statfs_64 iox_statfs_64_t;

/* ============================================================================
 * MMAP TYPES
 * ============================================================================
 */

/* Memory mapping flags */
typedef int32_t iox_mmap_prot_t;
typedef int32_t iox_mmap_flags_t;

/* Memory advice */
typedef int32_t iox_madvise_advice_t;

/* Memory sync flags */
typedef int32_t iox_msync_flags_t;

/* Memory lock flags */
typedef int32_t iox_mlock_flags_t;

/* ============================================================================
 * DIRECTORY TYPES
 * ============================================================================
 */

/* Directory entry (for getdents) */
struct iox_dirent_64 {
    uint64_t d_ino;          /* Inode number */
    int64_t d_off;           /* Offset to next dirent */
    unsigned short d_reclen; /* Length of this record */
    unsigned char d_type;    /* File type */
    char d_name[];           /* Filename (null-terminated) */
};

typedef struct iox_dirent_64 iox_dirent_64_t;

/* Directory entry types */
#define IOX_DT_UNKNOWN 0
#define IOX_DT_FIFO 1
#define IOX_DT_CHR 2
#define IOX_DT_DIR 4
#define IOX_DT_BLK 6
#define IOX_DT_REG 8
#define IOX_DT_LNK 10
#define IOX_DT_SOCK 12
#define IOX_DT_WHT 14

/* ============================================================================
 * AIO TYPES
 * ============================================================================
 */

/* AIO context */
typedef uint64_t iox_aio_context_t;

/* AIO event */
struct iox_io_event {
    uint64_t data; /* User data */
    uint64_t obj;  /* Object pointer */
    int64_t res;   /* Result code */
    int64_t res2;  /* Secondary result */
};

typedef struct iox_io_event iox_io_event_t;

/* ============================================================================
 * EVENTFD TYPES
 * ============================================================================
 */

/* Event file descriptor */
typedef int32_t iox_eventfd_t;

/* Event file descriptor flags */
#define IOX_EFD_SEMAPHORE 1
#define IOX_EFD_CLOEXEC 02000000
#define IOX_EFD_NONBLOCK 00004000

/* ============================================================================
 * TIMERFD TYPES
 * ============================================================================
 */

/* Timer file descriptor */
typedef int32_t iox_timerfd_t;

/* Timer file descriptor flags */
#define IOX_TFD_CLOEXEC 02000000
#define IOX_TFD_NONBLOCK 00004000

/* ============================================================================
 * SIGNALFD TYPES
 * ============================================================================
 */

/* Signal file descriptor */
typedef int32_t iox_signalfd_t;

/* Signal file descriptor flags */
#define IOX_SFD_CLOEXEC 02000000
#define IOX_SFD_NONBLOCK 00004000

/* Signalfd siginfo */
struct iox_signalfd_siginfo {
    uint32_t ssi_signo;    /* Signal number */
    int32_t ssi_errno;     /* Error number */
    int32_t ssi_code;      /* Signal code */
    uint32_t ssi_pid;      /* PID of sender */
    uint32_t ssi_uid;      /* Real UID of sender */
    int32_t ssi_fd;        /* File descriptor */
    uint32_t ssi_tid;      /* Kernel timer ID */
    uint32_t ssi_band;     /* Band event */
    uint32_t ssi_overrun;  /* POSIX timer overrun count */
    uint32_t ssi_trapno;   /* Trap number */
    int32_t ssi_status;    /* Exit status or signal */
    int32_t ssi_int;       /* Integer sent by sigqueue */
    uint64_t ssi_ptr;      /* Pointer sent by sigqueue */
    uint64_t ssi_utime;    /* User CPU time */
    uint64_t ssi_stime;    /* System CPU time */
    uint64_t ssi_addr;     /* Address that generated signal */
    uint16_t ssi_addr_lsb; /* LSB of address */
    uint16_t __pad2;
    int32_t ssi_syscall;    /* Syscall number */
    uint64_t ssi_call_addr; /* Syscall entry address */
    uint64_t ssi_arch;      /* AUDIT_ARCH_* value */
    uint8_t __pad[28];      /* Padding for future expansion */
};

typedef struct iox_signalfd_siginfo iox_signalfd_siginfo_t;

/* ============================================================================
 * PRCTL TYPES
 * ============================================================================
 */

/* PRCTL option */
typedef int32_t iox_prctl_option_t;

/* PRCTL return value */
typedef int64_t iox_prctl_ret_t;

/* ============================================================================
 * PERF TYPES
 * ============================================================================
 */

/* Performance event file descriptor */
typedef int32_t iox_perf_event_fd_t;

/* Performance event attributes (simplified) */
struct iox_perf_event_attr {
    uint32_t type;   /* Type of event */
    uint32_t size;   /* Size of attribute structure */
    uint64_t config; /* Type-specific configuration */
    /* ... additional fields omitted for brevity ... */
};

typedef struct iox_perf_event_attr iox_perf_event_attr_t;

/* ============================================================================
 * BPF TYPES
 * ============================================================================
 */

/* BPF command */
typedef int32_t iox_bpf_cmd_t;

/* BPF map file descriptor */
typedef int32_t iox_bpf_map_fd_t;

/* BPF program file descriptor */
typedef int32_t iox_bpf_prog_fd_t;

/* ============================================================================
 * LANDLOCK TYPES
 * ============================================================================
 */

/* Landlock ruleset file descriptor */
typedef int32_t iox_landlock_ruleset_fd_t;

/* Landlock rule type */
typedef int32_t iox_landlock_rule_type_t;

/* ============================================================================
 * FANOTIFY TYPES
 * ============================================================================
 */

/* Fanotify file descriptor */
typedef int32_t iox_fanotify_fd_t;

/* Fanotify event metadata */
struct iox_fanotify_event_metadata {
    uint8_t event_len; /* Size of event including path */
    uint8_t vers;      /* Version of fanotify */
    uint16_t res1;     /* Reserved */
    int32_t mask;      /* Bitmask of events */
    int32_t fd;        /* File descriptor (or -1) */
    int32_t pid;       /* Process ID that generated event */
};

typedef struct iox_fanotify_event_metadata iox_fanotify_event_metadata_t;

/* ============================================================================
 * MOUNT TYPES
 * ============================================================================
 */

/* Mount flags */
typedef uint64_t iox_mount_flags_t;

/* Filesystem type */
typedef const char *iox_fs_type_t;

/* ============================================================================
 * MODULE TYPES (for future kmod support)
 * ============================================================================
 */

/* Module info */
struct iox_module_info {
    uint64_t addr; /* Module address */
    uint64_t size; /* Module size */
    char name[64]; /* Module name */
};

typedef struct iox_module_info iox_module_info_t;

/* ============================================================================
 * DEBUG TYPES
 * ============================================================================
 */

/* Trace event ID */
typedef uint32_t iox_trace_event_id_t;

/* Probe address */
typedef uint64_t iox_probe_addr_t;

#ifdef __cplusplus
}
#endif

#endif /* IOX_LINUX_TYPES_H */

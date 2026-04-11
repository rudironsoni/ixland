#ifndef IXLAND_WASM_WASI_H
#define IXLAND_WASM_WASI_H

/* iXland Wasm WASI - WASI Guest ABI Policy
 *
 * This header defines how WASI syscalls map onto iXland host semantics.
 * WASI operations are translated to iXland host service calls.
 */

#include "types.h"
#include "host.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * WASI VERSIONS
 * ============================================================================ */

typedef enum {
    IXLAND_WASI_VERSION_PREVIEW0 = 0,  /* Legacy WASI snapshot 0 */
    IXLAND_WASI_VERSION_PREVIEW1 = 1,  /* WASI Preview 1 (stable) */
    IXLAND_WASI_VERSION_PREVIEW2 = 2   /* WASI Preview 2 (components) */
} ixland_wasi_version_t;

/* ============================================================================
 * WASI ERROR CODES (mapped to errno values)
 * ============================================================================ */

typedef enum {
    IXLAND_WASI_ESUCCESS = 0,
    IXLAND_WASI_E2BIG = 1,
    IXLAND_WASI_EACCES = 2,
    IXLAND_WASI_EADDRINUSE = 3,
    IXLAND_WASI_EADDRNOTAVAIL = 4,
    IXLAND_WASI_EAFNOSUPPORT = 5,
    IXLAND_WASI_EAGAIN = 6,
    IXLAND_WASI_EALREADY = 7,
    IXLAND_WASI_EBADF = 8,
    IXLAND_WASI_EBUSY = 9,
    IXLAND_WASI_ECHILD = 10,
    IXLAND_WASI_ECONNABORTED = 11,
    IXLAND_WASI_ECONNREFUSED = 12,
    IXLAND_WASI_ECONNRESET = 13,
    IXLAND_WASI_EDEADLK = 14,
    IXLAND_WASI_EDESTADDRREQ = 15,
    IXLAND_WASI_EDOM = 16,
    IXLAND_WASI_EEXIST = 17,
    IXLAND_WASI_EFAULT = 18,
    IXLAND_WASI_EFBIG = 19,
    IXLAND_WASI_EHOSTUNREACH = 20,
    IXLAND_WASI_EILSEQ = 21,
    IXLAND_WASI_EINPROGRESS = 22,
    IXLAND_WASI_EINTR = 23,
    IXLAND_WASI_EINVAL = 24,
    IXLAND_WASI_EIO = 25,
    IXLAND_WASI_EISCONN = 26,
    IXLAND_WASI_EISDIR = 27,
    IXLAND_WASI_ELOOP = 28,
    IXLAND_WASI_EMFILE = 29,
    IXLAND_WASI_EMLINK = 30,
    IXLAND_WASI_EMSGSIZE = 31,
    IXLAND_WASI_EMULTIHOP = 32,
    IXLAND_WASI_ENAMETOOLONG = 33,
    IXLAND_WASI_ENETDOWN = 34,
    IXLAND_WASI_ENETUNREACH = 35,
    IXLAND_WASI_ENFILE = 36,
    IXLAND_WASI_ENOBUFS = 37,
    IXLAND_WASI_ENODEV = 38,
    IXLAND_WASI_ENOENT = 39,
    IXLAND_WASI_ENOEXEC = 40,
    IXLAND_WASI_ENOLCK = 41,
    IXLAND_WASI_ENOLINK = 42,
    IXLAND_WASI_ENOMEM = 43,
    IXLAND_WASI_ENOMSG = 44,
    IXLAND_WASI_ENOPROTOOPT = 45,
    IXLAND_WASI_ENOSPC = 46,
    IXLAND_WASI_ENOSYS = 47,
    IXLAND_WASI_ENOTCONN = 48,
    IXLAND_WASI_ENOTDIR = 49,
    IXLAND_WASI_ENOTEMPTY = 50,
    IXLAND_WASI_ENOTRECOVERABLE = 51,
    IXLAND_WASI_ENOTSOCK = 52,
    IXLAND_WASI_ENOTSUP = 53,
    IXLAND_WASI_ENOTTY = 54,
    IXLAND_WASI_ENXIO = 55,
    IXLAND_WASI_EOVERFLOW = 56,
    IXLAND_WASI_EOWNERDEAD = 57,
    IXLAND_WASI_EPERM = 58,
    IXLAND_WASI_EPIPE = 59,
    IXLAND_WASI_EPROTO = 60,
    IXLAND_WASI_EPROTONOSUPPORT = 61,
    IXLAND_WASI_EPROTOTYPE = 62,
    IXLAND_WASI_ERANGE = 63,
    IXLAND_WASI_EROFS = 64,
    IXLAND_WASI_ESPIPE = 65,
    IXLAND_WASI_ESRCH = 66,
    IXLAND_WASI_ESTALE = 67,
    IXLAND_WASI_ETIMEDOUT = 68,
    IXLAND_WASI_ETXTBSY = 69,
    IXLAND_WASI_EXDEV = 70,
    IXLAND_WASI_ENOTCAPABLE = 71
} ixland_wasi_errno_t;

/* ============================================================================
 * WASI FILE TYPES
 * ============================================================================ */

typedef enum {
    IXLAND_WASI_FILETYPE_UNKNOWN = 0,
    IXLAND_WASI_FILETYPE_BLOCK_DEVICE = 1,
    IXLAND_WASI_FILETYPE_CHARACTER_DEVICE = 2,
    IXLAND_WASI_FILETYPE_DIRECTORY = 3,
    IXLAND_WASI_FILETYPE_REGULAR_FILE = 4,
    IXLAND_WASI_FILETYPE_SOCKET_DGRAM = 5,
    IXLAND_WASI_FILETYPE_SOCKET_STREAM = 6,
    IXLAND_WASI_FILETYPE_SYMBOLIC_LINK = 7
} ixland_wasi_filetype_t;

/* ============================================================================
 * WASI RIGHTS (capability-based)
 * ============================================================================ */

typedef uint64_t ixland_wasi_rights_t;

#define IXLAND_WASI_RIGHT_FD_DATASYNC        (1ULL << 0)
#define IXLAND_WASI_RIGHT_FD_READ            (1ULL << 1)
#define IXLAND_WASI_RIGHT_FD_SEEK            (1ULL << 2)
#define IXLAND_WASI_RIGHT_FD_FILESTAT_GET    (1ULL << 3)
#define IXLAND_WASI_RIGHT_FD_FILESTAT_SET_SIZE (1ULL << 4)
#define IXLAND_WASI_RIGHT_FD_FILESTAT_SET_TIMES (1ULL << 5)
#define IXLAND_WASI_RIGHT_FD_WRITE           (1ULL << 6)
#define IXLAND_WASI_RIGHT_FD_ADVISE          (1ULL << 7)
#define IXLAND_WASI_RIGHT_FD_ALLOCATE          (1ULL << 8)
#define IXLAND_WASI_RIGHT_PATH_CREATE_DIRECTORY (1ULL << 9)
#define IXLAND_WASI_RIGHT_PATH_CREATE_FILE   (1ULL << 10)
#define IXLAND_WASI_RIGHT_PATH_LINK_SOURCE   (1ULL << 11)
#define IXLAND_WASI_RIGHT_PATH_LINK_TARGET   (1ULL << 12)
#define IXLAND_WASI_RIGHT_PATH_OPEN          (1ULL << 13)
#define IXLAND_WASI_RIGHT_FD_READDIR         (1ULL << 14)
#define IXLAND_WASI_RIGHT_PATH_READLINK      (1ULL << 15)
#define IXLAND_WASI_RIGHT_PATH_RENAME_SOURCE (1ULL << 16)
#define IXLAND_WASI_RIGHT_PATH_RENAME_TARGET (1ULL << 17)
#define IXLAND_WASI_RIGHT_PATH_FILESTAT_GET  (1ULL << 18)
#define IXLAND_WASI_RIGHT_PATH_FILESTAT_SET_TIMES (1ULL << 19)
#define IXLAND_WASI_RIGHT_FD_SYMLINK         (1ULL << 20)
#define IXLAND_WASI_RIGHT_PATH_REMOVE_DIRECTORY (1ULL << 21)
#define IXLAND_WASI_RIGHT_PATH_UNLINK_FILE   (1ULL << 22)
#define IXLAND_WASI_RIGHT_POLL_FD_READWRITE  (1ULL << 23)
#define IXLAND_WASI_RIGHT_SOCK_SHUTDOWN      (1ULL << 24)
#define IXLAND_WASI_RIGHT_SOCK_ACCEPT        (1ULL << 25)
#define IXLAND_WASI_RIGHT_SOCK_CONNECT       (1ULL << 26)
#define IXLAND_WASI_RIGHT_SOCK_RECEIVE       (1ULL << 27)
#define IXLAND_WASI_RIGHT_SOCK_SEND          (1ULL << 28)
#define IXLAND_WASI_RIGHT_SOCK_BIND          (1ULL << 29)
#define IXLAND_WASI_RIGHT_SOCK_LISTEN        (1ULL << 30)

/* ============================================================================
 * WASI STRUCTURES
 * ============================================================================ */

/* File descriptor flags */
typedef struct {
    bool append;
    bool dsync;
    bool nonblock;
    bool rsync;
    bool sync;
} ixland_wasi_fd_flags_t;

/* Directory entry cookie */
typedef uint64_t ixland_wasi_dircookie_t;

/* INode number */
typedef uint64_t ixland_wasi_inode_t;

/* File timestamp */
typedef struct {
    uint64_t sec;
    uint32_t nsec;
} ixland_wasi_timestamp_t;

/* Filestat structure */
typedef struct {
    ixland_wasi_filetype_t filetype;
    uint64_t device;
    uint64_t inode;
    ixland_wasi_rights_t rights_base;
    ixland_wasi_rights_t rights_inheriting;
    uint64_t nlink;
    uint64_t size;
    ixland_wasi_timestamp_t atim;
    ixland_wasi_timestamp_t mtim;
    ixland_wasi_timestamp_t ctim;
} ixland_wasi_filestat_t;

/* Preopened directory description */
typedef struct {
    int fd;                        /* File descriptor */
    const char *path;              /* Guest-visible path */
    ixland_wasi_rights_t rights_base;
    ixland_wasi_rights_t rights_inheriting;
} ixland_wasi_preopen_t;

/* ============================================================================
 * WASI CONTEXT
 * ============================================================================ */

typedef struct ixland_wasi_context_s ixland_wasi_context_t;

/* Create a WASI context for a given host */
ixland_wasm_error_t ixland_wasi_context_create(
    ixland_wasm_host_t *host,
    const char *argv[],
    int argc,
    const char *envp[],
    ixland_wasi_preopen_t *preopens,
    int preopen_count,
    ixland_wasi_context_t **out_context
);

/* Destroy a WASI context */
void ixland_wasi_context_destroy(ixland_wasi_context_t *context);

/* Get WASI version */
ixland_wasi_version_t ixland_wasi_context_get_version(
    ixland_wasi_context_t *context
);

/* Get WASI error string */
const char* ixland_wasi_strerror(ixland_wasi_errno_t errno_code);

/* Convert WASI errno to system errno */
int ixland_wasi_errno_to_system(ixland_wasi_errno_t errno_code);

/* ============================================================================
 * SYSCALL MAPPING
 * ============================================================================
 *
 * WASI syscalls are mapped to ixland host service calls:
 *
 * fd_close(fd)        -> host->fd_close(fd)
 * fd_read(fd, ...)    -> host->fd_read(fd, ...)
 * fd_write(fd, ...)   -> host->fd_write(fd, ...)
 * path_open(...)      -> host->path_open(...) [then fd mapping]
 * clock_time_get(...) -> host->clock_gettime(...)
 * random_get(...)     -> host->random_get(...)
 * proc_exit(code)     -> host->proc_exit(code)
 *
 * All fd arguments are iXland virtual file descriptors.
 * Paths are resolved through iXland VFS.
 */

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_WASM_WASI_H */

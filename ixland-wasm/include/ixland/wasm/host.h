#ifndef IXLAND_WASM_HOST_H
#define IXLAND_WASM_HOST_H

/* iXland Wasm Host - Host-Service Contract
 *
 * This header defines the contract between WebAssembly guests
 * and the iXland host environment.
 */

#include "types.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* socklen_t may not be defined on all platforms */
#ifndef _SOCKLEN_T_DEFINED
typedef uint32_t socklen_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * HOST HANDLE
 * ============================================================================ */

/* Forward declaration - implementation-defined */
typedef struct ixland_wasm_host_s ixland_wasm_host_t;

/* ============================================================================
 * FILE DESCRIPTOR I/O
 * ============================================================================ */

typedef struct {
    int fd;                        /* File descriptor */
    int flags;                     /* Flags (read/write/append) */
    int64_t offset;              /* Current offset (for seekable) */
    void *userdata;              /* Host-specific data */
} ixland_wasm_fd_state_t;

/* Read from file descriptor */
typedef ssize_t (*ixland_wasm_host_fd_read_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    void *buf,
    size_t count
);

/* Write to file descriptor */
typedef ssize_t (*ixland_wasm_host_fd_write_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    const void *buf,
    size_t count
);

/* Close file descriptor */
typedef int (*ixland_wasm_host_fd_close_fn_t)(
    ixland_wasm_host_t *host,
    int fd
);

/* Seek in file descriptor */
typedef off_t (*ixland_wasm_host_fd_seek_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    off_t offset,
    int whence
);

/* Get file descriptor flags */
typedef int (*ixland_wasm_host_fd_get_flags_fn_t)(
    ixland_wasm_host_t *host,
    int fd
);

/* Set file descriptor flags */
typedef int (*ixland_wasm_host_fd_set_flags_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    int flags
);

/* ============================================================================
 * PATH AND FILESYSTEM
 * ============================================================================ */

/* Open file at path */
typedef int (*ixland_wasm_host_path_open_fn_t)(
    ixland_wasm_host_t *host,
    const char *path,
    int flags,
    mode_t mode,
    int *out_fd
);

/* Get file status */
typedef int (*ixland_wasm_host_path_stat_fn_t)(
    ixland_wasm_host_t *host,
    const char *path,
    struct stat *statbuf
);

/* Get file status via fd */
typedef int (*ixland_wasm_host_fd_stat_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    struct stat *statbuf
);

/* Create directory */
typedef int (*ixland_wasm_host_path_mkdir_fn_t)(
    ixland_wasm_host_t *host,
    const char *path,
    mode_t mode
);

/* Remove file */
typedef int (*ixland_wasm_host_path_unlink_fn_t)(
    ixland_wasm_host_t *host,
    const char *path
);

/* Remove directory */
typedef int (*ixland_wasm_host_path_rmdir_fn_t)(
    ixland_wasm_host_t *host,
    const char *path
);

/* Read symbolic link */
typedef ssize_t (*ixland_wasm_host_path_readlink_fn_t)(
    ixland_wasm_host_t *host,
    const char *path,
    char *buf,
    size_t bufsiz
);

/* Create symbolic link */
typedef int (*ixland_wasm_host_path_symlink_fn_t)(
    ixland_wasm_host_t *host,
    const char *target,
    const char *linkpath
);

/* Rename file/directory */
typedef int (*ixland_wasm_host_path_rename_fn_t)(
    ixland_wasm_host_t *host,
    const char *oldpath,
    const char *newpath
);

/* Check access permissions */
typedef int (*ixland_wasm_host_path_access_fn_t)(
    ixland_wasm_host_t *host,
    const char *path,
    int mode
);

/* ============================================================================
 * CLOCKS AND TIME
 * ============================================================================ */

/* Clock ID enumeration */
typedef enum {
    IXLAND_WASM_CLOCK_REALTIME = 0,
    IXLAND_WASM_CLOCK_MONOTONIC = 1,
    IXLAND_WASM_CLOCK_PROCESS_CPUTIME_ID = 2,
    IXLAND_WASM_CLOCK_THREAD_CPUTIME_ID = 3
} ixland_wasm_clock_id_t;

/* Get clock time */
typedef int (*ixland_wasm_host_clock_gettime_fn_t)(
    ixland_wasm_host_t *host,
    ixland_wasm_clock_id_t clock_id,
    uint64_t *out_sec,
    uint64_t *out_nsec
);

/* Get clock resolution */
typedef int (*ixland_wasm_host_clock_getres_fn_t)(
    ixland_wasm_host_t *host,
    ixland_wasm_clock_id_t clock_id,
    uint64_t *out_sec,
    uint64_t *out_nsec
);

/* Sleep (may be interrupted) */
typedef int (*ixland_wasm_host_nanosleep_fn_t)(
    ixland_wasm_host_t *host,
    uint64_t sec,
    uint64_t nsec,
    uint64_t *out_rem_sec,
    uint64_t *out_rem_nsec
);

/* ============================================================================
 * RANDOM NUMBER GENERATION
 * ============================================================================ */

/* Get cryptographically secure random bytes */
typedef int (*ixland_wasm_host_random_get_fn_t)(
    ixland_wasm_host_t *host,
    void *buf,
    size_t buflen
);

/* ============================================================================
 * SOCKETS AND NETWORKING
 * ============================================================================ */

/* Create socket */
typedef int (*ixland_wasm_host_socket_create_fn_t)(
    ixland_wasm_host_t *host,
    int domain,
    int type,
    int protocol,
    int *out_fd
);

/* Connect socket */
typedef int (*ixland_wasm_host_socket_connect_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    const struct sockaddr *addr,
    socklen_t addrlen
);

/* Bind socket */
typedef int (*ixland_wasm_host_socket_bind_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    const struct sockaddr *addr,
    socklen_t addrlen
);

/* Listen for connections */
typedef int (*ixland_wasm_host_socket_listen_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    int backlog
);

/* Accept connection */
typedef int (*ixland_wasm_host_socket_accept_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    struct sockaddr *addr,
    socklen_t *addrlen,
    int *out_fd
);

/* Send data */
typedef ssize_t (*ixland_wasm_host_socket_send_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    const void *buf,
    size_t len,
    int flags
);

/* Receive data */
typedef ssize_t (*ixland_wasm_host_socket_recv_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    void *buf,
    size_t len,
    int flags
);

/* Send to specific address */
typedef ssize_t (*ixland_wasm_host_socket_sendto_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    const void *buf,
    size_t len,
    int flags,
    const struct sockaddr *dest_addr,
    socklen_t addrlen
);

/* Receive from address */
typedef ssize_t (*ixland_wasm_host_socket_recvfrom_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    void *buf,
    size_t len,
    int flags,
    struct sockaddr *src_addr,
    socklen_t *addrlen
);

/* Shutdown socket */
typedef int (*ixland_wasm_host_socket_shutdown_fn_t)(
    ixland_wasm_host_t *host,
    int fd,
    int how
);

/* ============================================================================
 * PROCESS SEMANTICS
 * ============================================================================ */

/* Exit the process */
typedef void (*ixland_wasm_host_proc_exit_fn_t)(
    ixland_wasm_host_t *host,
    int status
);

/* Check if interrupted */
typedef bool (*ixland_wasm_host_proc_is_interrupted_fn_t)(
    ixland_wasm_host_t *host
);

/* Get process ID (virtual) */
typedef int (*ixland_wasm_host_proc_getpid_fn_t)(
    ixland_wasm_host_t *host
);

/* Get parent process ID (virtual) */
typedef int (*ixland_wasm_host_proc_getppid_fn_t)(
    ixland_wasm_host_t *host
);

/* ============================================================================
 * HOST VTABLE
 * ============================================================================ */

typedef struct {
    /* File descriptor I/O */
    ixland_wasm_host_fd_read_fn_t fd_read;
    ixland_wasm_host_fd_write_fn_t fd_write;
    ixland_wasm_host_fd_close_fn_t fd_close;
    ixland_wasm_host_fd_seek_fn_t fd_seek;
    ixland_wasm_host_fd_get_flags_fn_t fd_get_flags;
    ixland_wasm_host_fd_set_flags_fn_t fd_set_flags;

    /* Path and filesystem */
    ixland_wasm_host_path_open_fn_t path_open;
    ixland_wasm_host_path_stat_fn_t path_stat;
    ixland_wasm_host_fd_stat_fn_t fd_stat;
    ixland_wasm_host_path_mkdir_fn_t path_mkdir;
    ixland_wasm_host_path_unlink_fn_t path_unlink;
    ixland_wasm_host_path_rmdir_fn_t path_rmdir;
    ixland_wasm_host_path_readlink_fn_t path_readlink;
    ixland_wasm_host_path_symlink_fn_t path_symlink;
    ixland_wasm_host_path_rename_fn_t path_rename;
    ixland_wasm_host_path_access_fn_t path_access;

    /* Clocks */
    ixland_wasm_host_clock_gettime_fn_t clock_gettime;
    ixland_wasm_host_clock_getres_fn_t clock_getres;
    ixland_wasm_host_nanosleep_fn_t nanosleep;

    /* Random */
    ixland_wasm_host_random_get_fn_t random_get;

    /* Sockets */
    ixland_wasm_host_socket_create_fn_t socket_create;
    ixland_wasm_host_socket_connect_fn_t socket_connect;
    ixland_wasm_host_socket_bind_fn_t socket_bind;
    ixland_wasm_host_socket_listen_fn_t socket_listen;
    ixland_wasm_host_socket_accept_fn_t socket_accept;
    ixland_wasm_host_socket_send_fn_t socket_send;
    ixland_wasm_host_socket_recv_fn_t socket_recv;
    ixland_wasm_host_socket_sendto_fn_t socket_sendto;
    ixland_wasm_host_socket_recvfrom_fn_t socket_recvfrom;
    ixland_wasm_host_socket_shutdown_fn_t socket_shutdown;

    /* Process */
    ixland_wasm_host_proc_exit_fn_t proc_exit;
    ixland_wasm_host_proc_is_interrupted_fn_t proc_is_interrupted;
    ixland_wasm_host_proc_getpid_fn_t proc_getpid;
    ixland_wasm_host_proc_getppid_fn_t proc_getppid;
} ixland_wasm_host_vtable_t;

/* ============================================================================
 * HOST CREATION
 * ============================================================================ */

/* Create a host environment with the given vtable */
ixland_wasm_error_t ixland_wasm_host_create(
    const ixland_wasm_host_vtable_t *vtable,
    void *userdata,
    ixland_wasm_host_t **out_host
);

/* Destroy a host environment */
void ixland_wasm_host_destroy(ixland_wasm_host_t *host);

/* Get host userdata */
void* ixland_wasm_host_get_userdata(ixland_wasm_host_t *host);

/* Get host vtable */
const ixland_wasm_host_vtable_t* ixland_wasm_host_get_vtable(
    ixland_wasm_host_t *host
);

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_WASM_HOST_H */

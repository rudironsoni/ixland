/* iXland libc - Linux-compatible fcntl.h
 *
 * File control operations matching Linux syscall signatures.
 * These are iXland-compatible wrappers with ixland_ prefix.
 */

#ifndef IXLAND_LINUX_FCNTL_H
#define IXLAND_LINUX_FCNTL_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FILE OPEN FLAGS
 * ============================================================================ */

/* Access modes */
#define IXLAND_O_RDONLY 00000000  /* Read-only */
#define IXLAND_O_WRONLY 00000001  /* Write-only */
#define IXLAND_O_RDWR 00000002    /* Read-write */
#define IXLAND_O_ACCMODE 00000003 /* Mask for access modes */

/* Creation and file status flags (using octal for compatibility with Linux) */
#define IXLAND_O_CREAT 00000100  /* Create file if it doesn't exist */
#define IXLAND_O_EXCL 00000200   /* Fail if file already exists */
#define IXLAND_O_NOCTTY 00000400 /* Don't assign controlling terminal */
#define IXLAND_O_TRUNC 00001000  /* Truncate file to zero length */

/* File status flags */
#define IXLAND_O_APPEND 00002000    /* Append mode */
#define IXLAND_O_NONBLOCK 00004000  /* Non-blocking I/O */
#define IXLAND_O_DSYNC 00010000     /* Synchronize data */
#define IXLAND_O_FASYNC 00020000    /* Signal-driven I/O */
#define IXLAND_O_DIRECT 00040000    /* Direct I/O */
#define IXLAND_O_LARGEFILE 00100000 /* Allow large files */

/* iXland-specific file open flags (hex values to avoid conflicts) */
#ifndef IXLAND_O_CLOEXEC_DEFINED
#define IXLAND_O_CLOEXEC_DEFINED
#define IXLAND_O_CLOEXEC 0x80000   /* Close on exec */
#define IXLAND_O_DIRECTORY 0x20000 /* Must be directory */
#define IXLAND_O_NOFOLLOW 0x40000  /* Don't follow symlinks */
#define IXLAND_O_PATH 0x2000000    /* Path only, no I/O */
#define IXLAND_O_TMPFILE 0x404000  /* Create unnamed temp file */
#endif

#define IXLAND_O_NOATIME 01000000 /* Don't update access time */
#define IXLAND_O_SYNC 04010000    /* Synchronize I/O and data */

/* ============================================================================
 * FCNTL COMMANDS
 * ============================================================================ */

/* File descriptor manipulation */
#define IXLAND_F_DUPFD 0  /* Duplicate file descriptor */
#define IXLAND_F_GETFD 1  /* Get file descriptor flags */
#define IXLAND_F_SETFD 2  /* Set file descriptor flags */
#define IXLAND_F_GETFL 3  /* Get file status flags */
#define IXLAND_F_SETFL 4  /* Set file status flags */
#define IXLAND_F_GETLK 5  /* Get record locking info */
#define IXLAND_F_SETLK 6  /* Set record locking info (non-blocking) */
#define IXLAND_F_SETLKW 7 /* Set record locking info (blocking) */

/* File ownership */
#define IXLAND_F_GETOWN 9 /* Get owner (for SIGIO) */
#define IXLAND_F_SETOWN 8 /* Set owner (for SIGIO) */

/* File descriptor operations (Linux-specific) */
#define IXLAND_F_DUPFD_CLOEXEC 1030 /* Duplicate FD with close-on-exec */
#define IXLAND_F_SETSIG 10          /* Set signal number for SIGIO */
#define IXLAND_F_GETSIG 11          /* Get signal number for SIGIO */

/* File seal operations */
#define IXLAND_F_ADD_SEALS 1033 /* Add seals to file */
#define IXLAND_F_GET_SEALS 1034 /* Get seals from file */

/* File read/write hints */
#define IXLAND_F_GET_RW_HINT 1035      /* Get read/write hint */
#define IXLAND_F_SET_RW_HINT 1036      /* Set read/write hint */
#define IXLAND_F_GET_FILE_RW_HINT 1037 /* Get per-file read/write hint */
#define IXLAND_F_SET_FILE_RW_HINT 1038 /* Set per-file read/write hint */

/* ============================================================================
 * FILE DESCRIPTOR FLAGS
 * ============================================================================ */

#define IXLAND_FD_CLOEXEC 1 /* Close file descriptor on exec */

/* ============================================================================
 * ADVISORY LOCK TYPES
 * ============================================================================ */

#define IXLAND_F_RDLCK 0 /* Read (shared) lock */
#define IXLAND_F_WRLCK 1 /* Write (exclusive) lock */
#define IXLAND_F_UNLCK 2 /* Remove lock */

/* ============================================================================
 * FILE SEALS (for memfd_create)
 * ============================================================================ */

#define IXLAND_F_SEAL_SEAL 0x0001   /* Prevent further sealing */
#define IXLAND_F_SEAL_SHRINK 0x0002 /* Prevent file from shrinking */
#define IXLAND_F_SEAL_GROW 0x0004   /* Prevent file from growing */
#define IXLAND_F_SEAL_WRITE 0x0008  /* Prevent writes */

/* ============================================================================
 * READ/WRITE HINTS
 * ============================================================================ */

#define IXLAND_RWH_WRITE_LIFE_NONE 1    /* No write lifetime hint */
#define IXLAND_RWH_WRITE_LIFE_SHORT 2   /* Data has short write lifetime */
#define IXLAND_RWH_WRITE_LIFE_MEDIUM 3  /* Data has medium write lifetime */
#define IXLAND_RWH_WRITE_LIFE_LONG 4    /* Data has long write lifetime */
#define IXLAND_RWH_WRITE_LIFE_EXTREME 5 /* Data has extreme write lifetime */

/* ============================================================================
 * POSIX ADVISORY LOCK STRUCTURE
 * ============================================================================ */

/**
 * @brief File lock structure
 *
 * Used for advisory locking via fcntl().
 */
struct ixland_flock {
    int16_t l_type;   /* Lock type (F_RDLCK, F_WRLCK, F_UNLCK) */
    int16_t l_whence; /* SEEK_SET, SEEK_CUR, or SEEK_END */
    int64_t l_start;  /* Starting offset */
    int64_t l_len;    /* Number of bytes to lock (0 = to EOF) */
    pid_t l_pid;      /* PID of owner */
};

/* ============================================================================
 * MODE BITS FOR OPEN
 * ============================================================================ */

/* User permissions */
#define IXLAND_S_IRWXU 00700 /* User: read, write, execute */
#define IXLAND_S_IRUSR 00400 /* User: read */
#define IXLAND_S_IWUSR 00200 /* User: write */
#define IXLAND_S_IXUSR 00100 /* User: execute */

/* Group permissions */
#define IXLAND_S_IRWXG 00070 /* Group: read, write, execute */
#define IXLAND_S_IRGRP 00040 /* Group: read */
#define IXLAND_S_IWGRP 00020 /* Group: write */
#define IXLAND_S_IXGRP 00010 /* Group: execute */

/* Other permissions */
#define IXLAND_S_IRWXO 00007 /* Others: read, write, execute */
#define IXLAND_S_IROTH 00004 /* Others: read */
#define IXLAND_S_IWOTH 00002 /* Others: write */
#define IXLAND_S_IXOTH 00001 /* Others: execute */

/* Special bits */
#define IXLAND_S_ISUID 04000 /* Set-user-ID */
#define IXLAND_S_ISGID 02000 /* Set-group-ID */
#define IXLAND_S_ISVTX 01000 /* Sticky bit */

/* ============================================================================
 * AT_* FLAGS
 * ============================================================================ */

#define IXLAND_AT_FDCWD -100             /* Use current directory */
#define IXLAND_AT_SYMLINK_NOFOLLOW 0x100 /* Do not follow symlinks */
#define IXLAND_AT_EACCESS 0x200          /* Use effective IDs for access */
#define IXLAND_AT_REMOVEDIR 0x200        /* Remove directory instead of file */
#define IXLAND_AT_SYMLINK_FOLLOW 0x400   /* Follow symlinks (for linkat) */
#define IXLAND_AT_NO_AUTOMOUNT 0x800     /* Do not automount */
#define IXLAND_AT_EMPTY_PATH 0x1000      /* Allow empty pathname */

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

/**
 * @brief File control operations
 *
 * Performs various operations on an open file descriptor.
 *
 * @param fd File descriptor
 * @param cmd Command (F_DUPFD, F_GETFL, F_SETFL, etc.)
 * @param ... Arguments depend on cmd
 * @return int Varies by command, -1 on error
 */
int ixland_fcntl(int fd, int cmd, ...);

/**
 * @brief Open a file
 *
 * @param pathname Path to file
 * @param flags Open flags
 * @param mode File mode (when O_CREAT is used)
 * @return int File descriptor on success, -1 on error
 */
int ixland_open(const char *pathname, int flags, ...);

/**
 * @brief Open a file relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param flags Open flags
 * @param mode File mode (when O_CREAT is used)
 * @return int File descriptor on success, -1 on error
 */
int ixland_openat(int dirfd, const char *pathname, int flags, ...);

/**
 * @brief Open file with extended options
 *
 * Extended open interface with additional options.
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param flags Open flags
 * @param mode File mode
 * @param resolve Resolution flags
 * @return int File descriptor on success, -1 on error
 */
int ixland_openat2(int dirfd, const char *pathname, int flags, mode_t mode, uint64_t resolve);

/**
 * @brief Create a file
 *
 * Equivalent to open(path, O_CREAT|O_WRONLY|O_TRUNC, mode).
 *
 * @param pathname Path to file
 * @param mode File mode
 * @return int File descriptor on success, -1 on error
 */
int ixland_creat(const char *pathname, mode_t mode);

/**
 * @brief Create a file (64-bit)
 *
 * @param pathname Path to file
 * @param mode File mode
 * @return int File descriptor on success, -1 on error
 */
int ixland_creat64(const char *pathname, mode_t mode);

/**
 * @brief Create a directory
 *
 * @param pathname Path to directory
 * @param mode Directory permissions
 * @return int 0 on success, -1 on error
 */
int ixland_mkdir(const char *pathname, mode_t mode);

/**
 * @brief Create a directory relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to directory
 * @param mode Directory permissions
 * @return int 0 on success, -1 on error
 */
int ixland_mkdirat(int dirfd, const char *pathname, mode_t mode);

/**
 * @brief Create a special or ordinary file
 *
 * @param pathname Path to file
 * @param mode File mode and type
 * @param dev Device number (for special files)
 * @return int 0 on success, -1 on error
 */
int ixland_mknod(const char *pathname, mode_t mode, dev_t dev);

/**
 * @brief Create a special or ordinary file relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param mode File mode and type
 * @param dev Device number
 * @return int 0 on success, -1 on error
 */
int ixland_mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev);

/**
 * @brief Rename a file
 *
 * @param oldpath Old pathname
 * @param newpath New pathname
 * @return int 0 on success, -1 on error
 */
int ixland_rename(const char *oldpath, const char *newpath);

/**
 * @brief Rename a file relative to directories
 *
 * @param olddirfd Old directory file descriptor
 * @param oldpath Old pathname
 * @param newdirfd New directory file descriptor
 * @param newpath New pathname
 * @param flags Flags (RENAME_EXCHANGE, RENAME_NOREPLACE, etc.)
 * @return int 0 on success, -1 on error
 */
int ixland_renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);

/**
 * @brief Rename a file relative to directories with flags
 *
 * @param olddirfd Old directory file descriptor
 * @param oldpath Old pathname
 * @param newdirfd New directory file descriptor
 * @param newpath New pathname
 * @param flags Flags
 * @return int 0 on success, -1 on error
 */
int ixland_renameat2(int olddirfd, const char *oldpath, int newdirfd, const char *newpath,
                     unsigned int flags);

/* Rename flags */
#define IXLAND_RENAME_EXCHANGE 0x1  /* Exchange source and target */
#define IXLAND_RENAME_NOREPLACE 0x2 /* Don't replace target if exists */
#define IXLAND_RENAME_WHITEOUT 0x4  /* Create whiteout for overlayfs */

/**
 * @brief Create a symbolic link
 *
 * @param target Target path
 * @param linkpath Link path
 * @return int 0 on success, -1 on error
 */
int ixland_symlink(const char *target, const char *linkpath);

/**
 * @brief Create a symbolic link relative to directory
 *
 * @param target Target path
 * @param newdirfd Directory file descriptor
 * @param linkpath Link path
 * @return int 0 on success, -1 on error
 */
int ixland_symlinkat(const char *target, int newdirfd, const char *linkpath);

/**
 * @brief Read a symbolic link
 *
 * @param pathname Path to symlink
 * @param buf Buffer to store target
 * @param bufsiz Buffer size
 * @return ssize_t Bytes read, -1 on error
 */
ssize_t ixland_readlink(const char *pathname, char *buf, size_t bufsiz);

/**
 * @brief Read a symbolic link relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to symlink
 * @param buf Buffer to store target
 * @param bufsiz Buffer size
 * @return ssize_t Bytes read, -1 on error
 */
ssize_t ixland_readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);

/**
 * @brief Check file accessibility
 *
 * @param pathname Path to file
 * @param mode Access mode (F_OK, R_OK, W_OK, X_OK)
 * @return int 0 if accessible, -1 on error
 */
int ixland_access(const char *pathname, int mode);

/**
 * @brief Check file accessibility relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param mode Access mode
 * @param flags Flags (AT_EACCESS, AT_SYMLINK_NOFOLLOW)
 * @return int 0 if accessible, -1 on error
 */
int ixland_faccessat(int dirfd, const char *pathname, int mode, int flags);

/**
 * @brief Synchronize file data to disk
 *
 * @param fd File descriptor
 * @return int 0 on success, -1 on error
 */
int ixland_fsync(int fd);

/**
 * @brief Synchronize file data (metadata may be delayed)
 *
 * @param fd File descriptor
 * @return int 0 on success, -1 on error
 */
int ixland_fdatasync(int fd);

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_LINUX_FCNTL_H */

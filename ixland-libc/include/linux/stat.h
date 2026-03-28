/* iXland libc - Linux-compatible stat.h
 *
 * File status operations and struct stat definitions.
 * These match Linux syscall signatures with iox_ prefix.
 */

#ifndef IOX_LINUX_STAT_H
#define IOX_LINUX_STAT_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * STAT STRUCTURES
 * ============================================================================ */

/**
 * @brief File status structure
 *
 * Matches Linux x86_64 struct stat layout.
 */
struct iox_stat {
    uint64_t st_dev;    /* Device ID of containing device */
    uint64_t st_ino;    /* Inode number */
    uint64_t st_nlink;  /* Number of hard links */
    uint32_t st_mode;   /* File mode (permissions and type) */
    uint32_t st_uid;    /* User ID of owner */
    uint32_t st_gid;    /* Group ID of owner */
    uint32_t __pad0;    /* Padding */
    uint64_t st_rdev;   /* Device ID (if special file) */
    int64_t st_size;    /* File size in bytes */
    int64_t st_blksize; /* Block size for I/O */
    int64_t st_blocks;  /* Number of 512B blocks allocated */

    /* Timestamps */
    struct timespec st_atim; /* Last access time */
    struct timespec st_mtim; /* Last modification time */
    struct timespec st_ctim; /* Last status change time */

    /* Padding for future extensions */
    int64_t __pad1[3];
};

/**
 * @brief File status structure (64-bit)
 *
 * For large file support.
 */
struct iox_stat64 {
    uint64_t st_dev;   /* Device ID */
    uint64_t st_ino;   /* Inode number */
    uint64_t st_nlink; /* Number of hard links */
    uint32_t st_mode;  /* File mode */
    uint32_t st_uid;   /* User ID */
    uint32_t st_gid;   /* Group ID */
    uint32_t __pad0;
    uint64_t st_rdev;   /* Device ID (if special file) */
    int64_t st_size;    /* File size */
    int64_t st_blksize; /* Block size */
    int64_t st_blocks;  /* Number of blocks */

    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;

    int64_t __pad1[3];
};

/* Convenience macros for timestamp access */
#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec

/* ============================================================================
 * FILE TYPE MACROS
 * ============================================================================ */

/* File type bits in st_mode */
#define IOX_S_IFMT 0170000   /* Bit mask for file type */
#define IOX_S_IFDIR 0040000  /* Directory */
#define IOX_S_IFCHR 0020000  /* Character device */
#define IOX_S_IFBLK 0060000  /* Block device */
#define IOX_S_IFREG 0100000  /* Regular file */
#define IOX_S_IFIFO 0010000  /* FIFO/named pipe */
#define IOX_S_IFLNK 0120000  /* Symbolic link */
#define IOX_S_IFSOCK 0140000 /* Socket */

/* File type test macros */
#define IOX_S_ISDIR(m) (((m) & IOX_S_IFMT) == IOX_S_IFDIR)
#define IOX_S_ISCHR(m) (((m) & IOX_S_IFMT) == IOX_S_IFCHR)
#define IOX_S_ISBLK(m) (((m) & IOX_S_IFMT) == IOX_S_IFBLK)
#define IOX_S_ISREG(m) (((m) & IOX_S_IFMT) == IOX_S_IFREG)
#define IOX_S_ISFIFO(m) (((m) & IOX_S_IFMT) == IOX_S_IFIFO)
#define IOX_S_ISLNK(m) (((m) & IOX_S_IFMT) == IOX_S_IFLNK)
#define IOX_S_ISSOCK(m) (((m) & IOX_S_IFMT) == IOX_S_IFSOCK)

/* ============================================================================
 * PERMISSION BITS
 * ============================================================================ */

/* Owner permissions */
#define IOX_S_IRWXU 00700 /* Owner: read, write, execute */
#define IOX_S_IRUSR 00400 /* Owner: read */
#define IOX_S_IWUSR 00200 /* Owner: write */
#define IOX_S_IXUSR 00100 /* Owner: execute */

/* Group permissions */
#define IOX_S_IRWXG 00070 /* Group: read, write, execute */
#define IOX_S_IRGRP 00040 /* Group: read */
#define IOX_S_IWGRP 00020 /* Group: write */
#define IOX_S_IXGRP 00010 /* Group: execute */

/* Other permissions */
#define IOX_S_IRWXO 00007 /* Others: read, write, execute */
#define IOX_S_IROTH 00004 /* Others: read */
#define IOX_S_IWOTH 00002 /* Others: write */
#define IOX_S_IXOTH 00001 /* Others: execute */

/* Special bits */
#define IOX_S_ISUID 04000 /* Set-user-ID */
#define IOX_S_ISGID 02000 /* Set-group-ID */
#define IOX_S_ISVTX 01000 /* Sticky bit (save swapped text) */

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

/**
 * @brief Get file status
 *
 * Retrieves status information about the named file.
 * Follows symbolic links.
 *
 * @param pathname Path to file
 * @param statbuf Pointer to stat structure
 * @return int 0 on success, -1 on error
 */
int iox_stat(const char *pathname, struct iox_stat *statbuf);

/**
 * @brief Get file status via file descriptor
 *
 * @param fd File descriptor
 * @param statbuf Pointer to stat structure
 * @return int 0 on success, -1 on error
 */
int iox_fstat(int fd, struct iox_stat *statbuf);

/**
 * @brief Get file status without following symlinks
 *
 * @param pathname Path to file
 * @param statbuf Pointer to stat structure
 * @return int 0 on success, -1 on error
 */
int iox_lstat(const char *pathname, struct iox_stat *statbuf);

/**
 * @brief Get file status relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param statbuf Pointer to stat structure
 * @param flags Flags (AT_EMPTY_PATH, AT_NO_AUTOMOUNT, AT_SYMLINK_NOFOLLOW)
 * @return int 0 on success, -1 on error
 */
int iox_fstatat(int dirfd, const char *pathname, struct iox_stat *statbuf, int flags);

/**
 * @brief Get file status (extended)
 *
 * Extended stat interface with more options.
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param flags Flags
 * @param mask Requested attributes mask
 * @param statxbuf Pointer to statx structure
 * @return int 0 on success, -1 on error
 */
int iox_statx(int dirfd, const char *pathname, int flags, unsigned int mask,
              struct iox_stat *statxbuf);

/**
 * @brief Change file mode
 *
 * @param pathname Path to file
 * @param mode New file mode
 * @return int 0 on success, -1 on error
 */
int iox_chmod(const char *pathname, mode_t mode);

/**
 * @brief Change file mode via file descriptor
 *
 * @param fd File descriptor
 * @param mode New file mode
 * @return int 0 on success, -1 on error
 */
int iox_fchmod(int fd, mode_t mode);

/**
 * @brief Change file mode relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param mode New file mode
 * @param flags Flags
 * @return int 0 on success, -1 on error
 */
int iox_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);

/**
 * @brief Change file owner
 *
 * @param pathname Path to file
 * @param owner New owner user ID
 * @param group New owner group ID
 * @return int 0 on success, -1 on error
 */
int iox_chown(const char *pathname, uid_t owner, gid_t group);

/**
 * @brief Change file owner via file descriptor
 *
 * @param fd File descriptor
 * @param owner New owner user ID
 * @param group New owner group ID
 * @return int 0 on success, -1 on error
 */
int iox_fchown(int fd, uid_t owner, gid_t group);

/**
 * @brief Change file owner without following symlinks
 *
 * @param pathname Path to file
 * @param owner New owner user ID
 * @param group New owner group ID
 * @return int 0 on success, -1 on error
 */
int iox_lchown(const char *pathname, uid_t owner, gid_t group);

/**
 * @brief Change file owner relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param owner New owner user ID
 * @param group New owner group ID
 * @param flags Flags
 * @return int 0 on success, -1 on error
 */
int iox_fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);

/**
 * @brief Truncate file
 *
 * @param path Path to file
 * @param length New file length
 * @return int 0 on success, -1 on error
 */
int iox_truncate(const char *path, off_t length);

/**
 * @brief Truncate file via file descriptor
 *
 * @param fd File descriptor
 * @param length New file length
 * @return int 0 on success, -1 on error
 */
int iox_ftruncate(int fd, off_t length);

/**
 * @brief Check access permissions
 *
 * @param pathname Path to file
 * @param mode Access mode (F_OK, R_OK, W_OK, X_OK)
 * @return int 0 if allowed, -1 on error or denied
 */
int iox_access(const char *pathname, int mode);

/**
 * @brief Check access permissions relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param mode Access mode
 * @param flags Flags (AT_EACCESS)
 * @return int 0 if allowed, -1 on error or denied
 */
int iox_faccessat(int dirfd, const char *pathname, int mode, int flags);

/* ============================================================================
 * ACCESS MODE CONSTANTS
 * ============================================================================ */

/* Access modes for access() and faccessat() */
#define IOX_F_OK 0 /* Test for existence */
#define IOX_R_OK 4 /* Test for read permission */
#define IOX_W_OK 2 /* Test for write permission */
#define IOX_X_OK 1 /* Test for execute permission */

/* Flags for faccessat() */
#define IOX_AT_EACCESS 0x200          /* Use effective IDs */
#define IOX_AT_SYMLINK_NOFOLLOW 0x100 /* Do not follow symbolic links */
#define IOX_AT_EMPTY_PATH 0x1000      /* Allow empty pathname */
#define IOX_AT_NO_AUTOMOUNT 0x800     /* Do not automount */

#ifdef __cplusplus
}
#endif

#endif /* IOX_LINUX_STAT_H */

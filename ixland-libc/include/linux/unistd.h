/* iXland libc - Linux-compatible unistd.h
 *
 * Process and I/O syscall declarations matching Linux signatures.
 * These are iXland-compatible wrappers with iox_ prefix.
 */

#ifndef IOX_LINUX_UNISTD_H
#define IOX_LINUX_UNISTD_H

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * PROCESS MANAGEMENT
 * ============================================================================ */

/**
 * @brief Create a new process (fork)
 *
 * Creates a new process by duplicating the calling process.
 * Returns 0 to child, child's PID to parent.
 *
 * @return pid_t Child PID in parent, 0 in child, -1 on error
 */
pid_t iox_fork(void);

/**
 * @brief Create a new process (vfork)
 *
 * Similar to fork but shares address space until execve.
 * On iOS, this behaves like fork due to thread-based simulation.
 *
 * @return pid_t Child PID in parent, 0 in child, -1 on error
 */
pid_t iox_vfork(void);

/**
 * @brief Execute a program
 *
 * Replaces current process image with new program.
 *
 * @param pathname Path to executable
 * @param argv Argument vector
 * @param envp Environment vector
 * @return int -1 on error, does not return on success
 */
int iox_execve(const char *pathname, char *const argv[], char *const envp[]);

/**
 * @brief Execute a program (environment inherited)
 *
 * @param pathname Path to executable
 * @param argv Argument vector
 * @return int -1 on error, does not return on success
 */
int iox_execv(const char *pathname, char *const argv[]);

/**
 * @brief Terminate process with status
 *
 * @param status Exit status code
 */
void iox_exit(int status) __attribute__((noreturn));

/**
 * @brief Terminate process immediately
 *
 * Does not call atexit handlers or flush streams.
 *
 * @param status Exit status code
 */
void iox__exit(int status) __attribute__((noreturn));

/**
 * @brief Get process ID
 *
 * @return pid_t Current process ID
 */
pid_t iox_getpid(void);

/**
 * @brief Get parent process ID
 *
 * @return pid_t Parent process ID
 */
pid_t iox_getppid(void);

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

/**
 * @brief Get session ID
 *
 * @param pid Process ID (0 for current)
 * @return pid_t Session ID, -1 on error
 */
pid_t iox_getsid(pid_t pid);

/**
 * @brief Create new session
 *
 * Creates a new session and sets calling process as session leader.
 *
 * @return pid_t Session ID on success, -1 on error
 */
pid_t iox_setsid(void);

/**
 * @brief Wait for any child process
 *
 * @param stat_loc Pointer to store exit status (can be NULL)
 * @return pid_t Child PID on success, -1 on error
 */
pid_t iox_wait(int *stat_loc);

/**
 * @brief Wait for specific child process
 *
 * @param pid Process ID (-1 for any, >0 for specific)
 * @param stat_loc Pointer to store exit status
 * @param options Options (WNOHANG, WUNTRACED, etc.)
 * @return pid_t Child PID on success, 0 with WNOHANG, -1 on error
 */
pid_t iox_waitpid(pid_t pid, int *stat_loc, int options);

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */

/**
 * @brief Open a file
 *
 * @param pathname Path to file
 * @param flags Open flags (O_RDONLY, O_WRONLY, O_CREAT, etc.)
 * @param mode File mode for creation (optional)
 * @return int File descriptor on success, -1 on error
 */
int iox_open(const char *pathname, int flags, ...);

/**
 * @brief Open file relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param flags Open flags
 * @param mode File mode (optional)
 * @return int File descriptor on success, -1 on error
 */
int iox_openat(int dirfd, const char *pathname, int flags, ...);

/**
 * @brief Create a file
 *
 * @param pathname Path to file
 * @param mode File mode
 * @return int File descriptor on success, -1 on error
 */
int iox_creat(const char *pathname, mode_t mode);

/**
 * @brief Read from file descriptor
 *
 * @param fd File descriptor
 * @param buf Buffer to read into
 * @param count Number of bytes to read
 * @return ssize_t Bytes read, 0 at EOF, -1 on error
 */
ssize_t iox_read(int fd, void *buf, size_t count);

/**
 * @brief Write to file descriptor
 *
 * @param fd File descriptor
 * @param buf Buffer to write from
 * @param count Number of bytes to write
 * @return ssize_t Bytes written, -1 on error
 */
ssize_t iox_write(int fd, const void *buf, size_t count);

/**
 * @brief Close file descriptor
 *
 * @param fd File descriptor
 * @return int 0 on success, -1 on error
 */
int iox_close(int fd);

/**
 * @brief Change file offset
 *
 * @param fd File descriptor
 * @param offset New offset
 * @param whence SEEK_SET, SEEK_CUR, or SEEK_END
 * @return off_t New file offset, -1 on error
 */
off_t iox_lseek(int fd, off_t offset, int whence);

/**
 * @brief Read from file descriptor at offset
 *
 * @param fd File descriptor
 * @param buf Buffer to read into
 * @param count Number of bytes to read
 * @param offset Offset in file
 * @return ssize_t Bytes read, -1 on error
 */
ssize_t iox_pread(int fd, void *buf, size_t count, off_t offset);

/**
 * @brief Write to file descriptor at offset
 *
 * @param fd File descriptor
 * @param buf Buffer to write from
 * @param count Number of bytes to write
 * @param offset Offset in file
 * @return ssize_t Bytes written, -1 on error
 */
ssize_t iox_pwrite(int fd, const void *buf, size_t count, off_t offset);

/**
 * @brief Duplicate file descriptor
 *
 * @param oldfd File descriptor to duplicate
 * @return int New file descriptor, -1 on error
 */
int iox_dup(int oldfd);

/**
 * @brief Duplicate file descriptor to specific number
 *
 * @param oldfd File descriptor to duplicate
 * @param newfd Target file descriptor
 * @return int newfd on success, -1 on error
 */
int iox_dup2(int oldfd, int newfd);

/**
 * @brief Duplicate file descriptor to specific number (with flags)
 *
 * @param oldfd File descriptor to duplicate
 * @param newfd Target file descriptor
 * @param flags Flags (O_CLOEXEC)
 * @return int newfd on success, -1 on error
 */
int iox_dup3(int oldfd, int newfd, int flags);

/* ============================================================================
 * FILESYSTEM
 * ============================================================================ */

/**
 * @brief Change working directory
 *
 * @param path New working directory
 * @return int 0 on success, -1 on error
 */
int iox_chdir(const char *path);

/**
 * @brief Change working directory via file descriptor
 *
 * @param fd File descriptor of directory
 * @return int 0 on success, -1 on error
 */
int iox_fchdir(int fd);

/**
 * @brief Get current working directory
 *
 * @param buf Buffer to store path
 * @param size Buffer size
 * @return char* Pointer to buf on success, NULL on error
 */
char *iox_getcwd(char *buf, size_t size);

/**
 * @brief Create directory
 *
 * @param pathname Path to directory
 * @param mode Directory permissions
 * @return int 0 on success, -1 on error
 */
int iox_mkdir(const char *pathname, mode_t mode);

/**
 * @brief Create directory relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to directory
 * @param mode Directory permissions
 * @return int 0 on success, -1 on error
 */
int iox_mkdirat(int dirfd, const char *pathname, mode_t mode);

/**
 * @brief Remove directory
 *
 * @param pathname Path to directory
 * @return int 0 on success, -1 on error
 */
int iox_rmdir(const char *pathname);

/**
 * @brief Remove file
 *
 * @param pathname Path to file
 * @return int 0 on success, -1 on error
 */
int iox_unlink(const char *pathname);

/**
 * @brief Remove file relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to file
 * @param flags Flags
 * @return int 0 on success, -1 on error
 */
int iox_unlinkat(int dirfd, const char *pathname, int flags);

/**
 * @brief Create hard link
 *
 * @param oldpath Existing file path
 * @param newpath New link path
 * @return int 0 on success, -1 on error
 */
int iox_link(const char *oldpath, const char *newpath);

/**
 * @brief Create hard link relative to directories
 *
 * @param olddirfd Old directory file descriptor
 * @param oldpath Existing file path
 * @param newdirfd New directory file descriptor
 * @param newpath New link path
 * @param flags Flags
 * @return int 0 on success, -1 on error
 */
int iox_linkat(int olddirfd, const char *oldpath, int newdirfd,
               const char *newpath, int flags);

/**
 * @brief Create symbolic link
 *
 * @param target Target path
 * @param linkpath Link path
 * @return int 0 on success, -1 on error
 */
int iox_symlink(const char *target, const char *linkpath);

/**
 * @brief Create symbolic link relative to directory
 *
 * @param target Target path
 * @param newdirfd Directory file descriptor
 * @param linkpath Link path
 * @return int 0 on success, -1 on error
 */
int iox_symlinkat(const char *target, int newdirfd, const char *linkpath);

/**
 * @brief Read symbolic link target
 *
 * @param pathname Path to symbolic link
 * @param buf Buffer to store target
 * @param bufsiz Buffer size
 * @return ssize_t Bytes read, -1 on error
 */
ssize_t iox_readlink(const char *pathname, char *buf, size_t bufsiz);

/**
 * @brief Read symbolic link target relative to directory
 *
 * @param dirfd Directory file descriptor
 * @param pathname Path to symbolic link
 * @param buf Buffer to store target
 * @param bufsiz Buffer size
 * @return ssize_t Bytes read, -1 on error
 */
ssize_t iox_readlinkat(int dirfd, const char *pathname, char *buf,
                       size_t bufsiz);

/**
 * @brief Change root directory
 *
 * @param path New root directory
 * @return int 0 on success, -1 on error
 */
int iox_chroot(const char *path);

/* ============================================================================
 * IDENTITY
 * ============================================================================ */

/**
 * @brief Get real user ID
 *
 * @return uid_t User ID
 */
uid_t iox_getuid(void);

/**
 * @brief Get effective user ID
 *
 * @return uid_t Effective user ID
 */
uid_t iox_geteuid(void);

/**
 * @brief Get real group ID
 *
 * @return gid_t Group ID
 */
gid_t iox_getgid(void);

/**
 * @brief Get effective group ID
 *
 * @return gid_t Effective group ID
 */
gid_t iox_getegid(void);

/**
 * @brief Set real and effective user ID
 *
 * @param uid New user ID
 * @return int 0 on success, -1 on error
 */
int iox_setuid(uid_t uid);

/**
 * @brief Set effective user ID
 *
 * @param euid New effective user ID
 * @return int 0 on success, -1 on error
 */
int iox_seteuid(uid_t euid);

/**
 * @brief Set real and effective group ID
 *
 * @param gid New group ID
 * @return int 0 on success, -1 on error
 */
int iox_setgid(gid_t gid);

/**
 * @brief Set effective group ID
 *
 * @param egid New effective group ID
 * @return int 0 on success, -1 on error
 */
int iox_setegid(gid_t egid);

/**
 * @brief Get supplementary group IDs
 *
 * @param size Size of groups array
 * @param list Array to store group IDs
 * @return int Number of groups on success, -1 on error
 */
int iox_getgroups(int size, gid_t list[]);

/**
 * @brief Set supplementary group IDs
 *
 * @param size Number of groups
 * @param list Array of group IDs
 * @return int 0 on success, -1 on error
 */
int iox_setgroups(int size, const gid_t list[]);

/* ============================================================================
 * TIME
 * ============================================================================ */

/**
 * @brief Sleep for specified seconds
 *
 * @param seconds Seconds to sleep
 * @return unsigned int Remaining seconds (interrupted) or 0
 */
unsigned int iox_sleep(unsigned int seconds);

/**
 * @brief Sleep for microseconds
 *
 * @param usec Microseconds to sleep
 * @return int 0 on success, -1 on error
 */
int iox_usleep(useconds_t usec);

/**
 * @brief Sleep for specified time
 *
 * @param req Requested sleep time
 * @param rem Remaining time if interrupted
 * @return int 0 on success, -1 on error
 */
int iox_nanosleep(const struct timespec *req, struct timespec *rem);

/**
 * @brief Get current time
 *
 * @param tv Time value
 * @param tz Timezone (deprecated, should be NULL)
 * @return int 0 on success, -1 on error
 */
int iox_gettimeofday(struct timeval *tv, struct timezone *tz);

/**
 * @brief Get time
 *
 * @param tloc Location to store time (can be NULL)
 * @return time_t Current time
 */
time_t iox_time(time_t *tloc);

/* ============================================================================
 * MISC
 * ============================================================================ */

/**
 * @brief Get system page size
 *
 * @return long Page size in bytes
 */
long iox_getpagesize(void);

/**
 * @brief Get hostname
 *
 * @param name Buffer to store hostname
 * @param namelen Buffer size
 * @return int 0 on success, -1 on error
 */
int iox_gethostname(char *name, size_t namelen);

/**
 * @brief Get configurable system variables
 *
 * @param name Variable name
 * @return long Variable value, -1 on error
 */
long iox_sysconf(int name);

#ifdef __cplusplus
}
#endif

#endif /* IOX_LINUX_UNISTD_H */

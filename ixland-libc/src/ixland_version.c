/* iXland libc - Version and String Utilities
 *
 * Self-contained libc-facing utility implementations.
 * Extracted from ixland-system as the first real implementation target.
 */

#include <ixland/ixland.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * VERSION
 * ============================================================================ */

const char *ixland_version(void) {
    return IXLAND_VERSION_STRING;
}

/* ============================================================================
 * ERROR STRINGS
 * ============================================================================ */

static const char *ixland_error_strings[] = {
    "Success",                          /* IXLAND_OK = 0 */
    "Operation not permitted",          /* IXLAND_EPERM = -1 */
    "No such file or directory",        /* IXLAND_ENOENT = -2 */
    "No such process",                  /* IXLAND_ESRCH = -3 */
    "Interrupted system call",          /* IXLAND_EINTR = -4 */
    "Input/output error",               /* IXLAND_EIO = -5 */
    "No such device or address",        /* IXLAND_ENXIO = -6 */
    "Argument list too long",           /* IXLAND_E2BIG = -7 */
    "Exec format error",                /* IXLAND_ENOEXEC = -8 */
    "Bad file descriptor",              /* IXLAND_EBADF = -9 */
    "No child processes",               /* IXLAND_ECHILD = -10 */
    "Resource temporarily unavailable", /* IXLAND_EAGAIN = -11 */
    "Out of memory",                    /* IXLAND_ENOMEM = -12 */
    "Permission denied",                /* IXLAND_EACCES = -13 */
    "Bad address",                      /* IXLAND_EFAULT = -14 */
    "Block device required",            /* IXLAND_ENOTBLK = -15 */
    "Device or resource busy",          /* IXLAND_EBUSY = -16 */
    "File exists",                      /* IXLAND_EEXIST = -17 */
    "Invalid cross-device link",        /* IXLAND_EXDEV = -18 */
    "No such device",                   /* IXLAND_ENODEV = -19 */
    "Not a directory",                  /* IXLAND_ENOTDIR = -20 */
    "Is a directory",                   /* IXLAND_EISDIR = -21 */
    "Invalid argument",                 /* IXLAND_EINVAL = -22 */
    "Too many open files",              /* IXLAND_EMFILE = -24 */
    "Text file busy",                   /* IXLAND_ETXTBSY = -26 */
    "File too large",                   /* IXLAND_EFBIG = -27 */
    "No space left on device",          /* IXLAND_ENOSPC = -28 */
    "Illegal seek",                     /* IXLAND_ESPIPE = -29 */
    "Read-only file system",            /* IXLAND_EROFS = -30 */
    "Too many links",                   /* IXLAND_EMLINK = -31 */
    "Broken pipe",                      /* IXLAND_EPIPE = -32 */
    "Math argument out of domain",      /* IXLAND_EDOM = -33 */
    "Numerical result out of range",    /* IXLAND_ERANGE = -34 */
    "Resource deadlock avoided",        /* IXLAND_EDEADLK = -35 */
    "File name too long",               /* IXLAND_ENAMETOOLONG = -63 */
    "Directory not empty",              /* IXLAND_ENOTEMPTY = -39 */
    "Operation not supported"           /* IXLAND_EOPNOTSUPP = -95 */
};

static const int ixland_error_min = -95;
static const int ixland_error_max = 0;

const char *ixland_strerror(int errnum) {
    /* Map positive errno to negative ixland_error if needed */
    if (errnum > 0) {
        errnum = -errnum;
    }

    if (errnum < ixland_error_min || errnum > ixland_error_max) {
        return "Unknown error";
    }

    /* Convert to array index (errors are negative, so negate) */
    int idx = -errnum;

    if (idx >= (int)(sizeof(ixland_error_strings) / sizeof(ixland_error_strings[0]))) {
        return "Unknown error";
    }

    return ixland_error_strings[idx];
}

void ixland_perror(const char *s) {
    /* Minimal truthful implementation using write(2) to stderr
     * Avoids FILE* dependency while providing useful error output
     */
    extern ssize_t write(int fd, const void *buf, size_t count);
    const char *msg = ixland_strerror(errno);
    const char *sep = ": ";
    const char *nl = "\n";

    if (s && *s) {
        write(2, s, strlen(s));
        write(2, sep, 2);
    }
    write(2, msg, strlen(msg));
    write(2, nl, 1);
}

/* iXland libc - Version and String Utilities
 *
 * Self-contained libc-facing utility implementations.
 * Extracted from ixland-system as the first real implementation target.
 */

#include <iox/iox.h>
#include <string.h>

/* ============================================================================
 * VERSION
 * ============================================================================ */

const char *iox_version(void) {
    return IOX_VERSION_STRING;
}

/* ============================================================================
 * ERROR STRINGS
 * ============================================================================ */

static const char *iox_error_strings[] = {
    "Success",                           /* IOX_OK = 0 */
    "Operation not permitted",           /* IOX_EPERM = -1 */
    "No such file or directory",         /* IOX_ENOENT = -2 */
    "No such process",                   /* IOX_ESRCH = -3 */
    "Interrupted system call",           /* IOX_EINTR = -4 */
    "Input/output error",                /* IOX_EIO = -5 */
    "No such device or address",         /* IOX_ENXIO = -6 */
    "Argument list too long",            /* IOX_E2BIG = -7 */
    "Exec format error",                 /* IOX_ENOEXEC = -8 */
    "Bad file descriptor",               /* IOX_EBADF = -9 */
    "No child processes",                /* IOX_ECHILD = -10 */
    "Resource temporarily unavailable",  /* IOX_EAGAIN = -11 */
    "Out of memory",                     /* IOX_ENOMEM = -12 */
    "Permission denied",                 /* IOX_EACCES = -13 */
    "Bad address",                       /* IOX_EFAULT = -14 */
    "Block device required",             /* IOX_ENOTBLK = -15 */
    "Device or resource busy",           /* IOX_EBUSY = -16 */
    "File exists",                       /* IOX_EEXIST = -17 */
    "Invalid cross-device link",         /* IOX_EXDEV = -18 */
    "No such device",                    /* IOX_ENODEV = -19 */
    "Not a directory",                   /* IOX_ENOTDIR = -20 */
    "Is a directory",                    /* IOX_EISDIR = -21 */
    "Invalid argument",                  /* IOX_EINVAL = -22 */
    "Too many open files",               /* IOX_EMFILE = -24 */
    "Text file busy",                    /* IOX_ETXTBSY = -26 */
    "File too large",                    /* IOX_EFBIG = -27 */
    "No space left on device",           /* IOX_ENOSPC = -28 */
    "Illegal seek",                      /* IOX_ESPIPE = -29 */
    "Read-only file system",             /* IOX_EROFS = -30 */
    "Too many links",                    /* IOX_EMLINK = -31 */
    "Broken pipe",                       /* IOX_EPIPE = -32 */
    "Math argument out of domain",       /* IOX_EDOM = -33 */
    "Numerical result out of range",     /* IOX_ERANGE = -34 */
    "Resource deadlock avoided",         /* IOX_EDEADLK = -35 */
    "File name too long",                /* IOX_ENAMETOOLONG = -63 */
    "Directory not empty",               /* IOX_ENOTEMPTY = -39 */
    "Operation not supported"            /* IOX_EOPNOTSUPP = -95 */
};

static const int iox_error_min = -95;
static const int iox_error_max = 0;

const char *iox_strerror(int errnum) {
    /* Map positive errno to negative iox_error if needed */
    if (errnum > 0) {
        errnum = -errnum;
    }

    if (errnum < iox_error_min || errnum > iox_error_max) {
        return "Unknown error";
    }

    /* Convert to array index (errors are negative, so negate) */
    int idx = -errnum;

    if (idx >= (int)(sizeof(iox_error_strings) / sizeof(iox_error_strings[0]))) {
        return "Unknown error";
    }

    return iox_error_strings[idx];
}

void iox_perror(const char *s) {
    /* Note: Minimal implementation without FILE* dependency */
    /* Real implementation would use write(2) to stderr */
    (void)s;
    /* Placeholder - real implementation needs stdio or raw write */
}

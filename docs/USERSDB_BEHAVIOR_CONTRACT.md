# iXland Users Database Behavior Contract

## Overview

This document defines the formal behavior contract for the iXland users database API. The users database provides POSIX-compliant user and group information lookup functionality with both non-reentrant (legacy) and reentrant (thread-safe) interfaces.

This implementation is a **stub** suitable for mobile environments where only a limited set of users (mobile:501 and root:0) exist. The contract specifies exact return values, errno settings, and error conditions for all functions.

## Scope

This contract covers the following functions:
- Password database: `getpwnam`, `getpwuid`, `getpwnam_r`, `getpwuid_r`
- Group database: `getgrnam`, `getgrgid`, `getgrnam_r`, `getgrgid_r`

## Function Behavior Contract

### Non-Reentrant Functions (Non-Thread-Safe)

These functions use internal static buffers and are NOT thread-safe.

---

#### `getpwnam`

```c
struct passwd *getpwnam(const char *name);
```

| Condition | Return Value | errno |
|-----------|--------------|-------|
| **Success** (name="mobile") | Pointer to `struct passwd` | 0 (unchanged) |
| **Success** (name="root") | Pointer to `struct passwd` | 0 (unchanged) |
| **Not found** | NULL | ENOENT |
| **Invalid argument** (name=NULL) | NULL | EINVAL |

**Notes:**
- Returns pointer to internal static buffer (overwritten by subsequent calls)
- Only recognizes "mobile" (UID 501) and "root" (UID 0)

---

#### `getpwuid`

```c
struct passwd *getpwuid(uid_t uid);
```

| Condition | Return Value | errno |
|-----------|--------------|-------|
| **Success** (uid=501) | Pointer to `struct passwd` (mobile) | 0 (unchanged) |
| **Success** (uid=0) | Pointer to `struct passwd` (root) | 0 (unchanged) |
| **Not found** | NULL | ENOENT |

**Notes:**
- Returns pointer to internal static buffer (overwritten by subsequent calls)
- Recognizes UID 501 (mobile) and UID 0 (root)

---

#### `getgrnam`

```c
struct group *getgrnam(const char *name);
```

| Condition | Return Value | errno |
|-----------|--------------|-------|
| **Success** (name="mobile") | Pointer to `struct group` | 0 (unchanged) |
| **Not found** | NULL | ENOENT |
| **Invalid argument** (name=NULL) | NULL | EINVAL |

**Notes:**
- Returns pointer to internal static buffer (overwritten by subsequent calls)
- Only recognizes "mobile" group (GID 501)

---

#### `getgrgid`

```c
struct group *getgrgid(gid_t gid);
```

| Condition | Return Value | errno |
|-----------|--------------|-------|
| **Success** (gid=501) | Pointer to `struct group` | 0 (unchanged) |
| **Not found** | NULL | ENOENT |

**Notes:**
- Returns pointer to internal static buffer (overwritten by subsequent calls)
- Only recognizes GID 501 (mobile)

---

### Reentrant Functions (Thread-Safe)

These functions use caller-provided buffers and ARE thread-safe.

---

#### `getpwnam_r`

```c
int getpwnam_r(const char *name, struct passwd *pwd, char *buf,
               size_t buflen, struct passwd **result);
```

| Condition | Return Value | *result | errno |
|-----------|--------------|---------|-------|
| **Success** (name="mobile") | 0 | Points to `pwd` | 0 (unchanged) |
| **Success** (name="root") | 0 | Points to `pwd` | 0 (unchanged) |
| **Not found** | ENOENT | NULL | ENOENT |
| **Invalid argument** (name=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (pwd=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (buf=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (result=NULL) | EINVAL | NULL | EINVAL |
| **Buffer too small** | ERANGE | NULL | ERANGE |

**Notes:**
- Stores strings in caller-provided `buf` (size `buflen`)
- On success, `*result` points to `pwd` structure
- On failure, `*result` is set to NULL
- Return value equals errno on error

---

#### `getpwuid_r`

```c
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf,
               size_t buflen, struct passwd **result);
```

| Condition | Return Value | *result | errno |
|-----------|--------------|---------|-------|
| **Success** (uid=501) | 0 | Points to `pwd` | 0 (unchanged) |
| **Success** (uid=0) | 0 | Points to `pwd` | 0 (unchanged) |
| **Not found** | ENOENT | NULL | ENOENT |
| **Invalid argument** (pwd=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (buf=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (result=NULL) | EINVAL | NULL | EINVAL |
| **Buffer too small** | ERANGE | NULL | ERANGE |

**Notes:**
- Stores strings in caller-provided `buf` (size `buflen`)
- On success, `*result` points to `pwd` structure
- On failure, `*result` is set to NULL
- Return value equals errno on error

---

#### `getgrnam_r`

```c
int getgrnam_r(const char *name, struct group *grp, char *buf,
               size_t buflen, struct group **result);
```

| Condition | Return Value | *result | errno |
|-----------|--------------|---------|-------|
| **Success** (name="mobile") | 0 | Points to `grp` | 0 (unchanged) |
| **Not found** | ENOENT | NULL | ENOENT |
| **Invalid argument** (name=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (grp=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (buf=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (result=NULL) | EINVAL | NULL | EINVAL |
| **Buffer too small** | ERANGE | NULL | ERANGE |

**Notes:**
- Stores strings in caller-provided `buf` (size `buflen`)
- On success, `*result` points to `grp` structure
- On failure, `*result` is set to NULL
- Return value equals errno on error

---

#### `getgrgid_r`

```c
int getgrgid_r(gid_t gid, struct group *grp, char *buf,
               size_t buflen, struct group **result);
```

| Condition | Return Value | *result | errno |
|-----------|--------------|---------|-------|
| **Success** (gid=501) | 0 | Points to `grp` | 0 (unchanged) |
| **Not found** | ENOENT | NULL | ENOENT |
| **Invalid argument** (grp=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (buf=NULL) | EINVAL | NULL | EINVAL |
| **Invalid argument** (result=NULL) | EINVAL | NULL | EINVAL |
| **Buffer too small** | ERANGE | NULL | ERANGE |

**Notes:**
- Stores strings in caller-provided `buf` (size `buflen`)
- On success, `*result` points to `grp` structure
- On failure, `*result` is set to NULL
- Return value equals errno on error

---

## Error Code Reference

| Error Code | Value | Meaning |
|------------|-------|---------|
| `0` | 0 | Success (no error) |
| `ENOENT` | 2 | No such entry found |
| `EINVAL` | 22 | Invalid argument |
| `ERANGE` | 34 | Result too large (buffer too small) |

## Data Structure Definitions

### struct passwd

```c
struct passwd {
    char *pw_name;   /* User login name */
    char *pw_passwd; /* Encrypted password (or "*") */
    uid_t pw_uid;    /* User ID */
    gid_t pw_gid;    /* Group ID */
    char *pw_gecos;  /* Real name or comment field */
    char *pw_dir;    /* Home directory */
    char *pw_shell;  /* Default shell program */
};
```

### struct group

```c
struct group {
    char *gr_name;   /* Group name */
    char *gr_passwd; /* Group password (or "*") */
    gid_t gr_gid;    /* Group ID */
    char **gr_mem;   /* NULL-terminated array of member names */
};
```

## Stub Implementation Notes

### Recognized Entries

| Type | Name | ID | Home Directory | Shell |
|------|------|-----|----------------|-------|
| User | mobile | UID 501 | /var/mobile | /bin/sh |
| User | root | UID 0 | /var/root | /bin/sh |
| Group | mobile | GID 501 | - | - |

### Buffer Size Requirements

For reentrant functions, the following minimum buffer sizes are recommended:

| Function | Minimum buflen |
|----------|----------------|
| `getpwnam_r` / `getpwuid_r` | 64 bytes (mobile), 128 bytes (root) |
| `getgrnam_r` / `getgrgid_r` | 48 bytes |

The implementation defines `IOX_GETPW_R_SIZE_MAX` (1024) and `IOX_GETGR_R_SIZE_MAX` (1024) as safe upper bounds.

### Thread Safety

- **Non-reentrant functions** (`getpwnam`, `getpwuid`, `getgrnam`, `getgrgid`): NOT thread-safe. Use internal static buffers.
- **Reentrant functions** (`getpwnam_r`, `getpwuid_r`, `getgrnam_r`, `getgrgid_r`): Thread-safe. Use caller-provided buffers.

### POSIX Compliance

This implementation follows POSIX.1-2008 specifications with the following notes:

1. **Non-reentrant functions** set `errno` on error per POSIX.
2. **Reentrant functions** return error codes directly (matching `errno` values), as required by POSIX.
3. The stub implementation provides minimal functionality suitable for mobile environments.

### Future Extensions

Future implementations may:
- Support additional users from a runtime database
- Support file-backed `/etc/passwd` and `/etc/group` via VFS
- Support NSS (Name Service Switch) integration
- Support network directory services

When implementing extensions, this behavior contract must be maintained for backward compatibility.

---

*Document Version: 1.0*
*Last Updated: 2026-03-30*
*Implementation: ixland-libc/src/usersdb/*

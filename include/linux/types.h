/*
 * linux/types.h - Linux kernel types
 *
 * Kernel-specific types not provided by system headers
 */

#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Legacy Type Aliases (for kernel compatibility)
 * ============================================================================ */

typedef uint8_t     __u8;
typedef uint16_t    __u16;
typedef uint32_t    __u32;
typedef uint64_t    __u64;

typedef int8_t      __s8;
typedef int16_t     __s16;
typedef int32_t     __s32;
typedef int64_t     __s64;

/* ============================================================================
 * Endian-specific types
 * ============================================================================ */

typedef uint32_t    __le32;
typedef uint32_t    __be32;
typedef uint64_t    __le64;
typedef uint64_t    __be64;
typedef uint16_t    __le16;
typedef uint16_t    __be16;

/* ============================================================================
 * Kernel-specific types
 * ============================================================================ */

typedef unsigned long   __kernel_ulong_t;
typedef long            __kernel_long_t;

typedef long long   __kernel_loff_t;

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_TYPES_H */

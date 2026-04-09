#ifndef IXLAND_LINUX_ABI_H
#define IXLAND_LINUX_ABI_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IXLAND_LINUX_ABI_VERSION 1

bool ixland_linux_abi_is_syscall_supported(int syscall_number);

#ifdef __cplusplus
}
#endif

#endif
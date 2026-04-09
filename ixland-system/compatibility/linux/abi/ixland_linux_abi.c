#include "ixland_linux_abi.h"

bool ixland_linux_abi_is_syscall_supported(int syscall_number) {
    return syscall_number >= 0;
}
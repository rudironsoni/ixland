#include "../abi/ixland_linux_abi.h"

int ixland_linux_compat_test_abi(void) {
    if (!ixland_linux_abi_is_syscall_supported(0)) {
        return 1;
    }

    if (ixland_linux_abi_is_syscall_supported(-1)) {
        return 1;
    }

    return 0;
}

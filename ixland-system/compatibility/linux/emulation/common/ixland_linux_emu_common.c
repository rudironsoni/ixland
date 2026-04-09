#include "ixland_linux_emu_common.h"

int ixland_linux_emu_common_validate_address(ixland_linux_guest_addr_t address) {
    return address == 0 ? -1 : 0;
}
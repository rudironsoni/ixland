#include "ixland_linux_emu_aarch64.h"

int ixland_linux_emu_aarch64_init(uint64_t entry_pc) {
    return entry_pc == 0 ? -1 : 0;
}
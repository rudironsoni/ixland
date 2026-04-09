#ifndef IXLAND_LINUX_EMU_AARCH64_H
#define IXLAND_LINUX_EMU_AARCH64_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int ixland_linux_emu_aarch64_init(uint64_t entry_pc);

#ifdef __cplusplus
}
#endif

#endif
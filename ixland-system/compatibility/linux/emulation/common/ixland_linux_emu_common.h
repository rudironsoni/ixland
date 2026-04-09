#ifndef IXLAND_LINUX_EMU_COMMON_H
#define IXLAND_LINUX_EMU_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t ixland_linux_guest_addr_t;

int ixland_linux_emu_common_validate_address(ixland_linux_guest_addr_t address);

#ifdef __cplusplus
}
#endif

#endif
#ifndef IXLAND_LINUX_RUNTIME_H
#define IXLAND_LINUX_RUNTIME_H

#include "../../../runtime/linux_host/ixland_linux_compatibility_host.h"

#ifdef __cplusplus
extern "C" {
#endif

int ixland_linux_runtime_bootstrap(
    const ixland_linux_compatibility_runtime_handle_t *runtime_handle);
int ixland_linux_runtime_stop(void);

#ifdef __cplusplus
}
#endif

#endif
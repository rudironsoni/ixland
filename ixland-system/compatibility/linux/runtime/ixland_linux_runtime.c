#include "ixland_linux_runtime.h"

#define IXLAND_LINUX_RUNTIME_HANDLE_MAGIC 0x49584C414E44554CULL

int ixland_linux_runtime_bootstrap(
    const ixland_linux_compatibility_runtime_handle_t *runtime_handle) {
    if (runtime_handle == 0) {
        return -1;
    }

    if (runtime_handle->magic != IXLAND_LINUX_RUNTIME_HANDLE_MAGIC) {
        return -1;
    }

    if (!runtime_handle->active) {
        return -1;
    }

    return 0;
}

int ixland_linux_runtime_stop(void) {
    return 0;
}

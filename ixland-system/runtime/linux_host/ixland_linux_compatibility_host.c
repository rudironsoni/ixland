#include "ixland_linux_compatibility_host.h"

#include <string.h>

#define IXLAND_LINUX_RUNTIME_HANDLE_MAGIC 0x49584C414E44554CULL

int ixland_linux_compatibility_host_init(ixland_linux_compatibility_host_t *host,
                                         const ixland_linux_compatibility_host_config_t *config) {
    if (host == NULL || config == NULL) {
        return -1;
    }

    memset(host, 0, sizeof(*host));
    host->config = *config;
    host->runtime_handle.magic = IXLAND_LINUX_RUNTIME_HANDLE_MAGIC;
    host->runtime_handle.runtime_slot = 0;
    host->runtime_handle.active = false;
    host->initialized = true;

    return 0;
}

int ixland_linux_compatibility_host_attach_session(
    ixland_linux_compatibility_host_t *host, ixland_linux_compatibility_session_handle_t *session) {
    if (host == NULL || session == NULL || !host->initialized) {
        return -1;
    }

    if (session->session_id == NULL) {
        return -1;
    }

    session->session_slot = 0;
    return 0;
}

int ixland_linux_compatibility_host_get_runtime_handle(
    ixland_linux_compatibility_host_t *host,
    ixland_linux_compatibility_runtime_handle_t **runtime_handle) {
    if (host == NULL || runtime_handle == NULL || !host->initialized) {
        return -1;
    }

    host->runtime_handle.active = true;
    *runtime_handle = &host->runtime_handle;

    return 0;
}

int ixland_linux_compatibility_host_shutdown(ixland_linux_compatibility_host_t *host) {
    if (host == NULL || !host->initialized) {
        return -1;
    }

    host->runtime_handle.active = false;
    host->runtime_handle.magic = 0;
    host->initialized = false;
    return 0;
}

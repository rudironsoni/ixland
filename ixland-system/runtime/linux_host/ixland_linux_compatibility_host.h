#ifndef IXLAND_LINUX_COMPATIBILITY_HOST_H
#define IXLAND_LINUX_COMPATIBILITY_HOST_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ixland_linux_compatibility_host_config {
    const char *distro_root_path;
    const char *session_id;
    bool enable_tracing;
} ixland_linux_compatibility_host_config_t;

typedef struct ixland_linux_compatibility_runtime_handle {
    uint64_t magic;
    int runtime_slot;
    bool active;
} ixland_linux_compatibility_runtime_handle_t;

typedef struct ixland_linux_compatibility_session_handle {
    const char *session_id;
    int session_slot;
} ixland_linux_compatibility_session_handle_t;

typedef struct ixland_linux_compatibility_host {
    ixland_linux_compatibility_host_config_t config;
    ixland_linux_compatibility_runtime_handle_t runtime_handle;
    bool initialized;
} ixland_linux_compatibility_host_t;

int ixland_linux_compatibility_host_init(ixland_linux_compatibility_host_t *host,
                                         const ixland_linux_compatibility_host_config_t *config);
int ixland_linux_compatibility_host_attach_session(
    ixland_linux_compatibility_host_t *host, ixland_linux_compatibility_session_handle_t *session);
int ixland_linux_compatibility_host_get_runtime_handle(
    ixland_linux_compatibility_host_t *host,
    ixland_linux_compatibility_runtime_handle_t **runtime_handle);
int ixland_linux_compatibility_host_shutdown(ixland_linux_compatibility_host_t *host);

#ifdef __cplusplus
}
#endif

#endif
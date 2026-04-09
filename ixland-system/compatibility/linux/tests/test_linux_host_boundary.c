#include <stdbool.h>

#include "../../../runtime/linux_host/ixland_linux_compatibility_host.h"
#include "../runtime/ixland_linux_runtime.h"

int ixland_linux_compat_test_host_boundary(void) {
    ixland_linux_compatibility_host_t host;
    ixland_linux_compatibility_host_config_t config = {
        .distro_root_path = "/tmp/ixland-linux-rootfs",
        .session_id = "g2-boundary-session",
        .enable_tracing = false,
    };
    ixland_linux_compatibility_session_handle_t session = {
        .session_id = "g2-boundary-session",
        .session_slot = -1,
    };
    ixland_linux_compatibility_runtime_handle_t *runtime_handle = 0;

    if (ixland_linux_runtime_bootstrap(runtime_handle) == 0) {
        return 1;
    }

    if (ixland_linux_compatibility_host_init(&host, &config) != 0) {
        return 1;
    }

    if (ixland_linux_compatibility_host_attach_session(&host, &session) != 0) {
        return 1;
    }

    if (ixland_linux_compatibility_host_get_runtime_handle(&host, &runtime_handle) != 0) {
        return 1;
    }

    if (ixland_linux_runtime_bootstrap(runtime_handle) != 0) {
        return 1;
    }

    {
        ixland_linux_compatibility_runtime_handle_t forged = *runtime_handle;
        forged.magic = 0;
        forged.active = true;
        if (ixland_linux_runtime_bootstrap(&forged) == 0) {
            return 1;
        }
    }

    if (ixland_linux_compatibility_host_shutdown(&host) != 0) {
        return 1;
    }

    if (ixland_linux_runtime_bootstrap(runtime_handle) == 0) {
        return 1;
    }

    return 0;
}

#include "ixland_linux_distro.h"

int ixland_linux_distro_mount_rootfs(const char *rootfs_path) {
    return rootfs_path == 0 ? -1 : 0;
}
#include "ixland_linux_loader.h"

int ixland_linux_loader_prepare_guest(const char *rootfs_path) {
    return rootfs_path == 0 ? -1 : 0;
}
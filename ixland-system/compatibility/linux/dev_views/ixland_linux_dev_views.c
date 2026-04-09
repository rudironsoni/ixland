#include "ixland_linux_dev_views.h"

int ixland_linux_dev_views_resolve_tty_device(int minor) {
    return minor >= 0 ? 0 : -1;
}
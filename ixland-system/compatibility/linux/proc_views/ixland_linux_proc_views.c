#include "ixland_linux_proc_views.h"

int ixland_linux_proc_views_render_pid_status(int pid) {
    return pid > 0 ? 0 : -1;
}
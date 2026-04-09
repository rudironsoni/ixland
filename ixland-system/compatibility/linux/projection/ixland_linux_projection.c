#include "ixland_linux_projection.h"

int ixland_linux_projection_project_process(int task_pid) {
    return task_pid > 0 ? 0 : -1;
}
#ifndef IOX_EXEC_H
#define IOX_EXEC_H

#include "../task/task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Exec flags */
#define IOX_EXEC_CLOEXEC    0x01

/* Image type detection */
iox_image_type_t iox_exec_classify(const char *path);

/* Exec implementations */
int iox_execve(const char *pathname, char *const argv[], char *const envp[]);
int iox_exec_native(iox_task_t *task, const char *path, int argc, char **argv, char **envp);
int iox_exec_wasi(iox_task_t *task, const char *path, int argc, char **argv, char **envp);
int iox_exec_script(iox_task_t *task, const char *path, int argc, char **argv, char **envp);

/* Close FD_CLOEXEC descriptors */
int iox_exec_close_cloexec(iox_task_t *task);

/* Reset signal handlers on exec */
void iox_exec_reset_signals(iox_sighand_t *sighand);

#ifdef __cplusplus
}
#endif

#endif

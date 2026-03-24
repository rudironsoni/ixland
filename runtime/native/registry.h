#ifndef IOX_NATIVE_REGISTRY_H
#define IOX_NATIVE_REGISTRY_H

#include "../../kernel/task/task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Native command entry point signature */
typedef int (*iox_native_entry_t)(iox_task_t *task, int argc, char **argv, char **envp);

/* Registry entry */
typedef struct iox_native_cmd {
    const char *path;           /* Command path (e.g., "/bin/testcmd") */
    iox_native_entry_t entry;   /* Entry point */
    struct iox_native_cmd *next;
} iox_native_cmd_t;

/* Registry operations */
int iox_native_register(const char *path, iox_native_entry_t entry);
iox_native_entry_t iox_native_lookup(const char *path);
void iox_native_registry_init(void);
void iox_native_registry_clear(void);

#ifdef __cplusplus
}
#endif

#endif

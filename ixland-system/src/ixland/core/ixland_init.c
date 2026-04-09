/* iOS Subsystem for Linux - Library Initialization
 *
 * Automatic initialization using constructor attribute
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../internal/ixland_internal.h"

/* Global initialization flag */
static atomic_int ixland_initialized = 0;
static pthread_mutex_t ixland_init_lock = PTHREAD_MUTEX_INITIALIZER;

/* Global state */
ixland_context_t *ixland_global_context = NULL;

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

void ixland_library_init(void) __attribute__((constructor(101))) __attribute__((used));
void ixland_library_deinit(void) __attribute__((destructor)) __attribute__((used));

void ixland_library_init(void) {
    /* Check if already initialized */
    if (atomic_load(&ixland_initialized)) {
        return;
    }

    pthread_mutex_lock(&ixland_init_lock);

    /* Double-check after acquiring lock */
    if (atomic_load(&ixland_initialized)) {
        pthread_mutex_unlock(&ixland_init_lock);
        return;
    }

    /* Initialize VFS first (safe initialization) */
    int vfs_result = ixland_vfs_init();
    if (vfs_result != 0) {
        /* VFS init failed - continue anyway with defaults */
        /* This allows the library to work even if HOME is not set */
    }

    /* Initialize process simulation state before exposing process APIs */
    int proc_result = __ixland_init();
    if (proc_result != 0) {
        pthread_mutex_unlock(&ixland_init_lock);
        return;
    }

    /* Initialize context system */
    int ctx_result = ixland_context_init();
    if (ctx_result != 0) {
        /* Context init failed - mark as failed but don't crash */
        pthread_mutex_unlock(&ixland_init_lock);
        return;
    }

    /* Initialize file descriptor table */
    extern void __ixland_file_init_impl(void);
    __ixland_file_init_impl();

    /* Set initialized flag */
    atomic_store(&ixland_initialized, 1);

    pthread_mutex_unlock(&ixland_init_lock);
}

void ixland_library_deinit(void) {
    if (!atomic_load(&ixland_initialized)) {
        return;
    }

    ixland_context_deinit();
    ixland_vfs_deinit();

    atomic_store(&ixland_initialized, 0);

    if (getenv("IXLAND_DEBUG")) {
        fprintf(stderr, "ixland: Library deinitialized\n");
    }
}

/* ============================================================================
 * PUBLIC INITIALIZATION API
 * ============================================================================ */

int ixland_init(const ixland_config_t *config) {
    /* Initialization happens automatically via constructor,
     * but this function allows explicit initialization if needed */
    (void)config; /* Config ignored for now */
    if (!atomic_load(&ixland_initialized)) {
        ixland_library_init();
    }
    return atomic_load(&ixland_initialized) ? 0 : -1;
}

int ixland_is_initialized(void) {
    return atomic_load(&ixland_initialized);
}

const char *ixland_version(void) {
    return "1.0.0";
}

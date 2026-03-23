/* iOS Subsystem for Linux - Library Initialization
 *
 * Automatic initialization using constructor attribute
 */

#include <unistd.h>
#include "../internal/iox_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

/* Global initialization flag */
static atomic_int iox_initialized = 0;
static pthread_mutex_t iox_init_lock = PTHREAD_MUTEX_INITIALIZER;

/* Global state */
iox_context_t *iox_global_context = NULL;

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

void iox_library_init(void) __attribute__((constructor(101))) __attribute__((used));
void iox_library_deinit(void) __attribute__((destructor)) __attribute__((used));

void iox_library_init(void) {
    /* Check if already initialized */
    if (atomic_load(&iox_initialized)) {
        return;
    }
    
    pthread_mutex_lock(&iox_init_lock);
    
    /* Double-check after acquiring lock */
    if (atomic_load(&iox_initialized)) {
        pthread_mutex_unlock(&iox_init_lock);
        return;
    }
    
    /* Initialize VFS first (safe initialization) */
    int vfs_result = iox_vfs_init();
    if (vfs_result != 0) {
        /* VFS init failed - continue anyway with defaults */
        /* This allows the library to work even if HOME is not set */
    }
    
    /* Initialize process simulation state before exposing process APIs */
    int proc_result = __iox_init();
    if (proc_result != 0) {
        pthread_mutex_unlock(&iox_init_lock);
        return;
    }

    /* Initialize context system */
    int ctx_result = iox_context_init();
    if (ctx_result != 0) {
        /* Context init failed - mark as failed but don't crash */
        pthread_mutex_unlock(&iox_init_lock);
        return;
    }
    
    /* Initialize file descriptor table */
    extern void __iox_file_init_impl(void);
    __iox_file_init_impl();
    
    /* Set initialized flag */
    atomic_store(&iox_initialized, 1);
    
    pthread_mutex_unlock(&iox_init_lock);
}

void iox_library_deinit(void) {
    if (!atomic_load(&iox_initialized)) {
        return;
    }
    
    iox_context_deinit();
    iox_vfs_deinit();
    
    atomic_store(&iox_initialized, 0);
    
    if (getenv("IOX_DEBUG")) {
        fprintf(stderr, "iox: Library deinitialized\n");
    }
}

/* ============================================================================
 * PUBLIC INITIALIZATION API
 * ============================================================================ */

int iox_init(const iox_config_t *config) {
    /* Initialization happens automatically via constructor,
     * but this function allows explicit initialization if needed */
    (void)config;  /* Config ignored for now */
    if (!atomic_load(&iox_initialized)) {
        iox_library_init();
    }
    return atomic_load(&iox_initialized) ? 0 : -1;
}

int iox_is_initialized(void) {
    return atomic_load(&iox_initialized);
}

const char *iox_version(void) {
    return "1.0.0";
}

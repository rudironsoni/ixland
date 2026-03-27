#ifndef IXLAND_WASM_ENGINE_H
#define IXLAND_WASM_ENGINE_H

/* iXland Wasm Engine - Engine-Neutral Runtime Contract
 *
 * This header defines the contract for WebAssembly engine backends.
 * Different engines (WAMR, Wasmtime, etc.) implement this interface.
 */

#include "types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ENGINE CONFIGURATION
 * ============================================================================ */

typedef struct {
    ixland_wasm_backend_kind_t backend;
    size_t stack_size;             /* Stack size per instance */
    size_t heap_size;              /* Initial heap size */
    size_t heap_max;               /* Maximum heap size (0 = unlimited) */
    bool enable_wasi;              /* Enable WASI imports */
    bool enable_threading;         /* Enable threading (if supported) */
    bool enable_simd;              /* Enable SIMD (if supported) */
    bool enable_bulk_memory;       /* Enable bulk memory ops */
    bool enable_reference_types;   /* Enable reference types */
    const char *debug_name;        /* Optional debug name */
} ixland_wasm_engine_config_t;

/* ============================================================================
 * ENGINE OPERATIONS
 * ============================================================================ */

typedef struct ixland_wasm_engine_ops_s ixland_wasm_engine_ops_t;

struct ixland_wasm_engine_ops_s {
    /* Engine lifecycle */
    ixland_wasm_error_t (*create)(const ixland_wasm_engine_config_t *config,
                                   ixland_wasm_engine_t **out_engine);
    void (*destroy)(ixland_wasm_engine_t *engine);

    /* Module operations */
    ixland_wasm_error_t (*module_load)(ixland_wasm_engine_t *engine,
                                        const uint8_t *wasm_bytes,
                                        size_t wasm_size,
                                        ixland_wasm_module_t **out_module);
    void (*module_unload)(ixland_wasm_module_t *module);
    ixland_wasm_error_t (*module_get_info)(ixland_wasm_module_t *module,
                                            ixland_wasm_module_info_t *out_info);

    /* Instance operations */
    ixland_wasm_error_t (*instance_create)(ixland_wasm_module_t *module,
                                            ixland_wasm_host_t *host,
                                            ixland_wasm_instance_t **out_instance);
    void (*instance_destroy)(ixland_wasm_instance_t *instance);

    /* Execution */
    ixland_wasm_error_t (*instance_run)(ixland_wasm_instance_t *instance,
                                         const char *entry_point,
                                         int argc, const char *const *argv,
                                         ixland_wasm_result_t *out_result);

    /* Interrupt */
    ixland_wasm_error_t (*instance_interrupt)(ixland_wasm_instance_t *instance);
    bool (*instance_is_running)(ixland_wasm_instance_t *instance);

    /* Memory access (for host functions) */
    uint8_t* (*memory_get_ptr)(ixland_wasm_instance_t *instance,
                                uint32_t offset, uint32_t size);
    uint32_t (*memory_size)(ixland_wasm_instance_t *instance);  /* Pages */
    ixland_wasm_error_t (*memory_grow)(ixland_wasm_instance_t *instance,
                                        uint32_t delta_pages);
};

/* ============================================================================
 * ENGINE INTERFACE
 * ============================================================================ */

/* Initialize the engine system */
ixland_wasm_error_t ixland_wasm_engine_system_init(void);

/* Shutdown the engine system */
void ixland_wasm_engine_system_shutdown(void);

/* Register an engine backend */
ixland_wasm_error_t ixland_wasm_engine_register(
    ixland_wasm_backend_kind_t kind,
    const ixland_wasm_engine_ops_t *ops
);

/* Create an engine instance */
ixland_wasm_error_t ixland_wasm_engine_create(
    const ixland_wasm_engine_config_t *config,
    ixland_wasm_engine_t **out_engine
);

/* Destroy an engine */
void ixland_wasm_engine_destroy(ixland_wasm_engine_t *engine);

/* Get backend kind */
ixland_wasm_backend_kind_t ixland_wasm_engine_get_kind(
    ixland_wasm_engine_t *engine
);

/* ============================================================================
 * MODULE INTERFACE
 * ============================================================================ */

/* Load a WASM module from bytes */
ixland_wasm_error_t ixland_wasm_module_load(
    ixland_wasm_engine_t *engine,
    const uint8_t *wasm_bytes,
    size_t wasm_size,
    ixland_wasm_module_t **out_module
);

/* Unload a module */
void ixland_wasm_module_unload(ixland_wasm_module_t *module);

/* Get module information */
ixland_wasm_error_t ixland_wasm_module_get_info(
    ixland_wasm_module_t *module,
    ixland_wasm_module_info_t *out_info
);

/* ============================================================================
 * INSTANCE INTERFACE
 * ============================================================================ */

/* Create an instance from a module */
ixland_wasm_error_t ixland_wasm_instance_create(
    ixland_wasm_module_t *module,
    ixland_wasm_host_t *host,
    ixland_wasm_instance_t **out_instance
);

/* Destroy an instance */
void ixland_wasm_instance_destroy(ixland_wasm_instance_t *instance);

/* Run the instance starting at entry_point (NULL for "_start") */
ixland_wasm_error_t ixland_wasm_instance_run(
    ixland_wasm_instance_t *instance,
    const char *entry_point,
    int argc, const char *const *argv,
    ixland_wasm_result_t *out_result
);

/* Interrupt a running instance */
ixland_wasm_error_t ixland_wasm_instance_interrupt(
    ixland_wasm_instance_t *instance
);

/* Check if instance is running */
bool ixland_wasm_instance_is_running(ixland_wasm_instance_t *instance);

/* ============================================================================
 * MEMORY INTERFACE (for host functions)
 * ============================================================================ */

/* Get pointer to guest memory at offset */
uint8_t* ixland_wasm_memory_get_ptr(
    ixland_wasm_instance_t *instance,
    uint32_t offset,
    uint32_t size
);

/* Get current memory size in pages */
uint32_t ixland_wasm_memory_size(ixland_wasm_instance_t *instance);

/* Grow memory by delta_pages */
ixland_wasm_error_t ixland_wasm_memory_grow(
    ixland_wasm_instance_t *instance,
    uint32_t delta_pages
);

/* Validate memory slice is accessible */
bool ixland_wasm_memory_validate(
    ixland_wasm_instance_t *instance,
    uint32_t offset,
    uint32_t size
);

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_WASM_ENGINE_H */

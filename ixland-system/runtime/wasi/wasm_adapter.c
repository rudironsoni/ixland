/*
 * wasm_adapter.c - Internal Wasm Contract Adapter Implementation
 *
 * Maps current WAMR integration onto neutral ixland-wasm contract concepts.
 */

#include "wasm_adapter.h"
#include "iox_wamr.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * ADAPTER STATE
 * ============================================================================ */

static struct {
    bool initialized;
    ixland_wasm_engine_t engine;  /* Singleton engine for WAMR backend */
} adapter_state = { false };

/* ============================================================================
 * ADAPTER LIFECYCLE
 * ============================================================================ */

int ixland_wasm_adapter_init(void) {
    if (adapter_state.initialized) {
        return 0;
    }

    /* Initialize underlying WAMR runtime */
    if (iox_wamr_init() != 0) {
        return -1;
    }

    adapter_state.initialized = true;
    return 0;
}

void ixland_wasm_adapter_deinit(void) {
    if (!adapter_state.initialized) {
        return;
    }

    iox_wamr_deinit();
    adapter_state.initialized = false;
}

bool ixland_wasm_adapter_is_initialized(void) {
    return adapter_state.initialized;
}

/* ============================================================================
 * ENGINE ADAPTER
 * ============================================================================ */

ixland_wasm_engine_t *ixland_wasm_adapter_get_engine(void) {
    if (!adapter_state.initialized) {
        if (ixland_wasm_adapter_init() != 0) {
            return NULL;
        }
    }
    return &adapter_state.engine;
}

ixland_wasm_backend_kind_t ixland_wasm_adapter_get_backend(void) {
    return IXLAND_WASM_BACKEND_WAMR;
}

/* ============================================================================
 * MODULE ADAPTER
 * ============================================================================ */

ixland_wasm_module_t *ixland_wasm_adapter_load_module(
    const uint8_t *wasm_buf,
    uint32_t wasm_size
) {
    /* For now, delegate to WAMR implementation */
    if (iox_wamr_load_module(wasm_buf, wasm_size) != 0) {
        return NULL;
    }

    /* Return non-NULL to indicate success (actual handle managed by WAMR) */
    return (ixland_wasm_module_t *)1;
}

void ixland_wasm_adapter_unload_module(ixland_wasm_module_t *module) {
    (void)module;
    iox_wamr_unload_module();
}

/* ============================================================================
 * INSTANCE ADAPTER
 * ============================================================================ */

ixland_wasm_instance_t *ixland_wasm_adapter_instantiate(
    ixland_wasm_module_t *module
) {
    (void)module;
    /* WAMR instance is created automatically on module load */
    return (ixland_wasm_instance_t *)1;
}

void ixland_wasm_adapter_destroy_instance(ixland_wasm_instance_t *instance) {
    (void)instance;
    /* WAMR cleanup handled by module unload */
}

/* ============================================================================
 * HOST SERVICES ADAPTER
 * ============================================================================ */

int ixland_wasm_adapter_register_host(
    ixland_wasm_module_t *module,
    const ixland_wasm_host_import_t *imports,
    uint32_t import_count
) {
    (void)module;
    (void)imports;
    (void)import_count;
    /* TODO: Map to WAMR native registration when implemented */
    return 0;
}

int ixland_wasm_adapter_call(
    ixland_wasm_instance_t *instance,
    const char *func_name,
    const ixland_wasm_value_t *args,
    uint32_t argc,
    ixland_wasm_value_t *results,
    uint32_t result_count
) {
    (void)instance;
    (void)args;
    (void)argc;
    (void)results;
    (void)result_count;

    /* Convert arguments to WAMR format and call */
    uint32_t argv[16] = {0};
    uint32_t argc_wamr = (argc > 16) ? 16 : argc;

    for (uint32_t i = 0; i < argc_wamr; i++) {
        switch (args[i].kind) {
            case IXLAND_WASM_VALUE_I32:
                argv[i] = (uint32_t)args[i].value.i32;
                break;
            case IXLAND_WASM_VALUE_I64:
                /* Cannot pass i64 in single argv slot */
                argv[i] = 0;
                break;
            default:
                argv[i] = 0;
                break;
        }
    }

    return iox_wamr_call_function(func_name, argc_wamr, argv);
}

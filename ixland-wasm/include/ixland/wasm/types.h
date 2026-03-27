#ifndef IXLAND_WASM_TYPES_H
#define IXLAND_WASM_TYPES_H

/* iXland Wasm - Public Types
 *
 * Fundamental types for the WebAssembly boundary.
 * This is the Wasm equivalent of libc's types.h.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * OPAQUE HANDLES
 * ============================================================================ */

/* WebAssembly engine handle */
typedef struct ixland_wasm_engine_s ixland_wasm_engine_t;

/* WebAssembly module handle (compiled bytecode) */
typedef struct ixland_wasm_module_s ixland_wasm_module_t;

/* WebAssembly instance handle (runtime state) */
typedef struct ixland_wasm_instance_s ixland_wasm_instance_t;

/* Host environment handle */
typedef struct ixland_wasm_host_s ixland_wasm_host_t;

/* ============================================================================
 * BACKEND KIND
 * ============================================================================ */

typedef enum {
    IXLAND_WASM_BACKEND_UNKNOWN = 0,
    IXLAND_WASM_BACKEND_WAMR,      /* WebAssembly Micro Runtime */
    IXLAND_WASM_BACKEND_WASMTIME,  /* Wasmtime (future) */
    IXLAND_WASM_BACKEND_WAMR_AOT   /* WAMR AOT compiled */
} ixland_wasm_backend_kind_t;

/* ============================================================================
 * EXIT KIND
 * ============================================================================ */

typedef enum {
    IXLAND_WASM_EXIT_UNKNOWN = 0,
    IXLAND_WASM_EXIT_NORMAL,       /* Normal completion */
    IXLAND_WASM_EXIT_TRAP,         /* WASM trap (unreachable, etc) */
    IXLAND_WASM_EXIT_HOST,         /* Host-requested exit */
    IXLAND_WASM_EXIT_SIGNAL,       /* Interrupted by signal */
    IXLAND_WASM_EXIT_RESOURCE      /* Resource limit exceeded */
} ixland_wasm_exit_kind_t;

/* ============================================================================
 * ERROR KIND
 * ============================================================================ */

typedef enum {
    IXLAND_WASM_OK = 0,
    IXLAND_WASM_ERROR_GENERIC = -1,
    IXLAND_WASM_ERROR_INVALID = -2,       /* Invalid argument */
    IXLAND_WASM_ERROR_NOMEM = -3,         /* Out of memory */
    IXLAND_WASM_ERROR_NOTFOUND = -4,      /* Module/function not found */
    IXLAND_WASM_ERROR_MALFORMED = -5,    /* Malformed WASM */
    IXLAND_WASM_ERROR_UNSUPPORTED = -6,  /* Unsupported feature */
    IXLAND_WASM_ERROR_RUNTIME = -7,      /* Runtime error */
    IXLAND_WASM_ERROR_TRAP = -8,          /* WASM trap */
    IXLAND_WASM_ERROR_INTERRUPTED = -9,  /* Execution interrupted */
    IXLAND_WASM_ERROR_IO = -10           /* I/O error */
} ixland_wasm_error_t;

/* ============================================================================
 * MODULE INFO
 * ============================================================================ */

typedef struct {
    uint32_t version;              /* WASM version (e.g., 0x01 for MVP) */
    uint32_t memories;             /* Number of memories */
    uint32_t tables;               /* Number of tables */
    uint32_t globals;            /* Number of globals */
    uint32_t functions;          /* Number of functions */
    uint32_t exports;            /* Number of exports */
    uint32_t imports;            /* Number of imports */
    bool has_start;              /* Has start function */
    bool is_wasi;                /* Uses WASI imports */
} ixland_wasm_module_info_t;

/* ============================================================================
 * EXECUTION RESULT
 * ============================================================================ */

typedef struct {
    ixland_wasm_exit_kind_t kind;
    int32_t exit_code;             /* Exit code if normal */
    ixland_wasm_error_t error;     /* Error code if abnormal */
    const char *message;           /* Optional message */
} ixland_wasm_result_t;

/* ============================================================================
 * MEMORY ACCESS
 * ============================================================================ */

/* Guest memory slice */
typedef struct {
    uint32_t offset;               /* Offset in guest memory */
    uint32_t size;                 /* Size in bytes */
} ixland_wasm_mem_slice_t;

/* ============================================================================
 * CALLBACK TYPES
 * ============================================================================ */

/* Import resolution callback */
typedef ixland_wasm_error_t (*ixland_wasm_import_fn_t)(
    const char *module,
    const char *name,
    void **out_func,
    void *userdata
);

/* Memory growth notification */
typedef void (*ixland_wasm_memory_grow_fn_t)(
    uint32_t old_pages,
    uint32_t new_pages,
    void *userdata
);

/* Execution hook */
typedef void (*ixland_wasm_hook_fn_t)(
    ixland_wasm_instance_t *instance,
    const char *func_name,
    void *userdata
);

#ifdef __cplusplus
}
#endif

#endif /* IXLAND_WASM_TYPES_H */

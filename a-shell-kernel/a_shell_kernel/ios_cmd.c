//
//  ios_cmd.c
//  ios_system
//
//  Runtime command registration API
//  Part of M1: Platform Hardening
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include "ios_system.h"

// ============================================================================
// COMMAND REGISTRY STATE
// ============================================================================

// Maximum number of commands in the registry
#define ASHELL_MAX_COMMANDS 1024

// Command entry structure
typedef struct {
    char name[NAME_MAX];           // Command name (e.g., "ls")
    char library[PATH_MAX];        // Library path (e.g., "ls.framework/ls")
    char entry_point[NAME_MAX];    // Entry point function (e.g., "ls_main")
    char auth_string[256];         // Authentication string (optional)
    char type[16];                 // Command type ("file", "directory", "no")
    void* handle;                  // dlopen handle (NULL for built-in)
    ashell_command_func_t func;    // Direct function pointer (NULL if from library)
    bool is_dynamic;               // True if loaded from dynamic library
    bool is_active;                // False if command was replaced/unregistered
} ashell_command_entry_t;

// Global command registry
static ashell_command_entry_t g_command_registry[ASHELL_MAX_COMMANDS];
static int g_num_commands = 0;
static pthread_mutex_t g_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_registry_initialized = false;

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Initialize the command registry
static void init_registry(void) {
    pthread_mutex_lock(&g_cmd_mutex);
    if (!g_registry_initialized) {
        memset(g_command_registry, 0, sizeof(g_command_registry));
        g_num_commands = 0;
        g_registry_initialized = true;
        ashell_trace(ASHELL_TRACE_DEBUG, "[CMD] Command registry initialized");
    }
    pthread_mutex_unlock(&g_cmd_mutex);
}

// Find command by name (internal, assumes mutex is held)
static int find_command_index(const char* name) {
    if (!name) return -1;

    for (int i = 0; i < g_num_commands; i++) {
        if (g_command_registry[i].is_active &&
            strcasecmp(g_command_registry[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Find first available slot (internal, assumes mutex is held)
static int find_free_slot(void) {
    // First check for inactive slots we can reuse
    for (int i = 0; i < g_num_commands; i++) {
        if (!g_command_registry[i].is_active) {
            return i;
        }
    }

    // Check if we have room to expand
    if (g_num_commands < ASHELL_MAX_COMMANDS) {
        return g_num_commands++;
    }

    return -1;  // Registry full
}

// ============================================================================
// PUBLIC API: C API
// ============================================================================

// Register a command with a direct function pointer
// Returns 0 on success, -1 on error
int ashell_register_command(const char* name,
                            const char* entry_point,
                            ashell_command_func_t func,
                            const char* type) {
    if (!name || !entry_point || !func) {
        ashell_trace(ASHELL_TRACE_ERROR, "[CMD] Invalid parameters for command registration");
        return -1;
    }

    init_registry();

    pthread_mutex_lock(&g_cmd_mutex);

    // Check if command already exists
    int existing = find_command_index(name);
    if (existing >= 0) {
        ashell_trace(ASHELL_TRACE_WARN, "[CMD] Command '%s' already registered, replacing", name);
        // Mark old entry as inactive
        g_command_registry[existing].is_active = false;
    }

    // Find a slot
    int slot = find_free_slot();
    if (slot < 0) {
        ashell_trace(ASHELL_TRACE_ERROR, "[CMD] Command registry full");
        pthread_mutex_unlock(&g_cmd_mutex);
        return -1;
    }

    // Fill in the entry
    ashell_command_entry_t* entry = &g_command_registry[slot];
    strncpy(entry->name, name, NAME_MAX - 1);
    entry->name[NAME_MAX - 1] = '\0';

    strncpy(entry->entry_point, entry_point, NAME_MAX - 1);
    entry->entry_point[NAME_MAX - 1] = '\0';

    entry->func = func;
    entry->handle = NULL;
    entry->is_dynamic = false;
    entry->is_active = true;

    if (type) {
        strncpy(entry->type, type, 15);
        entry->type[15] = '\0';
    } else {
        strcpy(entry->type, "no");
    }

    pthread_mutex_unlock(&g_cmd_mutex);

    ashell_trace(ASHELL_TRACE_INFO, "[CMD] Registered command: %s -> %s", name, entry_point);
    return 0;
}

// Register a command from a dynamic library
// The library is loaded and the entry point is resolved
int ashell_register_command_lib(const char* name,
                                 const char* library_path,
                                 const char* entry_point,
                                 const char* type) {
    if (!name || !library_path || !entry_point) {
        ashell_trace(ASHELL_TRACE_ERROR, "[CMD] Invalid parameters for library command registration");
        return -1;
    }

    init_registry();

    // Load the library
    void* handle = dlopen(library_path, RTLD_LAZY);
    if (!handle) {
        ashell_trace(ASHELL_TRACE_ERROR, "[CMD] Failed to load library '%s': %s",
                     library_path, dlerror());
        return -1;
    }

    // Resolve the entry point
    ashell_command_func_t func = (ashell_command_func_t)dlsym(handle, entry_point);
    if (!func) {
        ashell_trace(ASHELL_TRACE_ERROR, "[CMD] Entry point '%s' not found in '%s': %s",
                     entry_point, library_path, dlerror());
        dlclose(handle);
        return -1;
    }

    pthread_mutex_lock(&g_cmd_mutex);

    // Check if command already exists
    int existing = find_command_index(name);
    if (existing >= 0) {
        ashell_trace(ASHELL_TRACE_WARN, "[CMD] Command '%s' already registered, replacing", name);
        // Clean up old library handle if it was dynamic
        if (g_command_registry[existing].is_dynamic && g_command_registry[existing].handle) {
            dlclose(g_command_registry[existing].handle);
        }
        g_command_registry[existing].is_active = false;
    }

    // Find a slot
    int slot = find_free_slot();
    if (slot < 0) {
        ashell_trace(ASHELL_TRACE_ERROR, "[CMD] Command registry full");
        pthread_mutex_unlock(&g_cmd_mutex);
        dlclose(handle);
        return -1;
    }

    // Fill in the entry
    ashell_command_entry_t* entry = &g_command_registry[slot];
    strncpy(entry->name, name, NAME_MAX - 1);
    entry->name[NAME_MAX - 1] = '\0';

    strncpy(entry->library, library_path, PATH_MAX - 1);
    entry->library[PATH_MAX - 1] = '\0';

    strncpy(entry->entry_point, entry_point, NAME_MAX - 1);
    entry->entry_point[NAME_MAX - 1] = '\0';

    entry->func = func;
    entry->handle = handle;
    entry->is_dynamic = true;
    entry->is_active = true;

    if (type) {
        strncpy(entry->type, type, 15);
        entry->type[15] = '\0';
    } else {
        strcpy(entry->type, "no");
    }

    pthread_mutex_unlock(&g_cmd_mutex);

    ashell_trace(ASHELL_TRACE_INFO, "[CMD] Registered command from library: %s -> %s@%s",
                 name, entry_point, library_path);
    return 0;
}

// Unregister a command
// Returns 0 on success, -1 if command not found
int ashell_unregister_command(const char* name) {
    if (!name) return -1;

    init_registry();

    pthread_mutex_lock(&g_cmd_mutex);

    int index = find_command_index(name);
    if (index < 0) {
        pthread_mutex_unlock(&g_cmd_mutex);
        ashell_trace(ASHELL_TRACE_WARN, "[CMD] Cannot unregister '%s': not found", name);
        return -1;
    }

    ashell_command_entry_t* entry = &g_command_registry[index];

    // Clean up library handle if dynamic
    if (entry->is_dynamic && entry->handle) {
        dlclose(entry->handle);
    }

    entry->is_active = false;

    pthread_mutex_unlock(&g_cmd_mutex);

    ashell_trace(ASHELL_TRACE_INFO, "[CMD] Unregistered command: %s", name);
    return 0;
}

// Replace a command (legacy API, now calls register)
void ashell_replace_command(const char* name,
                            const char* entry_point,
                            ashell_command_func_t func,
                            const char* type) {
    ashell_register_command(name, entry_point, func, type);
}

// Check if a command is registered
bool ashell_command_exists(const char* name) {
    if (!name) return false;

    init_registry();

    pthread_mutex_lock(&g_cmd_mutex);
    int index = find_command_index(name);
    pthread_mutex_unlock(&g_cmd_mutex);

    return (index >= 0);
}

// Get command info
int ashell_get_command_info(const char* name, ashell_command_info_t* info) {
    if (!name || !info) return -1;

    init_registry();

    pthread_mutex_lock(&g_cmd_mutex);

    int index = find_command_index(name);
    if (index < 0) {
        pthread_mutex_unlock(&g_cmd_mutex);
        return -1;
    }

    ashell_command_entry_t* entry = &g_command_registry[index];
    strncpy(info->name, entry->name, NAME_MAX - 1);
    info->name[NAME_MAX - 1] = '\0';

    strncpy(info->entry_point, entry->entry_point, NAME_MAX - 1);
    info->entry_point[NAME_MAX - 1] = '\0';

    strncpy(info->type, entry->type, 15);
    info->type[15] = '\0';

    info->is_dynamic = entry->is_dynamic;

    pthread_mutex_unlock(&g_cmd_mutex);
    return 0;
}

// Execute a registered command
// Returns the command's exit code, or -1 if command not found
int ashell_execute_command(const char* name, int argc, char** argv) {
    if (!name) return -1;

    init_registry();

    pthread_mutex_lock(&g_cmd_mutex);

    int index = find_command_index(name);
    if (index < 0) {
        pthread_mutex_unlock(&g_cmd_mutex);
        return -1;
    }

    ashell_command_entry_t* entry = &g_command_registry[index];
    ashell_command_func_t func = entry->func;

    pthread_mutex_unlock(&g_cmd_mutex);

    if (!func) {
        ashell_trace(ASHELL_TRACE_ERROR, "[CMD] Command '%s' has NULL function pointer", name);
        return -1;
    }

    ashell_trace_command_start(name, ios_currentPid());
    int result = func(argc, argv);
    ashell_trace_command_end(name, ios_currentPid(), result);

    return result;
}

// List all registered commands
// Returns number of commands, fills names array (up to max_commands)
int ashell_list_commands(char** names, int max_commands) {
    if (!names || max_commands <= 0) return 0;

    init_registry();

    pthread_mutex_lock(&g_cmd_mutex);

    int count = 0;
    for (int i = 0; i < g_num_commands && count < max_commands; i++) {
        if (g_command_registry[i].is_active) {
            names[count] = strdup(g_command_registry[i].name);
            count++;
        }
    }

    pthread_mutex_unlock(&g_cmd_mutex);

    return count;
}

// Free command list returned by ashell_list_commands
void ashell_free_command_list(char** names, int count) {
    if (!names) return;
    for (int i = 0; i < count; i++) {
        free(names[i]);
    }
}

// ============================================================================
// REGISTRY MANAGEMENT
// ============================================================================

// Clear all dynamically loaded commands (keeps built-in)
void ashell_clear_dynamic_commands(void) {
    init_registry();

    pthread_mutex_lock(&g_cmd_mutex);

    for (int i = 0; i < g_num_commands; i++) {
        if (g_command_registry[i].is_active && g_command_registry[i].is_dynamic) {
            if (g_command_registry[i].handle) {
                dlclose(g_command_registry[i].handle);
            }
            g_command_registry[i].is_active = false;
        }
    }

    pthread_mutex_unlock(&g_cmd_mutex);

    ashell_trace(ASHELL_TRACE_INFO, "[CMD] Cleared all dynamic commands");
}

// Get registry statistics
void ashell_get_registry_stats(ashell_registry_stats_t* stats) {
    if (!stats) return;

    init_registry();

    pthread_mutex_lock(&g_cmd_mutex);

    stats->total_slots = ASHELL_MAX_COMMANDS;
    stats->used_slots = g_num_commands;
    stats->active_commands = 0;
    stats->dynamic_commands = 0;

    for (int i = 0; i < g_num_commands; i++) {
        if (g_command_registry[i].is_active) {
            stats->active_commands++;
            if (g_command_registry[i].is_dynamic) {
                stats->dynamic_commands++;
            }
        }
    }

    pthread_mutex_unlock(&g_cmd_mutex);
}

// ============================================================================
// BACKWARD COMPATIBILITY
// ============================================================================

// Legacy compatibility with existing ios_system code
// These delegate to the new API

void replaceCommand(NSString* commandName, NSString* functionName, bool allOccurences) {
    // Convert to C strings and register
    const char* name = [commandName UTF8String];
    const char* entry = [functionName UTF8String];

    // For now, this just logs - actual implementation would need
    // the function pointer which we don't have here
    ashell_trace(ASHELL_TRACE_WARN, "[CMD] Legacy replaceCommand called for '%s'", name);
}


//
//  ios_trace.c
//  ios_system
//
//  Structured trace hooks for debugging and monitoring
//  Part of M1: Platform Hardening
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include "a_shell_system.h"

// ============================================================================
// TRACE STATE
// ============================================================================

static ashell_trace_level_t g_trace_level = ASHELL_TRACE_ERROR;
static pthread_mutex_t g_trace_mutex = PTHREAD_MUTEX_INITIALIZER;

// Category flags
static struct {
    int commands;
    int sessions;
    int io;
    int env;
} g_categories = { 1, 1, 0, 0 };  // Default: commands and sessions

// ============================================================================
// TRACE LEVEL MANAGEMENT
// ============================================================================

void ashell_set_trace_level(ashell_trace_level_t level) {
    pthread_mutex_lock(&g_trace_mutex);
    g_trace_level = level;
    pthread_mutex_unlock(&g_trace_mutex);
}

ashell_trace_level_t ashell_get_trace_level(void) {
    pthread_mutex_lock(&g_trace_mutex);
    ashell_trace_level_t level = g_trace_level;
    pthread_mutex_unlock(&g_trace_mutex);
    return level;
}

// ============================================================================
// CATEGORY MANAGEMENT
// ============================================================================

void ashell_trace_enable_category(const char* category) {
    pthread_mutex_lock(&g_trace_mutex);
    if (strcmp(category, ASHELL_TRACE_CAT_COMMANDS) == 0) {
        g_categories.commands = 1;
    } else if (strcmp(category, ASHELL_TRACE_CAT_SESSIONS) == 0) {
        g_categories.sessions = 1;
    } else if (strcmp(category, ASHELL_TRACE_CAT_IO) == 0) {
        g_categories.io = 1;
    } else if (strcmp(category, ASHELL_TRACE_CAT_ENV) == 0) {
        g_categories.env = 1;
    }
    pthread_mutex_unlock(&g_trace_mutex);
}

void ashell_trace_disable_category(const char* category) {
    pthread_mutex_lock(&g_trace_mutex);
    if (strcmp(category, ASHELL_TRACE_CAT_COMMANDS) == 0) {
        g_categories.commands = 0;
    } else if (strcmp(category, ASHELL_TRACE_CAT_SESSIONS) == 0) {
        g_categories.sessions = 0;
    } else if (strcmp(category, ASHELL_TRACE_CAT_IO) == 0) {
        g_categories.io = 0;
    } else if (strcmp(category, ASHELL_TRACE_CAT_ENV) == 0) {
        g_categories.env = 0;
    }
    pthread_mutex_unlock(&g_trace_mutex);
}

// ============================================================================
// CORE TRACE FUNCTION
// ============================================================================

void ashell_trace(ashell_trace_level_t level, const char* fmt, ...) {
    // Fast path: skip if level is too low
    if (level > g_trace_level) {
        return;
    }

    pthread_mutex_lock(&g_trace_mutex);

    // Get timestamp
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // Level string
    const char* level_str = "UNKNOWN";
    switch (level) {
        case ASHELL_TRACE_NONE:  level_str = "NONE"; break;
        case ASHELL_TRACE_ERROR: level_str = "ERROR"; break;
        case ASHELL_TRACE_WARN:  level_str = "WARN"; break;
        case ASHELL_TRACE_INFO:  level_str = "INFO"; break;
        case ASHELL_TRACE_DEBUG: level_str = "DEBUG"; break;
    }

    // Print header
    fprintf(stderr, "[ASHELL %s %lld.%09ld] ",
            level_str,
            (long long)ts.tv_sec,
            (long)ts.tv_nsec);

    // Print message
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
    fflush(stderr);

    pthread_mutex_unlock(&g_trace_mutex);
}

// ============================================================================
// SPECIALIZED TRACE HOOKS
// ============================================================================

void ashell_trace_command_start(const char* cmd, pid_t pid) {
    if (!g_categories.commands || g_trace_level < ASHELL_TRACE_INFO) {
        return;
    }
    ashell_trace(ASHELL_TRACE_INFO, "[COMMANDS] Start: '%s' (pid=%d)", cmd, pid);
}

void ashell_trace_command_end(const char* cmd, pid_t pid, int status) {
    if (!g_categories.commands || g_trace_level < ASHELL_TRACE_INFO) {
        return;
    }
    ashell_trace(ASHELL_TRACE_INFO, "[COMMANDS] End: '%s' (pid=%d, status=%d)",
                 cmd, pid, status);
}

void ashell_trace_session_switch(const void* from_session, const void* to_session) {
    if (!g_categories.sessions || g_trace_level < ASHELL_TRACE_DEBUG) {
        return;
    }
    ashell_trace(ASHELL_TRACE_DEBUG, "[SESSIONS] Switch: %p -> %p",
                 from_session, to_session);
}

void ashell_trace_io_operation(const char* op, size_t bytes) {
    if (!g_categories.io || g_trace_level < ASHELL_TRACE_DEBUG) {
        return;
    }
    ashell_trace(ASHELL_TRACE_DEBUG, "[IO] %s: %zu bytes", op, bytes);
}

void ashell_trace_error(const char* context, const char* error) {
    // Always trace errors regardless of category
    if (g_trace_level >= ASHELL_TRACE_ERROR) {
        ashell_trace(ASHELL_TRACE_ERROR, "[ERROR] %s: %s", context, error);
    }
}

// ============================================================================
// INITIALIZATION
// ============================================================================

__attribute__((constructor))
static void trace_init(void) {
    // Read trace level from environment
    const char* trace_env = getenv("ASHELL_TRACE_LEVEL");
    if (trace_env) {
        if (strcmp(trace_env, "error") == 0) {
            g_trace_level = ASHELL_TRACE_ERROR;
        } else if (strcmp(trace_env, "warn") == 0) {
            g_trace_level = ASHELL_TRACE_WARN;
        } else if (strcmp(trace_env, "info") == 0) {
            g_trace_level = ASHELL_TRACE_INFO;
        } else if (strcmp(trace_env, "debug") == 0) {
            g_trace_level = ASHELL_TRACE_DEBUG;
        }
    }

    // Read categories from environment
    const char* cats = getenv("ASHELL_TRACE_CATEGORIES");
    if (cats) {
        g_categories.commands = strstr(cats, "commands") != NULL;
        g_categories.sessions = strstr(cats, "sessions") != NULL;
        g_categories.io = strstr(cats, "io") != NULL;
        g_categories.env = strstr(cats, "env") != NULL;
    }
}

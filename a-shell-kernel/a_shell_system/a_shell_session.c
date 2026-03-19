//
//  ios_session.c
//  ios_system
//
//  Thread-safe session management for a-Shell
//  Part of M1: Platform Hardening
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "a_shell_system.h"

// ============================================================================
// SESSION MANAGEMENT STATE
// ============================================================================

// Maximum number of concurrent sessions
#define ASHELL_MAX_SESSIONS 64

// Session reference counting for safe cleanup
typedef struct {
    void* session_id;              // Unique session identifier
    void* session_params;          // Pointer to sessionParameters (opaque)
    int ref_count;                 // Reference count for safe cleanup
    pthread_mutex_t lock;          // Per-session lock
    bool is_active;                // Session validity flag
    time_t created_at;             // Creation timestamp
    time_t last_accessed;          // Last access timestamp
} ashell_session_entry_t;

// Global session registry
static ashell_session_entry_t g_sessions[ASHELL_MAX_SESSIONS];
static int g_session_count = 0;
static pthread_mutex_t g_session_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t g_session_rwlock = PTHREAD_RWLOCK_INITIALIZER;
static bool g_sessions_initialized = false;

// Thread-local current session (from ios_system.m)
extern __thread void* currentSession;

// External session list (from ios_system.m)
// We can't easily replace this, but we can wrap access to it

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Initialize the session management system
static void init_session_system(void) {
    pthread_mutex_lock(&g_session_mutex);

    if (!g_sessions_initialized) {
        memset(g_sessions, 0, sizeof(g_sessions));
        for (int i = 0; i < ASHELL_MAX_SESSIONS; i++) {
            pthread_mutex_init(&g_sessions[i].lock, NULL);
            g_sessions[i].is_active = false;
        }
        g_session_count = 0;
        g_sessions_initialized = true;

        ashell_trace(ASHELL_TRACE_DEBUG, "[SESSION] Session management system initialized");
    }

    pthread_mutex_unlock(&g_session_mutex);
}

// Find session by ID (internal, caller must hold appropriate lock)
static int find_session_index_unlocked(const void* session_id) {
    if (!session_id) return -1;

    for (int i = 0; i < ASHELL_MAX_SESSIONS; i++) {
        if (g_sessions[i].is_active && g_sessions[i].session_id == session_id) {
            return i;
        }
    }
    return -1;
}

// Find or create free slot (internal, caller must hold g_session_mutex)
static int find_or_create_session_slot(void) {
    // First try to find an existing slot for this session
    // Then try to reuse inactive slots
    for (int i = 0; i < ASHELL_MAX_SESSIONS; i++) {
        if (!g_sessions[i].is_active) {
            return i;
        }
    }
    return -1;  // No slots available
}

// ============================================================================
// REFERENCE COUNTING
// ============================================================================

// Acquire a reference to a session
// Returns 0 on success, -1 if session not found
int ashell_session_acquire(const void* session_id) {
    if (!session_id) return -1;

    init_session_system();

    // Use read lock for lookup (allows concurrent reads)
    pthread_rwlock_rdlock(&g_session_rwlock);
    int index = find_session_index_unlocked(session_id);
    pthread_rwlock_unlock(&g_session_rwlock);

    if (index < 0) return -1;

    // Acquire per-session lock and increment ref count
    pthread_mutex_lock(&g_sessions[index].lock);
    if (g_sessions[index].is_active) {
        g_sessions[index].ref_count++;
        g_sessions[index].last_accessed = time(NULL);
        pthread_mutex_unlock(&g_sessions[index].lock);
        return 0;
    }
    pthread_mutex_unlock(&g_sessions[index].lock);
    return -1;
}

// Release a reference to a session
// Returns 0 on success, -1 if session not found
int ashell_session_release(const void* session_id) {
    if (!session_id) return -1;

    pthread_rwlock_rdlock(&g_session_rwlock);
    int index = find_session_index_unlocked(session_id);
    pthread_rwlock_unlock(&g_session_rwlock);

    if (index < 0) return -1;

    pthread_mutex_lock(&g_sessions[index].lock);
    if (g_sessions[index].is_active && g_sessions[index].ref_count > 0) {
        g_sessions[index].ref_count--;
    }
    pthread_mutex_unlock(&g_sessions[index].lock);

    return 0;
}

// ============================================================================
// SESSION REGISTRY API
// ============================================================================

// Register a new session
// Returns 0 on success, -1 on error
int ashell_session_register(const void* session_id, void* session_params) {
    if (!session_id || !session_params) {
        ashell_trace(ASHELL_TRACE_ERROR, "[SESSION] Invalid parameters for session registration");
        return -1;
    }

    init_session_system();

    pthread_rwlock_wrlock(&g_session_rwlock);

    // Check if session already exists
    int existing = find_session_index_unlocked(session_id);
    if (existing >= 0) {
        pthread_rwlock_unlock(&g_session_rwlock);
        ashell_trace(ASHELL_TRACE_WARN, "[SESSION] Session already registered: %p", session_id);
        return -1;  // Session already exists
    }

    // Find a free slot
    int slot = find_or_create_session_slot();
    if (slot < 0) {
        pthread_rwlock_unlock(&g_session_rwlock);
        ashell_trace(ASHELL_TRACE_ERROR, "[SESSION] Maximum number of sessions reached (%d)", ASHELL_MAX_SESSIONS);
        return -1;
    }

    // Initialize the session entry
    ashell_session_entry_t* entry = &g_sessions[slot];

    pthread_mutex_lock(&entry->lock);

    entry->session_id = (void*)session_id;
    entry->session_params = session_params;
    entry->ref_count = 1;  // Initial reference
    entry->is_active = true;
    entry->created_at = time(NULL);
    entry->last_accessed = entry->created_at;

    pthread_mutex_unlock(&entry->lock);

    g_session_count++;

    pthread_rwlock_unlock(&g_session_rwlock);

    ashell_trace_session_switch(NULL, session_id);
    ashell_trace(ASHELL_TRACE_INFO, "[SESSION] Registered session: %p (slot %d)", session_id, slot);

    return 0;
}

// Unregister a session (marks for cleanup when ref_count reaches 0)
// Returns 0 on success, -1 if not found
int ashell_session_unregister(const void* session_id) {
    if (!session_id) return -1;

    init_session_system();

    pthread_rwlock_wrlock(&g_session_rwlock);

    int index = find_session_index_unlocked(session_id);
    if (index < 0) {
        pthread_rwlock_unlock(&g_session_rwlock);
        ashell_trace(ASHELL_TRACE_WARN, "[SESSION] Cannot unregister: session not found: %p", session_id);
        return -1;
    }

    ashell_session_entry_t* entry = &g_sessions[index];

    pthread_mutex_lock(&entry->lock);

    entry->is_active = false;
    void* params = entry->session_params;
    int refs = entry->ref_count;

    pthread_mutex_unlock(&entry->lock);
    pthread_mutex_destroy(&entry->lock);
    pthread_mutex_init(&entry->lock, NULL);

    g_session_count--;

    pthread_rwlock_unlock(&g_session_rwlock);

    ashell_trace(ASHELL_TRACE_INFO, "[SESSION] Unregistered session: %p (refs remaining: %d)",
                  session_id, refs);

    // Note: Actual cleanup of session_params is deferred until ref_count reaches 0
    // The caller is responsible for checking ref_count and cleaning up

    return 0;
}

// Get session parameters with reference counting
// Returns session params on success, NULL if not found
void* ashell_session_get(const void* session_id) {
    if (!session_id) return NULL;

    init_session_system();

    pthread_rwlock_rdlock(&g_session_rwlock);
    int index = find_session_index_unlocked(session_id);

    if (index < 0) {
        pthread_rwlock_unlock(&g_session_rwlock);
        return NULL;
    }

    ashell_session_entry_t* entry = &g_sessions[index];

    pthread_mutex_lock(&entry->lock);

    if (!entry->is_active) {
        pthread_mutex_unlock(&entry->lock);
        pthread_rwlock_unlock(&g_session_rwlock);
        return NULL;
    }

    void* params = entry->session_params;
    entry->last_accessed = time(NULL);

    pthread_mutex_unlock(&entry->lock);
    pthread_rwlock_unlock(&g_session_rwlock);

    return params;
}

// Check if a session is registered and active
bool ashell_session_exists(const void* session_id) {
    if (!session_id) return false;

    init_session_system();

    pthread_rwlock_rdlock(&g_session_rwlock);
    int index = find_session_index_unlocked(session_id);
    bool exists = false;

    if (index >= 0) {
        pthread_mutex_lock(&g_sessions[index].lock);
        exists = g_sessions[index].is_active;
        pthread_mutex_unlock(&g_sessions[index].lock);
    }

    pthread_rwlock_unlock(&g_session_rwlock);

    return exists;
}

// Update session last accessed time
void ashell_session_touch(const void* session_id) {
    if (!session_id) return;

    pthread_rwlock_rdlock(&g_session_rwlock);
    int index = find_session_index_unlocked(session_id);

    if (index >= 0) {
        pthread_mutex_lock(&g_sessions[index].lock);
        g_sessions[index].last_accessed = time(NULL);
        pthread_mutex_unlock(&g_sessions[index].lock);
    }

    pthread_rwlock_unlock(&g_session_rwlock);
}

// ============================================================================
// THREAD-SAFE SESSION OPERATIONS
// ============================================================================

// Thread-safe version of ios_switchSession
// This wraps the original ios_switchSession with proper synchronization
void ashell_session_switch(const void* session_id) {
    if (!session_id) return;

    init_session_system();

    void* from_session = NULL;
    if (currentSession != NULL) {
        // Get current session ID from the session params
        // This is stored in the context field
        from_session = currentSession;  // Simplified - actual implementation would extract ID
    }

    // Trace the switch
    ashell_trace_session_switch(from_session, session_id);

    // Call the original ios_switchSession
    // Note: In actual integration, ios_switchSession would be refactored to use
    // the session registry directly. This wrapper provides a transition path.
    extern void ios_switchSession(const void* sessionId);

    pthread_rwlock_rdlock(&g_session_rwlock);

    // Verify target session exists before switching
    int index = find_session_index_unlocked(session_id);
    if (index >= 0) {
        pthread_mutex_lock(&g_sessions[index].lock);
        if (g_sessions[index].is_active) {
            g_sessions[index].last_accessed = time(NULL);
        }
        pthread_mutex_unlock(&g_sessions[index].lock);
    }

    pthread_rwlock_unlock(&g_session_rwlock);

    // Perform the actual switch (this may create the session if needed)
    ios_switchSession(session_id);
}

// Thread-safe version of ios_closeSession
void ashell_session_close(const void* session_id) {
    if (!session_id) return;

    init_session_system();

    pthread_rwlock_wrlock(&g_session_rwlock);

    int index = find_session_index_unlocked(session_id);
    if (index >= 0) {
        ashell_session_entry_t* entry = &g_sessions[index];

        pthread_mutex_lock(&entry->lock);

        // Mark inactive but don't cleanup yet if references exist
        entry->is_active = false;
        int refs = entry->ref_count;

        pthread_mutex_unlock(&entry->lock);

        if (refs == 0) {
            // Safe to cleanup immediately
            pthread_mutex_destroy(&entry->lock);
            pthread_mutex_init(&entry->lock, NULL);
            memset(entry, 0, sizeof(ashell_session_entry_t));
            pthread_mutex_init(&entry->lock, NULL);
        }

        g_session_count--;
    }

    pthread_rwlock_unlock(&g_session_rwlock);

    // Call original close
    extern void ios_closeSession(const void* sessionId);
    ios_closeSession(session_id);

    ashell_trace(ASHELL_TRACE_INFO, "[SESSION] Closed session: %p", session_id);
}

// ============================================================================
// SESSION STATISTICS AND CLEANUP
// ============================================================================

// Get session statistics
void ashell_session_stats(ashell_session_stats_t* stats) {
    if (!stats) return;

    init_session_system();

    pthread_rwlock_rdlock(&g_session_rwlock);

    stats->max_sessions = ASHELL_MAX_SESSIONS;
    stats->active_sessions = 0;
    stats->total_ref_count = 0;

    time_t now = time(NULL);
    stats->oldest_session_age = 0;

    for (int i = 0; i < ASHELL_MAX_SESSIONS; i++) {
        if (g_sessions[i].is_active) {
            stats->active_sessions++;
            stats->total_ref_count += g_sessions[i].ref_count;

            time_t age = now - g_sessions[i].created_at;
            if (age > stats->oldest_session_age) {
                stats->oldest_session_age = age;
            }
        }
    }

    pthread_rwlock_unlock(&g_session_rwlock);
}

// Cleanup stale sessions (sessions with no references and old access time)
// Returns number of sessions cleaned up
int ashell_session_cleanup_stale(int max_age_seconds) {
    if (max_age_seconds <= 0) return 0;

    init_session_system();

    int cleaned = 0;
    time_t now = time(NULL);

    pthread_rwlock_wrlock(&g_session_rwlock);

    for (int i = 0; i < ASHELL_MAX_SESSIONS; i++) {
        ashell_session_entry_t* entry = &g_sessions[i];

        if (!entry->is_active) continue;

        pthread_mutex_lock(&entry->lock);

        time_t idle_time = now - entry->last_accessed;

        if (entry->ref_count == 0 && idle_time > max_age_seconds) {
            // Stale session - cleanup
            entry->is_active = false;

            pthread_mutex_unlock(&entry->lock);
            pthread_mutex_destroy(&entry->lock);
            pthread_mutex_init(&entry->lock, NULL);

            g_session_count--;
            cleaned++;

            ashell_trace(ASHELL_TRACE_INFO, "[SESSION] Cleaned stale session (idle %lds)", idle_time);
        } else {
            pthread_mutex_unlock(&entry->lock);
        }
    }

    pthread_rwlock_unlock(&g_session_rwlock);

    return cleaned;
}

// Validate all sessions (for debugging)
// Returns number of invalid sessions found
int ashell_session_validate(void) {
    init_session_system();

    int invalid = 0;

    pthread_rwlock_rdlock(&g_session_rwlock);

    for (int i = 0; i < ASHELL_MAX_SESSIONS; i++) {
        ashell_session_entry_t* entry = &g_sessions[i];

        pthread_mutex_lock(&entry->lock);

        if (entry->is_active) {
            // Basic validation
            if (entry->session_id == NULL || entry->session_params == NULL) {
                invalid++;
                ashell_trace(ASHELL_TRACE_ERROR,
                             "[SESSION] Invalid session at slot %d: id=%p params=%p",
                             i, entry->session_id, entry->session_params);
            }

            if (entry->ref_count < 0) {
                invalid++;
                ashell_trace(ASHELL_TRACE_ERROR,
                             "[SESSION] Negative ref_count at slot %d: %d",
                             i, entry->ref_count);
            }
        }

        pthread_mutex_unlock(&entry->lock);
    }

    pthread_rwlock_unlock(&g_session_rwlock);

    return invalid;
}


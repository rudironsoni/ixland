//
//  ios_env.c
//  ios_system
//
//  Standardized environment variable management for a-Shell
//  Part of M1: Platform Hardening
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Foundation/Foundation.h>
#include "a_shell_system.h"

// ============================================================================
// ENVIRONMENT STATE
// ============================================================================

static pthread_mutex_t g_env_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_env_initialized = 0;

// Standard environment variable names
#define ASHELL_ENV_PREFIX       "ASHELL_PREFIX"
#define ASHELL_ENV_HOME         "HOME"
#define ASHELL_ENV_TMPDIR       "TMPDIR"
#define ASHELL_ENV_PATH         "PATH"
#define ASHELL_ENV_TERM         "TERM"
#define ASHELL_ENV_SHELL        "SHELL"
#define ASHELL_ENV_PWD          "PWD"
#define ASHELL_ENV_OLDPWD       "OLDPWD"
#define ASHELL_ENV_USER         "USER"

// Default values
#define ASHELL_DEFAULT_TERM     "xterm-256color"
#define ASHELL_DEFAULT_SHELL    "sh"

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Thread-safe environment variable setting
static void ashell_setenv_safe(const char* name, const char* value, int overwrite) {
    pthread_mutex_lock(&g_env_mutex);
    setenv(name, value, overwrite);
    pthread_mutex_unlock(&g_env_mutex);
}

// Get the a-Shell PREFIX path
// Returns: ~/Library/ashell (or user-defined ASHELL_PREFIX)
static NSString* ashell_get_prefix_path(void) {
    // Check if ASHELL_PREFIX is already set
    const char* existing = getenv(ASHELL_ENV_PREFIX);
    if (existing != NULL) {
        return [NSString stringWithUTF8String:existing];
    }

    // Default: ~/Library/ashell
    NSString* libPath = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) lastObject];
    return [libPath stringByAppendingPathComponent:@"ashell"];
}

// Get the a-Shell configuration directory
// Returns: ~/Documents/.ashell
static NSString* ashell_get_config_path(void) {
    NSString* docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    return [docsPath stringByAppendingPathComponent:@".ashell"];
}

// Build PATH with standard directories
// Order: PREFIX/bin:~/Documents/bin:~/Library/bin:APPDIR/bin:APPDIR/Library/bin:system PATH
static NSString* ashell_build_path(NSString* prefix) {
    NSMutableArray* pathComponents = [NSMutableArray array];

    // 1. PREFIX/bin (highest priority for a-Shell packages)
    [pathComponents addObject:[prefix stringByAppendingPathComponent:@"bin"]];

    // 2. User directories (for user-installed scripts)
    NSString* docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString* libPath = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) lastObject];
    [pathComponents addObject:[docsPath stringByAppendingPathComponent:@"bin"]];
    [pathComponents addObject:[libPath stringByAppendingPathComponent:@"bin"]];

    // 3. App bundle directories
    NSString* appPath = [[NSBundle mainBundle] resourcePath];
    [pathComponents addObject:[appPath stringByAppendingPathComponent:@"bin"]];
    [pathComponents addObject:[[appPath stringByAppendingPathComponent:@"Library"] stringByAppendingPathComponent:@"bin"]];

    // 4. System PATH (lowest priority)
    const char* systemPath = getenv("PATH");
    if (systemPath != NULL) {
        [pathComponents addObject:[NSString stringWithUTF8String:systemPath]];
    }

    return [pathComponents componentsJoinedByString:@":"];
}

// Create directory structure if it doesn't exist
static void ashell_ensure_directories(NSString* prefix, NSString* config) {
    NSFileManager* fm = [NSFileManager defaultManager];
    NSError* error = nil;

    // Create PREFIX directories
    NSArray* prefixDirs = @[
        prefix,
        [prefix stringByAppendingPathComponent:@"bin"],
        [prefix stringByAppendingPathComponent:@"lib"],
        [prefix stringByAppendingPathComponent:@"include"],
        [prefix stringByAppendingPathComponent:@"libexec"],
        [prefix stringByAppendingPathComponent:@"share"],
    ];

    for (NSString* dir in prefixDirs) {
        if (![fm fileExistsAtPath:dir]) {
            [fm createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:&error];
            if (error) {
                ashell_trace(ASHELL_TRACE_WARN, "Failed to create directory: %s", [dir UTF8String]);
                error = nil;
            }
        }
    }

    // Create config directories
    NSArray* configDirs = @[
        config,
        [config stringByAppendingPathComponent:@"etc"],
        [config stringByAppendingPathComponent:@"var/log"],
        [config stringByAppendingPathComponent:@"tmp"],
    ];

    for (NSString* dir in configDirs) {
        if (![fm fileExistsAtPath:dir]) {
            [fm createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:&error];
            if (error) {
                ashell_trace(ASHELL_TRACE_WARN, "Failed to create directory: %s", [dir UTF8String]);
                error = nil;
            }
        }
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

// Initialize the standardized environment
// This should be called once at app startup, before any commands run
void ashell_env_initialize(void) {
    pthread_mutex_lock(&g_env_mutex);

    if (g_env_initialized) {
        pthread_mutex_unlock(&g_env_mutex);
        return;
    }

    ashell_trace(ASHELL_TRACE_INFO, "[ENV] Initializing a-Shell environment");

    // Get paths
    NSString* prefix = ashell_get_prefix_path();
    NSString* config = ashell_get_config_path();
    NSString* docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString* libPath = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) lastObject];

    // Ensure directory structure exists
    ashell_ensure_directories(prefix, config);

    // Set ASHELL_PREFIX
    setenv(ASHELL_ENV_PREFIX, [prefix UTF8String], 1);
    ashell_trace(ASHELL_TRACE_DEBUG, "[ENV] %s=%s", ASHELL_ENV_PREFIX, [prefix UTF8String]);

    // Set ASHELL_CONFIG
    setenv("ASHELL_CONFIG", [config UTF8String], 1);
    ashell_trace(ASHELL_TRACE_DEBUG, "[ENV] ASHELL_CONFIG=%s", [config UTF8String]);

    // Build and set PATH
    NSString* path = ashell_build_path(prefix);
    setenv(ASHELL_ENV_PATH, [path UTF8String], 1);
    ashell_trace(ASHELL_TRACE_DEBUG, "[ENV] %s=%s", ASHELL_ENV_PATH, [path UTF8String]);

    // Set standard Unix variables
    const char* home = getenv(ASHELL_ENV_HOME);
    if (home == NULL) {
        // iOS sets HOME, but ensure it's set
        setenv(ASHELL_ENV_HOME, [docsPath UTF8String], 1);
    }

    // Set TMPDIR if not set
    if (getenv(ASHELL_ENV_TMPDIR) == NULL) {
        NSString* tmpPath = [config stringByAppendingPathComponent:@"tmp"];
        setenv(ASHELL_ENV_TMPDIR, [tmpPath UTF8String], 1);
    }

    // Set TERM
    setenv(ASHELL_ENV_TERM, ASHELL_DEFAULT_TERM, 1);

    // Set SHELL
    setenv(ASHELL_ENV_SHELL, ASHELL_DEFAULT_SHELL, 0);

    // Set USER (iOS apps run as 'mobile' user)
    if (getenv(ASHELL_ENV_USER) == NULL) {
        setenv(ASHELL_ENV_USER, "mobile", 0);
    }

    // Configure colors
    setenv("CLICOLOR", "1", 1);
    setenv("LSCOLORS", "ExFxBxDxCxegedabagacad", 0);
    setenv("GREP_COLOR", "1;33", 0);

    // Set application directories
    setenv("APPDIR", [[NSBundle mainBundle] resourcePath].UTF8String, 1);

    // XDG directories (redirect to iOS-appropriate locations)
    setenv("XDG_CACHE_HOME", [libPath UTF8String], 1);
    setenv("XDG_CONFIG_HOME", [config stringByAppendingPathComponent:@"etc"].UTF8String, 1);
    setenv("XDG_DATA_HOME", [prefix stringByAppendingPathComponent:@"share"].UTF8String, 1);
    setenv("XDG_STATE_HOME", [config stringByAppendingPathComponent:@"var"].UTF8String, 1);

    // SSH/CURL/DIG config directories (in Documents for user accessibility)
    setenv("SSH_HOME", docsPath.UTF8String, 0);
    setenv("CURL_HOME", docsPath.UTF8String, 0);
    setenv("DIG_HOME", docsPath.UTF8String, 0);
    setenv("SSL_CERT_FILE", [docsPath stringByAppendingPathComponent:@"cacert.pem"].UTF8String, 0);
    setenv("CURLOPT_SSH_KNOWNHOSTS", [docsPath stringByAppendingPathComponent:@".ssh/known_hosts"].UTF8String, 0);

    // Locale (use system locale if available, otherwise C)
    if (getenv("LC_ALL") == NULL && getenv("LANG") == NULL) {
        setenv("LANG", "en_US.UTF-8", 0);
    }

    g_env_initialized = 1;
    pthread_mutex_unlock(&g_env_mutex);

    ashell_trace(ASHELL_TRACE_INFO, "[ENV] Environment initialization complete");
}

// Check if environment has been initialized
int ashell_env_is_initialized(void) {
    pthread_mutex_lock(&g_env_mutex);
    int result = g_env_initialized;
    pthread_mutex_unlock(&g_env_mutex);
    return result;
}

// Get the PREFIX path
const char* ashell_env_get_prefix(void) {
    return getenv(ASHELL_ENV_PREFIX);
}

// Get the CONFIG path
const char* ashell_env_get_config(void) {
    return getenv("ASHELL_CONFIG");
}

// Get PATH as an array of strings
// Returns: malloc'd array of strings, caller must free
char** ashell_env_get_path_array(int* count) {
    const char* path = getenv(ASHELL_ENV_PATH);
    if (path == NULL) {
        if (count) *count = 0;
        return NULL;
    }

    // Count components
    int n = 1;
    for (const char* p = path; *p; p++) {
        if (*p == ':') n++;
    }

    // Allocate array
    char** result = malloc((n + 1) * sizeof(char*));
    if (!result) return NULL;

    // Copy and split
    char* pathCopy = strdup(path);
    char* token = strtok(pathCopy, ":");
    int i = 0;
    while (token && i < n) {
        result[i] = strdup(token);
        token = strtok(NULL, ":");
        i++;
    }
    result[i] = NULL;
    if (count) *count = i;

    free(pathCopy);
    return result;
}

// Free path array returned by ashell_env_get_path_array
void ashell_env_free_path_array(char** paths) {
    if (!paths) return;
    for (int i = 0; paths[i]; i++) {
        free(paths[i]);
    }
    free(paths);
}

// Add a directory to PATH
// Returns 0 on success, -1 on failure
int ashell_env_path_append(const char* directory) {
    if (!directory) return -1;

    const char* current = getenv(ASHELL_ENV_PATH);
    if (!current) return -1;

    // Check if already in PATH
    if (strstr(current, directory) != NULL) {
        return 0;  // Already present
    }

    size_t newLen = strlen(current) + strlen(directory) + 2;  // +2 for ':' and '\0'
    char* newPath = malloc(newLen);
    if (!newPath) return -1;

    snprintf(newPath, newLen, "%s:%s", current, directory);
    ashell_setenv_safe(ASHELL_ENV_PATH, newPath, 1);
    free(newPath);

    ashell_trace(ASHELL_TRACE_DEBUG, "[ENV] Appended to PATH: %s", directory);
    return 0;
}

// Prepend a directory to PATH (higher priority)
int ashell_env_path_prepend(const char* directory) {
    if (!directory) return -1;

    const char* current = getenv(ASHELL_ENV_PATH);
    if (!current) return -1;

    // Check if already in PATH
    if (strstr(current, directory) != NULL) {
        return 0;  // Already present
    }

    size_t newLen = strlen(directory) + strlen(current) + 2;
    char* newPath = malloc(newLen);
    if (!newPath) return -1;

    snprintf(newPath, newLen, "%s:%s", directory, current);
    ashell_setenv_safe(ASHELL_ENV_PATH, newPath, 1);
    free(newPath);

    ashell_trace(ASHELL_TRACE_DEBUG, "[ENV] Prepended to PATH: %s", directory);
    return 0;
}

// Remove a directory from PATH
int ashell_env_path_remove(const char* directory) {
    if (!directory) return -1;

    const char* current = getenv(ASHELL_ENV_PATH);
    if (!current) return -1;

    // Find directory in PATH
    const char* found = strstr(current, directory);
    if (!found) return 0;  // Not present

    // Build new PATH without this component
    size_t dirLen = strlen(directory);
    size_t curLen = strlen(current);
    char* newPath = malloc(curLen + 1);
    if (!newPath) return -1;

    char* dst = newPath;
    const char* src = current;

    while (*src) {
        // Check if current position matches directory
        if (strncmp(src, directory, dirLen) == 0 && (src[dirLen] == ':' || src[dirLen] == '\0')) {
            // Skip this component
            src += dirLen;
            if (*src == ':') src++;  // Skip trailing colon
            if (dst > newPath && *src && *(dst-1) == ':') {
                dst--;  // Remove leading colon if not at start
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';

    // Remove trailing colon if present
    if (dst > newPath && *(dst-1) == ':') {
        *(dst-1) = '\0';
    }

    ashell_setenv_safe(ASHELL_ENV_PATH, newPath, 1);
    free(newPath);

    ashell_trace(ASHELL_TRACE_DEBUG, "[ENV] Removed from PATH: %s", directory);
    return 0;
}

// Print environment info (for debugging)
void ashell_env_print(void) {
    printf("a-Shell Environment:\n");
    printf("  ASHELL_PREFIX=%s\n", ashell_env_get_prefix() ? ashell_env_get_prefix() : "(not set)");
    printf("  ASHELL_CONFIG=%s\n", ashell_env_get_config() ? ashell_env_get_config() : "(not set)");
    printf("  HOME=%s\n", getenv("HOME") ? getenv("HOME") : "(not set)");
    printf("  PATH=%s\n", getenv("PATH") ? getenv("PATH") : "(not set)");
    printf("  TMPDIR=%s\n", getenv("TMPDIR") ? getenv("TMPDIR") : "(not set)");
    printf("  TERM=%s\n", getenv("TERM") ? getenv("TERM") : "(not set)");
    printf("  SHELL=%s\n", getenv("SHELL") ? getenv("SHELL") : "(not set)");
    printf("  USER=%s\n", getenv("USER") ? getenv("USER") : "(not set)");
}


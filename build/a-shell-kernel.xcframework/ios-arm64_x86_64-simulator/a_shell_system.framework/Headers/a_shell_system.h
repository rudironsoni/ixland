#ifndef A_SHELL_SYSTEM_INTERNAL_H
#define A_SHELL_SYSTEM_INTERNAL_H

//
//  ios_system.h
//  ios_system
//
//  Created by Nicolas Holzschuch on 04/12/2017.
//  Copyright © 2017 Nicolas Holzschuch. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

//! Project version number for ios_system.
FOUNDATION_EXPORT double ios_systemVersionNumber;

//! Project version string for ios_system.
FOUNDATION_EXPORT const unsigned char ios_systemVersionString[];

// Thread-local input and output streams
extern __thread FILE* thread_stdin;
extern __thread FILE* thread_stdout;
extern __thread FILE* thread_stderr;
extern __thread void* thread_context;

// Session parameters structure (must match definition in a_shell_system.m)
typedef struct _sessionParameters {
    bool isMainThread;
    char currentDir[MAXPATHLEN];
    char previousDirectory[MAXPATHLEN];
    char localMiniRoot[MAXPATHLEN];
    pthread_t current_command_root_thread;
    pthread_t lastThreadId;
    pthread_t mainThreadId;
    FILE* stdin;
    FILE* stdout;
    FILE* stderr;
    FILE* tty;
    const void* context;
    int global_errno;
    int numCommandsAllocated;
    int numCommand;
    char** commandName;
    char columns[5];
    char lines[5];
    bool activePager;
} sessionParameters;

// Current session pointer (defined in a_shell_system.m)
extern __thread sessionParameters* currentSession;

// rust doesn't support extern __thread vars yet
// see https://github.com/rust-lang/rust/issues/30795
// so we provide function accessors for them.
extern FILE* ios_stdin(void);
extern FILE* ios_stdout(void);
extern FILE* ios_stderr(void);
extern void* ios_context(void);

// set to true to have more commands available, more debugging information.
extern bool sideLoading;
// set to false to have the main thread run in detached mode (non blocking)
extern bool joinMainThread;

extern int ios_executable(const char* inputCmd); // does this command exist? (executable file or builtin command)
extern int ios_system(const char* inputCmd); // execute this command (executable file or builtin command)
extern FILE *ios_popen(const char *command, const char *type); // Execute this command and pipe the result
extern int ios_kill(void); // kill the current running command
extern int ios_killpid(pid_t pid, int sig); // kill the current running command
extern int chdir(const char* path);

extern int ios_isatty(int fd); // test whether a file descriptor refers to a terminal
extern pthread_t ios_getLastThreadId(void);
extern pthread_t ios_getThreadId(pid_t pid);
extern void ios_storeThreadId(pthread_t thread);
extern void ios_releaseThread(pthread_t thread);
extern void ios_releaseThreadId(pid_t pid);
extern pid_t ios_currentPid(void);
extern int ios_getCommandStatus(void);
extern const char* ios_progname(void);
extern char * ios_getenv(const char *name);
extern int ios_setenv(const char* variableName, const char* value, int overwrite);
int ios_unsetenv(const char* variableName);
extern char** environmentVariables(pid_t pid);
extern NSArray* environmentAsArray(void);
extern void storeEnvironment(char* envp[]);
extern pid_t ios_fork(void);
extern void ios_waitpid(pid_t pid);
extern pid_t ios_full_waitpid(pid_t pid, int *stat_loc, int options);
extern NSString *ios_getLogicalPWD(const void* sessionId);
void ios_setWindowSize(int width, int height, const void* sessionId);

extern NSString* commandsAsString(void);
extern NSArray* commandsAsArray(void);      // set of all commands, in an NSArray
extern NSArray* aliasesAsArray(void);       // set of all aliases defined, in an NSArray
extern NSString* aliasedCommand(NSString* command); // if the command is aliased, return the command it points to
extern NSString* getoptString(NSString* command);
extern NSString* operatesOn(NSString* command);
extern void initializeEnvironment(void);
extern int ios_setMiniRoot(NSString*);  // restricts operations to a certain hierarchy
extern int ios_setMiniRootURL(NSURL*);  // restricts operations to a certain hierarchy
extern int ios_setAllowedPaths(NSArray<NSString *> *paths);  // restricts operations to a certain hierarchy
extern void ios_setBookmarkDictionaryName(NSString*);  // name of the dictionary in user preferences, holding the bookmarks.
extern void ios_switchSession(const void* sessionid);
extern void ios_closeSession(const void* sessionid);
extern void ios_setStreams(FILE* _stdin, FILE* _stdout, FILE* _stderr);
extern void ios_settty(FILE* _tty);
extern int ios_getstdin(void);
extern int ios_gettty(void);
extern int ios_activePager(void);
extern void ios_setContext(const void *context);
extern const void* ios_getContext(void);
extern void ios_setDirectoryURL(NSURL* workingDirectoryURL);
extern void newPreviousDirectory(void);
extern void makeGlobal(void);
extern void makeLocal(void);
extern void replaceCommand(NSString* commandName, NSString* functionName, bool allOccurences);
extern NSError* addCommandList(NSString* fileLocation);
extern NSArray* backgroundCommandList;
extern int numPythonInterpreters;
extern int numPerlInterpreters;
extern int numTeXInterpreters;
extern int cd_main(int argc, char** argv);
extern const char* ios_getBookmarkedVersion(const char* p);
extern void ios_stopInteractive(void);
extern void finishedPreparingWebAssemblyCommand(void);
extern int webAssemblyCommandOrder(void);

// ============================================================================
// TRACE HOOKS (M1-I1: Structured tracing for debugging)
// ============================================================================

typedef enum {
    ASHELL_TRACE_NONE = 0,
    ASHELL_TRACE_ERROR,
    ASHELL_TRACE_WARN,
    ASHELL_TRACE_INFO,
    ASHELL_TRACE_DEBUG
} ashell_trace_level_t;

// Set global trace level
extern void ashell_set_trace_level(ashell_trace_level_t level);
extern ashell_trace_level_t ashell_get_trace_level(void);

// Trace with format string
extern void ashell_trace(ashell_trace_level_t level, const char* fmt, ...);

// Specialized trace hooks
extern void ashell_trace_command_start(const char* cmd, pid_t pid);
extern void ashell_trace_command_end(const char* cmd, pid_t pid, int status);
extern void ashell_trace_session_switch(const void* from_session, const void* to_session);
extern void ashell_trace_io_operation(const char* op, size_t bytes);
extern void ashell_trace_error(const char* context, const char* error);

// Enable/disable trace categories
extern void ashell_trace_enable_category(const char* category);
extern void ashell_trace_disable_category(const char* category);

// Trace categories
#define ASHELL_TRACE_CAT_COMMANDS   "commands"
#define ASHELL_TRACE_CAT_SESSIONS   "sessions"
#define ASHELL_TRACE_CAT_IO         "io"
#define ASHELL_TRACE_CAT_ENV        "environment"

// ============================================================================
// ENVIRONMENT MANAGEMENT (M1-I2: Standardized environment initialization)
// ============================================================================

// Initialize the standardized a-Shell environment
// Must be called once at app startup before any commands run
extern void ashell_env_initialize(void);

// Check if environment has been initialized
extern int ashell_env_is_initialized(void);

// Get the PREFIX path (e.g., ~/Library/ashell)
extern const char* ashell_env_get_prefix(void);

// Get the CONFIG path (e.g., ~/Documents/.ashell)
extern const char* ashell_env_get_config(void);

// PATH manipulation
// Get PATH as an array of strings (caller must free with ashell_env_free_path_array)
extern char** ashell_env_get_path_array(int* count);
extern void ashell_env_free_path_array(char** paths);

// Modify PATH (thread-safe)
extern int ashell_env_path_append(const char* directory);   // Add to end (lower priority)
extern int ashell_env_path_prepend(const char* directory);   // Add to start (higher priority)
extern int ashell_env_path_remove(const char* directory);    // Remove from PATH

// Print environment info (for debugging)
extern void ashell_env_print(void);

// Environment constants
#define ASHELL_PREFIX_ENV_VAR   "ASHELL_PREFIX"
#define ASHELL_CONFIG_ENV_VAR   "ASHELL_CONFIG"

// ============================================================================
// COMMAND REGISTRATION API (M1-I3: Runtime command registration)
// ============================================================================

// Command function type: int func(int argc, char** argv)
typedef int (*ashell_command_func_t)(int argc, char** argv);

// Command information structure
typedef struct {
    char name[256];           // Command name
    char entry_point[256];    // Entry point function name
    char type[16];            // "file", "directory", or "no"
    bool is_dynamic;          // Loaded from dynamic library
} ashell_command_info_t;

// Registry statistics
typedef struct {
    int total_slots;          // Total capacity
    int used_slots;           // Slots used (active + inactive)
    int active_commands;        // Currently active commands
    int dynamic_commands;     // Loaded from libraries
} ashell_registry_stats_t;

// Register a command with a direct function pointer
// Returns 0 on success, -1 on error
extern int ashell_register_command(const char* name,
                                   const char* entry_point,
                                   ashell_command_func_t func,
                                   const char* type);

// Register a command from a dynamic library
// Loads the library and resolves the entry point
extern int ashell_register_command_lib(const char* name,
                                       const char* library_path,
                                       const char* entry_point,
                                       const char* type);

// Unregister a command
// Returns 0 on success, -1 if not found
extern int ashell_unregister_command(const char* name);

// Replace a command (legacy compatibility, calls register)
extern void ashell_replace_command(const char* name,
                                   const char* entry_point,
                                   ashell_command_func_t func,
                                   const char* type);

// Check if a command is registered
extern bool ashell_command_exists(const char* name);

// Get command information
// Returns 0 on success, -1 if not found
extern int ashell_get_command_info(const char* name, ashell_command_info_t* info);

// Execute a registered command
// Returns command's exit code, or -1 if not found
extern int ashell_execute_command(const char* name, int argc, char** argv);

// List all registered commands
// Returns count, fills names array (caller must free strings)
extern int ashell_list_commands(char** names, int max_commands);
extern void ashell_free_command_list(char** names, int count);

// Clear all dynamically loaded commands
extern void ashell_clear_dynamic_commands(void);

// Get registry statistics
extern void ashell_get_registry_stats(ashell_registry_stats_t* stats);

// ============================================================================
// SYSTEM INFO API (M4-I1: Library APIs for system info)
// ============================================================================

// System information structure (replaces /proc/cpuinfo, /proc/meminfo)
typedef struct {
    uint64_t total_ram;           // Total physical RAM
    uint64_t available_ram;       // Available/free RAM
    uint32_t cpu_count;           // Number of CPU cores
    char cpu_arch[16];            // CPU architecture (e.g., "arm64")
    char os_version[32];          // iOS version string
} ios_sys_info_t;

// Process information structure (replaces /proc/PID/)
typedef struct {
    pid_t pid;                    // Process ID
    pid_t ppid;                   // Parent process ID
    uid_t uid;                    // User ID
    gid_t gid;                    // Group ID
    char name[256];               // Process name
    uint64_t memory_usage;        // Resident memory in bytes
} ios_proc_info_t;

// Get system information
// Thread-safe, cached for 5 seconds
extern ios_sys_info_t ios_getsys_info(void);

// Format system info as string (caller must free)
extern char* ios_sysinfo_format(const ios_sys_info_t* info);

// Print system info to stdout
extern void ios_print_sysinfo(void);

// Get process information for a specific PID
// Returns allocated struct (caller must free with ios_freeproc_info), or NULL
extern ios_proc_info_t* ios_getproc_info(pid_t pid);

// Free process info returned by ios_getproc_info
extern void ios_freeproc_info(ios_proc_info_t* info);

// Get current process info
extern ios_proc_info_t* ios_getproc_self(void);

// List all running processes
// Returns array of PIDs (caller must free), sets count
extern pid_t* ios_list_processes(int* count);

// Format process info as string (caller must free)
extern char* ios_procinfo_format(const ios_proc_info_t* info);

// Print process info to stdout
extern void ios_print_procinfo(pid_t pid);

// ============================================================================
// SESSION MANAGEMENT (M1-I4: Thread-safe session management)
// ============================================================================

// Session statistics structure
typedef struct {
    int max_sessions;             // Maximum concurrent sessions
    int active_sessions;          // Currently active sessions
    int total_ref_count;          // Sum of all reference counts
    time_t oldest_session_age;    // Age of oldest session in seconds
} ashell_session_stats_t;

// Session lifecycle
extern int ashell_session_register(const void* session_id, void* session_params);
extern int ashell_session_unregister(const void* session_id);
extern bool ashell_session_exists(const void* session_id);

// Reference counting for safe access
extern int ashell_session_acquire(const void* session_id);
extern int ashell_session_release(const void* session_id);

// Session access
extern void* ashell_session_get(const void* session_id);
extern void ashell_session_touch(const void* session_id);

// Thread-safe session operations
extern void ashell_session_switch(const void* session_id);
extern void ashell_session_close(const void* session_id);

// Session maintenance
extern void ashell_session_stats(ashell_session_stats_t* stats);
extern int ashell_session_cleanup_stale(int max_age_seconds);
extern int ashell_session_validate(void);

#endif /* A_SHELL_SYSTEM_INTERNAL_H */

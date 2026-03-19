//
//  ios_sysinfo.c
//  a_shell_system
//
//  System information APIs for iOS (replaces /proc filesystem)
//  Part of M4: Interactive UX & System Info
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <pthread.h>
#include "a_shell_system.h"

// Process info APIs are only available on macOS, not iOS
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#include <libproc.h>
#include <sys/proc_info.h>
#endif

// ============================================================================
// SYSTEM INFO IMPLEMENTATION
// ============================================================================

// Cache for system info (doesn't change often)
static a_shell_sys_info_t g_sysinfo_cache;
static time_t g_sysinfo_cache_time = 0;
static pthread_mutex_t g_sysinfo_mutex = PTHREAD_MUTEX_INITIALIZER;
static const int SYSINFO_CACHE_TTL = 5; // Cache for 5 seconds

// Get system info using iOS native APIs
// This replaces reading from /proc/cpuinfo, /proc/meminfo, etc.
a_shell_sys_info_t a_shell_getsys_info(void) {
    pthread_mutex_lock(&g_sysinfo_mutex);

    time_t now = time(NULL);

    // Return cached value if still valid
    if (g_sysinfo_cache_time > 0 && (now - g_sysinfo_cache_time) < SYSINFO_CACHE_TTL) {
        a_shell_sys_info_t result = g_sysinfo_cache;
        pthread_mutex_unlock(&g_sysinfo_mutex);
        return result;
    }

    a_shell_sys_info_t info;
    memset(&info, 0, sizeof(info));

    // Get CPU count
    int cpu_count;
    size_t len = sizeof(cpu_count);
    if (sysctlbyname("hw.ncpu", &cpu_count, &len, NULL, 0) == 0) {
        info.cpu_count = cpu_count;
    } else {
        info.cpu_count = 1; // Default to 1 CPU
    }

    // Get CPU architecture
    char cpu_arch[16];
    len = sizeof(cpu_arch);
    if (sysctlbyname("hw.machine", cpu_arch, &len, NULL, 0) == 0) {
        strncpy(info.cpu_arch, cpu_arch, 15);
        info.cpu_arch[15] = '\0';
    } else {
        strcpy(info.cpu_arch, "unknown");
    }

    // Get total RAM
    int64_t total_ram;
    len = sizeof(total_ram);
    if (sysctlbyname("hw.memsize", &total_ram, &len, NULL, 0) == 0) {
        info.total_ram = total_ram;
    }

    // Get available RAM using mach APIs
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    kern_return_t kr = host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm_stats, &count);

    if (kr == KERN_SUCCESS) {
        // Calculate available memory (free + inactive)
        natural_t pagesize;
        kr = host_page_size(mach_host_self(), &pagesize);
        if (kr == KERN_SUCCESS) {
            uint64_t free_mem = (uint64_t)vm_stats.free_count * pagesize;
            uint64_t inactive_mem = (uint64_t)vm_stats.inactive_count * pagesize;
            info.available_ram = free_mem + inactive_mem;
        }
    }

    // Get iOS version
    char os_version[32];
    len = sizeof(os_version);
    if (sysctlbyname("kern.osversion", os_version, &len, NULL, 0) == 0) {
        strncpy(info.os_version, os_version, 31);
        info.os_version[31] = '\0';
    }

    // Cache the result
    g_sysinfo_cache = info;
    g_sysinfo_cache_time = now;

    pthread_mutex_unlock(&g_sysinfo_mutex);

    return info;
}

// Format system info as string (for debugging/display)
char* a_shell_sysinfo_format(const a_shell_sys_info_t* info) {
    if (!info) return NULL;

    // Calculate sizes in human-readable format
    double total_gb = info->total_ram / (1024.0 * 1024.0 * 1024.0);
    double avail_gb = info->available_ram / (1024.0 * 1024.0 * 1024.0);

    size_t bufsize = 512;
    char* buf = malloc(bufsize);
    if (!buf) return NULL;

    snprintf(buf, bufsize,
        "System Information:\n"
        "  CPU Architecture: %s\n"
        "  CPU Count: %u\n"
        "  Total RAM: %.2f GB\n"
        "  Available RAM: %.2f GB\n"
        "  OS Version: %s\n",
        info->cpu_arch,
        info->cpu_count,
        total_gb,
        avail_gb,
        info->os_version
    );

    return buf;
}

// ============================================================================
// PROCESS INFO IMPLEMENTATION
// ============================================================================

// Maximum number of processes to enumerate
#define MAX_PROCESSES 1024

// Get process information for a specific PID
// Returns allocated struct (caller must free), or NULL on error
a_shell_proc_info_t* a_shell_getproc_info(pid_t pid) {
    if (pid <= 0) return NULL;

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
    // Use proc_pidinfo for process info (BSD API on macOS)
    struct proc_bsdinfo proc_info;
    int ret = proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &proc_info, sizeof(proc_info));

    if (ret <= 0) {
        return NULL; // Process doesn't exist or error
    }

    a_shell_proc_info_t* info = malloc(sizeof(a_shell_proc_info_t));
    if (!info) return NULL;

    memset(info, 0, sizeof(a_shell_proc_info_t));

    info->pid = pid;
    info->ppid = proc_info.pbi_ppid;

    // Copy process name
    strncpy(info->name, proc_info.pbi_name, 255);
    info->name[255] = '\0';

    // Get memory info
    struct proc_taskinfo task_info;
    ret = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));
    if (ret > 0) {
        info->memory_usage = task_info.pti_resident_size;
    }

    // UID and GID
    info->uid = proc_info.pbi_uid;
    info->gid = proc_info.pbi_gid;

    return info;
#else
    // On iOS, process info APIs are restricted
    // Return a minimal struct with just the PID
    a_shell_proc_info_t* info = malloc(sizeof(a_shell_proc_info_t));
    if (!info) return NULL;
    memset(info, 0, sizeof(a_shell_proc_info_t));
    info->pid = pid;
    info->ppid = 0;
    strncpy(info->name, "unknown", 255);
    return info;
#endif
}

// Free process info returned by a_shell_getproc_info
void a_shell_freeproc_info(a_shell_proc_info_t* info) {
    free(info);
}

// List all running processes
// Returns array of pids (caller must free), sets count
pid_t* a_shell_list_processes(int* count) {
    if (!count) return NULL;

    // Get list of all PIDs
    int mib[3] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
    size_t size;

    if (sysctl(mib, 3, NULL, &size, NULL, 0) < 0) {
        *count = 0;
        return NULL;
    }

    struct kinfo_proc* procs = malloc(size);
    if (!procs) {
        *count = 0;
        return NULL;
    }

    if (sysctl(mib, 3, procs, &size, NULL, 0) < 0) {
        free(procs);
        *count = 0;
        return NULL;
    }

    int nprocs = size / sizeof(struct kinfo_proc);

    // Allocate array for PIDs
    pid_t* pids = malloc(sizeof(pid_t) * nprocs);
    if (!pids) {
        free(procs);
        *count = 0;
        return NULL;
    }

    // Extract PIDs
    for (int i = 0; i < nprocs; i++) {
        pids[i] = procs[i].kp_proc.p_pid;
    }

    free(procs);
    *count = nprocs;

    return pids;
}

// Get current process info
a_shell_proc_info_t* a_shell_getproc_self(void) {
    return a_shell_getproc_info(getpid());
}

// Format process info as string (for debugging/display)
char* a_shell_procinfo_format(const a_shell_proc_info_t* info) {
    if (!info) return NULL;

    size_t bufsize = 512;
    char* buf = malloc(bufsize);
    if (!buf) return NULL;

    double mem_mb = info->memory_usage / (1024.0 * 1024.0);

    snprintf(buf, bufsize,
        "Process: %s\n"
        "  PID: %d\n"
        "  Parent PID: %d\n"
        "  UID: %d\n"
        "  GID: %d\n"
        "  Memory: %.2f MB\n",
        info->name,
        info->pid,
        info->ppid,
        info->uid,
        info->gid,
        mem_mb
    );

    return buf;
}

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

// Print system info to stdout (for debugging)
void a_shell_print_sysinfo(void) {
    a_shell_sys_info_t info = a_shell_getsys_info();
    char* formatted = a_shell_sysinfo_format(&info);
    if (formatted) {
        fputs(formatted, a_shell_stdout());
        free(formatted);
    }
}

// Print process info to stdout
void a_shell_print_procinfo(pid_t pid) {
    a_shell_proc_info_t* info = a_shell_getproc_info(pid);
    if (info) {
        char* formatted = a_shell_procinfo_format(info);
        if (formatted) {
            fputs(formatted, a_shell_stdout());
            free(formatted);
        }
        a_shell_freeproc_info(info);
    } else {
        fprintf(a_shell_stderr(), "Cannot get info for PID %d\n", pid);
    }
}


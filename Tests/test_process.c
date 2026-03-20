/*
 * test_process.c - Process syscall unit tests
 */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/a_shell_kernel.h"

void test_getpid() {
    printf("Test: a_shell_getpid()...\n");
    pid_t pid = a_shell_getpid();
    assert(pid > 0);
    printf("  ✓ PID is %d (valid)\n", pid);
}

void test_getppid() {
    printf("Test: a_shell_getppid()...\n");
    pid_t ppid = a_shell_getppid();
    assert(ppid >= 1);
    printf("  ✓ PPID is %d (valid)\n", ppid);
}

void test_exit_status() {
    printf("Test: exit status macros...\n");
    int status = 0;
    
    /* Simulate normal exit with code 42 */
    status = (42 << 8);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 42);
    printf("  ✓ WIFEXITED and WEXITSTATUS work correctly\n");
}

void test_getuid() {
    printf("Test: a_shell_getuid()...\n");
    uid_t uid = a_shell_getuid();
    printf("  ✓ UID is %u\n", uid);
}

void test_getgid() {
    printf("Test: a_shell_getgid()...\n");
    gid_t gid = a_shell_getgid();
    printf("  ✓ GID is %u\n", gid);
}

void test_signal() {
    printf("Test: a_shell_signal()...\n");
    
    /* Set up a signal handler using the typedef from our headers */
    a_shell_sighandler_t old_handler = a_shell_signal(SIGUSR1, SIG_IGN);
    assert(old_handler != SIG_ERR);
    printf("  ✓ Signal handler set successfully\n");
    
    /* Restore default handler */
    a_shell_signal(SIGUSR1, old_handler);
    printf("  ✓ Signal handler restored\n");
}

void test_alarm() {
    printf("Test: a_shell_alarm()...\n");
    
    /* Set alarm for 1 second */
    unsigned int old = a_shell_alarm(1);
    printf("  ✓ Alarm set (old value: %u)\n", old);
    
    /* Cancel alarm */
    a_shell_alarm(0);
    printf("  ✓ Alarm cancelled\n");
}

int main() {
    printf("\n========================================\n");
    printf("Process Syscall Unit Tests\n");
    printf("========================================\n\n");
    
    test_getpid();
    test_getppid();
    test_exit_status();
    test_getuid();
    test_getgid();
    test_signal();
    test_alarm();
    
    printf("\n========================================\n");
    printf("All process tests passed!\n");
    printf("========================================\n");
    
    return 0;
}

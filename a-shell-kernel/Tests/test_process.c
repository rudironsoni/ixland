/*
 * test_process.c - Process syscall tests
 *
 * Tests fork, waitpid, signals using explicit a_shell_*() API
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

/* Darwin compatibility: sighandler_t is not defined on macOS */
#ifndef sighandler_t
typedef void (*sighandler_t)(int);
#endif

void test_getpid() {
    printf("Test: getpid()...\n");
    pid_t pid = getpid();
    assert(pid > 0);
    printf("  ✓ PID is %d\n", pid);
}

void test_getppid() {
    printf("Test: getppid()...\n");
    pid_t ppid = getppid();
    assert(ppid > 0);
    printf("  ✓ PPID is %d\n", ppid);
}

void test_alarm() {
    printf("Test: alarm()...\n");
    
    /* Set alarm for 1 second */
    unsigned int old = alarm(1);
    printf("  ✓ Alarm set (old value: %u)\n", old);
    
    /* Cancel alarm */
    alarm(0);
    printf("  ✓ Alarm cancelled\n");
}

void test_signal_basic() {
    printf("Test: signal() basic...\n");
    
    /* Set up signal handler for SIGUSR1 */
    sighandler_t old_handler = signal(SIGUSR1, SIG_IGN);
    assert(old_handler != SIG_ERR);
    printf("  ✓ Signal handler set\n");
    
    /* Restore default */
    signal(SIGUSR1, old_handler);
    printf("  ✓ Signal handler restored\n");
}

void test_sleep() {
    printf("Test: sleep()...\n");
    
    unsigned int remaining = sleep(0);
    printf("  ✓ sleep(0) returned %u\n", remaining);
}

int main() {
    printf("\n========================================\n");
    printf("Process Syscall Tests\n");
    printf("========================================\n\n");
    
    test_getpid();
    test_getppid();
    test_alarm();
    test_signal_basic();
    test_sleep();
    
    printf("\n========================================\n");
    printf("Process tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

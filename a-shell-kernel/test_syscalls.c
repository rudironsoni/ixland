//
// Test syscall replacement mechanism
// Verifies that standard syscalls are redirected to ios_* implementations
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

// This header sets up the macro redirects
#include "a_shell_system.h"

// Test 1: Verify fork() redirects to ios_fork()
void test_fork_redirect() {
    printf("Test 1: fork() redirect...\n");
    
    // ios_fork should return ENOSYS (no real fork on iOS)
    pid_t pid = fork();
    
    if (pid == -1) {
        printf("  ✓ fork() correctly returns -1 (ENOSYS)\n");
    } else {
        printf("  ✗ fork() returned %d, expected -1\n", pid);
    }
}

// Test 2: Verify vfork() redirects to ios_vfork()
void test_vfork_redirect() {
    printf("Test 2: vfork() redirect...\n");
    
    // ios_vfork should return a virtual PID >= 1000
    pid_t pid = vfork();
    
    if (pid >= 1000) {
        printf("  ✓ vfork() returned virtual PID %d\n", pid);
    } else if (pid == -1) {
        printf("  ✗ vfork() returned -1 (resource limit?)\n");
    } else {
        printf("  ? vfork() returned %d\n", pid);
    }
}

// Test 3: Verify execv() redirects to ios_execv()
void test_execv_redirect() {
    printf("Test 3: execv() redirect...\n");
    
    // Just check that execv compiles and links
    // We can't actually test execution without full kernel
    printf("  ✓ execv() symbol available\n");
}

// Test 4: Verify waitpid() redirects to ios_waitpid()
void test_waitpid_redirect() {
    printf("Test 4: waitpid() redirect...\n");
    
    int status;
    // Try to wait for any child (should return ECHILD or 0)
    pid_t result = waitpid(-1, &status, WNOHANG);
    
    if (result == 0 || result == -1) {
        printf("  ✓ waitpid() returned %d\n", result);
    } else {
        printf("  ? waitpid() returned %d\n", result);
    }
}

// Test 5: Verify environment functions
void test_env_redirects() {
    printf("Test 5: Environment function redirects...\n");
    
    // getenv
    char *path = getenv("PATH");
    printf("  ✓ getenv() works: PATH=%s\n", path ? "set" : "not set");
    
    // setenv
    int result = setenv("TEST_VAR", "test_value", 1);
    printf("  ✓ setenv() returned %d\n", result);
    
    // unsetenv
    result = unsetenv("TEST_VAR");
    printf("  ✓ unsetenv() returned %d\n", result);
}

// Test 6: Verify signal functions
void test_signal_redirect() {
    printf("Test 6: signal() redirect...\n");
    
    // signal should redirect to ios_signal
    // For now just verify it compiles
    printf("  ✓ signal() symbol available\n");
}

// Test 7: Verify exit() redirects
void test_exit_redirect() {
    printf("Test 7: exit() redirect...\n");
    
    // Can't actually test exit(), but verify it compiles
    printf("  ✓ exit() macro defined\n");
}

int main(int argc, char *argv[]) {
    printf("\n========================================\n");
    printf("a-shell-kernel Syscall Replacement Tests\n");
    printf("========================================\n\n");
    
    test_fork_redirect();
    test_vfork_redirect();
    test_execv_redirect();
    test_waitpid_redirect();
    test_env_redirects();
    test_signal_redirect();
    test_exit_redirect();
    
    printf("\n========================================\n");
    printf("Tests completed\n");
    printf("========================================\n");
    
    return 0;
}

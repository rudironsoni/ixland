//
// Simple compilation test for a-shell-kernel
// This verifies headers are syntactically correct
//

#include <stdio.h>

// Include our headers
#include "a_shell_error.h"
#include "a_shell_system.h"

int main() {
    printf("a-shell-kernel Compilation Test\n");
    printf("================================\n\n");
    
    // Check that ios_* functions are declared
    printf("Checking ios_* function declarations...\n");
    
    void *ptr;
    ptr = (void*)a_shell_system;
    printf("  a_shell_system: %p\n", ptr);
    
    ptr = (void*)a_shell_fork;
    printf("  a_shell_fork: %p\n", ptr);
    
    ptr = (void*)a_shell_execv;
    printf("  a_shell_execv: %p\n", ptr);
    
    ptr = (void*)a_shell_getenv;
    printf("  a_shell_getenv: %p\n", ptr);
    
    ptr = (void*)a_shell_setenv;
    printf("  a_shell_setenv: %p\n", ptr);
    
    ptr = (void*)a_shell_exit;
    printf("  a_shell_exit: %p\n", ptr);
    
    printf("\nAll headers compiled successfully!\n");
    
    return 0;
}

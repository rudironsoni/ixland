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
    ptr = (void*)ios_system;
    printf("  ios_system: %p\n", ptr);
    
    ptr = (void*)ios_fork;
    printf("  ios_fork: %p\n", ptr);
    
    ptr = (void*)ios_execv;
    printf("  ios_execv: %p\n", ptr);
    
    ptr = (void*)ios_getenv;
    printf("  ios_getenv: %p\n", ptr);
    
    ptr = (void*)ios_setenv;
    printf("  ios_setenv: %p\n", ptr);
    
    ptr = (void*)ios_exit;
    printf("  ios_exit: %p\n", ptr);
    
    printf("\nAll headers compiled successfully!\n");
    
    return 0;
}

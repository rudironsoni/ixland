// Simple compilation test for a-shell-kernel
#include "a_shell_error.h"
#include "a_shell_system.h"
#include <stdio.h>

int main() {
    printf("Testing a-shell-kernel compilation\n");
    printf("ios_system available: %p\n", (void*)ios_system);
    printf("ios_fork available: %p\n", (void*)ios_fork);
    printf("ios_execv available: %p\n", (void*)ios_execv);
    printf("Test passed!\n");
    return 0;
}

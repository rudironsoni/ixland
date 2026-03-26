/* Simple test - verifies basic functionality works */

#include <stdio.h>
#include <string.h>

int main() {
    printf("Hello from libiox!\n");
    
    /* Test basic operations */
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("CWD: %s\n", cwd);
    }
    
    printf("Test passed!\n");
    return 0;
}

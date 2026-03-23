#include "iox_test.h"
#include <stdio.h>
#include <string.h>

void iox_test_fail(const char *file, int line, const char *expr) {
    fprintf(stderr, "  FAIL: %s:%d: %s\n", file, line, expr);
}

int iox_test_run_all(const char *filter) {
    iox_test_case_t *start = IOX_TEST_CASE_START;
    iox_test_case_t *stop = IOX_TEST_CASE_STOP;
    
    int tests_run = 0;
    int tests_passed = 0;
    
    printf("Running iox tests...\n\n");
    
    for (iox_test_case_t *tc = start; tc < stop; tc++) {
        if (filter && strstr(tc->name, filter) == NULL)
            continue;
        
        tests_run++;
        printf("  %s ... ", tc->name);
        fflush(stdout);
        
        if (tc->fn()) {
            printf("PASS\n");
            tests_passed++;
        } else {
            printf("FAIL\n");
        }
    }
    
    printf("\n");
    printf("Results: %d/%d passed\n", tests_passed, tests_run);
    
    return (tests_passed == tests_run) ? 0 : 1;
}

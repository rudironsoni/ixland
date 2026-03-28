#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <stdio.h>
#include <string.h>

#include "iox_test.h"

void iox_test_fail(const char *file, int line, const char *expr) {
    fprintf(stderr, "  FAIL: %s:%d: %s\n", file, line, expr);
}

int main(int argc, char **argv) {
    const char *filter = argc > 1 ? argv[1] : NULL;
    return iox_test_run_all(filter);
}

int iox_test_run_all(const char *filter) {
    uint32_t image_count = _dyld_image_count();
    iox_test_case_t *tests = NULL;
    unsigned long size = 0;

    // Iterate all loaded images to find the one with our section
    for (uint32_t i = 0; i < image_count; i++) {
        const struct mach_header *mh = _dyld_get_image_header(i);
        tests = (iox_test_case_t *)getsectiondata(mh, "__DATA", "iox_test_cases", &size);
        if (tests && size > 0) {
            break;
        }
    }

    if (!tests || size == 0) {
        printf("No tests found\n");
        return 1;
    }

    int count = size / sizeof(iox_test_case_t);
    int tests_run = 0;
    int tests_passed = 0;

    printf("Running iox tests...\n\n");

    for (int i = 0; i < count; i++) {
        iox_test_case_t *tc = &tests[i];
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

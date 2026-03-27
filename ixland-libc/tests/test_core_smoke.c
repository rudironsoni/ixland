/* ixland-libc-core smoke tests
 *
 * Minimal compile-check and basic validation for ixland-libc-core.
 */

#include <iox/iox.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* ============================================================================
 * VERSION TESTS
 * ============================================================================ */

static int test_iox_version(void) {
    const char *version = iox_version();
    assert(version != NULL);
    assert(strlen(version) > 0);
    printf("  iox_version: %s\n", version);
    return 0;
}

/* ============================================================================
 * ERROR STRING TESTS
 * ============================================================================ */

static int test_iox_strerror(void) {
    const char *msg;

    /* Test known error codes */
    msg = iox_strerror(IOX_OK);
    assert(msg != NULL);
    assert(strlen(msg) > 0);
    printf("  iox_strerror(IOX_OK): %s\n", msg);

    msg = iox_strerror(IOX_EPERM);
    assert(msg != NULL);
    printf("  iox_strerror(IOX_EPERM): %s\n", msg);

    msg = iox_strerror(IOX_ENOENT);
    assert(msg != NULL);
    printf("  iox_strerror(IOX_ENOENT): %s\n", msg);

    /* Test positive errno mapping */
    msg = iox_strerror(2);  /* Should map to IOX_ENOENT */
    assert(msg != NULL);
    printf("  iox_strerror(2): %s\n", msg);

    /* Test unknown error */
    msg = iox_strerror(-999);
    assert(msg != NULL);
    printf("  iox_strerror(-999): %s\n", msg);

    return 0;
}

/* ============================================================================
 * PERROR TESTS
 * ============================================================================ */

static int test_iox_perror(void) {
    /* These should not crash - actual output to stderr */
    printf("  iox_perror tests (output to stderr):\n");

    iox_perror("test_prefix");
    printf("    iox_perror(\"test_prefix\") - OK\n");

    iox_perror(NULL);
    printf("    iox_perror(NULL) - OK\n");

    iox_perror("");
    printf("    iox_perror(\"\") - OK\n");

    return 0;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    int result = 0;

    printf("ixland-libc-core smoke tests\n");
    printf("=============================\n\n");

    printf("Test: iox_version()\n");
    result |= test_iox_version();
    printf("\n");

    printf("Test: iox_strerror()\n");
    result |= test_iox_strerror();
    printf("\n");

    printf("Test: iox_perror()\n");
    result |= test_iox_perror();
    printf("\n");

    if (result == 0) {
        printf("All tests PASSED\n");
    } else {
        printf("Some tests FAILED\n");
    }

    return result;
}

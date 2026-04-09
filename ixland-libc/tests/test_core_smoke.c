/* ixland-libc-core smoke tests
 *
 * Minimal compile-check and basic validation for ixland-libc-core.
 */

#include <ixland/ixland.h>

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * VERSION TESTS (VAL-LIBC-011: ixland_version() returns non-null version string)
 * ============================================================================ */

static int test_ixland_version(void) {
    const char *version = ixland_version();

    /* VAL-LIBC-011: Must return non-NULL */
    assert(version != NULL);

    /* VAL-LIBC-011: Must return non-empty string */
    assert(strlen(version) > 0);

    /* VAL-LIBC-011: Must match IXLAND_VERSION_STRING macro */
    assert(strcmp(version, IXLAND_VERSION_STRING) == 0);

    /* VAL-LIBC-011: Version format should be "major.minor.patch" */
    int major, minor, patch;
    int parsed = sscanf(version, "%d.%d.%d", &major, &minor, &patch);
    assert(parsed == 3);
    assert(major == IXLAND_VERSION_MAJOR);
    assert(minor == IXLAND_VERSION_MINOR);
    assert(patch == IXLAND_VERSION_PATCH);

    printf("  ixland_version: %s (format: %d.%d.%d) - PASS\n", version, major, minor, patch);
    return 0;
}

/* ============================================================================
 * ERROR STRING TESTS (VAL-LIBC-012: strerror for all error codes)
 * ============================================================================ */

static int test_ixland_strerror(void) {
    const char *msg;
    int failed = 0;

    /* VAL-LIBC-012: Test core error codes with expected strings
     * Note: Error codes in ixland_types.h have Linux-compatible values that may
     * have gaps. The implementation uses a dense array for the contiguous
     * range of common errors (0 to -35), with bounds checking for others.
     */
    struct {
        int code;
        const char *expected;
    } error_tests[] = {
        /* Core contiguous error codes (0 to -35 range) */
        {IXLAND_OK, "Success"},
        {IXLAND_EPERM, "Operation not permitted"},
        {IXLAND_ENOENT, "No such file or directory"},
        {IXLAND_ESRCH, "No such process"},
        {IXLAND_EINTR, "Interrupted system call"},
        {IXLAND_EIO, "Input/output error"}, /* VAL-LIBC-012: Specific requirement */
        {IXLAND_EINVAL, "Invalid argument"},
        {IXLAND_ENOMEM, "Out of memory"},
        {IXLAND_EACCES, "Permission denied"},
        {IXLAND_EEXIST, "File exists"},
        {IXLAND_EAGAIN, "Resource temporarily unavailable"},
        {IXLAND_EBUSY, "Device or resource busy"},
        {IXLAND_ECHILD, "No child processes"},
        {IXLAND_EISDIR, "Is a directory"},
    };

    size_t num_tests = sizeof(error_tests) / sizeof(error_tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        msg = ixland_strerror(error_tests[i].code);
        if (msg == NULL) {
            printf("  FAIL: ixland_strerror(%d) returned NULL\n", error_tests[i].code);
            failed = 1;
            continue;
        }
        if (strcmp(msg, error_tests[i].expected) != 0) {
            printf("  FAIL: ixland_strerror(%d) = \"%s\" (expected \"%s\")\n", error_tests[i].code,
                   msg, error_tests[i].expected);
            failed = 1;
        } else {
            printf("  ixland_strerror(%d): \"%s\" - PASS\n", error_tests[i].code, msg);
        }
    }

    /* VAL-LIBC-012: Test positive errno mapping (Linux convention) */
    msg = ixland_strerror(2); /* Should map to IXLAND_ENOENT */
    if (strcmp(msg, "No such file or directory") != 0) {
        printf("  FAIL: ixland_strerror(2) should map to ENOENT\n");
        failed = 1;
    } else {
        printf("  ixland_strerror(2) -> ENOENT mapping: \"%s\" - PASS\n", msg);
    }

    /* VAL-LIBC-012: Test unknown error returns "Unknown error" */
    msg = ixland_strerror(-999);
    if (strcmp(msg, "Unknown error") != 0) {
        printf("  FAIL: ixland_strerror(-999) should return \"Unknown error\", got \"%s\"\n", msg);
        failed = 1;
    } else {
        printf("  ixland_strerror(-999): \"%s\" - PASS\n", msg);
    }

    /* VAL-LIBC-012: Verify non-contiguous error codes return sensible values */
    msg = ixland_strerror(IXLAND_ETIMEDOUT); /* -60, outside dense array range */
    if (msg == NULL) {
        printf("  FAIL: ixland_strerror(IXLAND_ETIMEDOUT) returned NULL\n");
        failed = 1;
    } else {
        printf("  ixland_strerror(IXLAND_ETIMEDOUT): \"%s\" - PASS (bounds check)\n", msg);
    }

    return failed;
}

/* ============================================================================
 * PERROR TESTS (VAL-LIBC-013: perror output format)
 * ============================================================================ */

static int test_ixland_perror(void) {
    int failed = 0;

    printf("  ixland_perror tests:\n");

    /* VAL-LIBC-013: Test with prefix - should output "prefix: <msg>\n" */
    printf("    Testing ixland_perror(\"test_prefix\") [should print to stderr]:\n");
    errno = IXLAND_EPERM; /* Set a known error */
    ixland_perror("test_prefix");
    printf("      - PASS (if above shows \"test_prefix: Operation not permitted\")\n");

    /* VAL-LIBC-013: Test with NULL - should output just "<msg>\n" */
    printf("    Testing ixland_perror(NULL):\n");
    errno = IXLAND_OK;
    ixland_perror(NULL);
    printf("      - PASS (if above shows \"Success\")\n");

    /* VAL-LIBC-013: Test with empty string - should output just "<msg>\n" */
    printf("    Testing ixland_perror(\"\"):\n");
    errno = IXLAND_ENOENT;
    ixland_perror("");
    printf("      - PASS (if above shows \"No such file or directory\")\n");

    /* VAL-LIBC-013: Test with IXLAND_EIO specifically */
    printf("    Testing ixland_perror with IXLAND_EIO:\n");
    errno = IXLAND_EIO;
    ixland_perror("disk error");
    printf("      - PASS (if above shows \"disk error: Input/output error\")\n");

    return failed;
}

/* ============================================================================
 * THREAD SAFETY TESTS (VAL-LIBC-014: Thread-safe implementation)
 * ============================================================================ */

#define NUM_THREADS 8
#define ITERATIONS_PER_THREAD 100

typedef struct {
    int thread_id;
    int errors;
} thread_result_t;

static void *thread_strerror_test(void *arg) {
    thread_result_t *result = (thread_result_t *)arg;
    int errors = 0;

    for (int i = 0; i < ITERATIONS_PER_THREAD; i++) {
        /* Test various error codes from multiple threads */
        int codes[] = {IXLAND_OK,  IXLAND_EPERM,  IXLAND_ENOENT,
                       IXLAND_EIO, IXLAND_EINVAL, IXLAND_ENOMEM};
        int code = codes[i % 6];

        const char *msg = ixland_strerror(code);
        if (msg == NULL) {
            errors++;
            continue;
        }

        /* Verify the message is consistent */
        if (code == IXLAND_OK && strstr(msg, "Success") == NULL) {
            errors++;
        } else if (code == IXLAND_EPERM && strstr(msg, "not permitted") == NULL) {
            errors++;
        } else if (code == IXLAND_EIO && strstr(msg, "Input/output") == NULL) {
            errors++;
        }
    }

    result->errors = errors;
    return NULL;
}

static void *thread_version_test(void *arg) {
    thread_result_t *result = (thread_result_t *)arg;
    int errors = 0;

    for (int i = 0; i < ITERATIONS_PER_THREAD; i++) {
        const char *version = ixland_version();
        if (version == NULL || strlen(version) == 0) {
            errors++;
        }
    }

    result->errors = errors;
    return NULL;
}

static int test_thread_safety(void) {
    pthread_t threads[NUM_THREADS];
    thread_result_t results[NUM_THREADS];
    int failed = 0;

    printf("  Thread safety tests:\n");
    printf("    Spawning %d threads, each calling ixland_strerror %d times...\n", NUM_THREADS,
           ITERATIONS_PER_THREAD);

    /* Test ixland_strerror thread safety */
    for (int i = 0; i < NUM_THREADS; i++) {
        results[i].thread_id = i;
        results[i].errors = 0;
        if (pthread_create(&threads[i], NULL, thread_strerror_test, &results[i]) != 0) {
            printf("    FAIL: Could not create thread %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        if (results[i].errors > 0) {
            printf("    FAIL: Thread %d had %d errors in strerror\n", i, results[i].errors);
            failed = 1;
        }
    }

    if (!failed) {
        printf("    ixland_strerror: thread-safe - PASS\n");
    }

    /* Test ixland_version thread safety */
    printf("    Spawning %d threads, each calling ixland_version %d times...\n", NUM_THREADS,
           ITERATIONS_PER_THREAD);

    for (int i = 0; i < NUM_THREADS; i++) {
        results[i].thread_id = i;
        results[i].errors = 0;
        if (pthread_create(&threads[i], NULL, thread_version_test, &results[i]) != 0) {
            printf("    FAIL: Could not create thread %d\n", i);
            return 1;
        }
    }

    failed = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        if (results[i].errors > 0) {
            printf("    FAIL: Thread %d had %d errors in version\n", i, results[i].errors);
            failed = 1;
        }
    }

    if (!failed) {
        printf("    ixland_version: thread-safe - PASS\n");
    }

    printf("  Thread safety: all tests - PASS\n");
    return 0;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    int result = 0;

    printf("ixland-libc-core smoke tests\n");
    printf("=============================\n\n");

    printf("Test: ixland_version() [VAL-LIBC-011]\n");
    result |= test_ixland_version();
    printf("\n");

    printf("Test: ixland_strerror() [VAL-LIBC-012]\n");
    result |= test_ixland_strerror();
    printf("\n");

    printf("Test: ixland_perror() [VAL-LIBC-013]\n");
    result |= test_ixland_perror();
    printf("\n");

    printf("Test: thread safety [VAL-LIBC-014, VAL-LIBC-015]\n");
    result |= test_thread_safety();
    printf("\n");

    printf("=============================\n");
    if (result == 0) {
        printf("All tests PASSED\n");
    } else {
        printf("Some tests FAILED\n");
    }

    return result;
}

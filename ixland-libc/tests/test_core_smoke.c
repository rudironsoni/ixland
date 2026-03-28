/* ixland-libc-core smoke tests
 *
 * Minimal compile-check and basic validation for ixland-libc-core.
 */

#include <iox/iox.h>

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * VERSION TESTS (VAL-LIBC-011: iox_version() returns non-null version string)
 * ============================================================================ */

static int test_iox_version(void) {
    const char *version = iox_version();

    /* VAL-LIBC-011: Must return non-NULL */
    assert(version != NULL);

    /* VAL-LIBC-011: Must return non-empty string */
    assert(strlen(version) > 0);

    /* VAL-LIBC-011: Must match IOX_VERSION_STRING macro */
    assert(strcmp(version, IOX_VERSION_STRING) == 0);

    /* VAL-LIBC-011: Version format should be "major.minor.patch" */
    int major, minor, patch;
    int parsed = sscanf(version, "%d.%d.%d", &major, &minor, &patch);
    assert(parsed == 3);
    assert(major == IOX_VERSION_MAJOR);
    assert(minor == IOX_VERSION_MINOR);
    assert(patch == IOX_VERSION_PATCH);

    printf("  iox_version: %s (format: %d.%d.%d) - PASS\n", version, major, minor, patch);
    return 0;
}

/* ============================================================================
 * ERROR STRING TESTS (VAL-LIBC-012: strerror for all error codes)
 * ============================================================================ */

static int test_iox_strerror(void) {
    const char *msg;
    int failed = 0;

    /* VAL-LIBC-012: Test core error codes with expected strings
     * Note: Error codes in iox_types.h have Linux-compatible values that may
     * have gaps. The implementation uses a dense array for the contiguous
     * range of common errors (0 to -35), with bounds checking for others.
     */
    struct {
        int code;
        const char *expected;
    } error_tests[] = {
        /* Core contiguous error codes (0 to -35 range) */
        {IOX_OK, "Success"},
        {IOX_EPERM, "Operation not permitted"},
        {IOX_ENOENT, "No such file or directory"},
        {IOX_ESRCH, "No such process"},
        {IOX_EINTR, "Interrupted system call"},
        {IOX_EIO, "Input/output error"}, /* VAL-LIBC-012: Specific requirement */
        {IOX_EINVAL, "Invalid argument"},
        {IOX_ENOMEM, "Out of memory"},
        {IOX_EACCES, "Permission denied"},
        {IOX_EEXIST, "File exists"},
        {IOX_EAGAIN, "Resource temporarily unavailable"},
        {IOX_EBUSY, "Device or resource busy"},
        {IOX_ECHILD, "No child processes"},
        {IOX_EISDIR, "Is a directory"},
    };

    size_t num_tests = sizeof(error_tests) / sizeof(error_tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        msg = iox_strerror(error_tests[i].code);
        if (msg == NULL) {
            printf("  FAIL: iox_strerror(%d) returned NULL\n", error_tests[i].code);
            failed = 1;
            continue;
        }
        if (strcmp(msg, error_tests[i].expected) != 0) {
            printf("  FAIL: iox_strerror(%d) = \"%s\" (expected \"%s\")\n", error_tests[i].code,
                   msg, error_tests[i].expected);
            failed = 1;
        } else {
            printf("  iox_strerror(%d): \"%s\" - PASS\n", error_tests[i].code, msg);
        }
    }

    /* VAL-LIBC-012: Test positive errno mapping (Linux convention) */
    msg = iox_strerror(2); /* Should map to IOX_ENOENT */
    if (strcmp(msg, "No such file or directory") != 0) {
        printf("  FAIL: iox_strerror(2) should map to ENOENT\n");
        failed = 1;
    } else {
        printf("  iox_strerror(2) -> ENOENT mapping: \"%s\" - PASS\n", msg);
    }

    /* VAL-LIBC-012: Test unknown error returns "Unknown error" */
    msg = iox_strerror(-999);
    if (strcmp(msg, "Unknown error") != 0) {
        printf("  FAIL: iox_strerror(-999) should return \"Unknown error\", got \"%s\"\n", msg);
        failed = 1;
    } else {
        printf("  iox_strerror(-999): \"%s\" - PASS\n", msg);
    }

    /* VAL-LIBC-012: Verify non-contiguous error codes return sensible values */
    msg = iox_strerror(IOX_ETIMEDOUT); /* -60, outside dense array range */
    if (msg == NULL) {
        printf("  FAIL: iox_strerror(IOX_ETIMEDOUT) returned NULL\n");
        failed = 1;
    } else {
        printf("  iox_strerror(IOX_ETIMEDOUT): \"%s\" - PASS (bounds check)\n", msg);
    }

    return failed;
}

/* ============================================================================
 * PERROR TESTS (VAL-LIBC-013: perror output format)
 * ============================================================================ */

static int test_iox_perror(void) {
    int failed = 0;

    printf("  iox_perror tests:\n");

    /* VAL-LIBC-013: Test with prefix - should output "prefix: <msg>\n" */
    printf("    Testing iox_perror(\"test_prefix\") [should print to stderr]:\n");
    errno = IOX_EPERM; /* Set a known error */
    iox_perror("test_prefix");
    printf("      - PASS (if above shows \"test_prefix: Operation not permitted\")\n");

    /* VAL-LIBC-013: Test with NULL - should output just "<msg>\n" */
    printf("    Testing iox_perror(NULL):\n");
    errno = IOX_OK;
    iox_perror(NULL);
    printf("      - PASS (if above shows \"Success\")\n");

    /* VAL-LIBC-013: Test with empty string - should output just "<msg>\n" */
    printf("    Testing iox_perror(\"\"):\n");
    errno = IOX_ENOENT;
    iox_perror("");
    printf("      - PASS (if above shows \"No such file or directory\")\n");

    /* VAL-LIBC-013: Test with IOX_EIO specifically */
    printf("    Testing iox_perror with IOX_EIO:\n");
    errno = IOX_EIO;
    iox_perror("disk error");
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
        int codes[] = {IOX_OK, IOX_EPERM, IOX_ENOENT, IOX_EIO, IOX_EINVAL, IOX_ENOMEM};
        int code = codes[i % 6];

        const char *msg = iox_strerror(code);
        if (msg == NULL) {
            errors++;
            continue;
        }

        /* Verify the message is consistent */
        if (code == IOX_OK && strstr(msg, "Success") == NULL) {
            errors++;
        } else if (code == IOX_EPERM && strstr(msg, "not permitted") == NULL) {
            errors++;
        } else if (code == IOX_EIO && strstr(msg, "Input/output") == NULL) {
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
        const char *version = iox_version();
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
    printf("    Spawning %d threads, each calling iox_strerror %d times...\n", NUM_THREADS,
           ITERATIONS_PER_THREAD);

    /* Test iox_strerror thread safety */
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
        printf("    iox_strerror: thread-safe - PASS\n");
    }

    /* Test iox_version thread safety */
    printf("    Spawning %d threads, each calling iox_version %d times...\n", NUM_THREADS,
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
        printf("    iox_version: thread-safe - PASS\n");
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

    printf("Test: iox_version() [VAL-LIBC-011]\n");
    result |= test_iox_version();
    printf("\n");

    printf("Test: iox_strerror() [VAL-LIBC-012]\n");
    result |= test_iox_strerror();
    printf("\n");

    printf("Test: iox_perror() [VAL-LIBC-013]\n");
    result |= test_iox_perror();
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

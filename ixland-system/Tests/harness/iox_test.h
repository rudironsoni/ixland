#ifndef IOX_TEST_H
#define IOX_TEST_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *name;
    bool (*fn)(void);
} iox_test_case_t;

#ifdef __APPLE__
#define IOX_SECTION __attribute__((used, section("__DATA,iox_test_cases")))
#define IOX_TEST_CASE_START __start_iox_test_cases
#define IOX_TEST_CASE_STOP __stop_iox_test_cases
#else
#define IOX_SECTION __attribute__((used, section("iox_test_cases")))
#define IOX_TEST_CASE_START __start_iox_test_cases
#define IOX_TEST_CASE_STOP __stop_iox_test_cases
#endif

#define IOX_TEST(name)                                                                \
    static bool test_##name(void);                                                    \
    static const iox_test_case_t test_case_##name IOX_SECTION = {#name, test_##name}; \
    static bool test_##name(void)

#define IOX_ASSERT(expr)                              \
    do {                                              \
        if (!(expr)) {                                \
            iox_test_fail(__FILE__, __LINE__, #expr); \
            return false;                             \
        }                                             \
    } while (0)

#define IOX_ASSERT_EQ(a, b) IOX_ASSERT((a) == (b))
#define IOX_ASSERT_NE(a, b) IOX_ASSERT((a) != (b))
#define IOX_ASSERT_GT(a, b) IOX_ASSERT((a) > (b))
#define IOX_ASSERT_LT(a, b) IOX_ASSERT((a) < (b))
#define IOX_ASSERT_GE(a, b) IOX_ASSERT((a) >= (b))
#define IOX_ASSERT_LE(a, b) IOX_ASSERT((a) <= (b))
#define IOX_ASSERT_NULL(p) IOX_ASSERT((p) == NULL)
#define IOX_ASSERT_NOT_NULL(p) IOX_ASSERT((p) != NULL)

void iox_test_fail(const char *file, int line, const char *expr);
int iox_test_run_all(const char *filter);
extern iox_test_case_t IOX_TEST_CASE_START[];
extern iox_test_case_t IOX_TEST_CASE_STOP[];

#ifdef __cplusplus
}
#endif

#endif

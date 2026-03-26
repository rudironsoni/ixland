#include "../harness/iox_test.h"
#include "../../kernel/task/task.h"
#include <stdbool.h>

IOX_TEST(pid_alloc_basic) {
    pid_t p1 = iox_alloc_pid();
    pid_t p2 = iox_alloc_pid();
    
    IOX_ASSERT_GT(p1, 0);
    IOX_ASSERT_GT(p2, 0);
    IOX_ASSERT_NE(p1, p2);
    
    iox_free_pid(p1);
    iox_free_pid(p2);
    return true;
}

IOX_TEST(pid_alloc_sequence) {
    pid_t p1 = iox_alloc_pid();
    pid_t p2 = iox_alloc_pid();
    pid_t p3 = iox_alloc_pid();
    
    IOX_ASSERT_EQ(p2, p1 + 1);
    IOX_ASSERT_EQ(p3, p2 + 1);
    
    iox_free_pid(p1);
    iox_free_pid(p2);
    iox_free_pid(p3);
    return true;
}

IOX_TEST(pid_alloc_many) {
    pid_t pids[10];
    for (int i = 0; i < 10; i++) {
        pids[i] = iox_alloc_pid();
        IOX_ASSERT_GT(pids[i], 0);
    }
    
    for (int i = 0; i < 10; i++) {
        iox_free_pid(pids[i]);
    }
    return true;
}

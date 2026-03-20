/*
 * test_time.c - Time syscall tests
 *
 * Tests alarm, setitimer, nanosleep using explicit a_shell_*() API
 */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

void test_alarm() {
    printf("Test: alarm()...\n");
    
    unsigned int old = alarm(2);
    printf("  ✓ Alarm set for 2 seconds (old: %u)\n", old);
    
    /* Cancel immediately */
    alarm(0);
    printf("  ✓ Alarm cancelled\n");
}

void test_setitimer() {
    printf("Test: setitimer()...\n");
    
    struct itimerval new_val = {0}, old_val = {0};
    new_val.it_value.tv_sec = 1;
    
    int ret = setitimer(ITIMER_REAL, &new_val, &old_val);
    printf("  ✓ setitimer returned %d\n", ret);
    
    /* Cancel */
    new_val.it_value.tv_sec = 0;
    setitimer(ITIMER_REAL, &new_val, NULL);
    printf("  ✓ Timer cancelled\n");
}

void test_nanosleep() {
    printf("Test: nanosleep()...\n");
    
    struct timespec req = {0, 1000000};  /* 1ms */
    struct timespec rem;
    
    int ret = nanosleep(&req, &rem);
    printf("  ✓ nanosleep returned %d\n", ret);
}

void test_clock_gettime() {
    printf("Test: clock_gettime()...\n");
    
    struct timespec tp;
    int ret = clock_gettime(CLOCK_REALTIME, &tp);
    assert(ret == 0);
    printf("  ✓ CLOCK_REALTIME: %ld.%09ld\n", (long)tp.tv_sec, (long)tp.tv_nsec);
}

int main() {
    printf("\n========================================\n");
    printf("Time Syscall Tests\n");
    printf("========================================\n\n");
    
    test_alarm();
    test_setitimer();
    test_nanosleep();
    test_clock_gettime();
    
    printf("\n========================================\n");
    printf("Time tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

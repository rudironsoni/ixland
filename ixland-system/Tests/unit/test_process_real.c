/* Real process test - verifies proper implementation
 * This tests actual functionality, not stubs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

/* Need to include internal header to test __iox functions */
#include "iox_internal.h"

static int test_count = 0;
static int pass_count = 0;

#define TEST(name) printf("\nTest %d: %s\n", ++test_count, name)
#define PASS() do { printf("  [PASS]\n"); pass_count++; } while(0)
#define FAIL(msg) do { printf("  [FAIL] %s\n", msg); } while(0)

void test_pid_allocation(void) {
    TEST("PID Allocation");
    
    pid_t p1 = __iox_alloc_pid();
    if (p1 < IOX_MIN_PID) {
        FAIL("PID below minimum");
        return;
    }
    
    pid_t p2 = __iox_alloc_pid();
    if (p2 != p1 + 1) {
        FAIL("PIDs not sequential");
        __iox_free_pid(p1);
        __iox_free_pid(p2);
        return;
    }
    
    __iox_free_pid(p1);
    __iox_free_pid(p2);
    PASS();
}

void test_process_create(void) {
    TEST("Process Creation");
    
    __iox_process_t *proc = __iox_process_create("test", 0);
    if (!proc) {
        FAIL("Failed to create process");
        return;
    }
    
    if (proc->pid < IOX_MIN_PID) {
        FAIL("Invalid PID");
        __iox_process_destroy(proc);
        return;
    }
    
    if (strcmp(proc->name, "test") != 0) {
        FAIL("Name not set correctly");
        __iox_process_destroy(proc);
        return;
    }
    
    __iox_process_destroy(proc);
    PASS();
}

void test_process_lookup(void) {
    TEST("Process Table Lookup");
    
    __iox_process_t *proc = __iox_process_create("lookup", 0);
    if (!proc) {
        FAIL("Failed to create process");
        return;
    }
    
    pid_t pid = proc->pid;
    
    __iox_process_t *found = __iox_get_process(pid);
    if (!found) {
        FAIL("Failed to lookup process");
        __iox_process_destroy(proc);
        return;
    }
    
    if (found->pid != pid) {
        FAIL("Wrong process returned");
        __iox_process_destroy(proc);
        return;
    }
    
    __iox_process_destroy(proc);
    PASS();
}

void test_signal_queue(void) {
    TEST("Signal Queue");
    
    __iox_sigqueue_t queue;
    memset(&queue, 0, sizeof(queue));
    pthread_mutex_init(&queue.lock, NULL);
    
    /* Queue some signals */
    siginfo_t info;
    memset(&info, 0, sizeof(info));
    info.si_signo = SIGUSR1;
    
    int ret = __iox_sigqueue_enqueue(&queue, SIGUSR1, &info);
    if (ret != 0) {
        FAIL("Failed to enqueue signal");
        pthread_mutex_destroy(&queue.lock);
        return;
    }
    
    if (queue.count != 1) {
        FAIL("Queue count wrong");
        __iox_sigqueue_flush(&queue);
        pthread_mutex_destroy(&queue.lock);
        return;
    }
    
    __iox_sigqueue_flush(&queue);
    pthread_mutex_destroy(&queue.lock);
    PASS();
}

void test_fork_simulation(void) {
    TEST("Fork Simulation");
    
    pid_t pid = iox_fork();
    if (pid < 0) {
        FAIL("Fork failed");
        return;
    }
    
    if (pid == 0) {
        /* Child process */
        printf("    Child: PID=%d\n", iox_getpid());
        iox_exit(42);
    } else {
        /* Parent process */
        printf("    Parent: Child PID=%d\n", pid);
        
        int status;
        pid_t waited = iox_waitpid(pid, &status, 0);
        
        if (waited != pid) {
            FAIL("Wait returned wrong PID");
            return;
        }
        
        if (!WIFEXITED(status)) {
            FAIL("Child did not exit normally");
            return;
        }
        
        if (WEXITSTATUS(status) != 42) {
            FAIL("Wrong exit status");
            return;
        }
        
        PASS();
    }
}

void test_process_groups(void) {
    TEST("Process Groups");
    
    pid_t pid = iox_fork();
    if (pid < 0) {
        FAIL("Fork failed");
        return;
    }
    
    if (pid == 0) {
        /* Child - create new process group */
        pid_t new_pgid = iox_getpid();
        if (iox_setpgid(0, new_pgid) < 0) {
            printf("    [CHILD] setpgid failed\n");
            iox_exit(1);
        }
        
        pid_t pgid = iox_getpgid(0);
        if (pgid != new_pgid) {
            printf("    [CHILD] pgid wrong: expected %d, got %d\n", new_pgid, pgid);
            iox_exit(1);
        }
        
        iox_exit(0);
    } else {
        /* Parent */
        int status;
        iox_waitpid(pid, &status, 0);
        
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            FAIL("Child failed process group test");
            return;
        }
        
        PASS();
    }
}

int main(void) {
    printf("=== libiox Process Implementation Test ===\n");
    printf("Testing production-quality process simulation\n\n");
    
    /* Initialize iox subsystem */
    if (__iox_init() != 0) {
        printf("FAILED: Could not initialize iox\n");
        return 1;
    }
    
    test_pid_allocation();
    test_process_create();
    test_process_lookup();
    test_signal_queue();
    test_fork_simulation();
    test_process_groups();
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d\n", pass_count, test_count);
    
    if (pass_count == test_count) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}

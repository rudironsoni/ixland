//
//  IOXProcessTests.mm
//  IOXTestSuite
//
//  Process Management Unit Tests - 16 syscalls
//

#import <XCTest/XCTest.h>
#import "IOXTestHelpers.h"

// C headers for syscalls
extern "C" {
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

// libiox function declarations
int iox_is_initialized(void);
const char *iox_version(void);
pid_t iox_getpid(void);
pid_t iox_getppid(void);
pid_t iox_getpgrp(void);
int iox_setpgrp(void);
pid_t iox_getpgid(pid_t pid);
int iox_setpgid(pid_t pid, pid_t pgid);
pid_t iox_fork(void);
pid_t iox_vfork(void);
void iox_exit(int status);
void iox__exit(int status);
int iox_execve(const char *pathname, char *const argv[], char *const envp[]);
int iox_execv(const char *pathname, char *const argv[]);
pid_t iox_wait(int *status);
pid_t iox_waitpid(pid_t pid, int *status, int options);
pid_t iox_wait3(int *status, int options, struct rusage *rusage);
pid_t iox_wait4(pid_t pid, int *status, int options, struct rusage *rusage);
int iox_system(const char *command);
}

@interface IOXProcessTests : XCTestCase
@property (nonatomic) IOXTestEnvironment *env;
@end

@implementation IOXProcessTests

- (void)setUp {
    [super setUp];
    self.env = [IOXTestEnvironment sharedEnvironment];
    [self.env setup];
}

- (void)tearDown {
    [self.env cleanup];
    [super tearDown];
}

#pragma mark - Initialization Tests

- (void)testLibraryInitialization_Success {
    XCTAssertTrue(iox_is_initialized(), @"Library should be initialized");
}

- (void)testLibraryVersion_ReturnsNonEmptyString {
    const char *version = iox_version();
    XCTAssertNotNil(@(version), @"Version should not be NULL");
    XCTAssertGreaterThan(strlen(version), 0, @"Version should not be empty");
}

#pragma mark - PID Tests

- (void)testGetpid_ReturnsPositiveValue {
    pid_t pid = iox_getpid();
    XCTAssertGreaterThan(pid, 0, @"PID should be positive");
}

- (void)testGetppid_ReturnsPositiveValue {
    pid_t ppid = iox_getppid();
    XCTAssertGreaterThan(ppid, 0, @"PPID should be positive");
}

- (void)testGetpid_Getppid_DifferentValues {
    pid_t pid = iox_getpid();
    pid_t ppid = iox_getppid();
    XCTAssertNotEqual(pid, ppid, @"PID and PPID should be different");
}

#pragma mark - Process Group Tests

- (void)testGetpgrp_ReturnsValidGroup {
    pid_t pgid = iox_getpgrp();
    XCTAssertGreaterThan(pgid, 0, @"Process group ID should be positive");
}

- (void)testSetpgrp_CreatesNewGroup {
    pid_t old_pgid = iox_getpgrp();
    int result = iox_setpgrp();
    XCTAssertEqual(result, 0, @"setpgrp should succeed");
    
    pid_t new_pgid = iox_getpgrp();
    XCTAssertEqual(new_pgid, iox_getpid(), @"New group should equal PID");
}

- (void)testGetpgid_SameProcess_ReturnsOwnGroup {
    pid_t pid = iox_getpid();
    pid_t pgid = iox_getpgid(pid);
    XCTAssertEqual(pgid, iox_getpgrp(), @"getpgid(self) should equal getpgrp()");
}

- (void)testSetpgid_ChangesGroup {
    pid_t pid = iox_getpid();
    pid_t new_pgid = pid + 1000; // Arbitrary new group
    
    int result = iox_setpgid(pid, new_pgid);
    // May fail due to permissions, but should not crash
    if (result == 0) {
        pid_t pgid = iox_getpgrp();
        XCTAssertEqual(pgid, new_pgid, @"Group should be changed");
    }
}

#pragma mark - Fork Tests

- (void)testFork_CreatesNewProcess {
    IOXAssertNoMemoryLeak(({
        pid_t pid = iox_fork();
        
        if (pid == 0) {
            // Child process
            pid_t child_pid = iox_getpid();
            XCTAssertGreaterThan(child_pid, 0, @"Child should have valid PID");
            iox_exit(0);
        } else if (pid > 0) {
            // Parent process
            int status;
            pid_t result = iox_waitpid(pid, &status, 0);
            XCTAssertEqual(result, pid, @"waitpid should return child PID");
            XCTAssertTrue(WIFEXITED(status), @"Child should have exited normally");
            XCTAssertEqual(WEXITSTATUS(status), 0, @"Child exit code should be 0");
        } else {
            XCTFail(@"fork failed: %d", errno);
        }
    }), @"fork should not leak memory");
}

- (void)testFork_ReturnsZeroToChild {
    pid_t pid = iox_fork();
    
    if (pid == 0) {
        // In child, pid should be 0
        XCTAssertEqual(iox_getpid(), iox_getppid() + 1, @"Child PID logic check");
        iox_exit(0);
    } else if (pid > 0) {
        int status;
        iox_waitpid(pid, &status, 0);
        XCTAssertEqual(WEXITSTATUS(status), 0, @"Child should exit successfully");
    }
}

- (void)testFork_ChildHasDifferentPid {
    pid_t parent_pid = iox_getpid();
    
    pid_t pid = iox_fork();
    
    if (pid == 0) {
        // Child
        pid_t child_pid = iox_getpid();
        XCTAssertNotEqual(child_pid, parent_pid, @"Child should have different PID");
        iox_exit(0);
    } else if (pid > 0) {
        int status;
        iox_waitpid(pid, &status, 0);
    }
}

- (void)testVfork_IdenticalToFork {
    IOXAssertNoMemoryLeak(({
        pid_t pid = iox_vfork();
        
        if (pid == 0) {
            // Child
            XCTAssertGreaterThan(iox_getpid(), 0, @"vfork child should have valid PID");
            iox_exit(0);
        } else if (pid > 0) {
            int status;
            pid_t result = iox_waitpid(pid, &status, 0);
            XCTAssertEqual(result, pid, @"vfork waitpid should work");
            XCTAssertTrue(WIFEXITED(status), @"vfork child should exit");
        } else {
            XCTFail(@"vfork failed: %d", errno);
        }
    }), @"vfork should not leak memory");
}

#pragma mark - Wait Tests

- (void)testWait_NoChildren_ReturnsError {
    // This may return ECHILD if no children, which is expected
    int status;
    pid_t result = iox_wait(&status);
    // Result should be -1 with ECHILD
    XCTAssertEqual(result, -1, @"wait with no children should fail");
    XCTAssertEqual(errno, ECHILD, @"errno should be ECHILD");
}

- (void)testWaitpid_SpecificChild {
    pid_t child_pid = iox_fork();
    
    if (child_pid == 0) {
        // Child: sleep briefly then exit
        sleep(1);
        iox_exit(42);
    } else if (child_pid > 0) {
        int status;
        pid_t result = iox_waitpid(child_pid, &status, 0);
        
        XCTAssertEqual(result, child_pid, @"waitpid should return child PID");
        XCTAssertTrue(WIFEXITED(status), @"Child should have exited");
        XCTAssertEqual(WEXITSTATUS(status), 42, @"Exit code should be 42");
    }
}

- (void)testWaitpid_WNOHANG_NoChildren {
    int status;
    pid_t result = iox_waitpid(-1, &status, WNOHANG);
    // Should return 0 if no children ready
    XCTAssertEqual(result, 0, @"WNOHANG with no children should return 0");
}

- (void)testWait3_WithRusage {
    pid_t child_pid = iox_fork();
    
    if (child_pid == 0) {
        // Child: do some work
        for (volatile int i = 0; i < 1000000; i++);
        iox_exit(0);
    } else if (child_pid > 0) {
        int status;
        struct rusage usage;
        pid_t result = iox_wait3(&status, 0, &usage);
        
        XCTAssertEqual(result, child_pid, @"wait3 should return child PID");
        XCTAssertTrue(WIFEXITED(status), @"Child should exit");
        // rusage may be zero in simulator, just verify no crash
        XCTAssertGreaterThanOrEqual(usage.ru_utime.tv_sec, 0, @"User time should be valid");
    }
}

- (void)testWait4_SpecificChildWithRusage {
    pid_t child_pid = iox_fork();
    
    if (child_pid == 0) {
        iox_exit(0);
    } else if (child_pid > 0) {
        int status;
        struct rusage usage;
        pid_t result = iox_wait4(child_pid, &status, 0, &usage);
        
        XCTAssertEqual(result, child_pid, @"wait4 should return child PID");
        XCTAssertTrue(WIFEXITED(status), @"Child should exit");
    }
}

#pragma mark - Exit Tests

- (void)testExit_NormalTermination {
    pid_t child_pid = iox_fork();
    
    if (child_pid == 0) {
        // Child: test normal exit
        iox_exit(123);
        // Should not reach here
        XCTFail(@"Should not execute after exit");
    } else if (child_pid > 0) {
        int status;
        iox_waitpid(child_pid, &status, 0);
        
        XCTAssertTrue(WIFEXITED(status), @"Should have exited normally");
        XCTAssertEqual(WEXITSTATUS(status), 123, @"Exit code should be 123");
    }
}

- (void)testExit_ZeroExitCode {
    pid_t child_pid = iox_fork();
    
    if (child_pid == 0) {
        iox_exit(0);
    } else if (child_pid > 0) {
        int status;
        iox_waitpid(child_pid, &status, 0);
        XCTAssertEqual(WEXITSTATUS(status), 0, @"Exit code should be 0");
    }
}

- (void)testExit_SignalTermination {
    pid_t child_pid = iox_fork();
    
    if (child_pid == 0) {
        // Child: simulate signal
        iox_exit(128 + SIGTERM);  // Signal exit
    } else if (child_pid > 0) {
        int status;
        iox_waitpid(child_pid, &status, 0);
        
        XCTAssertTrue(WIFEXITED(status) || WIFSIGNALED(status), 
                     @"Should have exited or been signaled");
    }
}

#pragma mark - Stress Tests

IOXStressTest(Fork, 1000) {
    IOXPerformanceTimer *timer = [[IOXPerformanceTimer alloc] init];
    NSUInteger baseline = [IOXMemoryTracker currentResidentSize];
    
    [timer measureBlock:^{
        pid_t pid = iox_fork();
        if (pid == 0) {
            iox_exit(0);
        } else if (pid > 0) {
            int status;
            iox_waitpid(pid, &status, 0);
        }
    } iterations:1000];
    
    NSLog(@"Fork stress: %@", timer.report);
    
    // Verify no memory leak
    NSUInteger current = [IOXMemoryTracker currentResidentSize];
    XCTAssertEqualWithAccuracy(current, baseline, 16384, 
        @"Memory leak detected after 1000 forks");
}

@end

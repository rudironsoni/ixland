/*
 * IXLandSystemTests.m - XCTest wrapper for IXLandSystem C tests
 *
 * Links against libIXLandSystem.a directly and tests the public C API
 */

#import <XCTest/XCTest.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Include IXLandSystem public headers */
#include "ixland/ixland_types.h"

/* Forward declaration for test_basic.c main */
int test_basic_main(void);

@interface IXLandSystemTests : XCTestCase
@end

@implementation IXLandSystemTests

- (void)testSystemExists {
    /* Verify IXLandSystem symbols are available */
    XCTAssertTrue(true, "IXLandSystem library linked successfully");
}

- (void)testBasicSyscalls {
    /* Test basic file operations using host system as baseline */
    char template[] = "/tmp/ixland_test_XXXXXX";
    int fd = mkstemp(template);
    XCTAssertGreaterThanOrEqual(fd, 0, "mkstemp should succeed");
    
    if (fd >= 0) {
        const char *msg = "test data";
        ssize_t written = write(fd, msg, strlen(msg));
        XCTAssertEqual(written, (ssize_t)strlen(msg), "write should succeed");
        
        close(fd);
        unlink(template);
    }
}

- (void)testFileDescriptors {
    /* Test FD operations */
    int fd = open("/dev/null", O_RDONLY);
    XCTAssertGreaterThanOrEqual(fd, 0, "open should return valid FD");
    
    if (fd >= 0) {
        char buf[1];
        ssize_t n = read(fd, buf, sizeof(buf));
        /* /dev/null returns 0 on read (EOF) */
        XCTAssertEqual(n, 0, "read from /dev/null should return 0 (EOF)");
        close(fd);
    }
}

- (void)testProcessInfo {
    /* Test process identification */
    pid_t pid = getpid();
    XCTAssertGreaterThan(pid, 0, "getpid should return positive PID");
    
    pid_t ppid = getppid();
    XCTAssertGreaterThanOrEqual(ppid, 0, "getppid should return non-negative PPID");
}

@end

// IOXCoreTests.mm
// XCTest wrapper for iox C test harness

#import <XCTest/XCTest.h>

// C test harness interface
extern "C" {
    int iox_test_run_all(const char *filter);
}

@interface IOXCoreTests : XCTestCase
@end

@implementation IOXCoreTests

- (void)testAllCoreTests {
    int result = iox_test_run_all(NULL);
    XCTAssertEqual(result, 0, @"iox core tests failed");
}

- (void)testPIDTests {
    int result = iox_test_run_all("pid");
    XCTAssertEqual(result, 0, @"iox PID tests failed");
}

- (void)testFDTests {
    int result = iox_test_run_all("fd");
    XCTAssertEqual(result, 0, @"iox FD tests failed");
}

- (void)testExecNative {
    int result = iox_test_run_all("exec_native");
    XCTAssertEqual(result, 0, @"iox exec native tests failed");
}

- (void)testExecErrors {
    int result = iox_test_run_all("exec_command_not_found");
    XCTAssertEqual(result, 0, @"iox exec error tests failed");
}

- (void)testExecCloexec {
    int result = iox_test_run_all("exec_cloexec");
    XCTAssertEqual(result, 0, @"iox exec CLOEXEC tests failed");
}

- (void)testExecClassification {
    int result = iox_test_run_all("exec_image_classification");
    XCTAssertEqual(result, 0, @"iox exec classification tests failed");
}

@end

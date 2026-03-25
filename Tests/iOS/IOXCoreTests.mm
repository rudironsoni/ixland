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

- (void)testSignalMask {
    int result = iox_test_run_all("sigprocmask");
    XCTAssertEqual(result, 0, @"iox signal mask tests failed");
}

- (void)testSignalExecReset {
    int result = iox_test_run_all("signal_reset_on_exec");
    XCTAssertEqual(result, 0, @"iox signal exec reset tests failed");
}

- (void)testTaskLifecycle {
    int result = iox_test_run_all("task_init_state|task_pid|task_refs|task_lookup|task_exit|task_sighand");
    XCTAssertEqual(result, 0, @"iox task lifecycle tests failed");
}

- (void)testWaitBasic {
    int result = iox_test_run_all("wait_parent");
    XCTAssertEqual(result, 0, @"iox wait basic tests failed");
}

- (void)testWaitErrors {
    int result = iox_test_run_all("wait_no_children|wait_wrong|wait_second_reap");
    XCTAssertEqual(result, 0, @"iox wait error tests failed");
}

- (void)testWaitSignaled {
    int result = iox_test_run_all("wait_signaled");
    XCTAssertEqual(result, 0, @"iox wait signaled tests failed");
}

- (void)testWaitNoHang {
    int result = iox_test_run_all("wnohang");
    XCTAssertEqual(result, 0, @"iox wait WNOHANG tests failed");
}

- (void)testWaitMultiple {
    int result = iox_test_run_all("wait_child_list|wait_refcount|wait_multiple");
    XCTAssertEqual(result, 0, @"iox wait multiple children tests failed");
}

- (void)testProcessGroups {
    int result = iox_test_run_all("pgrp_initial|pgrp_fork|pgrp_multiple|pgrp_child_lookup");
    XCTAssertEqual(result, 0, @"iox process group tests failed");
}

- (void)testSessions {
    int result = iox_test_run_all("pgrp_session|pgrp_zombie|pgrp_refcount");
    XCTAssertEqual(result, 0, @"iox session tests failed");
}

- (void)testSignalDelivery {
    int result = iox_test_run_all("signal_kill|signal_int|signal_ign|signal_forbidden");
    XCTAssertEqual(result, 0, @"iox signal delivery tests failed");
}

- (void)testSignalProcessGroups {
    int result = iox_test_run_all("signal_group");
    XCTAssertEqual(result, 0, @"iox signal process group tests failed");
}

- (void)testSignalForeground {
    int result = iox_test_run_all("signal_shell");
    XCTAssertEqual(result, 0, @"iox signal foreground tests failed");
}

- (void)testSignalErrors {
    int result = iox_test_run_all("signal_invalid|signal_null|signal_invalid_pid");
    XCTAssertEqual(result, 0, @"iox signal error tests failed");
}

- (void)testSignalWait {
    int result = iox_test_run_all("signal_pending|signal_disposition|signal_zero");
    XCTAssertEqual(result, 0, @"iox signal wait tests failed");
}

- (void)testSignalPending {
    int result = iox_test_run_all("pending_mask_basic|pending_delivery|pending_multiple|pending_ignored");
    XCTAssertEqual(result, 0, @"iox signal pending tests failed");
}

- (void)testSignalPendingErrors {
    int result = iox_test_run_all("pending_null|pending_invalid");
    XCTAssertEqual(result, 0, @"iox signal pending error tests failed");
}

- (void)testPathNormalization {
    int result = iox_test_run_all("path_normalize");
    XCTAssertEqual(result, 0, @"iox path normalization tests failed");
}

- (void)testFDTable {
    int result = iox_test_run_all("fdtable");
    XCTAssertEqual(result, 0, @"iox FD table tests failed");
}

@end

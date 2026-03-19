// a_shell_session_tests.m
// Unit tests for session management

#import <XCTest/XCTest.h>
#import "a_shell_system/a_shell_system.h"

@interface AShellSessionTests : XCTestCase
@property (nonatomic, assign) void* testSession;
@property (nonatomic, assign) int sessionId;
@end

@implementation AShellSessionTests

- (void)setUp {
    [super setUp];
    ashell_env_initialize();
    self.sessionId = arc4random();
    self.testSession = malloc(sizeof(sessionParameters));
    XCTAssertNotNil((__bridge id)self.testSession, @"Should allocate session");
    memset(self.testSession, 0, sizeof(sessionParameters));
}

- (void)tearDown {
    if (self.testSession) {
        free(self.testSession);
        self.testSession = NULL;
    }
    [super tearDown];
}

// Test session registration
- (void)testSessionRegistration {
    int result = ashell_session_register(&self.sessionId, self.testSession);
    XCTAssertEqual(result, 0, @"Session registration should succeed");
    
    // Verify session exists
    bool exists = ashell_session_exists(&self.sessionId);
    XCTAssertTrue(exists, @"Session should exist after registration");
    
    // Cleanup
    ashell_session_unregister(&self.sessionId);
}

// Test session lookup
- (void)testSessionLookup {
    ashell_session_register(&self.sessionId, self.testSession);
    
    void* found = ashell_session_get(&self.sessionId);
    XCTAssertTrue(found == self.testSession, @"Should find registered session");
    
    // Look up non-existent
    int nonExistent = 999999;
    void* notFound = ashell_session_get(&nonExistent);
    XCTAssertTrue(notFound == NULL, @"Should return NULL for non-existent session");
    
    ashell_session_unregister(&self.sessionId);
}

// Test session acquire and release
- (void)testSessionAcquireRelease {
    ashell_session_register(&self.sessionId, self.testSession);
    
    int result = ashell_session_acquire(&self.sessionId);
    XCTAssertEqual(result, 0, @"Session acquire should succeed");
    
    result = ashell_session_release(&self.sessionId);
    XCTAssertEqual(result, 0, @"Session release should succeed");
    
    ashell_session_unregister(&self.sessionId);
}

// Test session unregistration
- (void)testSessionUnregistration {
    ashell_session_register(&self.sessionId, self.testSession);
    
    int result = ashell_session_unregister(&self.sessionId);
    XCTAssertEqual(result, 0, @"Session unregistration should succeed");
    
    bool exists = ashell_session_exists(&self.sessionId);
    XCTAssertFalse(exists, @"Session should not exist after unregistration");
}

// Test session touch
- (void)testSessionTouch {
    ashell_session_register(&self.sessionId, self.testSession);
    
    // Should not crash
    XCTAssertNoThrow(ashell_session_touch(&self.sessionId), @"Touch should not throw");
    
    ashell_session_unregister(&self.sessionId);
}

// Test session stats
- (void)testSessionStats {
    ashell_session_stats_t stats;
    memset(&stats, 0, sizeof(stats));
    
    // Should not crash
    XCTAssertNoThrow(ashell_session_stats(&stats), @"Getting stats should not throw");
}

// Test session validation
- (void)testSessionValidation {
    int result = ashell_session_validate();
    XCTAssertEqual(result, 0, @"Session validation should succeed with no sessions");
    
    // Register a session and validate again
    ashell_session_register(&self.sessionId, self.testSession);
    result = ashell_session_validate();
    XCTAssertEqual(result, 0, @"Session validation should succeed with sessions");
    
    ashell_session_unregister(&self.sessionId);
}

// Test multiple sessions
- (void)testMultipleSessions {
    int id1 = arc4random();
    int id2 = arc4random();
    void* session1 = malloc(sizeof(sessionParameters));
    void* session2 = malloc(sizeof(sessionParameters));
    
    memset(session1, 0, sizeof(sessionParameters));
    memset(session2, 0, sizeof(sessionParameters));
    
    // Register both
    XCTAssertEqual(ashell_session_register(&id1, session1), 0, @"First session should register");
    XCTAssertEqual(ashell_session_register(&id2, session2), 0, @"Second session should register");
    
    // Both should exist
    XCTAssertTrue(ashell_session_exists(&id1), @"First session should exist");
    XCTAssertTrue(ashell_session_exists(&id2), @"Second session should exist");
    
    // Cleanup
    ashell_session_unregister(&id1);
    ashell_session_unregister(&id2);
    free(session1);
    free(session2);
}

// Test session switch
- (void)testSessionSwitch {
    ashell_session_register(&self.sessionId, self.testSession);
    
    // Switch to session
    XCTAssertNoThrow(ashell_session_switch(&self.sessionId), @"Switch should not throw");
    
    ashell_session_unregister(&self.sessionId);
}

@end
